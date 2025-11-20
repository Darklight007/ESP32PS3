#include "ui_helpers.h"
#include "functions.h"
#include "myFonts.h"
#include "buzzer.h"
#include "tabs.h"
#include "spinbox_pro.h"

// UI object definitions
lv_obj_t *label_legend1;
lv_obj_t *label_legend2;

void hide(lv_obj_t *obj)
{
    if (obj)
    {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}
void initBuckets(TFT_eSPI &tft)
{
    for (uint16_t x = 0; x < BUCKET_COUNT; x++)
    {
        dataBuckets[x] = 180;
        tft.fillRect(x * (SCREEN_WIDTH / BUCKET_COUNT), dataBuckets[x], DOT_SIZE, DOT_SIZE, TFT_WHITE); // Draw initial dots
    }
}

// Update a single X with a new Y
void plotToBucket(uint16_t x, uint16_t y, lv_obj_t *chart, lv_chart_series_t *series)
{
    if (!chart || !series || lv_obj_is_visible(chart) == false || Tabs::getCurrentPage() != 3) // || lv_tabview_get_tab_act(tabview_utility) !=3)
        return;

    if (y > 30 && y < 190 && x > 5)
    {
        y = CHART_Y_MAX - y; // Flip Y-axis so 0 is at the bottom

        // Plot the point at the correct position
        lv_chart_set_value_by_id(chart, series, (x - 20) / (100 / CHART_POINTS * 2.9), y + 80);

        // Refresh the chart
        lv_chart_refresh(chart);
    }
}

// GraphChart function
void GraphChart(lv_obj_t *parent, lv_coord_t x, lv_coord_t y)
{ /*Create a chart*/

    PowerSupply.graph.chart = lv_chart_create(parent);
    lv_obj_set_size(PowerSupply.graph.chart, 242, 154);
    lv_obj_align(PowerSupply.graph.chart, LV_ALIGN_DEFAULT, x, y);

    lv_chart_set_range(PowerSupply.graph.chart, LV_CHART_AXIS_PRIMARY_Y, -.3 * 1000, 32000);  // 40000mv
    lv_chart_set_range(PowerSupply.graph.chart, LV_CHART_AXIS_SECONDARY_Y, -.3 * 1000, 8000); // 8000ma
    lv_chart_set_range(PowerSupply.graph.chart, LV_CHART_AXIS_PRIMARY_X, 0, 300);

    lv_chart_set_div_line_count(PowerSupply.graph.chart, 9, 13);
    lv_obj_set_style_text_color(PowerSupply.graph.chart, lv_palette_main(LV_PALETTE_GREY), LV_PART_TICKS);
    lv_chart_set_axis_tick(PowerSupply.graph.chart, LV_CHART_AXIS_PRIMARY_Y, 5, 3, 9, 4, true, 40);
    lv_chart_set_axis_tick(PowerSupply.graph.chart, LV_CHART_AXIS_PRIMARY_X, 5, 3, 7, 10, true, 50);
    lv_chart_set_axis_tick(PowerSupply.graph.chart, LV_CHART_AXIS_SECONDARY_Y, 5, 3, 9, 10, true, 50);

    lv_chart_set_type(PowerSupply.graph.chart, LV_CHART_TYPE_LINE);

    lv_chart_set_update_mode(PowerSupply.graph.chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_obj_add_event_cb(PowerSupply.graph.chart, draw_event_cb2, LV_EVENT_DRAW_PART_BEGIN, NULL);

    PowerSupply.graph.serI = lv_chart_add_series(PowerSupply.graph.chart, lv_palette_main(LV_PALETTE_AMBER), LV_CHART_AXIS_SECONDARY_Y);
    PowerSupply.graph.serV = lv_chart_add_series(PowerSupply.graph.chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_point_count(PowerSupply.graph.chart, CHART_SIZE);
    PowerSupply.graph.serV->y_points = graph_data_V;
    PowerSupply.graph.serI->y_points = graph_data_I;

    static lv_style_t style_slider;
    lv_style_init(&style_slider);
    lv_style_set_bg_opa(&style_slider, LV_OPA_50);
    lv_style_set_bg_color(&style_slider, lv_palette_main(LV_PALETTE_LIGHT_GREEN));

    slider_x = lv_slider_create(parent);

    lv_obj_remove_style(slider_x, &style_slider, LV_STATE_DEFAULT);
    lv_slider_set_range(slider_x, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 120);
    lv_obj_add_event_cb(slider_x, slider_x_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(slider_x, slider_x_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_set_size(slider_x, 228, 6);
    lv_obj_align_to(slider_x, PowerSupply.graph.chart, LV_ALIGN_OUT_BOTTOM_MID, 0, -5);

    lv_obj_add_style(slider_x, &style_slider, LV_PART_INDICATOR);
    lv_obj_add_style(slider_x, &style_slider, LV_PART_MAIN);
    lv_obj_add_style(slider_x, &style_slider, LV_PART_KNOB);
    lv_obj_t *slider;
    slider = lv_slider_create(parent);
    lv_slider_set_range(slider, LV_IMG_ZOOM_NONE, LV_IMG_ZOOM_NONE * 20);
    lv_obj_add_event_cb(slider, slider_y_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_size(slider, 6, 140);
    lv_obj_align_to(slider, PowerSupply.graph.chart, LV_ALIGN_OUT_RIGHT_MID, -4, +2);

    lv_obj_add_style(slider, &style_slider, LV_PART_INDICATOR);
    lv_obj_add_style(slider, &style_slider, LV_PART_MAIN);
    lv_obj_add_style(slider, &style_slider, LV_PART_KNOB);

    legend(parent, lv_palette_main(LV_PALETTE_BLUE), "V-Set", lv_palette_main(LV_PALETTE_AMBER), "I-set", 25, 0);

    lv_obj_set_parent(PowerSupply.Voltage.statLabels.label_fft, parent);
    lv_obj_set_parent(PowerSupply.Current.statLabels.label_fft, parent);
}


// stat_measure function
void stat_measure(lv_obj_t *parent, lv_coord_t x, lv_coord_t y)
{
    static lv_style_t style_stats;
    lv_style_init(&style_stats);
    lv_style_set_text_font(&style_stats, &lv_font_unscii_8);
    lv_style_set_text_letter_space(&style_stats, 1);
    lv_style_set_text_color(&style_stats, lv_palette_main(LV_PALETTE_GREY));
    lv_style_set_text_letter_space(&style_stats, -1);

    lv_obj_set_parent(PowerSupply.Voltage.statLabels.label_legend, parent);
    lv_label_set_text(PowerSupply.Voltage.statLabels.label_legend, " Set      Mean     STD      Max      Min");
    //                                                            "+00.0000 +00.0000 +00.0000 +00.0000 +00.0000
    lv_obj_align(PowerSupply.Voltage.statLabels.label_legend, LV_ALIGN_DEFAULT, x, y);
    lv_obj_remove_style(PowerSupply.Voltage.statLabels.label_legend, &style_stats, LV_STATE_DEFAULT);
    lv_obj_add_style(PowerSupply.Voltage.statLabels.label_legend, &style_stats, LV_STATE_DEFAULT);

    // int offset = x;  unused variable
    int offsetY = 10 + y;

    lv_style_init(&PowerSupply.graph.style_statsVolt);
    lv_style_set_text_color(&PowerSupply.graph.style_statsVolt, lv_palette_main(LV_PALETTE_BLUE));

    lv_style_init(&PowerSupply.graph.style_statsCurrent);
    lv_style_set_text_color(&PowerSupply.graph.style_statsCurrent, lv_palette_main(LV_PALETTE_AMBER));
    // Display mean max min ...
    auto stat_obj = [](lv_obj_t *parent, DispObjects &dObj, lv_style_t *style_, lv_coord_t x, lv_coord_t y)
    {
        lv_style_set_text_font(style_, &lv_font_unscii_8);
        lv_style_set_text_letter_space(style_, 1);
        lv_style_set_text_letter_space(style_, -1);

        int space = 61;

        lv_obj_remove_style_all(dObj.statLabels.label_unit);
        lv_obj_set_parent(dObj.statLabels.label_unit, parent);
        lv_label_set_text(dObj.statLabels.label_unit, lv_label_get_text(dObj.label_unit));
        lv_obj_align(dObj.statLabels.label_unit, LV_ALIGN_DEFAULT, x + -10, y);
        lv_obj_add_style(dObj.statLabels.label_unit, style_, LV_STATE_DEFAULT);

        lv_obj_remove_style_all(dObj.statLabels.label_value);
        lv_obj_set_parent(dObj.statLabels.label_value, parent);
        lv_obj_align(dObj.statLabels.label_value, LV_ALIGN_DEFAULT, 0, -500);
        lv_obj_add_style(dObj.statLabels.label_value, style_, LV_STATE_DEFAULT);

        lv_obj_remove_style_all(dObj.statLabels.label_setSmallFont);
        lv_obj_set_parent(dObj.statLabels.label_setSmallFont, parent);
        lv_obj_align(dObj.statLabels.label_setSmallFont, LV_ALIGN_DEFAULT, x + space * 0, y);
        lv_obj_add_style(dObj.statLabels.label_setSmallFont, style_, LV_STATE_DEFAULT);

        lv_obj_remove_style_all(dObj.statLabels.label_mean);
        lv_obj_set_parent(dObj.statLabels.label_mean, parent);
        lv_obj_align(dObj.statLabels.label_mean, LV_ALIGN_DEFAULT, x + space * 1, y);
        lv_obj_add_style(dObj.statLabels.label_mean, style_, LV_STATE_DEFAULT);

        lv_obj_remove_style_all(dObj.statLabels.label_std);
        lv_obj_set_parent(dObj.statLabels.label_std, parent);
        lv_obj_align(dObj.statLabels.label_std, LV_ALIGN_DEFAULT, x + space * 2 + 4, y);
        lv_obj_add_style(dObj.statLabels.label_std, style_, LV_STATE_DEFAULT);

        lv_obj_remove_style_all(dObj.statLabels.label_max);
        lv_obj_set_parent(dObj.statLabels.label_max, parent);
        lv_obj_align(dObj.statLabels.label_max, LV_ALIGN_DEFAULT, x + space * 3, y);
        lv_obj_add_style(dObj.statLabels.label_max, style_, LV_STATE_DEFAULT);

        lv_obj_remove_style_all(dObj.statLabels.label_min);
        lv_obj_set_parent(dObj.statLabels.label_min, parent);
        lv_obj_align(dObj.statLabels.label_min, LV_ALIGN_DEFAULT, x + space * 4, y);
        lv_obj_add_style(dObj.statLabels.label_min, style_, LV_STATE_DEFAULT);
    };

    stat_obj(parent, PowerSupply.Voltage, &PowerSupply.graph.style_statsVolt, x - 8, offsetY);
    stat_obj(parent, PowerSupply.Current, &PowerSupply.graph.style_statsCurrent, x - 8, offsetY + 10);
}



// StatsChart function
void StatsChart(lv_obj_t *parent, lv_coord_t x, lv_coord_t y)
{ /*Create a chart*/

    PowerSupply.stats.chart = lv_chart_create(parent);
    lv_obj_set_size(PowerSupply.stats.chart, 260, 160);
    lv_obj_center(PowerSupply.stats.chart);
    lv_obj_align(PowerSupply.stats.chart, LV_ALIGN_DEFAULT, x, y + 0);

    lv_chart_set_range(PowerSupply.stats.chart, LV_CHART_AXIS_PRIMARY_Y, 0, 160);   // 40000mv
    lv_chart_set_range(PowerSupply.stats.chart, LV_CHART_AXIS_SECONDARY_Y, 0, 160); // 8000ma
    lv_chart_set_range(PowerSupply.stats.chart, LV_CHART_AXIS_PRIMARY_X, 0, 350);

    lv_chart_set_div_line_count(PowerSupply.stats.chart, 5, 13);
    lv_obj_set_style_text_color(PowerSupply.stats.chart, lv_palette_main(LV_PALETTE_GREY), LV_PART_TICKS);
    lv_chart_set_axis_tick(PowerSupply.stats.chart, LV_CHART_AXIS_PRIMARY_Y, 5, 3, 5, 4, true, 40);
    lv_chart_set_axis_tick(PowerSupply.stats.chart, LV_CHART_AXIS_PRIMARY_X, 5, 3, 3, 10, true, 50);

    lv_chart_set_type(PowerSupply.stats.chart, LV_CHART_TYPE_LINE);
    lv_chart_set_update_mode(PowerSupply.stats.chart, LV_CHART_UPDATE_MODE_CIRCULAR);
    lv_obj_add_event_cb(PowerSupply.stats.chart, draw_event_stat_chart_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
    lv_obj_set_style_line_width(PowerSupply.stats.chart, 2, LV_PART_ITEMS);

    lv_obj_set_style_size(PowerSupply.stats.chart, 0, LV_PART_INDICATOR);

    PowerSupply.stats.serI = lv_chart_add_series(PowerSupply.stats.chart, lv_palette_main(LV_PALETTE_AMBER), LV_CHART_AXIS_SECONDARY_Y);
    PowerSupply.stats.serV = lv_chart_add_series(PowerSupply.stats.chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);

    lv_chart_set_point_count(PowerSupply.stats.chart, 350);
    PowerSupply.stats.serI->y_points = PowerSupply.Current.hist.data;
    PowerSupply.stats.serV->y_points = PowerSupply.Voltage.hist.data;
}

#define NUM_LABELS 7


void GraphPush()
{
    // First shift everything left by one position
    memcpy(&graph_data_V[0], &graph_data_V[1], (CHART_SIZE - 1) * sizeof(graph_data_V[0]));
    memcpy(&graph_data_I[0], &graph_data_I[1], (CHART_SIZE - 1) * sizeof(graph_data_I[0]));

    // Now place the new value at the end
    graph_data_V[CHART_SIZE - 1] = PowerSupply.Voltage.measured.value * 1000.0;
    graph_data_I[CHART_SIZE - 1] = PowerSupply.Current.measured.value * 1000.0;
}

void HistPush()
{
    // Histogram
    PowerSupply.Current.hist[PowerSupply.Current.measured.Mean()];
    PowerSupply.Voltage.hist[PowerSupply.Voltage.measured.Mean()];
}

void legend(lv_obj_t *parent, lv_color16_t c1, const char *ser1, lv_color16_t c2, const char *ser2, int x, int y)
{
    int height = 0;

    lv_style_init(&PowerSupply.graph.style_legend1);
    lv_style_set_text_letter_space(&PowerSupply.graph.style_legend1, -2);
    lv_style_set_text_color(&PowerSupply.graph.style_legend1, c1);
    lv_style_set_text_font(&PowerSupply.graph.style_legend1, &Undertale_16b);
    lv_style_set_bg_color(&PowerSupply.graph.style_legend1, lv_palette_darken(LV_PALETTE_GREY, 4));
    lv_style_set_bg_opa(&PowerSupply.graph.style_legend1, LV_OPA_50);
    lv_style_set_border_opa(&PowerSupply.graph.style_legend1, LV_OPA_50);
    lv_style_set_border_width(&PowerSupply.graph.style_legend1, 2);

    label_legend1 = lv_label_create(parent);
    lv_label_set_text(label_legend1, "---  V");
    lv_obj_align(label_legend1, LV_ALIGN_DEFAULT, x, y);
    lv_obj_remove_style(label_legend1, &PowerSupply.graph.style_legend1, LV_STATE_DEFAULT);
    lv_obj_add_style(label_legend1, &PowerSupply.graph.style_legend1, LV_STATE_DEFAULT);

    lv_style_init(&PowerSupply.graph.style_legend2);
    lv_style_set_text_letter_space(&PowerSupply.graph.style_legend2, -2);
    lv_style_set_text_color(&PowerSupply.graph.style_legend2, c2);
    lv_style_set_text_font(&PowerSupply.graph.style_legend2, &Undertale_16b);
    lv_style_set_bg_color(&PowerSupply.graph.style_legend2, lv_palette_darken(LV_PALETTE_GREY, 4));
    lv_style_set_bg_opa(&PowerSupply.graph.style_legend2, LV_OPA_50);
    lv_style_set_border_opa(&PowerSupply.graph.style_legend2, LV_OPA_50);
    lv_style_set_border_width(&PowerSupply.graph.style_legend2, 2);

    label_legend2 = lv_label_create(parent);
    lv_label_set_text(label_legend2, "---  A");
    lv_obj_align(label_legend2, LV_ALIGN_DEFAULT, x, height + 4 + 8);
    lv_obj_remove_style(label_legend2, &PowerSupply.graph.style_legend2, LV_STATE_DEFAULT);
    lv_obj_add_style(label_legend2, &PowerSupply.graph.style_legend2, LV_STATE_DEFAULT);
}

void overlay(lv_obj_t *label, const char *text, lv_style_t *style, lv_color16_t c1, int x, int y)
{
    lv_style_init(style);
    lv_style_set_text_letter_space(style, -2);
    lv_style_set_text_color(style, c1);
    lv_style_set_text_font(style, &lv_font_unscii_8);
    lv_style_set_bg_color(style, lv_palette_darken(LV_PALETTE_GREY, 4));
    lv_style_set_bg_opa(style, LV_OPA_50);
    lv_style_set_border_opa(style, LV_OPA_50);
    lv_style_set_border_width(style, 2);

    lv_label_set_text(label, text);
    lv_obj_align(label, LV_ALIGN_DEFAULT, x, y);
    lv_obj_remove_style(label, style, LV_STATE_DEFAULT);
    lv_obj_add_style(label, style, LV_STATE_DEFAULT);
}

// UI event callbacks (moved from globalFunctions.h)

void draw_event_stat_chart_cb(lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

    // Check if the part being drawn is a tick label on the primary X-axis
    if (dsc->id == LV_CHART_AXIS_PRIMARY_Y &&
        dsc->text && dsc->label_dsc && dsc->part == LV_PART_TICKS)

    {
        dsc->label_dsc->font = &lv_font_montserrat_10;
        dsc->label_dsc->color = lv_color_hex(0xFFFFFF);
    }

    if (dsc->part == LV_PART_TICKS && dsc->id == LV_CHART_AXIS_PRIMARY_X && dsc->text)
    {

        int VI_hidden = 0;

        if (PowerSupply.stats.serV->hidden == PowerSupply.stats.serI->hidden &&
            PowerSupply.stats.serV->hidden == false)
            VI_hidden = 3;
        else if (PowerSupply.stats.serV->hidden == false)
            VI_hidden = 1;
        else if (PowerSupply.stats.serI->hidden == false)
            VI_hidden = 2;

        // Number of ticks is 7
        const int numTicks = 3;

        // Arrays to hold tick labels for 7 ticks
        char tickLabels[3][80]; // Adjust size as needed

        // Calculate the step sizes for voltage and current
        double voltageStep = (PowerSupply.Voltage.hist.histWinMax - PowerSupply.Voltage.hist.histWinMin) / (numTicks - 1);
        double currentStep = (PowerSupply.Current.hist.histWinMax - PowerSupply.Current.hist.histWinMin) / (numTicks - 1);

        // Generate tick labels
        for (int i = 0; i < numTicks; i++)
        {
            double voltageValue = PowerSupply.Voltage.hist.histWinMin + i * voltageStep;
            double currentValue = PowerSupply.Current.hist.histWinMin + i * currentStep;

            // Initialize units as empty strings
            char v_unit[3] = "V";
            char c_unit[3] = "A";

            // Format the voltage value with appropriate precision
            char voltageStr[20];
            if (fabs(voltageStep) >= 1.0)
                snprintf(voltageStr, sizeof(voltageStr), "%3.4f", voltageValue);
            else if (fabs(voltageStep) >= 0.1)
                snprintf(voltageStr, sizeof(voltageStr), "%3.1f", voltageValue);
            else if (fabs(voltageStep) >= 0.01)
                snprintf(voltageStr, sizeof(voltageStr), "%3.2f", voltageValue);
            else
            {
                // Convert to millivolts if the step is very small
                voltageValue *= 1000;
                snprintf(voltageStr, sizeof(voltageStr), "%3.2f", voltageValue);
                strcpy(v_unit, "mV");
            }

            // Format the current value with appropriate precision
            char currentStr[20];
            if (fabs(currentStep) >= 1.0)
                snprintf(currentStr, sizeof(currentStr), "%3.0f", currentValue);
            else if (fabs(currentStep) >= 0.1)
                snprintf(currentStr, sizeof(currentStr), "%3.1f", currentValue);
            else if (fabs(currentStep) >= 0.01)
                snprintf(currentStr, sizeof(currentStr), "%3.2f", currentValue);
            else
            {
                // Convert to milliamps if the step is very small
                currentValue *= 1000;
                snprintf(currentStr, sizeof(currentStr), "%3.2f", currentValue);
                strcpy(c_unit, "mA");
            }

            // **Add units only to the middle tick label (position 3)**
            if (i == 1)
            {
                strcat(voltageStr, v_unit);
                strcat(currentStr, c_unit);
            }

            // Combine voltage and current strings into one label

            switch (VI_hidden)
            {
            case 1:
                lv_obj_set_size(PowerSupply.stats.chart, 260, 150);
                snprintf(tickLabels[i], sizeof(tickLabels[i]), "%s", voltageStr);
                break;

            case 2:
                lv_obj_set_size(PowerSupply.stats.chart, 260, 150);
                snprintf(tickLabels[i], sizeof(tickLabels[i]), "%s", currentStr);
                break;

            case 3:
                lv_obj_set_size(PowerSupply.stats.chart, 260, 140);
                snprintf(tickLabels[i], sizeof(tickLabels[i]), "%s\n%s", voltageStr, currentStr);
                break;

            default:
                snprintf(tickLabels[i], sizeof(tickLabels[i]), "%s", "");
                break;
            }
        }

        // **Boundary Check to Prevent Out-of-Bounds Access**
        if (dsc->value >= 0 && dsc->value < numTicks)
        {
            // Set the tick label for the current value
            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabels[dsc->value]);
        }
        else
        {
            // If out of bounds, set an empty label
            lv_snprintf(dsc->text, dsc->text_length, "");
        }

        // Change the font size of the tick labels
        if (dsc->label_dsc)
        {
            dsc->label_dsc->font = &lv_font_montserrat_10;

            switch (VI_hidden)
            {
            case 1:
                dsc->label_dsc->color = lv_palette_main(LV_PALETTE_BLUE);
                break;

            case 2:
                dsc->label_dsc->color = lv_palette_main(LV_PALETTE_AMBER);
                break;

            case 3:

                dsc->label_dsc->color = lv_color_hex(0xFFFFFF);
                break;

            default:
                break;
            }
        }
    }
}

void draw_event_cb2(lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

    if (!dsc)
        return;

    /* Customize division lines */
    if (dsc->part == LV_PART_MAIN)
    {
        if (!dsc->line_dsc || !dsc->p1 || !dsc->p2)
            return;

        dsc->line_dsc->color = lv_palette_main(LV_PALETTE_GREY);

        /* Vertical lines */
        if (dsc->p1->x == dsc->p2->x)
        {
            if (dsc->id == 0)
            {
                dsc->line_dsc->width = 1;
                dsc->line_dsc->dash_gap = 0;
                dsc->line_dsc->dash_width = 0;
            }
            else
            {
                dsc->line_dsc->width = 1;
                dsc->line_dsc->dash_gap = 5;
                dsc->line_dsc->dash_width = 5;
            }
            return;
        }
        /* Horizontal lines */
        else
        {
            if (dsc->id == 8)
            {
                dsc->line_dsc->width = 1;
                dsc->line_dsc->dash_gap = 0;
            }
            else
            {
                dsc->line_dsc->width = 1;
                dsc->line_dsc->dash_gap = 5;
                dsc->line_dsc->dash_width = 5;
            }
            return;
        }
    }

    /* Customize tick labels */
    if (dsc->part == LV_PART_TICKS && dsc->text)
    {
        /* Handle LV_CHART_AXIS_PRIMARY_X */
        if (dsc->id == LV_CHART_AXIS_PRIMARY_X)
        {
            if (PowerSupply.settingParameters.graphXaxisTimeMode)
            {
                // Time mode: Show time in seconds, minutes, or hours
                uint16_t timeSpan = PowerSupply.settingParameters.graphTimeSpanSeconds;

                for (int i = 0; i < NUM_LABELS; i++)
                {
                    if (i == NUM_LABELS - 1)
                    {
                        strcpy(tickLabels_x[i], "0");
                    }
                    else
                    {
                        int timeValue = timeSpan * (NUM_LABELS - 1 - i) / (NUM_LABELS - 1);

                        if (timeSpan < 120)  // Less than 2 minutes, show in seconds
                        {
                            sprintf(tickLabels_x[i], "%ds", timeValue);
                        }
                        else if (timeSpan < 3600)  // Less than 1 hour, show in minutes
                        {
                            sprintf(tickLabels_x[i], "%dm", timeValue / 60);
                        }
                        else  // 1 hour or more, show in hours:minutes
                        {
                            int hours = timeValue / 3600;
                            int mins = (timeValue % 3600) / 60;
                            if (mins == 0)
                                sprintf(tickLabels_x[i], "%dh", hours);
                            else
                                sprintf(tickLabels_x[i], "%d:%02d", hours, mins);
                        }
                    }
                }
            }
            else
            {
                // Points mode: Show point numbers
                for (int i = 0; i < NUM_LABELS; i++)
                {
                    if (i == NUM_LABELS - 1)
                    {
                        strcpy(tickLabels_x[i], "0 pts");
                    }
                    else
                    {
                        int value = CHART_SIZE * (NUM_LABELS - 1 - i) / (NUM_LABELS - 1);
                        sprintf(tickLabels_x[i], "%d", value);
                    }
                }
            }

            static int index_x = 0;

            if (index_x == 7)
                index_x = 0;

            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabels_x[index_x++]);

            if (dsc->label_dsc)
                dsc->label_dsc->font = &lv_font_montserrat_10;
        }
        /* Handle LV_CHART_AXIS_PRIMARY_Y */
        else if (dsc->id == LV_CHART_AXIS_PRIMARY_Y)
        {
            static int index_y = 0;
            static char *tickLabels_y[] = {"32.0V", "28.0", "24.0", "20.0", "16.0", "12.0", "8.0", "4.0", "0.0"};

            if (strcmp(dsc->text, "32000") == 0)
                index_y = 0;

            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabels_y[index_y++]);

            if (dsc->label_dsc)
                dsc->label_dsc->font = &lv_font_montserrat_10;
        }
        /* Handle LV_CHART_AXIS_SECONDARY_Y */
        else if (dsc->id == LV_CHART_AXIS_SECONDARY_Y)
        {
            static int index_sy = 0;
            static char *tickLabels_sy[] = {"8.0A", "7.0", "6.0", "5.0", "4.0", "3.0", "2.0", "1.0", "0.0"};

            if (strcmp(dsc->text, "8000") == 0)
                index_sy = 0;

            lv_snprintf(dsc->text, dsc->text_length, "%s", tickLabels_sy[index_sy++]);

            if (dsc->label_dsc)
                dsc->label_dsc->font = &lv_font_montserrat_10;
        }
    }
}

void btn_function_gen_event_cb(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);

    if (lv_obj_has_state(btn_function_gen, LV_STATE_CHECKED))
    {
        lv_label_set_text(label, "ON");
        if (PowerSupply.getStatus() != DEVICE::FUN)
            PowerSupply.setStatus(DEVICE::FUN);
    }
    else
    {
        lv_label_set_text(label, "OFF");
        PowerSupply.setStatus(DEVICE::ON);
    }

    remove_selected_spinbox();
}
