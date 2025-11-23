#include "input_handler.h"
#include "device.hpp"
#include "globals.h"
#include "tabs.h"
#include "ui_helpers.h"
#include "spinbox_pro.h"
#include "table_pro.h"
#include "input_device.h"
#include "functions.h"
#include "power_management.h"
#include <algorithm>
#include "globalFunctions.h"
#include "waveform_generator.h"

// External references
extern TFT_eSPI tft;
extern Device PowerSupply;
extern lv_obj_t *slider_x;
extern Waveform waveforms[];
extern int numWaveforms;

// Touch attribute structure
struct TouchAttr_
{
    uint16_t x = 0, y = 0; // To store the touch coordinates
    bool pressed;

    bool getTouched(TFT_eSPI t)
    {
        return (t.getTouch(&x, &y));
    }
} TouchAttr;

// Helper function to show/hide Duty spinbox based on selected waveform
void updateDutySpinboxState()
{
    if (!Utility_objs.table_fun_gen_list || !Utility_objs.fun.Duty)
        return;

    // Get the selected row from the waveform table
    uint16_t selected_row = (uint16_t)(uintptr_t)lv_obj_get_user_data(Utility_objs.table_fun_gen_list);

    // Check if the selected waveform is PWM (index 4 in waveforms array)
    // Waveform list: 0=Sine, 1=Sawtooth, 2=Square, 3=Triangular, 4=PWM, ...
    bool isPWM = (selected_row == 4);

    if (isPWM)
    {
        // Show the Duty spinbox for PWM waveform
        lv_obj_clear_flag(Utility_objs.fun.Duty, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        // Hide the Duty spinbox for non-PWM waveforms
        lv_obj_add_flag(Utility_objs.fun.Duty, LV_OBJ_FLAG_HIDDEN);
    }
}

void touch_calibrate()
{
    uint16_t calData[5];
    uint8_t calDataOK = 0;

    // Calibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    Serial.println();
    Serial.println();
    Serial.println("// Use this calibration code in setup():");
    Serial.print("  uint16_t calData[5] = ");
    Serial.print("{ ");

    for (uint8_t i = 0; i < 5; i++)
    {
        Serial.print(calData[i]);
        if (i < 4)
            Serial.print(", ");
    }

    Serial.println(" };");
    Serial.print("  tft.setTouch(calData);");
    Serial.println();
    Serial.println();

    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");
    tft.println("Calibration code sent to Serial port.");

    delay(4000);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    uint16_t x, y;
    bool touched = tft.getTouch(&x, &y);

    if (TouchAttr.getTouched(tft))
    {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = TouchAttr.x;
        data->point.y = TouchAttr.y;

        if (touched)
        {
            if (dropdown_active)
                return; // Ignore touch when dropdown is open
            lv_obj_clear_flag(lv_obj_get_parent(PowerSupply.page[3]), LV_OBJ_FLAG_SCROLLABLE);
            plotToBucket(x, y, util_Arbit_chart, util_Arbit_chart_series);
        }
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
        lv_obj_add_flag(lv_obj_get_parent(util_Arbit_chart), LV_OBJ_FLAG_SCROLLABLE); // Re-enable scrolling
    }
}

void init_touch()
{
    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);
}

// ==================== Page Handler Functions ====================
// Moved from globalFunctions.h

// Encoder baselines and private menu index
static int32_t g_last_enc1_dac = 0, g_last_enc2_dac = 0;
static int32_t g_last_enc1_adc = 0, g_last_enc2_adc = 0;
static int32_t g_last_enc1_menu = 0, g_last_enc2_menu = 0;
static int g_menu_index = 0;             // replaces old global lastButton
static lv_obj_t *g_last_focus = nullptr; // to avoid "jump back" when focus changes

// ----- small helpers -----
enum class UIMode
{
    Menu,
    DAC,
    ADC
};

static inline lv_obj_t *current_sidebar_list()
{
    lv_obj_t *page = lv_menu_get_cur_sidebar_page(PowerSupply.gui.setting_menu);
    return page ? lv_obj_get_child(page, 0) : nullptr; // first child is the list container in LVGL menu
}

static inline UIMode detect_mode()
{
    const bool dac =
        PowerSupply.gui.calibration.win_DAC_calibration &&
        lv_obj_is_visible(PowerSupply.gui.calibration.win_DAC_calibration);

    const bool adc =
        (PowerSupply.gui.calibration.win_ADC_voltage_calibration && lv_obj_is_visible(PowerSupply.gui.calibration.win_ADC_voltage_calibration)) ||
        (PowerSupply.gui.calibration.win_ADC_current_calibration && lv_obj_is_visible(PowerSupply.gui.calibration.win_ADC_current_calibration)) ||
        (PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration && lv_obj_is_visible(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration)) ||
        (PowerSupply.gui.calibration.win_int_current_calibration && lv_obj_is_visible(PowerSupply.gui.calibration.win_int_current_calibration)); // NEW

    return (!dac && !adc) ? UIMode::Menu : (dac ? UIMode::DAC : UIMode::ADC);
}

// ----- handlers (no binding, pure polling) -----

