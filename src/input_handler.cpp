#include "input_handler.h"
#include "device.hpp"
#include "globals.h"
#include "tabs.h"
#include "ui_helpers.h"
#include "spinbox_pro.h"
#include "table_pro.h"
#include "input_device.h"
#include "functions.h"
#include <algorithm>

// External references
extern TFT_eSPI tft;
extern Device PowerSupply;
extern lv_obj_t *slider_x;

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
static int32_t g_last_enc2_menu = 0;
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
static void handle_menu_mode()
{
    lv_obj_t *list = current_sidebar_list();
    if (!list)
        return;

    if (encoder2_value != g_last_enc2_menu)
    {
        // Map wheel direction to list movement (flip if your hardware feels backwards)
        const int dir = (encoder2_value > g_last_enc2_menu) ? +1 : -1;
        const int count = lv_obj_get_child_cnt(list);
        g_menu_index = std::clamp(g_menu_index + dir, 0, std::max(0, count - 1));

        // "Activate" the target row
        lv_event_send(lv_obj_get_child(list, g_menu_index), LV_EVENT_CLICKED, nullptr);

        g_last_enc2_menu = encoder2_value;
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
        // Serial.printf("\nencoder2_value %i", encoder2_value);
        // Serial.printf("\nencoder2_last_value %i", encoder2_last_value);
        // int32_t _posX = lv_slider_get_value(slider_x);
        // static int32_t _posX = lv_slider_get_value(lv_obj_get_child(PowerSupply.page[1], 3));
        int32_t _posX = lv_slider_get_value(slider_x);

        if (encoder2_last_value < encoder2_value)
            _posX += 160;
        else if (encoder2_last_value > encoder2_value)
            _posX -= 160;
        // encoder2_last_value = encoder2_value;

        if (keyChar == 'W' && msg == " HOLD.")
        {
            // Scroll the graph horizontally
            _posX = std::clamp(_posX, 0, 57 * (lv_chart_get_zoom_x(PowerSupply.graph.chart) - 256) / 16);
            // _posY = std::clamp(_posY, 0, int(33.5 * (lv_chart_get_zoom_y(PowerSupply.graph.chart) - 256) / 64));
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

    // static int32_t encoder1_last_value = 0;
    // static int32_t encoder2_last_value = 0;
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
    // static int32_t encoder1_last_value = 0;
    // static int32_t encoder2_last_value = 0;
    // int32_t  encoder2_last_value;

    // **Handle Vertical Shift/Zoom with Encoder 1**
    if (encoder1_last_value != encoder1_value)
    {

        static u8_t _posY = 0;

        // // Determine the direction of encoder 1 rotation
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
        // // Determine the direction of encoder 1 rotation
        if (encoder2_last_value < encoder2_value)
        {

            if (lv_tabview_get_tab_act(lv_obj_get_child(PowerSupply.page[3], 0)) == 1)
                select_previous_row(Utility_objs.table_fun_gen_list, 25);
            else
                select_previous_row(Utility_objs.table_point_list, 2 * 7 + 2 * 5);
        }
        else if (encoder2_last_value > encoder2_value)
        {

            if (lv_tabview_get_tab_act(lv_obj_get_child(PowerSupply.page[3], 0)) == 1)
                select_next_row(Utility_objs.table_fun_gen_list, 25);
            else
                select_next_row(Utility_objs.table_point_list, 2 * 7 + 2 * 5);
        }

        // static lv_obj_t *table = lv_obj_get_child(PowerSupply.page[3], 0);

        // lv_tabview_set_act(table, _posY % 4, LV_ANIM_ON);
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

        // Serial.printf("\n***********************************");
        // print_obj_type(fgen_tabview); // Output: Object is************ a button
        // Serial.printf("\nSpinbox_pro%i", get_spinbox_data_by_id(fgen_tabview, 2));

        PowerSupply.funGenMem.amplitude = lv_spinbox_get_value(Utility_objs.fun.Amplitude) / 1000.0;
        PowerSupply.funGenMem.frequency = lv_spinbox_get_value(Utility_objs.fun.Frequency) / 1000.0;
        PowerSupply.funGenMem.offset = lv_spinbox_get_value(Utility_objs.fun.Offset) / 1000.0;
        PowerSupply.funGenMem.dutyCycle = lv_spinbox_get_value(Utility_objs.fun.Duty) / 10000.0;

        // PowerSupply.funGenMem.amplitude = double(get_spinbox_data_by_id(fgen_tabview, 0) / 1000.0);
        // PowerSupply.funGenMem.frequency = double(get_spinbox_data_by_id(fgen_tabview, 1) / 1000.0);
        // PowerSupply.funGenMem.offset = double(get_spinbox_data_by_id(fgen_tabview, 2) / 1000.0);
        // PowerSupply.funGenMem.dutyCycle = double(get_spinbox_data_by_id(fgen_tabview, 3) / 10000.0);

        if (lv_tabview_get_tab_act(lv_obj_get_child(PowerSupply.page[3], 0)) == 1)
            PowerSupply.SaveMemoryFgen("FunGen", PowerSupply.funGenMem);

        // Serial.printf("\n********************");
        // Serial.printf("\namplitude %f", PowerSupply.funGenMem.amplitude);
        // Serial.printf("\nfrequency %f", PowerSupply.funGenMem.frequency);
        // Serial.printf("\noffset %f", PowerSupply.funGenMem.offset);
        // Serial.printf("\ndutyCycle %f", PowerSupply.funGenMem.dutyCycle);
        // Serial.printf("\n********************");
        // print_obj_type(parent); // Output: Object is a button
        // Serial.printf("\nSpinbox_pro%i", get_spinbox_data_by_id(parent, 2));
    }
}

void getSettingEncoder(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    if (encoder2Flag /*&& (Tabs::getCurrentPage() == 4)*/)
    {
        // encoder2Flag = 0;

        int64_t count = PowerSupply.Current.encoder.getCount() >> 1; // / 2;
        // PowerSupply.Current.encoder.pauseCount();

        static int64_t rotaryOldValue2 = 0;

        // encoder2_value = count;
        if (count > rotaryOldValue2)
            encoder2_value++;
        // PowerSupply.Current.SetUpdate(PowerSupply.Current.adjValue + PowerSupply.Current.rotaryEncoderStep);

        else if (count < rotaryOldValue2)
            encoder2_value--;
        // PowerSupply.Current.SetUpdate(PowerSupply.Current.adjValue - PowerSupply.Current.rotaryEncoderStep);

        rotaryOldValue2 = count;
        // encoder2_value = std::clamp(encoder2_value, -500, 500);
        // encoder2Flag = (encoder2Flag > 300) ? 300 : encoder2Flag - 1;
        encoder2Flag = 0;
        // if (encoder2Flag == 0)
        //   PowerSupply.Current.encoder.setCount(0);
        switch (Tabs::getCurrentPage())
        {
        // case 4:
        //     break;
        case 2:
            PowerSupply.Current.SetEncoderUpdate();
            // PowerSupply.FlushSettings();
            // PowerSupply.Voltage.SetEncoderUpdate();
            break;
        }
        // LV_LOG_USER("encoder2_value:%i encoder2Flag:%i count:%i", encoder2_value, encoder2Flag, count);
        encoderTimeStamp = millis();
    }

    if (encoder1Flag /*&& (Tabs::getCurrentPage() == 4)*/)
    {

        int64_t count = PowerSupply.Voltage.encoder.getCount() >> 1; // / 2;
        // // PowerSupply.Current.encoder.pauseCount();

        static int64_t rotaryOldValue = 0;

        // encoder2_value = count;
        if (count > rotaryOldValue)
            encoder1_value++;

        else if (count < rotaryOldValue)
            encoder1_value--;

        rotaryOldValue = count;
        // encoder1Flag = (encoder1Flag > 300) ? 300 : encoder1Flag - 1;
        // encoder1_value = PowerSupply.Voltage.encoder.getCount() / 2;
        encoder1Flag = 0;
        switch (Tabs::getCurrentPage())
        {
        // case 4:
        //     break;
        case 2:
            PowerSupply.Voltage.SetEncoderUpdate();
            // PowerSupply.FlushSettings();

            // PowerSupply.Voltage.Flush();
            break;
        }
        // PowerSupply.Current.encoder.resumeCount();

        // LV_LOG_USER("encoder2_value:%i encoder1Flag:%i count:%i", 0, encoder1Flag,
        encoderTimeStamp = millis();
    }
}

void managePageEncoderInteraction()
{
    // // Variables to store the last encoder values
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
        // lv_slider_set_value(slider_x, encoder2_last_value,LV_ANIM_OFF);
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
