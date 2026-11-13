#pragma once

#include <lvgl.h>
#include <TFT_eSPI.h>

// Constants
// Parentheses are load-bearing: unparenthesised, `x / CHART_SIZE` expanded to
// `x / 240 * 5`, which (same precedence, left to right) is `(x / 240) * 5` -
// 25x the intended value. GraphPush()'s time-mode msPerPoint is the one place
// that divides by it, so a "1m" span produced 1250ms/point instead of 50 and
// the trace crawled: 1200 points x 1.25s = 25 minutes to fill a 60s window.
#define CHART_SIZE (240 * 5)
#define BUCKET_COUNT 100
#define NUM_LABELS 7
#define RECORDING_TABLE_SIZE 999  // Number of data points in recording table (float per row: 4 KB)

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
extern volatile bool blockAll;

// Input state
extern bool dropdown_active;
extern bool keyboardInputActive;  // Tracks if user is actively typing numbers
extern bool manualTabSelection;   // Tracks if user manually clicked a tab (disables encoder tab navigation)

// Utility charts
extern lv_obj_t *util_Arbit_chart;
extern lv_chart_series_t *util_Arbit_chart_series;

// Which arbitrary-waveform bank (0 or 1) the ARBT chart editor is currently
// drawing/editing on. Set by loadFromBank() when the user picks a page;
// touch-drag editing, "Clear", and waveform presets all target this bank.
extern int g_arbtActiveBank;

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
    lv_obj_t *switch_fun_only;      // Toggle for FUN-only mode (disable everything else)
    lv_obj_t *switch_fun_only_label;
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
    lv_obj_t *table_length_spinbox;  // Table length limit for FGen playback
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
