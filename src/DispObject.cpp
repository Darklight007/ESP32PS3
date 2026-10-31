#include "DispObject.h"
#include "device.hpp"   // PowerSupply.mA_Active for measured-bar range scaling
#include <esp32-hal.h>  // xPortGetCoreID

extern volatile bool lvglIsBusy, blockAll;
extern Device PowerSupply;

// =====================================================================
// REVERT SWITCH for SetUpdate() LVGL writes from Core 0:
//   1 → Skip LVGL writes inside SetUpdate when running on Core 0. The
//        function generator (waveform_generator.cpp:212) calls SetUpdate
//        from Task_ADC at 200-500 Hz; direct LVGL writes from Core 0
//        race with Core 1 rendering and corrupt the LVGL object tree,
//        which eventually wedges Core 1 in draw_scrollbar / refr_obj.
//        Flush() runs on Core 1 and picks up adjValueChanged to update
//        the label and bar at its own pace.
//   0 → Original behavior: SetUpdate always writes LVGL (unsafe from
//        Core 0).
// =====================================================================
#ifndef DEFER_DISPOBJ_LVGL_TO_CORE1
#define DEFER_DISPOBJ_LVGL_TO_CORE1 1
#endif
void DispObjects::SetEncoderPins(int aPintNumber, int bPinNumber, enc_isr_cb_t enc_isr_cb = nullptr)
{
    // NOTE: encoder is a member variable (not a pointer), so no memory leak
    // It's constructed once with the DispObjects instance
    encoder.always_interrupt = true;
    encoder._enc_isr_cb = enc_isr_cb;
    encoder.attachHalfQuad(aPintNumber, bPinNumber); // attachFullQuad attachHalfQuad attachSingleEdge
    encoder.clearCount();
    encoder.setCount(0);
    encoder.setFilter(500);
    // ESP32Encoder::useInternalWeakPullResistors = UP;
};

DispObjects::DispObjects() {};
DispObjects::~DispObjects() {};

void DispObjects::measureUpdate(double value)
{
    measured(value);
    rawBarValue = value;  // CRITICAL: Bar uses raw value
    Bar.changed = true;

    // Fast path: skip statistics if power (calculated from V×I, not measured directly)
    // isPowerObj is cached in setup(); reading the unit label here meant an
    // LVGL access from Core 0 on every sample.
    if (!isPowerObj) {
        StatisticsUpdate(value);
    }

    // Update display ready flag for all measurements (V, I, and P) every N samples
    if (++sampleCountSinceDisplay >= (measured.NofAvgs)) {
        displayReady = true;
        sampleCountSinceDisplay = 0;
    }

    // Always set changed flag for all measurements (including Power)
    if (oldValue != value) {
        oldValue = value;
        changed = true;
    }
}

void DispObjects::StatisticsUpdate(double value)
{
    Statistics(value);

    // EXPERIMENTAL: ER() returns a 0.0 sentinel (not NaN/inf) whenever the
    // window doesn't have enough samples yet for a real variance (e.g. right
    // after ResetStats()). Pushing that placeholder into effectiveResolution
    // as if it were a real "0-bit" reading briefly drags the moving average
    // down for no physical reason. Skip it until Statistics has >= 2 samples.
    if (Statistics.windowSizeIndex_ >= 2)
    {
        double er_sample = Statistics.ER(2 * adc_maxValue); // maxValue
        // ER() returns a 0.0 sentinel whenever stdDev isn't positive - not a
        // real "0 bits" reading. Averaging those zeros into effectiveResolution
        // dragged the displayed ER far below the true value (seen live: ~6 bits
        // on a signal with no visible noise, where the genuine samples were
        // ~18). A perfectly stable window means "can't measure it this window",
        // so skip it and keep the last good average instead of poisoning it.
        if (std::isfinite(er_sample) && er_sample > 0.0)
            effectiveResolution(er_sample);
    }
}

// Set a label's text but redraw only the characters that changed. The
// readings are fixed-format numbers where usually just the last digit or two
// move, while lv_label_set_text() repaints the whole 258x64 label every time -
// that was ~37k px per frame on the main page. Only safe when every changed
// glyph has the same advance width as the one it replaces (then no other
// character moves, whatever the alignment); otherwise fall back to a full set.
// Repaint only columns [from, to] (offsets from the object's left edge,
// inclusive) of an object, full height plus its ext draw size.
static void invalidateColumns(lv_obj_t *obj, lv_coord_t from, lv_coord_t to)
{
    lv_area_t a = obj->coords;
    const lv_coord_t x0 = a.x1;
    a.x1 = x0 + from;
    a.x2 = x0 + to;
    lv_obj_invalidate_area(obj, &a);
}

