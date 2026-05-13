#include "tabs.h"
#include "device.hpp"
#include "config.hpp"
#include "freeze_trace.h"
#include <esp32-hal.h>  // xPortGetCoreID

// =====================================================================
// REVERT SWITCH:
//   1  → defer LVGL writes in setCurrentPage()/nextPage()/previousPage()
//         to Core 1 when called from Core 0 (fix for the freeze caused by
//         lv_tabview_set_act() running on Core 0).
//   0  → original behavior: lv_tabview_set_act() runs on whichever core
//         called the function (unsafe from Core 0 key handlers — keeps
//         the freeze bug; flip back here to compare/revert quickly).
// =====================================================================
#ifndef DEFER_PAGE_CHANGE_TO_CORE1
#define DEFER_PAGE_CHANGE_TO_CORE1 1
#endif

extern Device PowerSupply;

uint8_t Tabs::numberOfPage = 0;
uint8_t Tabs::defaultPage;
lv_obj_t *Tabs::tabview;

Tabs::Tabs() {};
Tabs::~Tabs() {}

// lv_tabview_set_act doesn't fire LV_EVENT_VALUE_CHANGED — only tab-button clicks do.
// Set this flag from any thread (incl. Core 0 key handlers) when the tab changes
// programmatically; Core 1's main loop drains it and fires the event safely.
// Plain volatile bool avoids any LVGL allocator/timer calls from Core 0.
volatile bool g_tabValueChangedPending = false;

// Page-change deferral state. nextPage/previousPage/setCurrentPage from Core 0
// stash the target here; drainPendingPageChange() on Core 1 does the real LVGL work.
volatile bool g_pageChangePending = false;
volatile int  g_pageChangeTarget  = -1;

void Tabs::setup_(lv_obj_t *parent)
{
    Tabs::tabview = lv_tabview_create(parent, LV_DIR_TOP, 15);
    static lv_style_t style_tabview_df;
    static lv_style_t style_tabview_pr;

    lv_style_init(&style_tabview_df);
    lv_style_init(&style_tabview_pr);
    lv_style_set_border_width(&style_tabview_df, 0);
    lv_style_set_text_font(&style_tabview_df, &lv_font_montserrat_12);
    // lv_obj_remove_style_all(Tabs::tabview);
    lv_obj_add_style(Tabs::tabview, &style_tabview_df, LV_STATE_DEFAULT);

    static lv_style_t style_tabview_df_btn;

    lv_style_init(&style_tabview_df_btn);
    lv_style_set_bg_color(&style_tabview_df_btn, lv_palette_darken(LV_PALETTE_GREY, 4));
    lv_style_set_border_width(&style_tabview_df_btn, 0);

    lv_obj_t *tab_btns = lv_tabview_get_tab_btns(Tabs::tabview);

    // lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    // lv_obj_set_style_text_color(tab_btns, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_NONE, LV_PART_ITEMS | LV_STATE_CHECKED);

    // lv_obj_set_style_border_color (tab_btns, lv_palette_lighten(LV_PALETTE_GREY, 3), 0);

    lv_style_set_bg_opa(&style_tabview_df_btn, LV_OPA_100);
    lv_style_set_bg_color(&style_tabview_df_btn, lv_palette_darken(LV_PALETTE_YELLOW, 4));
    lv_style_set_text_color(&style_tabview_df_btn, lv_palette_main(LV_PALETTE_YELLOW));

    lv_obj_add_style(tab_btns, &style_tabview_df_btn, LV_PART_ITEMS | LV_STATE_CHECKED);

    lv_style_set_text_font(&style_tabview_pr, &lv_font_montserrat_14);
    lv_style_set_border_width(&style_tabview_pr, 0);
    lv_obj_add_style((Tabs::tabview), &style_tabview_pr, LV_STATE_PRESSED);
}

void Tabs::goToHomeTab()
{
    lv_tabview_set_act(Tabs::tabview, Tabs::defaultPage, LV_ANIM_OFF);
};

void Tabs::setDefaultPage(int n)
{
    Tabs::defaultPage = n;
    lv_tabview_set_act(Tabs::tabview, n, LV_ANIM_OFF);
}

