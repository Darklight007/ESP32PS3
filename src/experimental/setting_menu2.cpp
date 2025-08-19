#include "setting_menu2.h"

// ===== heavy/private includes live ONLY here =====
#include "spinbox_pro.h"
#include "version.h"
#include "buzzer.h"
#include "TFT_eSPI.h"
#include "tabs.h"
#include "device.h"          // your backend types (if you still need them)
#include "lv_gui_helper.h"

#include <cmath>
#include <map>
#include <new>               // std::nothrow

namespace ui::settings {

// ----------------- internal context -----------------
struct Ctx {
  Deps deps;
  Handles h;

  // minor state
  bool  isMyTextBoxHidden{false};
  int8_t lastButton{0};

  // styles
  lv_style_t style_hide{};
  lv_style_t style_unhide{};
  lv_style_t style_border_def{};
  lv_style_t style_border_select{};
  bool styles_inited{false};

  // DAC window spinboxes + labels
  lv_obj_t *s_zv=nullptr,*s_mv=nullptr,*s_zc=nullptr,*s_mc=nullptr;
  lv_obj_t *s_label_maxV=nullptr,*s_label_maxC=nullptr;

  // ADC calib GUI structs you already have
  setting_GUI Calib_GUI_voltage{};
  setting_GUI Calib_GUI_current{};
};

// Keep a weak pointer for the optional legacy bridge:
static Ctx* g_last_ctx = nullptr;
const Handles* LastHandles() { return g_last_ctx ? &g_last_ctx->h : nullptr; }

// ----------------- tiny helpers -----------------
double get_m(double code1, double code2, double vin1, double vin2) { return (code2 - code1) / (vin2 - vin1); }
double get_b(double code,  double m,     double vin)  { return code - m * vin; }

static void tone_pressed(lv_event_t*)  { myTone(NOTE_A4, 5); }
static void tone_released(lv_event_t*) { myTone(NOTE_A4, 5); }

static Ctx* ctx_from(lv_event_t* e) {
  return static_cast<Ctx*>(lv_event_get_user_data(e));
}

static void on_delete_free_ctx(lv_event_t* e) {
  if (lv_event_get_code(e) != LV_EVENT_DELETE) return;
  auto* ctx = ctx_from(e);
  if (g_last_ctx == ctx) g_last_ctx = nullptr;
  delete ctx;
}

// ----------------- slider handlers -----------------
static void slider_backlight_event_cb(lv_event_t* e) {
  auto* slider = lv_event_get_target(e);
  int32_t v = lv_slider_get_value(slider);
  char buf[8]; lv_snprintf(buf, sizeof(buf), "%d", v);
  lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), buf);
  // If you wire your backlight PWM: ledcWrite(lcdBacklightChannel, v);
}

static void slider_adcRate_event_cb(lv_event_t* e) {
  auto* ctx = ctx_from(e);
  auto* slider = lv_event_get_target(e);
  uint8_t idx = lv_slider_get_value(slider); // 0..3
  static const char* ADC_SPS[] = {"20","90","330","1000"};
  lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), ADC_SPS[idx]);
  if (ctx->deps.set_adc_sps) ctx->deps.set_adc_sps(idx);
  if (ctx->deps.reset_stats) ctx->deps.reset_stats();
}

static void slider_adcAVG_event_cb(lv_event_t* e) {
  auto* ctx = ctx_from(e);
  auto* slider = lv_event_get_target(e);
  uint16_t exp = lv_slider_get_value(slider);
  char buf[6]; lv_snprintf(buf, sizeof(buf), "%-d", (int)std::pow(2, exp));
  lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), buf);
  if (ctx->deps.set_avg_pow2) ctx->deps.set_avg_pow2(exp);
}

static void slider_decimalPoints_event_cb(lv_event_t* e) {
  auto* ctx = ctx_from(e);
  auto* slider = lv_event_get_target(e);
  uint16_t digits = lv_slider_get_value(slider); // 1..4
  char buf[4]; lv_snprintf(buf, sizeof(buf), "%u", (unsigned)digits);
  lv_label_set_text(lv_obj_get_child(lv_obj_get_parent(slider), 1), buf);
  if (ctx->deps.set_digits) ctx->deps.set_digits(digits);
}