static void setLabelTextChangedOnly(lv_obj_t *lbl, const char *txt)
{
    const char *old = lv_label_get_text(lbl);
    const size_t n = strlen(txt);
    if (!old || strlen(old) != n || n > 32 || lv_obj_has_flag(lbl, LV_OBJ_FLAG_HIDDEN))
    {
        lv_label_set_text(lbl, txt);
        return;
    }
    const lv_font_t *font = lv_obj_get_style_text_font(lbl, LV_PART_MAIN);
    // Ink box of a glyph relative to its letter position (how lv_draw_label
    // places it), so only drawn pixels are repainted - not the advance width
    // plus an overhang margin.
    auto inkBox = [font](uint8_t c, lv_area_t &b)
    {
        lv_font_glyph_dsc_t g;
        if (!lv_font_get_glyph_dsc(font, &g, c, 0) || g.box_w == 0)
            return false;
        b.x1 = g.ofs_x;
        b.x2 = g.ofs_x + g.box_w - 1;
        b.y1 = font->line_height - font->base_line - g.box_h - g.ofs_y;
        b.y2 = b.y1 + g.box_h - 1;
        return true;
    };
    uint32_t changed = 0; // bit i = char i differs (old text is freed by set_text)
    lv_area_t box[32];    // per changed char: union of old and new ink boxes
    for (size_t i = 0; i < n; i++)
    {
        if (old[i] == txt[i])
            continue;
        if (lv_font_get_glyph_width(font, (uint8_t)old[i], 0) != lv_font_get_glyph_width(font, (uint8_t)txt[i], 0))
        {
            lv_label_set_text(lbl, txt);
            return;
        }
        lv_area_t bo, bn;
        const bool ho = inkBox((uint8_t)old[i], bo), hn = inkBox((uint8_t)txt[i], bn);
        if (!ho && !hn)
            continue; // space -> space-like: nothing drawn either way
        if (!ho)
            bo = bn;
        if (!hn)
            bn = bo;
        box[i].x1 = LV_MIN(bo.x1, bn.x1);
        box[i].y1 = LV_MIN(bo.y1, bn.y1);
        box[i].x2 = LV_MAX(bo.x2, bn.x2);
        box[i].y2 = LV_MAX(bo.y2, bn.y2);
        changed |= 1u << i;
    }
    if (!changed)
    {
        if (strcmp(old, txt) != 0)
            lv_label_set_text(lbl, txt); // only blank glyphs changed - keep the text right
        return;
    }

    // Every glyph keeps its width, so the label's size can't change: undo the
    // layout-dirty marks lv_label_set_text() sets on a content-sized label.
    // Left set, they make the next refresh re-walk the whole object tree
    // (all five tabs) - ~1 ms per frame, ~2.4 ms in FUN mode, measured.
    lv_obj_t *scr = lv_obj_get_screen(lbl);
    const bool objLayoutInv = lbl->layout_inv;
    const bool scrLayoutInv = scr->scr_layout_inv;
    const bool inv = lv_disp_is_invalidation_enabled(NULL);
    lv_disp_enable_invalidation(NULL, false);
    lv_label_set_text(lbl, txt);
    lv_disp_enable_invalidation(NULL, inv);
    lbl->layout_inv = objLayoutInv;
    scr->scr_layout_inv = scrLayoutInv;

    // Exactly the ink boxes: no margin, so the areas stay inside the label and
    // an opaque label covers them (see opaqueMeasureBackground). Changed glyphs
    // at most one unchanged glyph apart share one area: every area costs ~1 ms
    // of fixed LVGL overhead (top-object search, style lookups, a flush) on top
    // of its pixels, and in FUN mode 4-5 neighbouring digits change per frame.
    // Further apart (a sign flip plus a last-digit change) they stay separate,
    // so the digits in between aren't repainted.
    lv_area_t content;
    lv_obj_get_content_coords(lbl, &content);
    lv_area_t u;
    int last = -2; // index of the last changed glyph in u, -2 = u empty
    for (size_t i = 0; i < n; i++)
    {
        if (!(changed & (1u << i)))
            continue;
        lv_point_t p;
        lv_label_get_letter_pos(lbl, i, &p);
        lv_area_t a;
        a.x1 = content.x1 + p.x + box[i].x1;
        a.x2 = content.x1 + p.x + box[i].x2;
        a.y1 = content.y1 + p.y + box[i].y1;
        a.y2 = content.y1 + p.y + box[i].y2;
        if (last >= 0 && (int)i - last <= 2)
        {
            u.x1 = LV_MIN(u.x1, a.x1);
            u.y1 = LV_MIN(u.y1, a.y1);
            u.x2 = LV_MAX(u.x2, a.x2);
            u.y2 = LV_MAX(u.y2, a.y2);
        }
        else
        {
            if (last >= 0)
                lv_obj_invalidate_area(lbl, &u);
            u = a;
        }
        last = (int)i;
    }
    if (last >= 0)
        lv_obj_invalidate_area(lbl, &u);
}

void DispObjects::displayUpdate(void)
{
    // Only update display after NofAvgs samples collected
    if (!displayReady)
        return;

    displayReady = false;

    double mean_ = measured.Mean();
    // Own "last shown" value: oldValue is Core 0's last sample, so comparing
    // the mean against it skipped every redraw whenever mean == last sample
    // (always at avg 1) and froze the label.
    if (shownValue != mean_ && !blockAll)
    {
        // Power's range (up into the hundreds of Watts) can exceed what V/I's
        // shared decimal-count setting assumes (tuned for 0-32V / 0-5A) - at
        // the finest (4-decimal) setting, >=100W would print a 9-character
        // "+100.0000", overflowing the box the setting already sized for 8.
        // Drop to 3 decimals ("+100.000", still 8 chars) once Power crosses
        // 100 at that setting; other decimal settings are already short
        // enough not to need this. isPowerObj is cached once in setup() and
        // the format check is a pointer compare (both come from device.cpp's
        // same `formats[]`/kFmt4Decimals) - no strcmp on the hot path.
        const char *fmt = restrict;
        if (isPowerObj && restrict == kFmt4Decimals && fabs(mean_) >= 100.0)
            fmt = "%+08.3f";

        // Display mean of measured data
        char buf[24];
        lv_snprintf(buf, sizeof(buf), fmt, mean_);
        setLabelTextChangedOnly(label_measureValue, buf);
        shownValue = mean_;
    }
}

void DispObjects::displayUpdate(bool update)
{
    double mean_ = measured.Mean();
    // Display mean  of measured data not the value
    lv_label_set_text_fmt(label_measureValue, restrict, mean_);
    shownValue = NAN; // next displayUpdate() rewrites with the settled mean
}

void DispObjects::statUpdate(void)
{
    // Changed characters only (and no layout pass) - 12 labels, 3x a second.
    char buf[24];
    auto put = [&buf](lv_obj_t *lbl, const char *fmt, double v)
    {
        lv_snprintf(buf, sizeof(buf), fmt, v);
        setLabelTextChangedOnly(lbl, buf);
    };
    put(statLabels.label_setSmallFont, "%+08.4f", (adjValue - adjOffset) / adjFactor);
    put(statLabels.label_value, "%+08.4f", Statistics.value);
    put(statLabels.label_mean, "%+08.4f", Statistics.Mean());
    put(statLabels.label_std, "%07.4f", Statistics.StandardDeviation());
    put(statLabels.label_max, "%+08.4f", Statistics.absMax);
    put(statLabels.label_min, "%+08.4f", Statistics.absMin);
}