// Helper: Navigate sidebar menu and auto-scroll sidebar
static void navigate_sidebar_menu(int dir)
{
    lv_obj_t *list = current_sidebar_list();
    if (!list)
        return;

    const int count = lv_obj_get_child_cnt(list);
    g_menu_index = std::clamp(g_menu_index + dir, 0, std::max(0, count - 1));

    // "Activate" the target row (this selects the menu item)
    lv_obj_t *selected_item = lv_obj_get_child(list, g_menu_index);
    lv_event_send(selected_item, LV_EVENT_CLICKED, nullptr);

    // Auto-scroll sidebar if the selected item is outside the visible area
    lv_obj_t *sidebar_page = lv_menu_get_cur_sidebar_page(PowerSupply.gui.setting_menu);
    if (sidebar_page && selected_item)
    {
        lv_area_t item_area;
        lv_obj_get_coords(selected_item, &item_area);

        lv_area_t page_area;
        lv_obj_get_coords(sidebar_page, &page_area);

        // Check if item is below visible area
        if (item_area.y2 > page_area.y2)
        {
            lv_coord_t scroll_amount = -(item_area.y2 - page_area.y2 + 5);
            lv_obj_scroll_by(sidebar_page, 0, scroll_amount, LV_ANIM_ON);
        }
        // Check if item is above visible area
        else if (item_area.y1 < page_area.y1)
        {
            lv_coord_t scroll_amount = page_area.y1 - item_area.y1 + 5;
            lv_obj_scroll_by(sidebar_page, 0, scroll_amount, LV_ANIM_ON);
        }
    }
}

// Helper: Scroll content page
static void scroll_content_page(int dir)
{
    lv_obj_t *main_page = lv_menu_get_cur_main_page(PowerSupply.gui.setting_menu);
    if (main_page)
    {
        lv_coord_t scroll_amount = dir * 30;  // Scroll 30 pixels per encoder step
        lv_obj_scroll_by(main_page, 0, scroll_amount, LV_ANIM_ON);
    }
}

static void handle_menu_mode()
{
    // Handle encoder2 (current encoder) for sidebar navigation with auto-scroll
    if (encoder2_value != g_last_enc2_menu)
    {
        const int dir = (encoder2_value > g_last_enc2_menu) ? +1 : -1;
        navigate_sidebar_menu(dir);  // Navigates AND auto-scrolls sidebar when item goes off-screen
        g_last_enc2_menu = encoder2_value;
    }

    // Handle encoder1 (voltage encoder) for content page scrolling only
    if (encoder1_value != g_last_enc1_menu)
    {
        const int dir = (encoder1_value > g_last_enc1_menu) ? -1 : +1;  // Inverted for natural scroll
        scroll_content_page(dir);  // Scrolls the right side content
        g_last_enc1_menu = encoder1_value;
    }
}

static void handle_dac_mode()
{
    if (!get_selected_spinbox())
        return;

    // Resync baseline when focus changes (e.g., touch switched spinbox)
    if (g_last_focus != get_selected_spinbox())
    {
        g_last_focus = get_selected_spinbox();
        g_last_enc2_dac = encoder2_value;
    }

    // Encoder 2 selects digit
    if (encoder2_value != g_last_enc2_dac)
    {
        (encoder2_value > g_last_enc2_dac) ? move_left(get_selected_spinbox())
                                           : move_right(get_selected_spinbox());
        g_last_enc2_dac = encoder2_value;
    }

    // Encoder 1 changes value
    if (encoder1_value != g_last_enc1_dac)
    {
        (encoder1_value > g_last_enc1_dac) ? increment(get_selected_spinbox())
                                           : decrement(get_selected_spinbox());
        g_last_enc1_dac = encoder1_value;
    }
}

static void handle_adc_mode()
{
    if (!get_selected_spinbox())
        return;

    if (g_last_focus != get_selected_spinbox())
    {
        g_last_focus = get_selected_spinbox();
        g_last_enc2_adc = encoder2_value;
    }

    // Encoder 2 moves cursor/digit
    if (encoder2_value != g_last_enc2_adc)
    {
        (encoder2_value > g_last_enc2_adc) ? move_left(get_selected_spinbox())
                                           : move_right(get_selected_spinbox());
        g_last_enc2_adc = encoder2_value;
        return; // only moved cursor this tick
    }

    // Encoder 1 changes value
    if (encoder1_value != g_last_enc1_adc)
    {
        (encoder1_value > g_last_enc1_adc) ? increment(get_selected_spinbox())
                                           : decrement(get_selected_spinbox());
        g_last_enc1_adc = encoder1_value;
    }
}

// ----- single public entry you call repeatedly -----
void handleCalibrationPage(int32_t /*unused1*/, int32_t /*unused2*/)
{
    switch (detect_mode())
    {
    case UIMode::Menu:
        handle_menu_mode();
        break;
    case UIMode::DAC:
        handle_dac_mode();
        break;
    case UIMode::ADC:
        handle_adc_mode();
        break;
    }
}

void handleGraphPage(int32_t &encoder1_last_value, int32_t &encoder2_last_value)
{
    // Handle horizontal scrolling and zooming with encoder 2
    if (encoder2_last_value != encoder2_value)
    {
        int32_t _posX = lv_slider_get_value(slider_x);

        if (encoder2_last_value < encoder2_value)
            _posX += 160;
        else if (encoder2_last_value > encoder2_value)
            _posX -= 160;

        if (keyChar == 'W' && msg == " HOLD.")
        {
            // Scroll the graph horizontally
            _posX = std::clamp(_posX, 0, 57 * (lv_chart_get_zoom_x(PowerSupply.graph.chart) - 256) / 16);
            if (!lvglChartIsBusy)
            {
                lvglChartIsBusy = true;
                lv_obj_scroll_to_x(PowerSupply.graph.chart, _posX, LV_ANIM_OFF);
                lvglChartIsBusy = false;
            }
        }
        else
        {
            // Zoom the graph horizontally
            _posX = std::clamp(_posX, 256, LV_IMG_ZOOM_NONE * 120);
            lv_slider_set_value(slider_x, _posX, LV_ANIM_OFF);

            if (!lvglChartIsBusy)
            {
                lvglChartIsBusy = true;
                lv_event_send(lv_obj_get_child(PowerSupply.page[1], 1), LV_EVENT_VALUE_CHANGED, NULL);
                lvglChartIsBusy = false;
            }
        }
    }

    // Handle vertical scrolling and zooming with encoder 1
    if (encoder1_last_value != encoder1_value)
    {
        int32_t _posY = lv_slider_get_value(lv_obj_get_child(PowerSupply.page[1], 2));

        if (encoder1_last_value < encoder1_value)
            _posY += 64;
        else if (encoder1_last_value > encoder1_value)
            _posY -= 64;
        // encoder1_last_value = encoder1_value;

        if (keyChar == 'W' && msg == " HOLD.")
        {
            // Scroll the graph vertically
            _posY = std::clamp(_posY, 0, int(33.5 * (lv_chart_get_zoom_y(PowerSupply.graph.chart) - 256) / 64));
            if (!lvglChartIsBusy)
            {
                lvglChartIsBusy = true;
                lv_obj_scroll_to_y(PowerSupply.graph.chart, _posY, LV_ANIM_OFF);
                lvglChartIsBusy = false;
            }
        }
        else
        {
            // Zoom the graph vertically
            _posY = std::clamp(_posY, 256, 5120);
            lv_slider_set_value(lv_obj_get_child(PowerSupply.page[1], 2), _posY, LV_ANIM_OFF);

            if (!lvglChartIsBusy)
            {
                lvglChartIsBusy = true;
                lv_event_send(lv_obj_get_child(PowerSupply.page[1], 2), LV_EVENT_VALUE_CHANGED, NULL);

                autoScrollY();

                lvglChartIsBusy = false;
            }
        }
    }
}