// ----------------- switch handlers -----------------
static void switch_buzzer_event_cb(lv_event_t* e) {
  auto* ctx = ctx_from(e);
  if (lv_event_get_code(e) != LV_EVENT_SHORT_CLICKED) return;
  auto* sw = lv_event_get_target(e);
  if (ctx->deps.buzzer_flag) {
    *ctx->deps.buzzer_flag = lv_obj_has_state(sw, LV_STATE_CHECKED);
    myTone(*ctx->deps.buzzer_flag ? NOTE_A5 : NOTE_A3, 10, *ctx->deps.buzzer_flag);
  }
}

// ----------------- Save/Load handlers -----------------
static void save_cb(lv_event_t* e) {
  auto* ctx = ctx_from(e);
  if (ctx->deps.save_calibration) ctx->deps.save_calibration();
  tone_pressed(e);
}
static void load_cb(lv_event_t* e) {
  auto* ctx = ctx_from(e);
  if (ctx->deps.load_calibration) ctx->deps.load_calibration();

  // Refresh visible ADC windows with prefilled values
  if (ctx->h.win_ADC_voltage_calibration &&
      !lv_obj_has_flag(ctx->h.win_ADC_voltage_calibration, LV_OBJ_FLAG_HIDDEN) &&
      ctx->deps.prefill_voltage_cal) {
    int c1{}, c2{}; double v1{}, v2{};
    ctx->deps.prefill_voltage_cal(&c1,&c2,&v1,&v2);
    lv_spinbox_set_value(ctx->Calib_GUI_voltage.code_1, c1);
    lv_spinbox_set_value(ctx->Calib_GUI_voltage.code_2, c2);
    lv_spinbox_set_value(ctx->Calib_GUI_voltage.vin_1, (int32_t)llround(10000.0 * v1));
    lv_spinbox_set_value(ctx->Calib_GUI_voltage.vin_2, (int32_t)llround(10000.0 * v2));
  }
  if (ctx->h.win_ADC_current_calibration &&
      !lv_obj_has_flag(ctx->h.win_ADC_current_calibration, LV_OBJ_FLAG_HIDDEN) &&
      ctx->deps.prefill_current_cal) {
    int c1{}, c2{}; double v1{}, v2{};
    ctx->deps.prefill_current_cal(&c1,&c2,&v1,&v2);
    lv_spinbox_set_value(ctx->Calib_GUI_current.code_1, c1);
    lv_spinbox_set_value(ctx->Calib_GUI_current.code_2, c2);
    lv_spinbox_set_value(ctx->Calib_GUI_current.vin_1, (int32_t)llround(10000.0 * v1));
    lv_spinbox_set_value(ctx->Calib_GUI_current.vin_2, (int32_t)llround(10000.0 * v2));
  }

  tone_pressed(e);
}

// ----------------- DAC change handlers -----------------
static void DAC_voltage_calib_change_event_cb(lv_event_t* e) {
  auto* ctx = ctx_from(e);
  if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;

  double zv = lv_spinbox_get_value(ctx->s_zv);
  double mv = lv_spinbox_get_value(ctx->s_mv);
  lv_label_set_text_fmt(ctx->s_label_maxV, "%+6.4fV", (mv - zv) / 2000.0);

  if (ctx->deps.apply_dac_voltage_limits) {
    ctx->deps.apply_dac_voltage_limits((int)zv,(int)mv);
  }
  if (ctx->deps.save_dac_data) {
    DAC_codes d = ctx->deps.load_dac_data ? ctx->deps.load_dac_data() : DAC_codes{};
    // update & persist
    // (Assume fields exist; adapt to your struct.)
    d.zero_voltage = (int)zv;
    d.max_voltage  = (int)mv;
    ctx->deps.save_dac_data(d);
  }
}

