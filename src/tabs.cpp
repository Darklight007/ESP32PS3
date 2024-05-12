#include "tabs.h"

uint8_t Tabs::numberOfPage = 0;
uint8_t Tabs::defaultPage;
lv_obj_t *Tabs::tabview;

Tabs::Tabs(){};
Tabs::~Tabs() {}

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
    lv_tabview_set_act(Tabs::tabview, n, LV_ANIM_ON);
}

void Tabs::nextPage()
{
    lv_tabview_set_act(Tabs::tabview, getCurrentPage() + 1, LV_ANIM_ON);
}

void Tabs::previousPage()
{
    if (getCurrentPage()>0)
    lv_tabview_set_act(Tabs::tabview, std::max(getCurrentPage() - 1, 0), LV_ANIM_ON);
else
setCurrentPage(4);
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