void handleHistogramPage(int32_t &encoder1_last_value, int32_t &encoder2_last_value)
{
    bool histogramUpdated = false; // Flag to indicate if histogram needs to be reset

    // **Handle Vertical Shift/Zoom with Encoder 1**
    if (encoder1_last_value != encoder1_value)
    {
        int32_t _posY = 0;

        // Determine the direction of encoder 1 rotation
        if (encoder1_last_value < encoder1_value)
            _posY = 1; // Rotated clockwise
        else if (encoder1_last_value > encoder1_value)
            _posY = -1; // Rotated counter-clockwise

        encoder1_last_value = encoder1_value;
        if (keyChar == 'W' && msg == " HOLD.")
        {
            // **Shift the histogram window up or down**
            double shiftAmountVoltage = 0.025; // Adjust as needed
            double shiftAmountCurrent = 0.025; // Adjust as needed

            if (_posY > 0)
            {
                // Shift up
                PowerSupply.Voltage.hist.histWinMin += shiftAmountVoltage;
                PowerSupply.Voltage.hist.histWinMax += shiftAmountVoltage;

                PowerSupply.Current.hist.histWinMin += shiftAmountCurrent;
                PowerSupply.Current.hist.histWinMax += shiftAmountCurrent;
            }
            else if (_posY < 0)
            {
                // Shift down
                PowerSupply.Voltage.hist.histWinMin -= shiftAmountVoltage;
                PowerSupply.Voltage.hist.histWinMax -= shiftAmountVoltage;

                PowerSupply.Current.hist.histWinMin -= shiftAmountCurrent;
                PowerSupply.Current.hist.histWinMax -= shiftAmountCurrent;
            }
        }
        else
        {
            // **Zoom the histogram window vertically centered around the middle value**
            if (_posY > 0)
            {
                // Zoom in (reduce range)
                if (PowerSupply.stats.serV->hidden == 0)
                {
                    double midValueVoltage = (PowerSupply.Voltage.hist.histWinMin + PowerSupply.Voltage.hist.histWinMax) / 2.0;
                    double rangeVoltage = (PowerSupply.Voltage.hist.histWinMax - PowerSupply.Voltage.hist.histWinMin) / 2.0;
                    double newRangeVoltage = rangeVoltage * 0.5; // Zoom in by reducing the range by half
                    if (newRangeVoltage < .00004)
                        return;
                    PowerSupply.Voltage.hist.histWinMin = midValueVoltage - newRangeVoltage;
                    PowerSupply.Voltage.hist.histWinMax = midValueVoltage + newRangeVoltage;
                }
                if (PowerSupply.stats.serI->hidden == 0)
                {
                    double midValueCurrent = (PowerSupply.Current.hist.histWinMin + PowerSupply.Current.hist.histWinMax) / 2.0;
                    double rangeCurrent = (PowerSupply.Current.hist.histWinMax - PowerSupply.Current.hist.histWinMin) / 2.0;
                    double newRangeCurrent = rangeCurrent * 0.5;
                    if (newRangeCurrent < .00001)
                        return;
                    PowerSupply.Current.hist.histWinMin = midValueCurrent - newRangeCurrent;
                    PowerSupply.Current.hist.histWinMax = midValueCurrent + newRangeCurrent;
                }
            }
            else if (_posY < 0)
            {
                // Zoom out (increase range)
                if (PowerSupply.stats.serV->hidden == 0)
                {
                    double midValueVoltage = (PowerSupply.Voltage.hist.histWinMin + PowerSupply.Voltage.hist.histWinMax) / 2.0;
                    double rangeVoltage = (PowerSupply.Voltage.hist.histWinMax - PowerSupply.Voltage.hist.histWinMin) / 2.0;
                    double newRangeVoltage = rangeVoltage * 2.0; // Zoom out by doubling the range
                    if (newRangeVoltage > 50)
                        return;
                    PowerSupply.Voltage.hist.histWinMin = midValueVoltage - newRangeVoltage;
                    PowerSupply.Voltage.hist.histWinMax = midValueVoltage + newRangeVoltage;
                }
                if (PowerSupply.stats.serI->hidden == 0)
                {
                    double midValueCurrent = (PowerSupply.Current.hist.histWinMin + PowerSupply.Current.hist.histWinMax) / 2.0;
                    double rangeCurrent = (PowerSupply.Current.hist.histWinMax - PowerSupply.Current.hist.histWinMin) / 2.0;
                    double newRangeCurrent = rangeCurrent * 2.0;
                    if (newRangeCurrent > 25)
                        return;
                    PowerSupply.Current.hist.histWinMin = midValueCurrent - newRangeCurrent;
                    PowerSupply.Current.hist.histWinMax = midValueCurrent + newRangeCurrent;
                }
            }
        }

        encoder1_last_value = encoder1_value;
        histogramUpdated = true; // Mark histogram as updated
    }

    // **Handle Horizontal Shift with Encoder 2**
    if (encoder2_last_value != encoder2_value)
    {
        int32_t _posX = 0;

        // Determine the direction of encoder 2 rotation
        if (encoder2_last_value < encoder2_value)
            _posX = 1; // Rotated clockwise
        else if (encoder2_last_value > encoder2_value)
            _posX = -1; // Rotated counter-clockwise
        encoder2_last_value = encoder2_value;
        // Shift the histogram window left or right by 10% of the window size
        double windowSizeVoltage = PowerSupply.Voltage.hist.histWinMax - PowerSupply.Voltage.hist.histWinMin;
        double shiftAmountVoltage = 0.10 * windowSizeVoltage; // 10% of voltage window size

        double windowSizeCurrent = PowerSupply.Current.hist.histWinMax - PowerSupply.Current.hist.histWinMin;
        double shiftAmountCurrent = 0.10 * windowSizeCurrent; // 10% of current window size

        if (_posX > 0)
        {
            // Shift right
            if (PowerSupply.stats.serV->hidden == 0)
            {
                if (PowerSupply.Voltage.hist.histWinMin > 50)
                    return;
                PowerSupply.Voltage.hist.histWinMin += shiftAmountVoltage;
                PowerSupply.Voltage.hist.histWinMax += shiftAmountVoltage;
            }
            if (PowerSupply.stats.serI->hidden == 0)
            {
                if (PowerSupply.Current.hist.histWinMin > 20)
                    return;

                PowerSupply.Current.hist.histWinMin += shiftAmountCurrent;
                PowerSupply.Current.hist.histWinMax += shiftAmountCurrent;
            }
        }
        else if (_posX < 0)
        {
            // Shift left
            if (PowerSupply.stats.serV->hidden == 0)
            {
                if (PowerSupply.Voltage.hist.histWinMin < -50)
                    return;
                PowerSupply.Voltage.hist.histWinMin -= shiftAmountVoltage;
                PowerSupply.Voltage.hist.histWinMax -= shiftAmountVoltage;
            }
            if (PowerSupply.stats.serI->hidden == 0)
            {
                if (PowerSupply.Current.hist.histWinMin < -20)
                    return;
                PowerSupply.Current.hist.histWinMin -= shiftAmountCurrent;
                PowerSupply.Current.hist.histWinMax -= shiftAmountCurrent;
            }
        }

        histogramUpdated = true; // Mark histogram as updated
    }

    // **Reset Histograms if Updated**
    if (histogramUpdated)
    {
        // Reset histograms to apply new window settings
        if (PowerSupply.stats.serV->hidden == 0)
            PowerSupply.Voltage.hist.Reset();
        if (PowerSupply.stats.serI->hidden == 0)
            PowerSupply.Current.hist.Reset();
    }
}