static void DAC_current_calib_change_event_cb(lv_event_t* e) {
  auto* ctx = ctx_from(e);
  if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;

  double zc = lv_spinbox_get_value(ctx->s_zc);
  double mc = lv_spinbox_get_value(ctx->s_mc);
  lv_label_set_text_fmt(ctx->s_label_maxC, "%+6.4fA", (mc - zc) / 10000.0);

  if (ctx->deps.apply_dac_current_limits) {
    ctx->deps.apply_dac_current_limits((int)zc,(int)mc);
  }
  if (ctx->deps.save_dac_data) {
    DAC_codes d = ctx->deps.load_dac_data ? ctx->deps.load_dac_data() : DAC_codes{};
    d.zero_current = (int)zc;
    d.max_current  = (int)mc;
    ctx->deps.save_dac_data(d);
  }
}

// ----------------- LCD touch calibration -----------------
static void LCD_Calibration_cb(lv_event_t* e) {
  auto* ctx = ctx_from(e);
  if (!ctx->deps.tft) return;

  uint16_t calData[5];
  ctx->deps.tft->fillScreen(TFT_BLACK);
  ctx->deps.tft->setCursor(20, 0);
  ctx->deps.tft->setTextFont(2);
  ctx->deps.tft->setTextSize(1);
  ctx->deps.tft->setTextColor(TFT_WHITE, TFT_BLACK);
  ctx->deps.tft->println("Touch corners as indicated");
  ctx->deps.tft->setTextFont(1);
  ctx->deps.tft->println();
  delay(2000);
  ctx->deps.tft->calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
  ctx->deps.tft->setTouch(calData);
  lv_obj_invalidate(Tabs::tabview);  // refresh LVGL
}

// ----------------- small UI factories -----------------
static lv_obj_t* create_text(lv_obj_t* parent, const char* icon, const char* txt) {
  (void)icon;
  auto* row = lv_menu_cont_create(parent);
  auto* label = lv_label_create(row);
  lv_label_set_text(label, txt);
  lv_obj_set_height(label, 11);
  return row;
}

static lv_obj_t* create_switch_item(lv_obj_t* parent, const char* txt, bool checked,
                                    lv_event_cb_t cb, void* user) {
  auto* row = lv_menu_cont_create(parent);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  auto* label = lv_label_create(row);
  lv_label_set_text(label, txt);
  lv_obj_set_flex_grow(label, 2);

  auto* sw = lv_switch_create(row);
  if (checked) lv_obj_add_state(sw, LV_STATE_CHECKED);
  lv_obj_add_flag(sw, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);

  lv_obj_add_event_cb(sw, tone_pressed,  LV_EVENT_PRESSED,  nullptr);
  lv_obj_add_event_cb(sw, tone_released, LV_EVENT_RELEASED, nullptr);
  lv_obj_add_event_cb(sw, cb, LV_EVENT_SHORT_CLICKED, user);
  return row;
}

static lv_obj_t* create_slider_item(lv_obj_t* parent, const char* txt,
                                    int32_t min, int32_t max, int32_t val,
                                    lv_event_cb_t cb, void* user) {
  auto* row = lv_menu_cont_create(parent);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END);

  auto* label = lv_label_create(row);
  lv_label_set_text(label, txt);
  lv_obj_set_flex_grow(label, 2);

  auto* val_lbl = lv_label_create(row);
  lv_label_set_text(val_lbl, "0");

  auto* slider = lv_slider_create(row);
  lv_obj_set_flex_grow(slider, 15);
  lv_obj_add_flag(slider, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
  lv_slider_set_range(slider, min, max);
  lv_slider_set_value(slider, val, LV_ANIM_OFF);

  lv_obj_add_event_cb(slider, cb, LV_EVENT_VALUE_CHANGED, user);
  lv_obj_add_event_cb(slider, tone_pressed,  LV_EVENT_PRESSED,  nullptr);
  lv_obj_add_event_cb(slider, tone_released, LV_EVENT_RELEASED, nullptr);

  // reflect initial label text
  lv_event_send(slider, LV_EVENT_VALUE_CHANGED, nullptr);
  return row;
}

