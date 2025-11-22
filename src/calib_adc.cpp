#include "calib_adc.h"
#include "calib_internal_leakage.h"
#include "calib_log_window.h"
#include "device.hpp"
#include "spinbox_pro.h"
#include "lv_gui_helper.h"
#include "myFonts.h"
#include "buzzer.h"
#include "esp_task_wdt.h"
#include <cmath>
#include <cstring>

// External references
extern Device PowerSupply;
extern CalibrationGui Calib_GUI;

namespace
{
    // Forward declaration
    static void auto_zero_event_cb(lv_event_t *e);

    // Callbacks for save/load buttons in ADC calibration window
    static void save_cb(lv_event_t *)
    {
        PowerSupply.SaveCalibrationData();
        myTone(NOTE_A4, 200);
    }

    static void load_cb(lv_event_t *)
    {
        PowerSupply.LoadCalibrationData();
        myTone(NOTE_A4, 200);
    }

    static void autoZeroCurrent_cb(lv_event_t *)
    {
        Warning_msgbox("Auto Zero", auto_zero_event_cb);
    }

    // Dummy callback for error message boxes
    static void error_msgbox_dummy_cb(lv_event_t *e) {
        // Do nothing - just close the message box
        lv_obj_t *obj = lv_event_get_current_target(e);
        if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
            lv_msgbox_close_async(obj);
        }
    }

    static void safe_calibration_task(void *parameter) {
        Serial.println("Starting safe calibration task...");
        
        // Remove this task from watchdog monitoring
        esp_task_wdt_delete(NULL);
        
        // Check memory before starting
        size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        size_t free_stack = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("Calibration task - Free heap: %d, Free stack: %d\n", free_heap, free_stack);
        
        if (free_heap < 20000 || free_stack < 2048) {
            Serial.println("ERROR: Insufficient resources for calibration");
            vTaskDelete(NULL);
            return;
        }
        
        try {
            // Perform calibration with frequent yields
            start_current_zero_calibration(nullptr);
        } catch (...) {
            Serial.println("Exception during calibration!");
        }
        
        Serial.println("Calibration task completed");
        vTaskDelete(NULL);
    }

static void auto_zero_event_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_current_target(e);
    
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
    {
        const char *txt = lv_msgbox_get_active_btn_text(obj);
        
        // Do NOT use lv_msgbox_close(obj); here.
        // It deletes the object immediately, causing the crash when this function returns.
        
        // Use this instead:
        lv_msgbox_close_async(obj); 
        // OR if your LVGL version is older and doesn't have close_async:
        // lv_obj_del_async(obj);

        bool is_ok = (txt && strcmp(txt, "OK") == 0);

            if (is_ok)
            {
                // Check system resources first
                size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
                size_t largest_block = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
                
                Serial.printf("System check - Free heap: %d, Largest block: %d\n", 
                             free_heap, largest_block);
                
                if (free_heap < 30000 || largest_block < 10000) {
                    Serial.println("ERROR: System resources too low for calibration");
                    Warning_msgbox("Error: Low Resources", error_msgbox_dummy_cb);
                    return;
                }
                
                create_log_window();
                log_reset();
                log_clear();
                
                // Run calibration in separate task with ample stack
                BaseType_t result = xTaskCreate(
                    safe_calibration_task,
                    "calibration_task",
                    12288,  // 12KB stack - INCREASED from 8KB
                    NULL,
                    tskIDLE_PRIORITY + 1,  // Low priority
                    NULL
                );
                
                if (result != pdPASS) {
                    Serial.println("ERROR: Failed to create calibration task");
                    Warning_msgbox("Error: Task Failed", error_msgbox_dummy_cb);
                } else {
                    Serial.println("Calibration task created successfully");
                }
            }
        }
    }
}

