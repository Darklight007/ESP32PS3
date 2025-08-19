#pragma once
#include <cstdint>
#include <lvgl.h>

// Forward decls to keep the header light.
struct DAC_codes;
class TFT_eSPI;

namespace ui::settings {

/// Hooks the settings module needs from your app.
/// Prefer function pointers here so this module does not depend on your Device type.
struct Deps {
  // --- Optional: simple shared state you want toggled from UI ---
  bool* buzzer_flag{};         // if non-null, UI toggles *buzzer_flag

  // --- Optional: display/touch hook for LCD calibration page ---
  TFT_eSPI* tft{};             // used only by the "LCD Touch" action

  // --- Required ops used by the UI (wire these to your backend) ---
  void (*set_adc_sps)(int idx)                           = nullptr; // idx: 0..3 (20/90/330/1000 SPS)
  void (*set_avg_pow2)(uint16_t exp)                     = nullptr; // window = 2^exp
  void (*set_digits)(uint16_t digits)                    = nullptr; // 1..4
  void (*reset_stats)()                                  = nullptr;

  void (*save_calibration)()                             = nullptr;
  void (*load_calibration)()                             = nullptr;

  // DAC calibration persistence + “apply” hooks
  DAC_codes (*load_dac_data)()                           = nullptr;
  void (*save_dac_data)(const DAC_codes&)                = nullptr;
  void (*apply_dac_voltage_limits)(int zero_code, int max_code) = nullptr;
  void (*apply_dac_current_limits)(int zero_code, int max_code) = nullptr;

  // Prefill ADC calibration windows (your code fills code1,code2,val1,val2)
  void (*prefill_voltage_cal)(int* code1,int* code2,double* val1,double* val2) = nullptr;
  void (*prefill_current_cal)(int* code1,int* code2,double* val1,double* val2) = nullptr;
};

/// Convenient references to the widgets this module creates.
/// (Return these so other files can *optionally* interact without globals.)
struct Handles {
  lv_obj_t* menu{};
  lv_obj_t* root_page{};

  // Calibration popups (hidden when not used)
  lv_obj_t* win_DAC_calibration{};
  lv_obj_t* win_ADC_voltage_calibration{};
  lv_obj_t* win_ADC_current_calibration{};

  // A few knobs other code might touch
  lv_obj_t* slider_Avgs{};
  lv_obj_t* btn_load{};

  // Optional scratch UI (if you create it here)
  lv_obj_t* myTextBox{};
};

/// Build the settings UI into `parent`. Fills `out` with handles you can keep.
void Build(lv_obj_t* parent, const Deps& deps, Handles& out);

// Small math helpers (exported because you used them from outside before)
double get_m(double code1, double code2, double vin1, double vin2);
double get_b(double code,  double m,     double vin);

// Optional legacy bridge: returns last built handles (nullptr if none).
// Prefer holding your own `Handles`, but this eases migration.
const Handles* LastHandles();

} // namespace ui::settings