void handleUtilityPage(int32_t encoder1_last_value, int32_t encoder2_last_value)
{
    // **Handle Vertical Shift/Zoom with Encoder 1**
    if (encoder1_last_value != encoder1_value)
    {
        static u8_t _posY = 0;

        // Determine the direction of encoder 1 rotation
        if (encoder1_last_value < encoder1_value)
            _posY++; // Rotated clockwise
        else if (encoder1_last_value > encoder1_value)
            _posY--; // Rotated counter-clockwise

        static lv_obj_t *table = lv_obj_get_child(PowerSupply.page[3], 0);
        lv_tabview_set_act(table, _posY % 4, LV_ANIM_ON);
    }

    if (encoder2_last_value != encoder2_value)
    {
        static u8_t _posX = 0;
        // Determine the direction of encoder 2 rotation
        if (encoder2_last_value < encoder2_value)
        {
            if (lv_tabview_get_tab_act(lv_obj_get_child(PowerSupply.page[3], 0)) == 1)
            {
                select_previous_row(Utility_objs.table_fun_gen_list, 25);
                updateDutySpinboxState(); // Update duty state when selection changes
            }
            else
                select_previous_row(Utility_objs.table_point_list, 2 * 7 + 2 * 5);
        }
        else if (encoder2_last_value > encoder2_value)
        {
            if (lv_tabview_get_tab_act(lv_obj_get_child(PowerSupply.page[3], 0)) == 1)
            {
                select_next_row(Utility_objs.table_fun_gen_list, 25);
                updateDutySpinboxState(); // Update duty state when selection changes
            }
            else
                select_next_row(Utility_objs.table_point_list, 2 * 7 + 2 * 5);
        }
    }
}

void handleUtility_function_Page(int32_t encoder1_last_value, int32_t encoder2_last_value)
{
    if (!get_selected_spinbox())
    {

        handleUtilityPage(encoder1_last_value, encoder2_last_value);
    }
    else
    {
        static int32_t cursor_pos = 0;

        // Check if encoder values have changed
        if (encoder2_last_value == encoder2_value && encoder1_last_value == encoder1_value)
            return;

        // Update cursor position based on encoder 2
        if (encoder2_last_value < encoder2_value)
        {
            move_left(get_selected_spinbox());
            encoder2_last_value = encoder2_value;
            return;
        }

        else if (encoder2_last_value > encoder2_value)
        {
            move_right(get_selected_spinbox());
            encoder2_last_value = encoder2_value;
            return;
        }

        if (encoder1_last_value < encoder1_value)
            increment(get_selected_spinbox());

        else if (encoder1_last_value > encoder1_value)
            decrement(get_selected_spinbox());

        encoder1_last_value = encoder1_value;

        PowerSupply.funGenMem.amplitude = lv_spinbox_get_value(Utility_objs.fun.Amplitude) / 1000.0;
        PowerSupply.funGenMem.frequency = lv_spinbox_get_value(Utility_objs.fun.Frequency) / 1000.0;
        PowerSupply.funGenMem.offset = lv_spinbox_get_value(Utility_objs.fun.Offset) / 1000.0;
        PowerSupply.funGenMem.dutyCycle = lv_spinbox_get_value(Utility_objs.fun.Duty) / 10000.0;

        if (lv_tabview_get_tab_act(lv_obj_get_child(PowerSupply.page[3], 0)) == 1)
            PowerSupply.SaveMemoryFgen("FunGen", PowerSupply.funGenMem);
    }
}