void DispObjects::barUpdate(void)
{
    // Guard: skip if Core 1 is rendering — lv_obj_set_x() is not safe from Core 0
    // (camera test 2026-07-23 confirmed: removing this produced stale-pixel
    // corruption within seconds and zero speed gain)
    if (blockAll || lvglIsBusy) return;
    if (!Bar.bar || !Bar.curValuePtr) return;  // Add null check for Power measurement

    // Pre-compute scale factors (per-instance cache in DispObject.h)
    lv_coord_t barMax = lv_bar_get_max_value(Bar.bar);
    if (barMax != cachedBarMax || cachedScaleFactor == 0) {
        cachedBarMax = barMax;
        cachedBarWidth = lv_obj_get_width(Bar.bar);
        cachedBarX = lv_obj_get_x(Bar.bar);
        cachedScaleFactor = adjFactor / maxValue;
    }

    // MEASURED bar only (shadow/setpoint bar is handled by shadowBarWidth()).
    // In mA mode the measured value is mA-numeric (0..8.192) but
    // cachedScaleFactor assumes the A range's full scale of 6.5536 A. The mA
    // range's full scale is 8.192 mA = 6.5536 x 1.25, so divide the scale by
    // 1.25 for the Current instance when mA is active. mA_Active is a bool
    // (atomic read on Xtensa), computed fresh each call - no cached value to
    // go stale on a range toggle, which is what sank an earlier attempt here.
    double scale = cachedScaleFactor;
    if (this == &PowerSupply.Current && PowerSupply.mA_Active)
        scale /= 1.25; // full bar at 8.192 mA instead of 6.5536 A

    // Compute bar value before critical section
    int32_t newBarValue = rawBarValue * scale * cachedBarMax;

    // OPTIMIZED: Direct pointer write instead of lv_bar_set_value()
    if (*Bar.curValuePtr != newBarValue) {
        // Repaint only the strip between the old and new indicator end, not
        // the whole 290 px bar (the indicator's tiled image is anchored at its
        // left edge, so nothing left of the change moves). Range == width
        // (0..barLength px), but convert anyway in case that ever changes.
        const int32_t oldV = *Bar.curValuePtr;
        *Bar.curValuePtr = newBarValue;
        const int32_t lo = std::max<int32_t>(0, std::min(oldV, newBarValue));
        const int32_t hi = std::min<int32_t>(cachedBarMax, std::max(oldV, newBarValue));
        invalidateColumns(Bar.bar, (lv_coord_t)((int64_t)lo * cachedBarWidth / cachedBarMax) - 1,
                          (lv_coord_t)((int64_t)hi * cachedBarWidth / cachedBarMax) + 1);
    }

    // Update max/min markers only on change — unconditional lv_obj_set_x() did a
    // style lookup every 1ms tick (~2000/s) even when the markers hadn't moved
    // (AUDIT_BARGRAPH_ENCODER #3), stealing Core 0 time from the ADC path.
    // Finite guard BEFORE the int() casts: absMin/absMax are +/-inf right
    // after ResetStats(), and casting inf to int is UB (documented hang risk
    // in input_handler.cpp's 'T' handler) - skip markers until samples exist.
    if (std::isfinite(measured.absMax)) {
        int newMaxX = cachedBarX + int(measured.absMax * scale * cachedBarWidth) - 3;
        if (newMaxX != lastMaxMarkerX) { lv_obj_set_x(Bar.bar_maxMarker, newMaxX); lastMaxMarkerX = newMaxX; }
    }
    if (std::isfinite(measured.absMin)) {
        int newMinX = cachedBarX + int(measured.absMin * scale * cachedBarWidth) - 3;
        if (newMinX != lastMinMarkerX) { lv_obj_set_x(Bar.bar_minMarker, newMinX); lastMinMarkerX = newMinX; }
    }

    Bar.changed = false;
}


// void DispObjects::barUpdate(void)
// {
//     // ULTRA-FAST PATH: Maximum speed bar update with direct memory manipulation
//     // Every nanosecond counts in the Core 0 ADC critical path!

//     // Fast null check - branch prediction friendly (usually true)
//     if (__builtin_expect((!Bar.bar || !Bar.curValuePtr), 0)) return;

//     // Pre-compute scale factors (cached - only on first call)
//     static lv_coord_t cachedBarMax = 0;
//     static lv_coord_t cachedBarX = 0;
//     static float cachedBarScaleFactor = 0.0f;      // float is faster than double on ESP32
//     static float cachedMarkerScaleFactor = 0.0f;   // Pre-multiplied for markers

//     // Initialization path (taken once)
//     if (__builtin_expect((cachedBarMax == 0), 0)) {
//         cachedBarMax = lv_bar_get_max_value(Bar.bar);
//         lv_coord_t cachedBarWidth = lv_obj_get_width(Bar.bar);
//         cachedBarX = lv_obj_get_x(Bar.bar);
//         cachedBarScaleFactor = (adjFactor / maxValue) * cachedBarMax;     // Pre-multiply
//         cachedMarkerScaleFactor = (adjFactor / maxValue) * cachedBarWidth; // Pre-multiply
//     }

//     // DIRECT MEMORY WRITE: Bar value (no function calls, no validation)
//     int32_t newBarValue = static_cast<int32_t>(rawBarValue * cachedBarScaleFactor);

//     if (*Bar.curValuePtr != newBarValue) {
//         *Bar.curValuePtr = newBarValue;  // Direct write to lv_bar_t.cur_value
//         lv_obj_invalidate(Bar.bar);      // Mark for redraw (TODO: can we bypass this too?)
//     }

//     // DIRECT MEMORY WRITE: Marker positions (bypass lv_obj_set_x completely)
//     // Writing directly to coords.x1 - MAXIMUM SPEED!
//     if (__builtin_expect((Bar.maxMarkerXPtr && Bar.minMarkerXPtr), 1)) {
//         *Bar.maxMarkerXPtr = cachedBarX + static_cast<lv_coord_t>(measured.absMax * cachedMarkerScaleFactor) - 3;
//         *Bar.minMarkerXPtr = cachedBarX + static_cast<lv_coord_t>(measured.absMin * cachedMarkerScaleFactor) - 3;

//         // Mark markers as dirty (faster than full lv_obj_set_x which does layout recalc)
//         lv_obj_invalidate(Bar.bar_maxMarker);
//         lv_obj_invalidate(Bar.bar_minMarker);
//     }

//     Bar.changed = false;
// }

