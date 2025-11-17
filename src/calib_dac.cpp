#include "calib_dac.h"
#include "device.hpp"
#include "setting_menu.h"
#include "spinbox_pro.h"
#include "myFonts.h"

// External references
extern Device PowerSupply;

namespace
{
    // DAC window widgets we need in its callbacks
    lv_obj_t *s_zv = nullptr; // zero voltage spin
    lv_obj_t *s_mv = nullptr; // max  voltage spin
    lv_obj_t *s_zc = nullptr; // zero current spin
    lv_obj_t *s_mc = nullptr; // max  current spin

    lv_obj_t *s_label_maxV = nullptr;
    lv_obj_t *s_label_maxC = nullptr;

    // DAC handlers
    static void DAC_voltage_calib_change_event_cb(lv_event_t *e)
    {
        if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
            return;
        double zv = lv_spinbox_get_value(s_zv);
        double mv = lv_spinbox_get_value(s_mv);
        lv_label_set_text_fmt(s_label_maxV, "%+6.4fV", (mv - zv) / 2000.0);

        PowerSupply.dac_data.zero_voltage = lv_spinbox_get_value(s_zv);
        PowerSupply.dac_data.max_voltage = lv_spinbox_get_value(s_mv);
        PowerSupply.dac_data.zero_current = lv_spinbox_get_value(s_zc);
        PowerSupply.dac_data.max_current = lv_spinbox_get_value(s_mc);

        PowerSupply.Voltage.adjOffset = PowerSupply.dac_data.zero_voltage;
        PowerSupply.Voltage.minValue = (-PowerSupply.dac_data.zero_voltage);
        PowerSupply.Voltage.maxValue = (mv - zv);
        PowerSupply.SaveDACdata("dac_data_", PowerSupply.dac_data);
        PowerSupply.Voltage.SetEncoderUpdate();
    }

    static void DAC_current_calib_change_event_cb(lv_event_t *e)
    {
        if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
            return;
        double zc = lv_spinbox_get_value(s_zc);
        double mc = lv_spinbox_get_value(s_mc);
        lv_label_set_text_fmt(s_label_maxC, "%+6.4fA", (mc - zc) / 10000.0);

        PowerSupply.dac_data.zero_voltage = lv_spinbox_get_value(s_zv);
        PowerSupply.dac_data.max_voltage = lv_spinbox_get_value(s_mv);
        PowerSupply.dac_data.zero_current = lv_spinbox_get_value(s_zc);
        PowerSupply.dac_data.max_current = lv_spinbox_get_value(s_mc);

        PowerSupply.Current.adjOffset = PowerSupply.dac_data.zero_current;
        PowerSupply.Current.minValue = (-PowerSupply.dac_data.zero_current) / 10000.0;
        PowerSupply.Current.maxValue = (mc - zc) / 10000.0;
        PowerSupply.SaveDACdata("dac_data_", PowerSupply.dac_data);
        PowerSupply.Current.SetEncoderUpdate();
    }
}

// Open/create the DAC calibration window
void open_dac_calibration_cb(lv_event_t *)
{
    if (PowerSupply.gui.calibration.win_DAC_calibration)
    {
        lv_obj_clear_flag(PowerSupply.gui.calibration.win_DAC_calibration, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    PowerSupply.gui.calibration.win_DAC_calibration = lv_win_create(lv_scr_act(), 36);
    lv_obj_set_size(PowerSupply.gui.calibration.win_DAC_calibration, 320, 226);
    lv_win_add_title(PowerSupply.gui.calibration.win_DAC_calibration, "DAC Calibration");
    auto *close = lv_win_add_btn(PowerSupply.gui.calibration.win_DAC_calibration, LV_SYMBOL_CLOSE, 60);
    lv_obj_add_event_cb(close, btn_close_hide_obj_cb, LV_EVENT_CLICKED, nullptr);
    auto *cont = lv_win_get_content(PowerSupply.gui.calibration.win_DAC_calibration);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    auto *title = lv_label_create(cont);
    lv_label_set_text(title, "DAC Code      Output V/C");
    lv_obj_align(title, LV_ALIGN_OUT_LEFT_TOP, 95, 10);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0x00FFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    int xPos = 94, yPos = 40, yOffset = 25, xOffset = 10;

    s_zv = spinbox_pro(cont, "#FFFFF7 Zero Voltage:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 0, 86, 15, &graph_R_16);
    s_mv = spinbox_pro(cont, "#FFFFF7 Max  Voltage:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 1, 86, 16, &graph_R_16);
    s_zc = spinbox_pro(cont, "#FFFFF7 Zero Current:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 2.3, 86, 17, &graph_R_16);
    s_mc = spinbox_pro(cont, "#FFFFF7 Max  Current:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 3.3, 86, 18, &graph_R_16);

    auto addLabel = [&](lv_obj_t *parent, const char *fmt, double value) -> lv_obj_t *
    {
        lv_obj_t *parent_child = lv_obj_get_child(parent, 1);
        lv_obj_align(parent_child, LV_ALIGN_OUT_RIGHT_MID, -90, 0);
        auto *lbl = lv_label_create(parent);
        lv_obj_add_flag(parent, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
        lv_obj_align_to(lbl, parent, LV_ALIGN_OUT_RIGHT_MID, xOffset, 0);
        lv_label_set_text_fmt(lbl, fmt, value);
        lv_obj_set_style_text_font(lbl, &graph_R_16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        return lbl;
    };

    addLabel(s_zv, "+%07.4fV", 0.0);
    s_label_maxV = addLabel(s_mv, "+%07.4fV", 32.750);

    addLabel(s_zc, "%+06.4fA", 0.0);
    s_label_maxC = addLabel(s_mc, "%+06.4fA", 6.5535);

    // Load persisted DAC data and update limits
    PowerSupply.dac_data = PowerSupply.LoadDACdata("dac_data_");
    lv_spinbox_set_value(s_zv, PowerSupply.dac_data.zero_voltage);
    lv_spinbox_set_value(s_mv, PowerSupply.dac_data.max_voltage);
    lv_spinbox_set_value(s_zc, PowerSupply.dac_data.zero_current);
    lv_spinbox_set_value(s_mc, PowerSupply.dac_data.max_current);

    PowerSupply.Voltage.adjOffset = PowerSupply.dac_data.zero_voltage;
    PowerSupply.Voltage.minValue = (-PowerSupply.dac_data.zero_voltage);
    PowerSupply.Voltage.maxValue = (PowerSupply.dac_data.max_voltage - PowerSupply.dac_data.zero_voltage);

    PowerSupply.Current.adjOffset = PowerSupply.dac_data.zero_current;
    PowerSupply.Current.minValue = (-PowerSupply.dac_data.zero_current);
    PowerSupply.Current.maxValue = (PowerSupply.dac_data.max_current - PowerSupply.dac_data.zero_current);

    lv_obj_add_event_cb(s_zv, DAC_voltage_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, s_label_maxV);
    lv_obj_add_event_cb(s_mv, DAC_voltage_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, s_label_maxV);
    lv_obj_add_event_cb(s_zc, DAC_current_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
    lv_obj_add_event_cb(s_mc, DAC_current_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);
}