void getSettingEncoder(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    if (encoder2Flag)
    {
        int64_t count = PowerSupply.Current.encoder.getCount() >> 1;

        static int64_t rotaryOldValue2 = 0;

        if (count > rotaryOldValue2)
            encoder2_value++;
        else if (count < rotaryOldValue2)
            encoder2_value--;

        rotaryOldValue2 = count;
        encoder2Flag = 0;
        switch (Tabs::getCurrentPage())
        {
        case 2:
            PowerSupply.Current.SetEncoderUpdate();
            break;
        }
        encoderTimeStamp = millis();
    }

    if (encoder1Flag)
    {
        int64_t count = PowerSupply.Voltage.encoder.getCount() >> 1;

        static int64_t rotaryOldValue = 0;

        if (count > rotaryOldValue)
            encoder1_value++;
        else if (count < rotaryOldValue)
            encoder1_value--;

        rotaryOldValue = count;
        encoder1Flag = 0;
        switch (Tabs::getCurrentPage())
        {
        case 2:
            PowerSupply.Voltage.SetEncoderUpdate();
            break;
        }
        encoderTimeStamp = millis();
    }
}

void managePageEncoderInteraction()
{
    // Variables to store the last encoder values
    static int32_t encoder1_last_value = 0;
    static int32_t encoder2_last_value = 0;

    // Get the current page from the Tabs
    switch (Tabs::getCurrentPage())
    {
    case 0: // Histogram Page
        handleHistogramPage(encoder1_last_value, encoder2_last_value);
        break;

    case 1: // Graph Page
        handleGraphPage(encoder1_last_value, encoder2_last_value);
        break;

    case 2: // main Page
        break;
    case 3:
        handleUtility_function_Page(encoder1_last_value, encoder2_last_value);
        break;

    case 4: // Calibration Page
        handleCalibrationPage(encoder1_last_value, encoder2_last_value);
        break;

    default:
        break;
    }

    if (encoder1_last_value != encoder1_value)
        encoder1_last_value = encoder1_value;
    if (encoder2_last_value != encoder2_value)
        encoder2_last_value = encoder2_value;
}

// ==================== Keyboard Input Handler ====================
// Moved from globalFunctions.h