// Remove the duplicate declaration - use the one from calib_adc.h
void build_adc_calibration_window(lv_obj_t **win_holder,
                                   const char *title,
                                   AdcCalibrationControls &gui,
                                   const CalPrefill &pf)
{
    if (*win_holder)
    {
        lv_spinbox_set_value(gui.code_1, pf.code1);
        lv_spinbox_set_value(gui.code_2, pf.code2);
        lv_spinbox_set_value(gui.vin_1, (int32_t)llround(10000.0 * pf.val1));
        lv_spinbox_set_value(gui.vin_2, (int32_t)llround(10000.0 * pf.val2));

        lv_obj_clear_flag(*win_holder, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_t *win = lv_win_create(lv_scr_act(), 32);
    *win_holder = win;
    lv_obj_set_size(win, 320, 226);
    lv_win_add_title(win, title); // LVGL 8 win API.

    lv_obj_t *btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 60);
    lv_obj_add_event_cb(btn, btn_close_hide_obj_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *cont = lv_win_get_content(win);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);

    static lv_style_t style_lbl, style_val;
    static bool styles_inited{};
    if (!styles_inited)
    {
        lv_style_init(&style_lbl);
        lv_style_set_text_font(&style_lbl, &lv_font_montserrat_10);
        lv_style_init(&style_val);
        lv_style_set_text_font(&style_val, &graph_R_16);
        lv_style_set_text_color(&style_val, lv_color_hex(0xFFFFFF));
        styles_inited = true;
    }

    lv_point_t offset{10, 20};
    lv_coord_t w = 110, col = w + 30, row = 38;
    lv_point_t pad{5, 5};

    gui.vin_1 = spinbox_pro(cont, "#FFFFF7 Vin1:#", -10000, 400000, 6, 2, LV_ALIGN_TOP_LEFT, offset.x, offset.y, w, 10, &graph_R_16);
    gui.code_1 = spinbox_pro(cont, "#FFFFF7 Code1:#", -10000, 8388608, 7, 0, LV_ALIGN_TOP_LEFT, offset.x, offset.y + row, w, 11, &graph_R_16);
    gui.vin_2 = spinbox_pro(cont, "#FFFFF7 Vin2:#", -10000, 400000, 6, 2, LV_ALIGN_TOP_LEFT, offset.x + col, offset.y, w, 12, &graph_R_16);
    gui.code_2 = spinbox_pro(cont, "#FFFFF7 Code2:#", -10000, 8388608, 7, 0, LV_ALIGN_TOP_LEFT, offset.x + col, offset.y + row, w, 13, &graph_R_16);

    lv_spinbox_set_value(gui.code_1, pf.code1);
    lv_spinbox_set_value(gui.code_2, pf.code2);
    lv_spinbox_set_value(gui.vin_1, (int32_t)llround(10000.0 * pf.val1));
    lv_spinbox_set_value(gui.vin_2, (int32_t)llround(10000.0 * pf.val2));

    auto *lbl_raw = LVLabel::create(cont, "Raw Code:", gui.code_1, 0, pad.y, &style_lbl);
    gui.lbl_rawCode = LVLabel::create(cont, "#FFFFF7 00000000#", lbl_raw, 0, pad.y - 2, &style_val);

    auto *lbl_rawA = LVLabel::create(cont, "Avg Raw:", gui.lbl_rawCode, 0, pad.y, &style_lbl);
    gui.lbl_rawAVG_ = LVLabel::create(cont, "+00000000", lbl_rawA, 0, pad.y - 2, &style_val);

    auto *lbl_calV = LVLabel::create(cont, "Calibrated Value:", gui.code_2, 0, pad.y, &style_lbl);
    gui.lbl_calibratedValue = LVLabel::create(cont, (pf.unit && pf.unit[0] == 'A') ? "0.0000A" : "0.0000V",
                                              lbl_calV, 0, pad.y - 2, &style_val);

    auto *lbl_cvA = LVLabel::create(cont, "Avg Calibrated Value:", gui.lbl_calibratedValue, 0, pad.y, &style_lbl);
    gui.lbl_calibValueAVG_ = LVLabel::create(cont, "+00.0000", lbl_cvA, 0, pad.y - 2, &style_val);

    auto *lbl_er = LVLabel::create(cont, "Effective Resolution:", gui.lbl_rawAVG_, 0, pad.y, &style_lbl);
    gui.lbl_ER = LVLabel::create(cont, "00.00", lbl_er, 0, pad.y - 2, &style_val);

    lv_point_t btn_pos{140 + offset.x, 160};
    LVButton btnSave(cont, "Save", btn_pos.x + 62, btn_pos.y, 54, 26, nullptr, save_cb);
    LVButton btnLoad(cont, "Load", btn_pos.x, btn_pos.y, 54, 26, nullptr, load_cb);

    LVButton btnAutoZeros(cont, "Auto zeros", btn_pos.x + 5, btn_pos.y + 31, 2 * 54, 26, nullptr, autoZeroCurrent_cb);

    // m, b, and Vin_cal notes (simple labels)
    auto *label_m = lv_label_create(cont);
    auto *label_m_num = lv_label_create(cont);
    auto *label_m_den = lv_label_create(cont);
    auto *label_b = lv_label_create(cont);
    auto *label_vin = lv_label_create(cont);

    lv_label_set_text(label_m_num, "Code2 - Code1");
    lv_label_set_text(label_m, "m = --------------------- =");
    lv_label_set_text(label_m_den, "Vin2 - Vin1");

    int verPad = 160 + 50;
    lv_obj_align(label_m_num, LV_ALIGN_TOP_LEFT, 35, verPad + 10);
    lv_obj_align(label_m, LV_ALIGN_TOP_LEFT, 0, verPad + 20);
    lv_obj_align(label_m_den, LV_ALIGN_TOP_LEFT, 50, verPad + 30);

    gui.lbl_calib_m = lv_label_create(cont);
    gui.lbl_calib_b = lv_label_create(cont);
    lv_obj_align(gui.lbl_calib_m, LV_ALIGN_TOP_LEFT, 160, verPad + 20);

    lv_label_set_text(label_b, "b = Code1 - m * Vin1 =");
    lv_obj_align(label_b, LV_ALIGN_TOP_LEFT, 0, verPad + 60);
    lv_obj_align(gui.lbl_calib_b, LV_ALIGN_TOP_LEFT, 160, verPad + 60);

    lv_label_set_text(label_vin, "Vin_cal = (Code-b)/m");
    lv_obj_align(label_vin, LV_ALIGN_TOP_LEFT, 0, verPad + 90);
}

// Button callbacks
void btn_calibration_ADC_voltage_event_cb(lv_event_t *)
{
    auto &cal = PowerSupply.CalBank[PowerSupply.bankCalibId].vCal;
    CalPrefill pf{cal.code_1, cal.code_2, cal.value_1, cal.value_2, "V"};
    build_adc_calibration_window(&PowerSupply.gui.calibration.win_ADC_voltage_calibration, "ADC Voltage Calibration", Calib_GUI.Voltage, pf);
}

void btn_calibration_ADC_current_event_cb(lv_event_t *)
{
    auto &cal = PowerSupply.CalBank[PowerSupply.bankCalibId].iCal;
    CalPrefill pf{cal[PowerSupply.mA_Active].code_1, cal[PowerSupply.mA_Active].code_2, cal[PowerSupply.mA_Active].value_1, cal[PowerSupply.mA_Active].value_2, "A"};
    build_adc_calibration_window(&PowerSupply.gui.calibration.win_ADC_current_calibration, "ADC Current Calibration [A]", Calib_GUI.Current, pf);
}