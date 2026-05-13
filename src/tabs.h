#pragma once
#ifndef TABS_H
#define TABS_H

#include <lvgl.h>
#include <algorithm>

// Set true when a programmatic tab change needs LV_EVENT_VALUE_CHANGED fired.
// Drained on Core 1 by the main loop (see drainPendingTabEvent()).
extern volatile bool g_tabValueChangedPending;
void drainPendingTabEvent();

// Page-change deferral (Core 0 → Core 1).
// Set from any thread; drained on Core 1 by drainPendingPageChange().
extern volatile bool g_pageChangePending;
extern volatile int  g_pageChangeTarget;
void drainPendingPageChange();

class Tabs
{
public:
    static lv_obj_t *tabview;
    static uint8_t numberOfPage;
    static uint8_t defaultPage;
    lv_style_t *style_shadow_;

    Tabs();
    ~Tabs();

    void setup_(lv_obj_t *);
    void addPage(lv_obj_t *parent, lv_obj_t **tab, const char *name);
    static void goToHomeTab();
    static void setDefaultPage(int n);
    static int getCurrentPage();
    static void setCurrentPage(int n);
    static void nextPage();
    static void previousPage();
    static int getNumberOfPage();
    void addPanel(lv_obj_t *parent, lv_style_t *style_shadow, lv_color_t color);
};

#endif // TABS_H
#pragma once