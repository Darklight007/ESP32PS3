#include "disp_perf.h"
#include "esp_timer.h"
#include "device.hpp"

extern Device PowerSupply;

// All counters are touched only on Core 1 (LVGL refresh + main loop).
static int64_t frameStartUs = 0;
static int64_t flushStartUs = 0;
static uint32_t frames = 0, px = 0;
static int64_t frameUs = 0, flushUs = 0;
static int64_t windowStartUs = 0;
static DispPerfStats stats{};
static lv_obj_t *overlay = nullptr;

static void renderStart(lv_disp_drv_t *)
{
    frameStartUs = esp_timer_get_time();
}

static void monitor(lv_disp_drv_t *, uint32_t, uint32_t pxNum)
{
    if (!frameStartUs)
        return;
    frameUs += esp_timer_get_time() - frameStartUs;
    frameStartUs = 0;
    frames++;
    px += pxNum;
}

void dispPerfAttach(lv_disp_drv_t *drv)
{
    drv->render_start_cb = renderStart;
    drv->monitor_cb = monitor;
}

void dispPerfFlushBegin() { flushStartUs = esp_timer_get_time(); }
void dispPerfFlushEnd() { flushUs += esp_timer_get_time() - flushStartUs; }

void dispPerfUpdate()
{
    const int64_t now = esp_timer_get_time();
    if (!windowStartUs)
        windowStartUs = now;
    const int64_t span = now - windowStartUs;
    if (span < 1000000)
        return;

    stats.fps = frames * 1e6f / span;
    stats.pxPerFrame = frames ? px / frames : 0;
    stats.flushMs = frames ? flushUs / 1000.0f / frames : 0;
    stats.renderMs = frames ? (frameUs - flushUs) / 1000.0f / frames : 0;
    stats.busyPct = frameUs * 100.0f / span;
    frames = px = 0;
    frameUs = flushUs = 0;
    windowStartUs = now;

    if (overlay)
        lv_label_set_text_fmt(overlay, "%2d fps %5lu px  R %4.1f  F %3.1f ms  %2d%%",
                              (int)(stats.fps + 0.5f), (unsigned long)stats.pxPerFrame,
                              stats.renderMs, stats.flushMs, (int)(stats.busyPct + 0.5f));
}

void dispPerfShow(bool on)
{
    if (on && !overlay)
    {
        overlay = lv_label_create(lv_layer_sys());
        lv_obj_set_style_text_font(overlay, &lv_font_unscii_8, 0);
        lv_obj_set_style_text_color(overlay, lv_color_white(), 0);
        lv_obj_set_style_bg_color(overlay, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(overlay, LV_OPA_COVER, 0);
        lv_obj_set_style_pad_all(overlay, 1, 0);
        lv_obj_align(overlay, LV_ALIGN_BOTTOM_LEFT, 0, 0);
        lv_label_set_text(overlay, "measuring...");
    }
    else if (!on && overlay)
    {
        lv_obj_del(overlay);
        overlay = nullptr;
    }
}

static lv_obj_t *settingSwitch = nullptr;
void dispPerfSetSwitch(lv_obj_t *sw) { settingSwitch = sw; }

void dispPerfSetSaved(bool on)
{
    dispPerfShow(on);
    if (settingSwitch)
    {
        if (on)
            lv_obj_add_state(settingSwitch, LV_STATE_CHECKED);
        else
            lv_obj_clear_state(settingSwitch, LV_STATE_CHECKED);
    }
    if (PowerSupply.settingParameters.showFps != (on ? 1 : 0))
    {
        PowerSupply.settingParameters.showFps = on ? 1 : 0;
        PowerSupply.SaveSetting();
    }
}

bool dispPerfShown() { return overlay != nullptr; }
const DispPerfStats &dispPerfStats() { return stats; }
