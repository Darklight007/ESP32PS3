#include "globals.h"

// Global variable definitions
lv_coord_t graph_data_V[CHART_SIZE] = {0};
lv_coord_t graph_data_I[CHART_SIZE] = {0};

int32_t barGraph_V[1];
int32_t barGraph_I[1];

lv_obj_t *tabview_utility;
lv_obj_t *btn_function_gen;

int globalSliderXValue;
int32_t encoder1_value = 0, encoder2_value = 0;

struct objs_list Utility_objs;

// Input state
bool dropdown_active = false;
bool keyboardInputActive = false;

// Utility charts
lv_obj_t *util_Arbit_chart;
lv_chart_series_t *util_Arbit_chart_series;

// Bucket and label data definitions
uint16_t dataBuckets[BUCKET_COUNT];
char tickLabels_x[NUM_LABELS][10];
