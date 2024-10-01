#include "DispObject.h"

extern bool lvglIsBusy;
void DispObjects::SetEncoderPins(int aPintNumber, int bPinNumber, enc_isr_cb_t enc_isr_cb = nullptr)
{
    encoder = new ESP32Encoder;
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
    double rv = round(value * 320);
    if (Bar.oldValue != rv)
    {

        Bar.changed = true;
        Bar.oldValue = rv;
    }
    if (oldValue != value)
    {
        oldValue = value;
        changed = true;
    }
    Statistics(value);

    double er_sample = Statistics.ER(2 * maxValue);
    if (!std::isinf(er_sample))
        effectiveResolution(er_sample);
}

void DispObjects::displayUpdate(void)
{
    static double mean_;
    mean_ = measured.Mean();
    if (oldValue != mean_)
    {
        // Display mean  of measured data not the value
        lv_label_set_text_fmt(label_measureValue, restrict, mean_);
        // lv_obj_invalidate(label_measureValue);
        changed = true;
        oldValue = mean_;
    }

    // if ((millis() - timeKeeper) >= 250)
    // {
    // timeKeeper = millis();

    // lv_label_set_text_fmt(label_setSmallFont, "%+08.4f", adjValue+adjOffset);
    //     lv_label_set_text_fmt(label_value, "%+08.4f", measured.value);
    //     lv_label_set_text_fmt(label_mean, "%+08.4f", mean);
    //     lv_label_set_text_fmt(label_std, "%07.4f", measured.StandardDeviation());
    //     lv_label_set_text_fmt(label_max, "%+08.4f", measured.absMax);
    //     lv_label_set_text_fmt(label_min, "%+08.4f", measured.absMin);
    // }
}

void DispObjects::statUpdate(void)
{

    lv_label_set_text_fmt(statLabels.label_setSmallFont, "%+08.4f", adjValue + adjOffset);
    lv_label_set_text_fmt(statLabels.label_value, "%+08.4f", Statistics.value);
    lv_label_set_text_fmt(statLabels.label_mean, "%+08.4f", Statistics.Mean());
    lv_label_set_text_fmt(statLabels.label_std, "%07.4f", Statistics.StandardDeviation());
    lv_label_set_text_fmt(statLabels.label_max, "%+08.4f", Statistics.absMax);
    lv_label_set_text_fmt(statLabels.label_min, "%+08.4f", Statistics.absMin);
}

void DispObjects::barUpdate(void)
{

    if (Bar.changed)
    {
        lv_bar_set_value(Bar.bar, Statistics.value / maxValue * lv_bar_get_max_value(Bar.bar), LV_ANIM_OFF);
        lv_obj_invalidate(Bar.bar); // it will update only when lv_timer_handler is called so no matter how offen this is called

        // lv_obj_set_width(Bar.bar_adjValue, adjValue / maxValue * lv_bar_get_max_value(Bar.bar));

        // lv_obj_align(Bar.bar_minMarker, LV_ALIGN_TOP_LEFT, lv_obj_get_x(Bar.bar) + int(measured.absMin / denum) - 3, lv_obj_get_y(Bar.bar) + 4);
        // lv_obj_align(Bar.bar_maxMarker, LV_ALIGN_TOP_LEFT, lv_obj_get_x(Bar.bar) + int(measured.absMax /denum) - 3, lv_obj_get_y(Bar.bar) + 4);

        static double oldMaxValue{0};

        if (Statistics.absMax != oldMaxValue)
        {
            lv_obj_set_x(Bar.bar_maxMarker, lv_obj_get_x(Bar.bar) + int(Statistics.absMax * lv_obj_get_width(Bar.bar) / maxValue) - 3);
            oldMaxValue = Statistics.absMax;
        }
        static double oldMinValue{0};
        if (Statistics.absMin != oldMinValue)
        {
            lv_obj_set_x(Bar.bar_minMarker, lv_obj_get_x(Bar.bar) + int(Statistics.absMin * lv_obj_get_width(Bar.bar) / maxValue) - 3);
            oldMinValue = Statistics.absMin;
        }
        // LV_LOG_USER("Voltage max bar:%f",Statistics.value);
        // Bar.changed = false;
        // oldMaxValue=-INFINITY;
        // oldMinValue = INFINITY;
    }
}

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