static lv_obj_t* create_button_item(lv_obj_t* parent, lv_event_cb_t cb, void* user, const char* caption) {
  auto* row = lv_menu_cont_create(parent);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_all(row, 3, LV_PART_ITEMS);
  lv_obj_set_style_pad_all(row, 3, LV_PART_MAIN);

  auto* btn = lv_btn_create(row);
  lv_obj_set_size(btn, 120, 30);
  auto* label = lv_label_create(btn);
  lv_label_set_text(label, caption);
  lv_obj_center(label);

  lv_obj_add_event_cb(btn, tone_pressed,  LV_EVENT_PRESSED,  nullptr);
  lv_obj_add_event_cb(btn, tone_released, LV_EVENT_RELEASED, nullptr);
  if (cb) lv_obj_add_event_cb(btn, cb, LV_EVENT_RELEASED, user);
  return row;
}

// ----------------- ADC calibration window builder -----------------
struct CalPrefill { int32_t code1, code2; double val1, val2; const char* unit; };

static void build_adc_calibration_window(Ctx* ctx, lv_obj_t** win_holder,
                                         const char* title,
                                         setting_GUI& gui,
                                         const CalPrefill& pf)
{
  if (*win_holder) {
    lv_obj_clear_flag(*win_holder, LV_OBJ_FLAG_HIDDEN);
    return;
  }

  auto* win = lv_win_create(lv_scr_act(), 32);
  *win_holder = win;
  lv_obj_set_size(win, 320, 226);
  lv_win_add_title(win, title);

  auto* close = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 60);
  lv_obj_add_event_cb(close, [](lv_event_t* e){
    auto* btn = lv_event_get_target(e);
    lv_obj_add_flag(lv_obj_get_parent(lv_obj_get_parent(btn)), LV_OBJ_FLAG_HIDDEN);
  }, LV_EVENT_CLICKED, nullptr);

  auto* cont = lv_win_get_content(win);
  lv_obj_set_style_pad_all(cont, 0, LV_PART_ITEMS);
  lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);

  // spinboxes
  lv_point_t offset{10, 20};
  lv_coord_t w = 110, col = w + 30, row = 38;

  gui.vin_1  = spinbox_pro(cont, "#FFFFF7 Vin1:#",  -10000, 400000, 6, 2, LV_ALIGN_TOP_LEFT, offset.x,           offset.y,           w, 10);
  gui.code_1 = spinbox_pro(cont, "#FFFFF7 Code1:#", -10000, 8388608, 7, 0, LV_ALIGN_TOP_LEFT, offset.x,           offset.y + row,     w, 11);
  gui.vin_2  = spinbox_pro(cont, "#FFFFF7 Vin2:#",  -10000, 400000, 6, 2, LV_ALIGN_TOP_LEFT, offset.x + col,     offset.y,           w, 12);
  gui.code_2 = spinbox_pro(cont, "#FFFFF7 Code2:#", -10000, 8388608, 7, 0, LV_ALIGN_TOP_LEFT, offset.x + col,     offset.y + row,     w, 13);

  lv_spinbox_set_value(gui.code_1, pf.code1);
  lv_spinbox_set_value(gui.code_2, pf.code2);
  lv_spinbox_set_value(gui.vin_1,  (int32_t)llround(10000.0 * pf.val1));
  lv_spinbox_set_value(gui.vin_2,  (int32_t)llround(10000.0 * pf.val2));

  // Value/AVG/ER labels
  auto* lbl_raw  = LVLabel::create(cont, "Raw Code:", gui.code_1, 0, 5, nullptr);
  gui.lbl_rawCode     = LVLabel::create(cont, "#FFFFF7 00000000#", lbl_raw, 0, 3, nullptr);
  auto* lbl_rawA      = LVLabel::create(cont, "Avg Raw:", gui.lbl_rawCode, 0, 5, nullptr);
  gui.lbl_rawAVG_     = LVLabel::create(cont, "+00000000", lbl_rawA, 0, 3, nullptr);
  auto* lbl_calV      = LVLabel::create(cont, "Calibrated Value:", gui.code_2, 0, 5, nullptr);
  gui.lbl_calibratedValue = LVLabel::create(cont, (pf.unit && pf.unit[0]=='A')?"0.0000A":"0.0000V", lbl_calV, 0, 3, nullptr);
  auto* lbl_cvA       = LVLabel::create(cont, "Avg Calibrated Value:", gui.lbl_calibratedValue, 0, 5, nullptr);
  gui.lbl_calibValueAVG_  = LVLabel::create(cont, "+00.0000", lbl_cvA, 0, 3, nullptr);
  auto* lbl_er        = LVLabel::create(cont, "Effective Resolution:", gui.lbl_rawAVG_, 0, 5, nullptr);
  gui.lbl_ER          = LVLabel::create(cont, "00.00", lbl_er, 0, 3, nullptr);

  // Save/Load
  LVButton btnLoad(cont, "Load", 170, 160, 54, 26, nullptr, load_cb);
  LVButton btnSave(cont, "Save", 230, 160, 54, 26, nullptr, save_cb);
  lv_obj_add_event_cb(btnLoad.btn, load_cb, LV_EVENT_CLICKED, ctx);
  lv_obj_add_event_cb(btnSave.btn, save_cb, LV_EVENT_CLICKED, ctx);

  // Display the simple m,b formulas
  auto* label_m_num = lv_label_create(cont); lv_label_set_text(label_m_num, "Code2 - Code1");
  auto* label_m     = lv_label_create(cont); lv_label_set_text(label_m,     "m = --------------------- =");
  auto* label_m_den = lv_label_create(cont); lv_label_set_text(label_m_den, "Vin2 - Vin1");
  lv_obj_align(label_m_num, LV_ALIGN_TOP_LEFT, 35, 210);
  lv_obj_align(label_m,     LV_ALIGN_TOP_LEFT,  0, 220);
  lv_obj_align(label_m_den, LV_ALIGN_TOP_LEFT, 50, 230);

  gui.lbl_calib_m = lv_label_create(cont);
  gui.lbl_calib_b = lv_label_create(cont);
  lv_obj_align(gui.lbl_calib_m, LV_ALIGN_TOP_LEFT, 160, 220);
  auto* label_b   = lv_label_create(cont);
  lv_label_set_text(label_b, "b = Code1 - m * Vin1 =");
  lv_obj_align(label_b, LV_ALIGN_TOP_LEFT, 0, 250);
  lv_obj_align(gui.lbl_calib_b, LV_ALIGN_TOP_LEFT, 160, 250);

  auto* label_vin = lv_label_create(cont);
  lv_label_set_text(label_vin, "Vin_cal = (Code-b)/m");
  lv_obj_align(label_vin, LV_ALIGN_TOP_LEFT, 0, 280);
}