void DispObjects::enableSetting(bool onOff)
{
    if (onOff)
        lv_obj_clear_flag(label_set, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(label_set, LV_OBJ_FLAG_HIDDEN);
}
char *DispObjects::getValue()
{
    return lv_label_get_text(label_measureValue);
}

void DispObjects::SetRotaryStep(double val)
{
    rotaryEncoderStep = val;
}

lv_coord_t DispObjects::shadowBarWidth(void)
{
    // Base fraction: setpoint counts over the full DAC-count span. adjValue,
    // adjOffset and maxValue are all DAC counts, so no adjFactor here.
    double frac = (adjValue - adjOffset) / maxValue;

    // mA range (Current only): the setpoint stays in A-range counts
    // (10 counts/mA - confirmed on hardware: entering "40 mA" = 400 counts;
    // the shunt change did NOT alter this side), but the bar's full scale in
    // mA mode is 8.192 mA = 81.92 counts. So the on-bar fraction is 800x the
    // count fraction (65536/10 / 8.192 = 800 exactly). Same fresh-read
    // pattern as barUpdate(): bool read is atomic, nothing cached to go stale.
    if (this == &PowerSupply.Current && PowerSupply.mA_Active)
        frac *= 800.0;

    // Setpoints above the mA range's 8.192 mA ceiling would overflow the
    // track - clamp to the bar's full width.
    if (frac > 1.0) frac = 1.0;
    if (frac < 0.0) frac = 0.0;

    return (lv_coord_t)(frac * lv_bar_get_max_value(Bar.bar));
}

void DispObjects::SetUpdate(int value, bool bypassLock)
{
    // value-= adjOffset;
    // if (lock || value / adjFactor < minValue || value / adjFactor > maxValue)
    if ((lock && !bypassLock) || (value) < 0 || (value - adjOffset) > maxValue)
    {
        myTone(NOTE_A5, 50);
        return;
    }
    // adjValue = std::max(std::min(value, maxValue), minValue);
    // adjValue = std::max(std::min(value, int(maxValue * adjFactor)), int(minValue * adjFactor));
    // adjValue = std::clamp(value, minValue, maxValue); => already taken care of
    adjValue = value;

    if (adjValue != adjValueOld)
    {
        adjValueChanged = true;
        adjValueOld = adjValue;
        myTone(NOTE_A4, 3);
    }

#if DEFER_DISPOBJ_LVGL_TO_CORE1
    // Function generator on Core 0 (Task_ADC) calls SetUpdate at 200-500 Hz.
    // Skip the LVGL writes here when not on Core 1; Flush() runs on Core 1
    // (via FlushMeasuresInterval) and updates the label/bar from adjValueChanged.
    if (xPortGetCoreID() != 1) return;
#endif

    // LV_LOG_USER("LOG: %s", "Beep!");
    // lv_disp_enable_invalidation( lv_disp_get_default(), false);
    // if (!lvglIsBusy)

    // Null pointer checks to prevent crashes during calibration or when UI not initialized
    if (label_setValue && label_unit) {
        // Changed characters only. This used to repaint the whole parent
        // (306x83) "to clean artifacts from overlapping transparent widgets";
        // a full-screen stale-pixel check (forced full redraw compared with
        // the incremental result, every second while the setpoint changed
        // repeatedly) found no artifacts without it.
        char buf[24];
        lv_snprintf(buf, sizeof(buf), "%+08.4f%s", (adjValue - adjOffset) / adjFactor, lv_label_get_text(label_unit));
        setLabelTextChangedOnly(label_setValue, buf);
    }
    updateShadowBar();

    // Serial.printf("\nmaxValue %15.5f  ",maxValue);
}

// The shadow bar object stays full width with a transparent background and
// draws its own rectangle of width shadowW (shadowBarDraw). Changing an
// object's width (the old lv_obj_set_width) marks the layout dirty, and the
// next refresh re-walks the whole object tree (~1-4 ms/frame measured in FUN
// mode, where the setpoint changes at 200-500 Hz) and repaints the whole bar
// row. This does neither; the radius margin covers the rounded end moving.
// Give the big reading an opaque background in the colour already behind it.
// LVGL redraws an area starting from the topmost object that fully covers it;
// with a transparent label that was the page, so every changed digit also
// redrew the page background, the "V-Set:" label and the screen under it
// (~0.45 ms of ~2.3 ms per digit, measured). Called once the UI is built and
// again whenever the page colour changes (setStatus).
// Skipped if anything between the label and the first opaque parent is
// partly transparent, or that parent isn't a plain colour.
void DispObjects::opaqueMeasureBackground(void)
{
    lv_obj_t *p = lv_obj_get_parent(label_measureValue);
    while (p && lv_obj_get_style_bg_opa(p, LV_PART_MAIN) == LV_OPA_TRANSP)
        p = lv_obj_get_parent(p);
    if (!p || lv_obj_get_style_bg_opa(p, LV_PART_MAIN) != LV_OPA_COVER ||
        lv_obj_get_style_bg_grad_dir(p, LV_PART_MAIN) != LV_GRAD_DIR_NONE || lv_obj_get_style_bg_img_src(p, LV_PART_MAIN))
        return;
    lv_obj_set_style_bg_color(label_measureValue, lv_obj_get_style_bg_color(p, LV_PART_MAIN), 0);
    lv_obj_set_style_bg_opa(label_measureValue, LV_OPA_COVER, 0);
}

void DispObjects::updateShadowBar(void)
{
    if (!Bar.bar_adjValue || !Bar.bar)
        return;
    const lv_coord_t newW = shadowBarWidth();
    const lv_coord_t oldW = shadowW;
    if (newW == oldW)
        return;
    shadowW = newW;
    const lv_coord_t r = lv_obj_get_style_radius(Bar.bar_adjValue, LV_PART_MAIN);
    invalidateColumns(Bar.bar_adjValue, std::min(oldW, newW) - r - 1, std::max(oldW, newW) + 1);
}

// Same lv_draw_rect() call lv_obj makes for its own background, on an area
// shadowW wide instead of the object's coords - so the pixels match what the
// old resized object drew.
static void shadowBarDraw(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    const DispObjects *d = (const DispObjects *)lv_event_get_user_data(e);
    if (d->shadowW <= 0)
        return;
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    lv_obj_init_draw_rect_dsc(obj, LV_PART_MAIN, &dsc);
    // The object's own bg is transparent, so init_draw_rect_dsc() skipped the
    // bg colour; fill it in exactly as it would have (no gradient here).
    dsc.bg_opa = DispObjects::kShadowBarOpa;
    dsc.bg_color = lv_obj_get_style_bg_color_filtered(obj, LV_PART_MAIN);
    dsc.bg_grad.dir = LV_GRAD_DIR_NONE;
    dsc.bg_grad.stops[0].color = dsc.bg_color;
    lv_area_t a = obj->coords;
    a.x2 = a.x1 + d->shadowW - 1;
    lv_draw_rect(lv_event_get_draw_ctx(e), &dsc, &a);
}

void DispObjects::Flush(void)
{

    if (adjValueChanged)
    {
        // static char *a = (char *)malloc(8);
        // char * a="%+08.4f";
        // strcpy(a, "%+08.4f");
        // strcat(a, lv_label_get_text(label_unit));
        // lv_label_set_text_fmt(label_setValue, a, adjValue +adjOffset);
        // Changed characters only - in FUN mode this runs at the 50 Hz flush rate.
        char buf[24];
        lv_snprintf(buf, sizeof(buf), strcmp(lv_label_get_text(label_unit), "A") ? "%+08.4fV" : "%+08.4fA",
                    (adjValue - adjOffset) / adjFactor);
        setLabelTextChangedOnly(label_setValue, buf);

        // Update the setpoint shadow bar - shared formula with SetUpdate()'s
        // direct-write path (see shadowBarWidth()). Keeping ONE implementation
        // matters: these two paths once drifted apart (Flush had an extra
        // /adjFactor) and the shadow bar "vanished" on every encoder turn
        // (Core 0 defers LVGL writes to this Flush) while keypad entry
        // (Core 1, SetUpdate's own write) restored it.
        updateShadowBar();
        adjValueChanged = false;
        // lv_obj_invalidate(label_setValue);
        // Serial.printf("\n%10.4f", (adjValue - adjOffset) / adjFactor);
    }
    // _lv_disp_refr_timer(NULL);
}

void DispObjects::SetEncoderUpdate(void)
{
    // static int counter = 0;
    // if ((counter++ % 4) == 3)

    static int64_t rotaryOldValue = 0;
    int64_t count = encoder.getCount() >> 1;
    if (rotaryOldValue == count)
        return;

    // Locked: still track the encoder position (so unlocking later doesn't
    // suddenly jump by however far it turned while locked), just don't act on it.
    if (!lock)
    {
        if (count > rotaryOldValue)
            SetUpdate(adjValue + rotaryEncoderStep);

        else if (count < rotaryOldValue)
            SetUpdate(adjValue - rotaryEncoderStep);
    }

    rotaryOldValue = count;
    // adjValueChanged=true;
}

void DispObjects::setLock(bool lck)
{
    if (lck)
    {
        lv_obj_clear_flag(lock_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(unlock_img, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_clear_flag(unlock_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(lock_img, LV_OBJ_FLAG_HIDDEN);
    }
    lock = lck;
}
bool DispObjects::getLock()
{
    return lock;
}

void DispObjects::setMeasureColor(lv_color_t color)
{

    lv_style_set_text_color(&style_deviceColor, color);

    lv_obj_remove_style(label_measureValue, &style_deviceColor, LV_STATE_DEFAULT);
    lv_obj_add_style(label_measureValue, &style_deviceColor, LV_STATE_DEFAULT);

    lv_obj_remove_style(label_unit, &style_deviceColor, LV_STATE_DEFAULT);
    lv_obj_add_style(label_unit, &style_deviceColor, LV_STATE_DEFAULT);

    lv_obj_remove_style(label_si_prefix, &style_deviceColor, LV_STATE_DEFAULT);
    lv_obj_add_style(label_si_prefix, &style_deviceColor, LV_STATE_DEFAULT);

    lv_style_set_bg_color(&style_deviceColor, color);
    lv_obj_remove_style(Bar.bar, &style_deviceColor, LV_PART_INDICATOR);
    lv_obj_add_style(Bar.bar, &style_deviceColor, LV_PART_INDICATOR);

    // if (label_unit)
    //     lv_obj_add_style(label_std, &style_deviceColor, LV_PART_INDICATOR);
}

void DispObjects::setStatsColor(lv_color_t color)
{

    // Serial.println(lv_label_get_text(label_unit));

    if (strcmp(lv_label_get_text(label_unit), "V") == 0)
    {
        lv_style_set_text_color(&stat_style1, color);

        lv_obj_remove_style(statLabels.label_value, &stat_style1, LV_STATE_DEFAULT);
        lv_obj_add_style(statLabels.label_value, &stat_style1, LV_STATE_DEFAULT);

        lv_obj_remove_style(statLabels.label_mean, &stat_style1, LV_STATE_DEFAULT);
        lv_obj_add_style(statLabels.label_mean, &stat_style1, LV_STATE_DEFAULT);

        lv_obj_remove_style(statLabels.label_std, &stat_style1, LV_STATE_DEFAULT);
        lv_obj_add_style(statLabels.label_std, &stat_style1, LV_STATE_DEFAULT);

        lv_obj_remove_style(statLabels.label_max, &stat_style1, LV_STATE_DEFAULT);
        lv_obj_add_style(statLabels.label_max, &stat_style1, LV_STATE_DEFAULT);

        lv_obj_remove_style(statLabels.label_min, &stat_style1, LV_STATE_DEFAULT);
        lv_obj_add_style(statLabels.label_min, &stat_style1, LV_STATE_DEFAULT);
    }
    else
    {
        lv_style_set_text_color(&stat_style2, color);

        lv_obj_remove_style(statLabels.label_std, &stat_style2, LV_STATE_DEFAULT);
        lv_obj_add_style(statLabels.label_std, &stat_style2, LV_STATE_DEFAULT);

        lv_obj_remove_style(statLabels.label_value, &stat_style2, LV_STATE_DEFAULT);
        lv_obj_add_style(statLabels.label_value, &stat_style2, LV_STATE_DEFAULT);

        lv_obj_remove_style(statLabels.label_mean, &stat_style2, LV_STATE_DEFAULT);
        lv_obj_add_style(statLabels.label_mean, &stat_style2, LV_STATE_DEFAULT);

        lv_obj_remove_style(statLabels.label_max, &stat_style2, LV_STATE_DEFAULT);
        lv_obj_add_style(statLabels.label_max, &stat_style2, LV_STATE_DEFAULT);

        lv_obj_remove_style(statLabels.label_min, &stat_style2, LV_STATE_DEFAULT);
        lv_obj_add_style(statLabels.label_min, &stat_style2, LV_STATE_DEFAULT);
    }
}

void DispObjects::SetupStyles()
{

    // lv_style_init(&style_set);
}

void DispObjects::setup(lv_obj_t *parent, const char *_text, int x, int y, const char *_unit, double maxValue_, double minValue_, double mTick,
                        double offset_, double factor_, const char *si_prefix,
                        const lv_font_t *font_measure, const lv_font_t *font_unit)
{
    maxValue = maxValue_;
    minValue = minValue_;
    isPowerObj = (strcmp(_unit, "W") == 0);

    // SetRotaryStep(maxValue_ / 65535);
    SetRotaryStep(1);
    measured.SetWindowSize(4);
    adjOffset = offset_; // / 1000.0; // Convert from milli volts/current to volt and current
    adjFactor = factor_;
    // const lv_font_t *font_measure = &dseg_b_48;
    // const lv_font_t *font_set = &Undertale_16b1; // WindyCity_16b4; // Undertale_16b;
    measureColor = {lv_palette_darken(LV_PALETTE_GREY, 3)};

    // effectiveResolution.SetWindowSize(4);
    /******************************
     **   Init hist
     ******************************/

    hist.histWinMax = maxValue;
    hist.histWinMin = minValue;

    /******************************
     **   Init styles
     ******************************/
    lv_style_init(&style_deviceColor);
    lv_style_init(&stat_style2);
    lv_style_init(&stat_style1);
    lv_style_init(&style_set);
    lv_style_init(&style_highlight_adjValue);
    lv_style_init(&style_set);
    /******************************
     **   Set
     ******************************/
    label_set = lv_label_create(parent);

    lv_label_set_text(label_set, _text);
    lv_obj_remove_style_all(label_set);
    lv_obj_align(label_set, LV_ALIGN_DEFAULT, x + 4, y);
    lv_style_set_text_font(&style_set, font_set);
    lv_style_set_text_letter_space(&style_set, -1);
    lv_obj_add_style(label_set, &style_set, LV_STATE_DEFAULT);
    label_setValue = lv_label_create(label_set);
    lv_label_set_text(label_setValue, "+00.0000");
    lv_obj_align(label_setValue, LV_ALIGN_DEFAULT, x + 85, 0);

    /******************************
     **   Highlight
     ******************************/
    highlight_adjValue = lv_obj_create(parent);
    lv_obj_remove_style_all(highlight_adjValue);
    lv_obj_set_size(highlight_adjValue, 12, 17);
    lv_obj_align(highlight_adjValue, 0, 8 * 12 - 2 - kHighlightXShift, -1);

    // static lv_style_t style_highlight_adjValue;
    // lv_style_init(&style_highlight_adjValue);
    lv_style_set_radius(&style_highlight_adjValue, 0);
    lv_style_set_border_width(&style_highlight_adjValue, 0);
    lv_style_set_bg_opa(&style_highlight_adjValue, LV_OPA_100);
    lv_obj_add_flag(highlight_adjValue, LV_OBJ_FLAG_FLOATING);
    lv_style_set_bg_color(&style_highlight_adjValue, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
    lv_obj_remove_style(highlight_adjValue, &style_highlight_adjValue, LV_STATE_DEFAULT);
    lv_obj_add_style(highlight_adjValue, &style_highlight_adjValue, LV_STATE_DEFAULT);
    lv_obj_move_background(highlight_adjValue);
    lv_obj_add_flag(highlight_adjValue, LV_OBJ_FLAG_HIDDEN);

    /******************************
     **   Lock/Unlock
     ******************************/
    lock_img = lv_img_create(parent);
    lv_obj_remove_style_all(lock_img);

    lv_img_set_src(lock_img, &p_l);
    lv_obj_align(lock_img, LV_ALIGN_DEFAULT, x + 186, y - 2);
    lv_obj_set_style_img_recolor_opa(lock_img, LV_OPA_100, 0);
    lv_obj_set_style_img_recolor(lock_img, lv_color_white(), 0);

    unlock_img = lv_img_create(parent);
    lv_obj_remove_style_all(unlock_img);
    lv_img_set_src(unlock_img, &p_u);
    lv_obj_align(unlock_img, LV_ALIGN_DEFAULT, x + 186, y - 2);
    lv_obj_set_style_img_recolor_opa(unlock_img, LV_OPA_100, 0);
    lv_obj_set_style_img_recolor(unlock_img, lv_color_white(), 0);

    lv_obj_add_flag(lock_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(unlock_img, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_clear_flag(img2, LV_OBJ_FLAG_HIDDEN);

    /******************************
     **   Stats
     ******************************/

    // label_setSmallFont = lv_label_create(label_set);
    // label_value = lv_label_create(label_set);
    // label_mean = lv_label_create(label_set);
    // label_std = lv_label_create(label_set);
    // label_max = lv_label_create(label_set);
    // label_min = lv_label_create(label_set);

    statLabels.label_unit = lv_label_create(label_set);
    lv_obj_align(statLabels.label_unit, LV_ALIGN_DEFAULT, -500, -500);
    statLabels.label_setSmallFont = lv_label_create(label_set);
    lv_obj_align(statLabels.label_setSmallFont, LV_ALIGN_DEFAULT, -500, -500);
    statLabels.label_value = lv_label_create(label_set);
    lv_obj_align(statLabels.label_value, LV_ALIGN_DEFAULT, -500, -500);
    statLabels.label_mean = lv_label_create(label_set);
    lv_obj_align(statLabels.label_mean, LV_ALIGN_DEFAULT, -500, -500);
    statLabels.label_max = lv_label_create(label_set);
    lv_obj_align(statLabels.label_max, LV_ALIGN_DEFAULT, -500, -500);
    statLabels.label_min = lv_label_create(label_set);
    lv_obj_align(statLabels.label_min, LV_ALIGN_DEFAULT, -500, -500);
    statLabels.label_std = lv_label_create(label_set);
    lv_obj_align(statLabels.label_std, LV_ALIGN_DEFAULT, -500, -500);
    statLabels.label_fft = lv_label_create(label_set);
    lv_obj_align(statLabels.label_fft, LV_ALIGN_DEFAULT, 0, -500);
    statLabels.label_legend = lv_label_create(label_set);
    lv_obj_align(statLabels.label_legend, LV_ALIGN_DEFAULT, 0, -500);

    /******************************
     **   Measure
     ******************************/
    label_measureValue = lv_label_create(parent); // parent
    lv_obj_remove_style_all(label_measureValue);
    lv_label_set_text(label_measureValue, "+00.000");
    lv_obj_align(label_measureValue, LV_ALIGN_DEFAULT, -14 - 10, y + 17);

    lv_style_init(&style_measure);
    lv_style_set_text_letter_space(&style_measure, 0);
    lv_style_set_text_color(&style_measure, measureColor);
    lv_style_set_text_font(&style_measure, font_measure);
    lv_obj_remove_style(label_measureValue, &style_measure, LV_STATE_DEFAULT);
    lv_obj_add_style(label_measureValue, &style_measure, LV_STATE_DEFAULT);
    // LVGL pads every label's redraw area by font_height/4 on all sides (for
    // italic glyphs) - 12 px here, turning each 261x48 digit update into a
    // ~285x72 repaint (+64%). The DSEG digit fonts overhang their box by at most
    // 3 px sideways and never vertically, so 3 px is enough. Runs after the
    // label class handler (user callbacks follow it), so it overrides it.
    lv_obj_add_event_cb(label_measureValue, [](lv_event_t *e) {
        *(lv_coord_t *)lv_event_get_param(e) = 3;
    }, LV_EVENT_REFR_EXT_DRAW_SIZE, nullptr);
    lv_obj_refresh_ext_draw_size(label_measureValue);

    /******************************
     **   Unit
     ******************************/
    label_unit = lv_label_create(parent);
    lv_obj_remove_style_all(label_unit);
    lv_label_set_text(label_unit, _unit);
    lv_obj_align(label_unit, LV_ALIGN_DEFAULT, x + 269, y + 3);

    // lv_style_init(&style_unit);
    lv_style_set_text_font(&style_unit, font_unit);
    lv_style_set_text_color(&style_unit, measureColor);
    lv_obj_remove_style(label_unit, &style_unit, LV_STATE_DEFAULT);
    lv_obj_add_style(label_unit, &style_unit, LV_STATE_DEFAULT);

    /******************************
     **   SI_Prefix
     ******************************/
    // Sibling of label_unit, not its child: as a child it needed
    // LV_OBJ_FLAG_OVERFLOW_VISIBLE on the unit label (it sits 12 px left of
    // it), and LVGL 8 draws an overflow-visible object for EVERY invalidated
    // area - the big V/A/W glyphs were laid out ~3x per frame, ~4 ms/frame.
    // Same position: label_unit is at (x+269, y+3) with no padding.
    label_si_prefix = lv_label_create(parent);
    lv_obj_remove_style_all(label_si_prefix);
    lv_label_set_text(label_si_prefix, si_prefix); // µΩm
    lv_obj_align(label_si_prefix, LV_ALIGN_DEFAULT, x + 269 - 12, y + 3 + 2);

    lv_style_init(&style_si_prefix);
    lv_style_set_text_letter_space(&style_si_prefix, 0);
    lv_style_set_text_color(&style_si_prefix, measureColor);
    lv_style_set_text_font(&style_si_prefix, &DejaVuSans_b_22); //&DejaVuSans_R_24 &Tauri_R_20
    lv_obj_remove_style(label_si_prefix, &style_si_prefix, LV_STATE_DEFAULT);
    lv_obj_add_style(label_si_prefix, &style_si_prefix, LV_STATE_DEFAULT);

    lv_obj_add_flag(label_si_prefix, LV_OBJ_FLAG_HIDDEN);

    /******************************
     **   Bar
     ******************************/
    auto barLength = 290.0;

    /******************************
    **   Bar Setting
    ******************************/
    if (strcmp(_unit, "W") != 0)
    {
        // Stays full width; the setpoint width is drawn by shadowBarDraw()
        // (see updateShadowBar() for why it is no longer resized).
        Bar.bar_adjValue = lv_obj_create(label_set);
        lv_obj_set_size(Bar.bar_adjValue, barLength, 5);
        lv_obj_align(Bar.bar_adjValue, 0, 5, 70);
        shadowW = barLength;
        lv_obj_add_event_cb(Bar.bar_adjValue, shadowBarDraw, LV_EVENT_DRAW_MAIN, this);
        static lv_style_t style_bar_adjValue;
        static bool style_bar_adjValue_inited = false;
        if (!style_bar_adjValue_inited) {
            style_bar_adjValue_inited = true;
            lv_style_init(&style_bar_adjValue);
            lv_style_set_border_width(&style_bar_adjValue, 0);
            // Darker + lower opacity than the original 0xB1BFB1 @ 40% - the
            // shadow should read as a faint reference mark, not compete with
            // the measured bar.
            lv_style_set_bg_opa(&style_bar_adjValue, LV_OPA_TRANSP); // drawn by shadowBarDraw() at kShadowBarOpa
            lv_style_set_bg_color(&style_bar_adjValue, lv_color_hex(0x5A625A));
        }
        lv_obj_remove_style(Bar.bar_adjValue, &style_bar_adjValue, LV_STATE_DEFAULT);
        lv_obj_add_style(Bar.bar_adjValue, &style_bar_adjValue, LV_STATE_DEFAULT);
    }

    /******************************
     **   Bar.bar
     ******************************/

    Bar.bar = lv_bar_create(label_set);

    /******************************
     ** no need bars and ticks for power (save momory)
     ******************************/
    if (strcmp(_unit, "W") == 0)
        return;

    LV_IMG_DECLARE(img_skew_strip328);
    lv_bar_set_range(Bar.bar, 0, barLength);
    static lv_style_t style_indic;
    static bool style_indic_inited = false;
    if (!style_indic_inited) {
        style_indic_inited = true;
        lv_style_init(&style_indic);
        lv_style_set_bg_img_src(&style_indic, &img_skew_strip328);
        lv_style_set_bg_img_tiled(&style_indic, 40);
        lv_style_set_bg_img_opa(&style_indic, LV_OPA_50);
        lv_style_set_radius(&style_indic, 0);
    }
    lv_obj_remove_style(Bar.bar, NULL, LV_PART_MAIN);
    lv_obj_remove_style(Bar.bar, &style_indic, LV_STATE_DEFAULT);
    lv_obj_add_style(Bar.bar, &style_indic, LV_PART_INDICATOR);
    lv_obj_set_size(Bar.bar, barLength, 5);
    lv_obj_align(Bar.bar, 0, 2, 70);
    lv_bar_set_value(Bar.bar, 0, LV_ANIM_OFF);

    /******************************
     **   Bar ticks
     ******************************/
    static lv_point_t majorTick[] = {{0, 0}, {0, 3}};
    static lv_point_t minorTick[] = {{0, 0}, {0, 1}};

    lv_obj_t *lineMj[7];
    lv_obj_t *lineMn[32];

    double tickDist = barLength / floor(maxValue / adjFactor * 5.0 / mTick);

    for (auto i = 0; i < 7; i++)
    {
        lineMj[i] = lv_line_create(label_set);
        lv_line_set_points(lineMj[i], majorTick, 2); /*Set the points*/
        lv_obj_align(lineMj[i], 0, 2 + i * tickDist * 5, 70 + 1);
    }

    for (auto i = 0; i < 32; i++)
    {
        lineMn[i] = lv_line_create(label_set);
        lv_line_set_points(lineMn[i], minorTick, 2); /*Set the points*/
        lv_obj_align(lineMn[i], 0, 2 + i * tickDist, 70 + 4);
    }

    // The 39 tick lines sit under the bar, so every bar frame repainted 39
    // widgets - per-object draw overhead was 4.4 of a 13 ms bar frame. Keep
    // them as hidden geometry/style references and draw them all from one
    // transparent host, created right here so it keeps their z-order (above
    // the bars, below the markers). Same math as lv_line's own DRAW_MAIN.
    for (auto i = 0; i < 7; i++)
        lv_obj_add_flag(lineMj[i], LV_OBJ_FLAG_HIDDEN);
    for (auto i = 0; i < 32; i++)
        lv_obj_add_flag(lineMn[i], LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *tickHost = lv_obj_create(label_set);
    lv_obj_remove_style_all(tickHost);
    lv_obj_clear_flag(tickHost, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(tickHost, 0, 66); // covers ticks at y 71..75, x 2..barLength+2
    lv_obj_set_size(tickHost, barLength + 16, 14);
    lv_obj_add_event_cb(tickHost, [](lv_event_t *e) {
        lv_obj_t *set = (lv_obj_t *)lv_event_get_user_data(e);
        lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);
        lv_draw_line_dsc_t dsc;
        bool dscReady = false;
        for (uint32_t c = 0; c < lv_obj_get_child_cnt(set); c++)
        {
            lv_obj_t *ln = lv_obj_get_child(set, c);
            if (!lv_obj_check_type(ln, &lv_line_class))
                continue;
            if (!dscReady) // all ticks share the default line style
            {
                lv_draw_line_dsc_init(&dsc);
                lv_obj_init_draw_line_dsc(ln, LV_PART_MAIN, &dsc);
                dscReady = true;
            }
            const lv_point_t *pts = ((lv_line_t *)ln)->point_array; // no getter in LVGL 8
            lv_area_t a;
            lv_obj_get_coords(ln, &a);
            lv_point_t p1 = {(lv_coord_t)(pts[0].x + a.x1), (lv_coord_t)(pts[0].y + a.y1)};
            lv_point_t p2 = {(lv_coord_t)(pts[1].x + a.x1), (lv_coord_t)(pts[1].y + a.y1)};
            lv_draw_line(draw_ctx, &dsc, &p1, &p2);
        }
    }, LV_EVENT_DRAW_MAIN, label_set);

    /******************************
    **   Bar max Value
    ******************************/
    Bar.bar_maxValue = lv_label_create(label_set);
    static char buffer[64];

    lv_label_set_text(Bar.bar_maxValue, buffer);
    static lv_style_t style_barMaxValue;
    static bool style_barMaxValue_inited = false;
    if (!style_barMaxValue_inited) {
        style_barMaxValue_inited = true;
        lv_style_init(&style_barMaxValue);
        lv_style_set_text_letter_space(&style_barMaxValue, 0);
        lv_style_set_text_color(&style_barMaxValue, lv_palette_lighten(LV_PALETTE_AMBER, 5));
        lv_style_set_text_font(&style_barMaxValue, &ATARISTOCRAT_16bt);
    }
    lv_obj_remove_style(Bar.bar_maxValue, &style_barMaxValue, LV_STATE_DEFAULT);
    lv_obj_add_style(Bar.bar_maxValue, &style_barMaxValue, LV_STATE_DEFAULT);
    lv_obj_align(Bar.bar_maxValue, LV_ALIGN_DEFAULT, x + barLength + 26, 70 - 2);
    // voltageScaleFactor
    Bar.scaleFactor = lv_bar_get_max_value(Bar.bar) / (maxValue / adjFactor) * .5;
    Bar.curValuePtr = &((lv_bar_t *)Bar.bar)->cur_value;

    //  int32_t *curValuePtr = &((lv_bar_t *)PowerSupply.Voltage.Bar.bar)->cur_value;

    /******************************
     **   Bar min Marker
     ******************************/
    Bar.bar_minMarker = lv_label_create(label_set);
    lv_label_set_text(Bar.bar_minMarker, "^");
    static lv_style_t style_barLabelMinMarker;
    static bool style_barMinMarker_inited = false;
    if (!style_barMinMarker_inited) {
        style_barMinMarker_inited = true;
        lv_style_init(&style_barLabelMinMarker);
        lv_style_set_text_font(&style_barLabelMinMarker, &lv_font_unscii_8);
        lv_style_set_text_color(&style_barLabelMinMarker, lv_palette_lighten(LV_PALETTE_AMBER, 1));
    }
    lv_obj_remove_style(Bar.bar_minMarker, &style_barLabelMinMarker, LV_STATE_DEFAULT);
    lv_obj_add_style(Bar.bar_minMarker, &style_barLabelMinMarker, LV_STATE_DEFAULT);

    lv_obj_align(Bar.bar_minMarker, 0, -1, 70 + 4);

    /******************************
     **   Bar max Marker
     ******************************/
    Bar.bar_maxMarker = lv_label_create(label_set);
    lv_label_set_text(Bar.bar_maxMarker, "^");
    static lv_style_t style_barLabelMaxMarker;
    static bool style_barMaxMarker_inited = false;
    if (!style_barMaxMarker_inited) {
        style_barMaxMarker_inited = true;
        lv_style_init(&style_barLabelMaxMarker);
        lv_style_set_text_font(&style_barLabelMaxMarker, &lv_font_unscii_8);
        lv_style_set_text_color(&style_barLabelMaxMarker, lv_palette_lighten(LV_PALETTE_RED, 1));
    }
    lv_obj_remove_style(Bar.bar_maxMarker, &style_barLabelMaxMarker, LV_STATE_DEFAULT);
    lv_obj_add_style(Bar.bar_maxMarker, &style_barLabelMaxMarker, LV_STATE_DEFAULT);

    lv_obj_align(Bar.bar_maxMarker, 0, -1, 70 + 4);

    // SPEED OPTIMIZATION: Direct pointers to marker X coordinates
    // Bypasses lv_obj_set_x() function call overhead for maximum performance
    Bar.minMarkerXPtr = &(Bar.bar_minMarker->coords.x1);
    Bar.maxMarkerXPtr = &(Bar.bar_maxMarker->coords.x1);
}
