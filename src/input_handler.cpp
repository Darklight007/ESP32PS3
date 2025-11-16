#include "input_handler.h"
#include "device.hpp"
#include "globals.h"
#include "tabs.h"
#include "ui_helpers.h"

// External references
extern TFT_eSPI tft;
extern Device PowerSupply;

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