void DispObjects::SetUpdate(double value)
{
    // value+=-adjOffset;
    if (lock || value < minValue || value > maxValue)
    {
        myTone(NOTE_A5, 50);
        return;
    }

    adjValue = std::max(std::min(value, maxValue), minValue);
    // adjValue = std::clamp(value, minValue, maxValue); => already taken care of
    adjValue = value;
    myTone(NOTE_A4, 3);

    if (adjValue != adjValueOld)
    {
        adjValueChanged = true;
        adjValueOld = adjValue;
    }

    // LV_LOG_USER("LOG: %s", "Beep!");
    // lv_disp_enable_invalidation( lv_disp_get_default(), false);
    // if (!lvglIsBusy)
    lv_label_set_text_fmt(label_setValue, "%+08.4f", adjValue);
    // lv_disp_enable_invalidation(lv_disp_get_default(), true);
    // lv_obj_invalidate(label_setValue);
    // toneOff();
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
        if (strcmp(lv_label_get_text(label_unit), "A"))

            lv_label_set_text_fmt(label_setValue, "%+08.4fV", adjValue + adjOffset);
        else
            lv_label_set_text_fmt(label_setValue, "%+08.4fA", adjValue + adjOffset);

        // update bar setting shadaow
        lv_obj_set_width(Bar.bar_adjValue, (adjValue + adjOffset) / maxValue * lv_bar_get_max_value(Bar.bar));
        // adjValueChanged = false;
        lv_obj_invalidate(label_setValue);
        // Serial.printf("\n%10.4f",adjValue+adjOffset);
    }
    // _lv_disp_refr_timer(NULL);
}