int Tabs::getCurrentPage()
{
    return lv_tabview_get_tab_act(Tabs::tabview);
}
void Tabs::setCurrentPage(int n)
{
    // Save FGen settings when leaving Utility page (page 3) - only if changed.
    // This is preferences/SPIFFS I/O, NOT an LVGL call — safe on either core.
    if (getCurrentPage() == 3 && n != 3 && PowerSupply.funGenMemDirty)
    {
        PowerSupply.SaveMemoryFgen("FunGen", PowerSupply.funGenMem);
        PowerSupply.funGenMemDirty = false;  // Clear dirty flag after save
    }

#if DEFER_PAGE_CHANGE_TO_CORE1
    // If called from anywhere other than Core 1 (main loop / setup), defer the LVGL
    // work to Core 1 to avoid cross-core lv_tabview_set_act races. Core 1 picks this
    // up via drainPendingPageChange() and re-enters setCurrentPage where this guard
    // becomes false, so the work happens directly below.
    if (xPortGetCoreID() != 1)
    {
        TRACE("setPage_defer_to_c1");
        g_pageChangeTarget = n;
        g_pageChangePending = true;
        return;
    }
#endif

    TRACE("setPage_pre_setact");
    bool new_tab = (getCurrentPage() != n);
    blockAll = true;
    lv_tabview_set_act(Tabs::tabview, n, LV_ANIM_ON);
    blockAll = false;
    TRACE("setPage_post_setact");
    if (new_tab) g_tabValueChangedPending = true;
    lv_obj_invalidate(lv_scr_act());
    TRACE("setPage_done");
}

void Tabs::nextPage()
{
    int current = getCurrentPage();
    int target  = (current < 4) ? (current + 1) : 0;
    // setCurrentPage handles the FGen save, the Core 0 deferral, and tab-event flagging.
    setCurrentPage(target);
}

void Tabs::previousPage()
{
    int current = getCurrentPage();
    int target  = (current > 0) ? (current - 1) : 4;
    setCurrentPage(target);
}

// Called from Core 1 main loop — picks up a Core 0-deferred page change and
// runs setCurrentPage() on Core 1 (where the LVGL writes are safe).
void drainPendingPageChange()
{
    if (!g_pageChangePending) return;
    TRACE("drainPage_begin");
    int target = g_pageChangeTarget;
    g_pageChangePending = false;
    Tabs::setCurrentPage(target);
    TRACE("drainPage_end");
}

int Tabs::getNumberOfPage()
{

    return Tabs::numberOfPage;
}

void addPanel(lv_obj_t *parent, lv_style_t *style_shadow, lv_color_t color)
{
    // static lv_style_t style_shadow;
    lv_style_init(style_shadow);

    lv_style_set_shadow_width(style_shadow, 6);
    lv_style_set_shadow_spread(style_shadow, 3);
    lv_style_set_shadow_color(style_shadow, color);

    lv_style_set_width(style_shadow, 0);
    lv_style_set_height(style_shadow, 0);
    lv_style_set_pad_all(style_shadow, 0);
    lv_style_set_border_width(style_shadow, 0);
    lv_style_set_outline_width(style_shadow, 0);

    lv_style_set_bg_color(style_shadow, lv_color_hex(0x000000));
    lv_style_set_x(style_shadow, -8);
    lv_style_set_y(style_shadow, -6);

    lv_obj_t *obj = lv_obj_create(parent);
    // lv_obj_set_parent(obj, parent);
    lv_obj_add_style(obj, style_shadow, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
}

void Tabs::addPage(lv_obj_t *parent, lv_obj_t **tab, const char *name)
{
    *tab = lv_tabview_add_tab(parent, name);

    if (Tabs::numberOfPage % 1 == 0)
    {
        lv_obj_set_style_bg_color(*tab, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_opa(*tab, LV_OPA_COVER, 0);
    }

    Tabs::numberOfPage++;
}

// Called from main loop on Core 1 — safe place to fire LVGL events.
void drainPendingTabEvent()
{
    if (!g_tabValueChangedPending) return;
    TRACE("drain_event_begin");
    g_tabValueChangedPending = false;
    if (Tabs::tabview) lv_event_send(Tabs::tabview, LV_EVENT_VALUE_CHANGED, NULL);
    TRACE("drain_event_end");
}