// ----------------- DAC window open -----------------
static void open_dac_calibration_cb(lv_event_t* e) {
  auto* ctx = ctx_from(e);

  if (ctx->h.win_DAC_calibration) {
    lv_obj_clear_flag(ctx->h.win_DAC_calibration, LV_OBJ_FLAG_HIDDEN);
    return;
  }

  ctx->h.win_DAC_calibration = lv_win_create(lv_scr_act(), 36);
  auto* win = ctx->h.win_DAC_calibration;
  lv_obj_set_size(win, 320, 226);
  lv_win_add_title(win, "DAC Calibration");

  auto* close = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 60);
  lv_obj_add_event_cb(close, [](lv_event_t* e2){
    auto* btn = lv_event_get_target(e2);
    lv_obj_add_flag(lv_obj_get_parent(lv_obj_get_parent(btn)), LV_OBJ_FLAG_HIDDEN);
  }, LV_EVENT_CLICKED, nullptr);

  auto* cont = lv_win_get_content(win);
  lv_obj_set_style_pad_all(cont, 0, LV_PART_ITEMS);
  lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

  auto* title = lv_label_create(cont);
  lv_label_set_text(title, "DAC Code      Output V/C");
  lv_obj_align(title, LV_ALIGN_OUT_LEFT_TOP, 95, 10);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_color(title, lv_color_hex(0x00FFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

  int xPos = 94, yPos = 40, yOffset = 25, xOffset = 10;

  ctx->s_zv = spinbox_pro(cont, "#FFFFF7 Zero Voltage:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 0, 86, 15);
  ctx->s_mv = spinbox_pro(cont, "#FFFFF7 Max  Voltage:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 1, 86, 16);
  ctx->s_zc = spinbox_pro(cont, "#FFFFF7 Zero Current:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 2.3, 86, 17);
  ctx->s_mc = spinbox_pro(cont, "#FFFFF7 Max  Current:#", 0, 65535, 5, 0, LV_ALIGN_DEFAULT, xPos, yPos + yOffset * 3.3, 86, 18);

  auto addLabel = [&](lv_obj_t* parent, const char* fmt, double value) -> lv_obj_t* {
    auto* parent_child = lv_obj_get_child(parent, 1);
    lv_obj_align(parent_child, LV_ALIGN_OUT_RIGHT_MID, -90, 0);
    auto* lbl = lv_label_create(parent);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_align_to(lbl, parent, LV_ALIGN_OUT_RIGHT_MID, xOffset, 0);
    lv_label_set_text_fmt(lbl, fmt, value);
    lv_obj_set_style_text_font(lbl, &graph_R_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    return lbl;
  };

  addLabel(ctx->s_zv, "+%07.4fV", 0.0);
  ctx->s_label_maxV = addLabel(ctx->s_mv, "+%07.4fV", 32.750);
  addLabel(ctx->s_zc, "%+06.4fA", 0.0);
  ctx->s_label_maxC = addLabel(ctx->s_mc, "%+06.4fA", 6.5535);

  // Load current DAC data from deps, if provided
  DAC_codes d = ctx->deps.load_dac_data ? ctx->deps.load_dac_data() : DAC_codes{};
  lv_spinbox_set_value(ctx->s_zv, d.zero_voltage);
  lv_spinbox_set_value(ctx->s_mv, d.max_voltage);
  lv_spinbox_set_value(ctx->s_zc, d.zero_current);
  lv_spinbox_set_value(ctx->s_mc, d.max_current);

  // Register change callbacks and carry ctx
  lv_obj_add_event_cb(ctx->s_zv, DAC_voltage_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, ctx);
  lv_obj_add_event_cb(ctx->s_mv, DAC_voltage_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, ctx);
  lv_obj_add_event_cb(ctx->s_zc, DAC_current_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, ctx);
  lv_obj_add_event_cb(ctx->s_mc, DAC_current_calib_change_event_cb, LV_EVENT_VALUE_CHANGED, ctx);
}

// ----------------- Public entry -----------------
void Build(lv_obj_t* parent, const Deps& deps, Handles& out) {
  // Create and attach a context
  auto* ctx = new(std::nothrow) Ctx{};
  *ctx = Ctx{deps, Handles{}};
  g_last_ctx = ctx;

  // Menu root
  auto* menu = lv_menu_create(parent);
  ctx->h.menu = menu;
  lv_menu_set_mode_root_back_btn(menu, LV_MENU_ROOT_BACK_BTN_DISABLED);
  lv_obj_set_size(menu, 294, 200);
  lv_obj_center(menu);

  // (a) Display page
  auto* sub_display = lv_menu_page_create(menu, nullptr);
  auto* section = lv_menu_section_create(sub_display);
  create_slider_item(section, "Backlight", 0, 255, 255, slider_backlight_event_cb, ctx);

  // (b) Sound page
  auto* sub_sound = lv_menu_page_create(menu, nullptr);
  section = lv_menu_section_create(sub_sound);
  create_switch_item(section, "Buzzer", deps.buzzer_flag && *deps.buzzer_flag, switch_buzzer_event_cb, ctx);

  // (c) Measure page
  auto* sub_measure = lv_menu_page_create(menu, nullptr);
  section = lv_menu_section_create(sub_measure);
  create_slider_item(section, "ADC SPS", 0, 3, 0 /* set your current idx */, slider_adcRate_event_cb, ctx);
  ctx->h.slider_Avgs = create_slider_item(section, "ADC # of Avgs", 0, (int)std::log2(MAX_NO_OF_AVG), 0 /* exp */, slider_adcAVG_event_cb, ctx);
  create_slider_item(section, "Number of Digits", 1, 4, 3 /* digits */, slider_decimalPoints_event_cb, ctx);
  // Example extra toggle
  create_switch_item(section, "Auto Bar-Graph", false, [](lv_event_t*){}, ctx);

  // (d) Calibration page
  auto* sub_cal = lv_menu_page_create(menu, nullptr);
  section = lv_menu_section_create(sub_cal);

  // ADC Voltage window
  create_button_item(section, [](lv_event_t* e){
    auto* ctx = ctx_from(e);
    int c1{},c2{}; double v1{},v2{};
    if (ctx->deps.prefill_voltage_cal) ctx->deps.prefill_voltage_cal(&c1,&c2,&v1,&v2);
    CalPrefill pf{c1,c2,v1,v2,"V"};
    build_adc_calibration_window(ctx, &ctx->h.win_ADC_voltage_calibration,
                                 "ADC Voltage Calibration", ctx->Calib_GUI_voltage, pf);
  }, ctx, "ADC Voltage");

  // ADC Current window
  create_button_item(section, [](lv_event_t* e){
    auto* ctx = ctx_from(e);
    int c1{},c2{}; double v1{},v2{};
    if (ctx->deps.prefill_current_cal) ctx->deps.prefill_current_cal(&c1,&c2,&v1,&v2);
    CalPrefill pf{c1,c2,v1,v2,"A"};
    build_adc_calibration_window(ctx, &ctx->h.win_ADC_current_calibration,
                                 "ADC Current Calibration", ctx->Calib_GUI_current, pf);
  }, ctx, "ADC Current");

  // DAC window (private)
  create_button_item(section, open_dac_calibration_cb, ctx, "V/I DAC");

  // Extra utilities
  create_button_item(section, [](lv_event_t* e){ auto* c=ctx_from(e); if(c->deps.reset_stats) c->deps.reset_stats(); }, ctx, "Reset Stats");
  create_button_item(section, LCD_Calibration_cb, ctx, "LCD Touch");

  // (e) Save/Load page
  auto* sub_save_load = lv_menu_page_create(menu, nullptr);
  section = lv_menu_section_create(sub_save_load);
  ctx->h.btn_load = create_button_item(section, load_cb, ctx, "Load");
  create_button_item(section, save_cb, ctx, "Save");

  // (f) About page
  auto* sub_software_info_page = lv_menu_page_create(menu, nullptr);
  section = lv_menu_section_create(sub_software_info_page);
  char version_text[48];
  lv_snprintf(version_text, sizeof(version_text), "Version %s", SOFTWARE_VERSION);
  create_text(section, nullptr, version_text);

  auto* sub_about_page = lv_menu_page_create(menu, nullptr);
  section = lv_menu_section_create(sub_about_page);
  auto* obj0 = create_text(section, nullptr, "Software information");
  lv_menu_set_load_page_event(menu, obj0, sub_software_info_page);

  // Root page (sidebar)
  auto* root = lv_menu_page_create(menu, nullptr);
  ctx->h.root_page = root;
  section = lv_menu_section_create(root);

  auto mk = [&](const char* label, lv_obj_t* page){
    auto* itm = create_text(section, nullptr, label);
    lv_menu_set_load_page_event(menu, itm, page);
  };
  mk("Display",     sub_display);
  mk("Sound",       sub_sound);
  mk("Measure",     sub_measure);
  mk("Calibration", sub_cal);
  mk("Save/Load",   sub_save_load);
  mk("About",       sub_about_page);

  // Show sidebar
  lv_menu_set_sidebar_page(menu, root);
  // Cleanup context when menu is deleted
  lv_obj_add_event_cb(menu, on_delete_free_ctx, LV_EVENT_DELETE, ctx);

  // Export handles
  out = ctx->h;
}

} // namespace ui::settings