void DispObjects::SetEncoderUpdate(void)
{
    // if (lock)
    // return;
    // static int counter = 0;
    // if ((counter++ % 4) == 3)

    static int64_t rotaryOldValue = 0;
    int64_t count = encoder.getCount() / 2;
    if (rotaryOldValue == count)
        return;

    if (count > rotaryOldValue)
        SetUpdate(adjValue + rotaryEncoderStep);

    else if (count < rotaryOldValue)
        SetUpdate(adjValue - rotaryEncoderStep);

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

void DispObjects::setup(lv_obj_t *parent, const char *_text, int x, int y, const char *_unit, double maxValue_, double mTick,
                        double offset_, double factor_,
                        const lv_font_t *font_measure, const lv_font_t *font_unit)
{
    maxValue = maxValue_;
    minValue = 0.0;
    SetRotaryStep(maxValue_ / 65535);
    measured.SetWindowSize(4);
    adjOffset = offset_ / 1000.0; // Convert from milli volts/current to volt and current
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
    lv_obj_align(highlight_adjValue, 0, 8 * 12 - 2, -1);

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
     **   Bar
     ******************************/
    auto barLength = 290.0;

    /******************************
    **   Bar Setting
    ******************************/
    if (strcmp(_unit, "W") != 0)
    {
        Bar.bar_adjValue = lv_obj_create(label_set);
        lv_obj_set_size(Bar.bar_adjValue, barLength, 5);
        lv_obj_align(Bar.bar_adjValue, 0, 5, 70);
        static lv_style_t style_bar_adjValue;
        lv_style_init(&style_bar_adjValue);
        // lv_style_set_radius(&style_bar_adjValue, 0);
        lv_style_set_border_width(&style_bar_adjValue, 0);

        lv_style_set_bg_opa(&style_bar_adjValue, LV_OPA_40);
        lv_style_set_bg_color(&style_bar_adjValue, lv_color_hex(0xB1BFB1)); // lv_color_hex(0x919F91) //lv_palette_darken(LV_PALETTE_GREY, 1)
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
    lv_style_init(&style_indic);
    lv_style_set_bg_img_src(&style_indic, &img_skew_strip328);
    lv_style_set_bg_img_tiled(&style_indic, 40);
    //  lv_img_set_zoom(img1, 256 / 11);

    lv_style_set_bg_img_opa(&style_indic, LV_OPA_50);
    lv_style_set_radius(&style_indic, 0);
    lv_obj_remove_style(Bar.bar, NULL, LV_PART_MAIN);
    // lv_style_set_bg_color(&style_indic, lv_color_hex(0x000000)); // lv_palette_darken(LV_PALETTE_BLUE, 3)
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

    double tickDist = barLength / floor(maxValue * 5.0 / mTick);

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

    /******************************
    **   Bar max Value
    ******************************/
    Bar.bar_maxValue = lv_label_create(label_set);
    static char buffer[64];

    lv_label_set_text(Bar.bar_maxValue, buffer);
    static lv_style_t style_barMaxValue;
    lv_style_init(&style_barMaxValue);
    lv_style_set_text_letter_space(&style_barMaxValue, 0);
    lv_style_set_text_color(&style_barMaxValue, lv_palette_lighten(LV_PALETTE_AMBER, 5));
    lv_style_set_text_font(&style_barMaxValue, &ATARISTOCRAT_16bt);
    lv_obj_remove_style(Bar.bar_maxValue, &style_barMaxValue, LV_STATE_DEFAULT);
    lv_obj_add_style(Bar.bar_maxValue, &style_barMaxValue, LV_STATE_DEFAULT);
    lv_obj_align(Bar.bar_maxValue, LV_ALIGN_DEFAULT, x + barLength + 26, 70 - 2);

    /******************************
     **   Bar min Marker
     ******************************/
    Bar.bar_minMarker = lv_label_create(label_set);
    lv_label_set_text(Bar.bar_minMarker, "^");
    static lv_style_t style_barLabelMinMarker;
    lv_style_init(&style_barLabelMinMarker);
    lv_style_set_text_font(&style_barLabelMinMarker, &lv_font_unscii_8); // Tauri_R_56,SairaExtraCondensed_R_56, PathwayGothicOne_R_56
    lv_style_set_text_color(&style_barLabelMinMarker, lv_palette_lighten(LV_PALETTE_AMBER, 1));
    lv_obj_remove_style(Bar.bar_minMarker, &style_barLabelMinMarker, LV_STATE_DEFAULT);
    lv_obj_add_style(Bar.bar_minMarker, &style_barLabelMinMarker, LV_STATE_DEFAULT);

    lv_obj_align(Bar.bar_minMarker, 0, -1, 70 + 4);

    /******************************
     **   Bar max Marker
     ******************************/
    Bar.bar_maxMarker = lv_label_create(label_set);
    lv_label_set_text(Bar.bar_maxMarker, "^");
    static lv_style_t style_barLabelMaxMarker;
    lv_style_init(&style_barLabelMaxMarker);
    lv_style_set_text_font(&style_barLabelMaxMarker, &lv_font_unscii_8); // Tauri_R_56,SairaExtraCondensed_R_56, PathwayGothicOne_R_56
    lv_style_set_text_color(&style_barLabelMaxMarker, lv_palette_lighten(LV_PALETTE_RED, 1));
    lv_obj_remove_style(Bar.bar_maxMarker, &style_barLabelMaxMarker, LV_STATE_DEFAULT);
    lv_obj_add_style(Bar.bar_maxMarker, &style_barLabelMaxMarker, LV_STATE_DEFAULT);

    lv_obj_align(Bar.bar_maxMarker, 0, -1, 70 + 4);
}