void keyCheckLoop()
{
    getKeys();
    if (msg == " IDLE.")
        return;

    keyMenus('O', " RELEASED.", []
             {
                blockAll = true;
                PowerSupply.toggle();
                blockAll = false;
             });

    keyMenus('O', " HOLD.", [] // Output button
             {
             });

    keyMenus('m', " HOLD.", [] // Output button
             {
                myTone(NOTE_A5, 200, true);
                myTone(NOTE_A3, 200, true);
                PowerSupply.SaveSetting();
                return; });

    // Previous Page
    keyMenus('l', " RELEASED.",
             [&]
             {
                 if (!lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN))
                     return;
                 Tabs::previousPage();
             });

    // Next page
    keyMenus('k', " RELEASED.",
             [&]
             {
                 if (!lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN))
                     return;
                 Tabs::nextPage();
             });

    keyMenus('H', " RELEASED.", []
             {
                 Tabs::goToHomeTab();
                 hide(PowerSupply.gui.calibration.win_ADC_voltage_calibration);
                 hide(PowerSupply.gui.calibration.win_ADC_current_calibration);
                 hide(PowerSupply.gui.calibration.win_int_current_calibration);
                 hide(PowerSupply.gui.calibration.win_ADC_INL_Voltage_calibration);
                 hide(PowerSupply.gui.calibration.win_DAC_calibration);
                 lv_obj_invalidate(lv_scr_act());
             });

    keyMenus('H', " HOLD.", [] // Home button
             {
                 myTone(NOTE_A3, 200, true);
                 myTone(NOTE_A5, 200, true);
                 ESP.restart(); });

    keyMenus('M', " RELEASED.", []
             { Tabs::setCurrentPage(4); });

    keyMenus('m', " RELEASED.", []
             {
                 if (!lv_obj_has_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN))
                     return;
                 Tabs::setCurrentPage(3);
             });

    // Long press T - REL mode: zero current display (like Keithley 2010 REL)
    // Must be before RELEASED handlers to set the flag
    static bool T_hold_triggered = false;
    keyMenus('T', " HOLD.", [&]
             {
                 T_hold_triggered = true;
                 myTone(NOTE_A4, 50);
                 if (PowerSupply.mA_Active) {
                     // Toggle REL for mA range
                     if (PowerSupply.currentRelActive_mA) {
                         PowerSupply.currentRelActive_mA = false;
                         PowerSupply.currentRelOffset_mA = 0.0;
                     } else {
                         PowerSupply.currentRelOffset_mA = PowerSupply.Current.measured.value;
                         PowerSupply.currentRelActive_mA = true;
                     }
                 } else {
                     // Toggle REL for A range
                     if (PowerSupply.currentRelActive_A) {
                         PowerSupply.currentRelActive_A = false;
                         PowerSupply.currentRelOffset_A = 0.0;
                     } else {
                         PowerSupply.currentRelOffset_A = PowerSupply.Current.measured.value;
                         PowerSupply.currentRelActive_A = true;
                     }
                 }
                 // Update REL label visibility
                 bool relActive = PowerSupply.mA_Active ? PowerSupply.currentRelActive_mA : PowerSupply.currentRelActive_A;
                 if (PowerSupply.gui.label_current_rel) {
                     if (relActive)
                         lv_obj_clear_flag(PowerSupply.gui.label_current_rel, LV_OBJ_FLAG_HIDDEN);
                     else
                         lv_obj_add_flag(PowerSupply.gui.label_current_rel, LV_OBJ_FLAG_HIDDEN);
                 }
             });

    keyMenusPage('T', " RELEASED.", 2, [&]
                 {
                     if (T_hold_triggered) { T_hold_triggered = false; return; }
                     myTone(NOTE_A4, 10);
                     PowerSupply.toggle_measure_unit();

                     blockAll = true;
                     btn_calibration_ADC_current_event_cb(nullptr); // Refresh the calibration window if open
                     lv_obj_add_flag(PowerSupply.gui.calibration.win_ADC_current_calibration, LV_OBJ_FLAG_HIDDEN);
                     PowerSupply.calibrationUpdate();
                     PowerSupply.Current.displayUpdate(true);
                     // Update REL label for new range
                     bool relActive = PowerSupply.mA_Active ? PowerSupply.currentRelActive_mA : PowerSupply.currentRelActive_A;
                     if (PowerSupply.gui.label_current_rel) {
                         if (relActive) lv_obj_clear_flag(PowerSupply.gui.label_current_rel, LV_OBJ_FLAG_HIDDEN);
                         else lv_obj_add_flag(PowerSupply.gui.label_current_rel, LV_OBJ_FLAG_HIDDEN);
                     }
                     blockAll = false; });

    keyMenusPage('T', " RELEASED.", 4, [&]
                 {
                     if (T_hold_triggered) { T_hold_triggered = false; return; }
                     myTone(NOTE_A4, 10, false);
                     PowerSupply.toggle_measure_unit();

                     blockAll = true;
                     btn_calibration_ADC_current_event_cb(nullptr); // Refresh the calibration window if open
                     lv_obj_invalidate(PowerSupply.gui.calibration.win_ADC_current_calibration);
                     blockAll = false; });

    keyMenusPage('-', " RELEASED.", 0, [&]
                 {
                     int32_t newEncoder1Value = encoder1_value +1;
                     int32_t newEncoder2Value = encoder2_value;
                     handleHistogramPage(newEncoder1Value, newEncoder2Value);

                     PowerSupply.Current.hist.Reset();
                     PowerSupply.Voltage.hist.Reset(); });

    keyMenusPage('+', " RELEASED.", 0, [&]
                 {
                     int32_t newEncoder1Value = encoder1_value - 1;
                     int32_t newEncoder2Value = encoder2_value;
                     handleHistogramPage(newEncoder1Value, newEncoder2Value);

                     PowerSupply.Current.hist.Reset();
                     PowerSupply.Voltage.hist.Reset(); });


    keyMenusPage('V', " RELEASED.", 0, []
                 {
                     static bool hide = false;
                     hide = !hide;
                     lv_chart_hide_series(PowerSupply.stats.chart, PowerSupply.stats.serV, hide);
                     lv_chart_hide_series(PowerSupply.graph.chart, PowerSupply.graph.serV, hide);

                     if (!hide)
                         lv_obj_clear_flag(label_legend1, LV_OBJ_FLAG_HIDDEN);

                     else
                         lv_obj_add_flag(label_legend1, LV_OBJ_FLAG_HIDDEN); });

    keyMenusPage('A', " RELEASED.", 0, []
                 {
                     static bool hide = false;
                     hide = !hide;
                     lv_chart_hide_series( PowerSupply.stats.chart,  PowerSupply.stats.serI,hide);
                     lv_chart_hide_series(PowerSupply.graph.chart, PowerSupply.graph.serI, hide);

                     if (!hide)
                         lv_obj_clear_flag(label_legend2, LV_OBJ_FLAG_HIDDEN);

                     else
                         lv_obj_add_flag(label_legend2, LV_OBJ_FLAG_HIDDEN); });

    keyMenusPage('V', " RELEASED.", 1, []
                 {
                     static bool hide = false;
                     hide = !hide;
                     lv_chart_hide_series(PowerSupply.stats.chart, PowerSupply.stats.serV, hide);
                     lv_chart_hide_series(PowerSupply.graph.chart, PowerSupply.graph.serV, hide);

                     if (!hide)
                         lv_obj_clear_flag(label_legend1, LV_OBJ_FLAG_HIDDEN);

                     else
                         lv_obj_add_flag(label_legend1, LV_OBJ_FLAG_HIDDEN); });

    keyMenusPage('A', " RELEASED.", 1, []
                 {
                     static bool hide = false;
                     hide = !hide;
                     lv_chart_hide_series(PowerSupply.graph.chart, PowerSupply.graph.serI, hide);
                     lv_chart_hide_series( PowerSupply.stats.chart,  PowerSupply.stats.serI,hide);
                     if (!hide)
                         lv_obj_clear_flag(label_legend2, LV_OBJ_FLAG_HIDDEN);

                     else
                         lv_obj_add_flag(label_legend2, LV_OBJ_FLAG_HIDDEN); });

    // Long press 'j' (AVG button) - Reset energy counter (mWh)
    keyMenus('j', " HOLD.", [&]
             {
                 myTone(NOTE_C5, 100);
                 resetEnergyCounter();
             });

    // Statistics Reset and auto ajdust histogram window
    keyMenusPage('j', " RELEASED.", 0, []
                 {
                     if (PowerSupply.stats.serV->hidden == 0)
                     {
                         PowerSupply.Voltage.hist.histWinMin = PowerSupply.Voltage.measured.Mean() -
                                                               PowerSupply.Voltage.Statistics.StandardDeviation() * 6;

                         PowerSupply.Voltage.hist.histWinMax = PowerSupply.Voltage.measured.Mean() +
                                                               PowerSupply.Voltage.Statistics.StandardDeviation() * 6;
                     }
                     if (PowerSupply.stats.serI->hidden == 0)
                     {
                         PowerSupply.Current.hist.histWinMin = PowerSupply.Current.measured.Mean() -
                                                               PowerSupply.Current.Statistics.StandardDeviation() * 6;

                         PowerSupply.Current.hist.histWinMax = PowerSupply.Current.measured.Mean() +
                                                               PowerSupply.Current.Statistics.StandardDeviation() * 6;
                     }
                     //  PowerSupply.stats.serI->y_points = PowerSupply.Current.hist.data;
                     //  PowerSupply.stats.serV->y_points = PowerSupply.Voltage.hist.data;

                     PowerSupply.ResetStats(); });

    keyMenusPage('j', " RELEASED.", 1, []
                 {
                     if (!lvglChartIsBusy)
                     {
                         lvglChartIsBusy = true;
                         autoScrollY();
                         lvglChartIsBusy = false;
                     }
                 });

    keyMenusPage('j', " RELEASED.", 2, []
                 { PowerSupply.ResetStats(); });

    keyMenusPage('j', " RELEASED.", 4, []
                 { PowerSupply.ResetStats(); });

    keyMenus('Z', " RELEASED.", []
             {
                uint16_t w;
                w = PowerSupply.Voltage.measured.NofAvgs;
                w = (w == MAX_NO_OF_AVG) ? 1 : w * 2;
                // PowerSupply.Voltage.effectiveResolution(64);
                PowerSupply.ResetStats();

                lv_slider_set_value(lv_obj_get_child(PowerSupply.gui.slider_Avgs, -1), log2(w), LV_ANIM_OFF);
                lv_event_send(lv_obj_get_child(PowerSupply.gui.slider_Avgs, -1), LV_EVENT_VALUE_CHANGED, NULL); });

    if ((Tabs::getCurrentPage() == 3))
    {
        int a = -1;

        keyMenus('9', " RELEASED.", [&]
                 { a = 9; });
        keyMenus('8', " RELEASED.", [&]
                 { a = 8; });
        keyMenus('7', " RELEASED.", [&]
                 { a = 7; });
        keyMenus('6', " RELEASED.", [&]
                 { a = 6; });
        keyMenus('5', " RELEASED.", [&]
                 { a = 5; });
        keyMenus('4', " RELEASED.", [&]
                 { a = 4; });
        keyMenus('3', " RELEASED.", [&]
                 { a = 3; });
        keyMenus('2', " RELEASED.", [&]
                 { a = 2; });
        keyMenus('1', " RELEASED.", [&]
                 { a = 1; });
        keyMenus('0', " RELEASED.", [&]
                 { a = 0; });

        if (a >= 0) //-> released
        {
            lv_obj_t *tab = lv_obj_get_child(lv_obj_get_child(lv_obj_get_child(PowerSupply.page[3], 0), 1), 0);
            lv_obj_t *btn = find_btn_by_tag(tab, a);
            loadMemory(btn);
            return;
        }

        keyMenus('0', " HOLD.", [&]
                 { a = 0; });
        keyMenus('1', " HOLD.", [&]
                 { a = 1; });
        keyMenus('2', " HOLD.", [&]
                 { a = 2; });
        keyMenus('3', " HOLD.", [&]
                 { a = 3; });
        keyMenus('4', " HOLD.", [&]
                 { a = 4; });
        keyMenus('5', " HOLD.", [&]
                 { a = 5; });
        keyMenus('6', " HOLD.", [&]
                 { a = 6; });
        keyMenus('7', " HOLD.", [&]
                 { a = 7; });
        keyMenus('8', " HOLD.", [&]
                 { a = 8; });
        keyMenus('9', " HOLD.", [&]
                 { a = 9; });

        if (a >= 0)
        {
            myTone(NOTE_A4, 150);

            lv_obj_t *tab = lv_obj_get_child(lv_obj_get_child(lv_obj_get_child(PowerSupply.page[3], 0), 1), 0);
            lv_obj_t *btn = find_btn_by_tag(tab, a);
            saveMemory(btn);
        }
    }

    // Numerical keys only active on Main page (2), NOT on Settings page (4)
    if (Tabs::getCurrentPage() == 2)
    {
        keyMenus('7', " RELEASED.", []
                 { key_event_handler(0); });
        keyMenus('8', " RELEASED.", []
                 { key_event_handler(1); });
        keyMenus('9', " RELEASED.", []
                 { key_event_handler(2); });
        keyMenus('<', " RELEASED.", []
                 { key_event_handler(3); });

        keyMenus('4', " RELEASED.", []
                 { key_event_handler(5); });
        keyMenus('5', " RELEASED.", []
                 { key_event_handler(6); });
        keyMenus('6', " RELEASED.", []
                 { key_event_handler(7); });

        keyMenus('1', " RELEASED.", []
                 { key_event_handler(10); });
        keyMenus('2', " RELEASED.", []
                 { key_event_handler(11); });
        keyMenus('3', " RELEASED.", []
                 { key_event_handler(12); });

        keyMenus('0', " RELEASED.", []
                 { key_event_handler(15); });
        keyMenus('.', " RELEASED.", []
                 { key_event_handler(16); });

        keyMenusPage('E', " RELEASED.", 2, []
                     {  if (!lv_obj_is_visible(PowerSupply.gui.textarea_set_value))
                        lv_obj_clear_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
                    else
                        lv_obj_add_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
                        ismyTextHiddenChange = true; });

        keyMenusPage('E', " RELEASED.", 4, []
                     {
                     });
    }

    keyMenusPage('^', " RELEASED.", 2, []
                 { PowerSupply.Voltage.setLock(!PowerSupply.Voltage.getLock()); });
    keyMenusPage('>', " RELEASED.", 2, []
                 { PowerSupply.Current.setLock(!PowerSupply.Current.getLock()); });

    keyMenusPage('V', " RELEASED.", 2, []
                 {
                     if (!lv_obj_is_visible(PowerSupply.gui.textarea_set_value))
                     {
                         lv_obj_clear_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
                         key_event_handler_readBack(PowerSupply.Voltage);
                         ismyTextHiddenChange = true;
                         delay(100);
                     }
                     else if (strcmp(lv_label_get_text(unit_label), "V") == 0 || strcmp(lv_label_get_text(unit_label), "mV/V/mA/A") == 0)

                         key_event_handler(8);
                        lv_obj_invalidate(lv_scr_act()); });

    keyMenusPage('v', " RELEASED.", 2, []
                 {
                     if (!lv_obj_is_visible(PowerSupply.gui.textarea_set_value))
                     {
                         lv_obj_clear_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
                         key_event_handler_readBack_k(PowerSupply.Voltage);
                         ismyTextHiddenChange = true;
                         delay(100);
                     }
                     else if (strcmp(lv_label_get_text(unit_label), "mV") == 0 || strcmp(lv_label_get_text(unit_label), "mV/V/mA/A") == 0)
                         key_event_handler(9);lv_obj_invalidate(lv_scr_act());  });

    keyMenusPage('A', " RELEASED.", 2, []
                 {
                     if (!lv_obj_is_visible(PowerSupply.gui.textarea_set_value))
                     {
                         lv_obj_clear_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
                         key_event_handler_readBack(PowerSupply.Current);
                         ismyTextHiddenChange = true;
                           delay(100);
                     }
                     else if (strcmp(lv_label_get_text(unit_label), "A") == 0 || strcmp(lv_label_get_text(unit_label), "mV/V/mA/A") == 0)
                         key_event_handler(13); lv_obj_invalidate(lv_scr_act()); });

    keyMenusPage('a', " RELEASED.", 2, []
                 {
            if (!lv_obj_is_visible(PowerSupply.gui.textarea_set_value))
            {
                lv_obj_clear_flag(PowerSupply.gui.textarea_set_value, LV_OBJ_FLAG_HIDDEN);
                key_event_handler_readBack_k(PowerSupply.Current);
                ismyTextHiddenChange = true;
                  delay(100);
            }
            else if (strcmp(lv_label_get_text(unit_label), "mA") == 0 || strcmp(lv_label_get_text(unit_label), "mV/V/mA/A") == 0)
                key_event_handler(14);lv_obj_invalidate(lv_scr_act());  });

    keyMenusPage('W', " RELEASED.", 2, []
                 {
                     PowerSupply.Voltage.SetRotaryStep(1); //0.0005
                     PowerSupply.Current.SetRotaryStep(1);
                     lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 10 * 12, -1000);
                     lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 10 * 12, -1000); });

    keyMenusPage('W', " RELEASED.", 1, []
                 {
                     static bool chart_mode = false;
                     if (chart_mode)
                         lv_chart_set_update_mode(PowerSupply.graph.chart, LV_CHART_UPDATE_MODE_SHIFT);
                     else
                         lv_chart_set_update_mode(PowerSupply.graph.chart, LV_CHART_UPDATE_MODE_CIRCULAR);

                     chart_mode = !chart_mode; });

    keyMenusPage('X', " RELEASED.", 2, []
                 {
                     PowerSupply.Voltage.SetRotaryStep(1); //0.0005
                     PowerSupply.Current.SetRotaryStep(1);
                     lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 10 * 12, -1000);
                     lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 10 * 12, -1000); });

    keyMenusPage('Y', " RELEASED.", 2, []
                 {
                     PowerSupply.Voltage.SetRotaryStep(1); //0.0005
                     PowerSupply.Current.SetRotaryStep(1);
                     lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 10 * 12, -1000);
                     lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 10 * 12, -1000); });

    keyMenusPage('W', " HOLD.", 2, []
                 {
            PowerSupply.Voltage.SetRotaryStep(2000.0000);
            PowerSupply.Current.SetRotaryStep(10000.000);
            lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 7 * 12, -10);
            lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 7 * 12, 72);
            lv_obj_clear_flag(PowerSupply.Voltage.highlight_adjValue, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(PowerSupply.Current.highlight_adjValue, LV_OBJ_FLAG_HIDDEN); });

    keyMenusPage('X', " HOLD.", 2, []
                 {
            PowerSupply.Voltage.SetRotaryStep(200);
            PowerSupply.Current.SetRotaryStep(1000.000);
            lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 9 * 12, -10);
            lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 9 * 12, 72);
            lv_obj_clear_flag(PowerSupply.Voltage.highlight_adjValue, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(PowerSupply.Current.highlight_adjValue, LV_OBJ_FLAG_HIDDEN); });

    keyMenusPage('Y', " HOLD.", 2, []
                 {
            PowerSupply.Voltage.SetRotaryStep(20);
            PowerSupply.Current.SetRotaryStep(100.000);
            lv_obj_align(PowerSupply.Voltage.highlight_adjValue, 0, 10 * 12, -10);
            lv_obj_align(PowerSupply.Current.highlight_adjValue, 0, 10 * 12, 72);
            lv_obj_clear_flag(PowerSupply.Voltage.highlight_adjValue, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(PowerSupply.Current.highlight_adjValue, LV_OBJ_FLAG_HIDDEN); });

    if ((msg == " IDLE.") && (Tabs::getCurrentPage() == 2))
    {
        keyChar = ' ';
    }
}
