#pragma once

#include <lvgl.h>
#include <TFT_eSPI.h>

// Constants
#define CHART_SIZE 240 * 5
#define BUCKET_COUNT 100
#define NUM_LABELS 7
#define RECORDING_TABLE_SIZE 250  // Number of data points in recording table

// Global variables for chart data
extern lv_coord_t graph_data_V[CHART_SIZE];
extern lv_coord_t graph_data_I[CHART_SIZE];

extern int32_t barGraph_V[1];
extern int32_t barGraph_I[1];

// UI objects
extern lv_obj_t *tabview_utility;
extern lv_obj_t *btn_function_gen;

// Encoder values u
extern int globalSliderXValue;
extern int32_t encoder1_value, encoder2_value;

// Waveform data
extern bool blockAll;

// Input state
extern bool dropdown_active;
extern bool keyboardInputActive;  // Tracks if user is actively typing numbers
extern bool manualTabSelection;   // Tracks if user manually clicked a tab (disables encoder tab navigation)

// Utility charts
extern lv_obj_t *util_Arbit_chart;
extern lv_chart_series_t *util_Arbit_chart_series;

// Bucket and label data
extern uint16_t dataBuckets[BUCKET_COUNT];
extern char tickLabels_x[NUM_LABELS][10];

// Function generator structures
struct function
{
    lv_obj_t *Amplitude;
    lv_obj_t *Frequency;
    lv_obj_t *Offset;
    lv_obj_t *Duty;
};

struct objs_list
{
    lv_obj_t *table_fun_gen_list;
    function fun;
    lv_obj_t *table_point_list;
    lv_obj_t *table_spinbox_value;
    double table_current_value;
    lv_obj_t *switch_keys_scan;
    lv_obj_t *switch_keys_label;
    lv_coord_t arbitrary_points[20][2];

    // Record tab elements
    lv_obj_t *record_btn;
    lv_obj_t *record_play_btn;
    lv_obj_t *record_stop_btn;
    lv_obj_t *record_loop_switch;
    lv_obj_t *record_status_label;
    lv_obj_t *record_chart;
    lv_chart_series_t *record_chart_series;
    lv_obj_t *record_sample_rate_spinbox;
    lv_obj_t *record_duration_spinbox;
};

extern struct objs_list Utility_objs;

// Waveform function pointer type
typedef double (*WaveformFunction)(double);

// Struct for waveform function and name
typedef struct
{
    const char *name;
    WaveformFunction function;
} Waveform;

// Array of waveform structs
extern Waveform waveforms[];
extern int numWaveforms;
