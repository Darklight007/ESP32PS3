#include "SpinboxPro.hpp"

// Default ctor -> delegates to value-ctor with defaults
SpinboxPro::SpinboxPro()
: SpinboxPro(SpinboxPro::Config{}, nullptr) {}

SpinboxPro::SpinboxPro(Config cfg, BeepFn beep)
: cfg_(std::move(cfg)), beep_(std::move(beep)) {}


// ---------------- private: styles ----------------
void SpinboxPro::ensureStyles() {
    if (stylesInited_) return;
    stylesInited_ = true;

    lv_style_init(&styleRedBorder_);
    lv_style_set_border_width(&styleRedBorder_, 2);
    lv_style_set_border_color(&styleRedBorder_, lv_color_hex(0xFF0000));
    lv_style_set_border_opa(&styleRedBorder_, LV_OPA_COVER);
    lv_style_set_radius(&styleRedBorder_, 3);

    lv_style_init(&styleSpinbox_);
    if (cfg_.spinboxFont) lv_style_set_text_font(&styleSpinbox_, cfg_.spinboxFont);
    lv_style_set_pad_ver(&styleSpinbox_, 2);
    lv_style_set_pad_left(&styleSpinbox_, 2);
    lv_style_set_pad_right(&styleSpinbox_, 0);
    lv_style_set_radius(&styleSpinbox_, 3);
    lv_style_set_border_width(&styleSpinbox_, 2);
    lv_style_set_border_color(&styleSpinbox_, lv_color_hex(0x8f8f8f));
    lv_style_set_text_color(&styleSpinbox_, lv_color_hex(0xFFFF00));

    lv_style_init(&styleLabel_);
    lv_style_set_text_font(&styleLabel_, cfg_.labelFont ? cfg_.labelFont : &lv_font_montserrat_12);
    lv_style_set_bg_color(&styleLabel_, lv_color_hex(0xFF0000));
}

void SpinboxPro::addRedBorder(lv_obj_t* sb)    { lv_obj_add_style(sb, &styleRedBorder_, LV_PART_MAIN); }
void SpinboxPro::removeRedBorder(lv_obj_t* sb) { if (sb) lv_obj_remove_style(sb, &styleRedBorder_, LV_PART_MAIN); }

// SpinboxPro.cpp (add this overload)
lv_obj_t* SpinboxPro::create(const CreateInfo& ci) {
    return create(ci.parent, ci.label,
                  ci.rangeMin, ci.rangeMax,
                  ci.digitCnt, ci.sepPos,
                  ci.align, ci.xOfs, ci.yOfs,
                  ci.width, ci.id);
}


// ---------------- public: create ----------------
lv_obj_t* SpinboxPro::create(lv_obj_t* parent, const char* labelText,
                             int32_t range_min, int32_t range_max,
                             uint8_t digit_count, uint8_t separator_position,
                             lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs,
                             lv_coord_t width, uint8_t id) {
    ensureStyles();

    // Spinbox
    lv_obj_t* sb = lv_spinbox_create(parent);
    lv_obj_set_width(sb, width);
    lv_obj_align_to(sb, parent, align, x_ofs, y_ofs);
    lv_spinbox_set_range(sb, range_min, range_max);
    lv_spinbox_set_digit_format(sb, digit_count, separator_position);
    lv_spinbox_set_cursor_pos(sb, 0);
    lv_spinbox_step_prev(sb);

    // Flags
    lv_obj_add_flag(sb, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_clear_flag(sb, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_clear_flag(sb, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_clear_flag(sb, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_clear_flag(sb, LV_OBJ_FLAG_SCROLLABLE);

    // Styles
    lv_obj_add_style(sb, &styleSpinbox_, LV_STATE_DEFAULT);

    // Label
    lv_obj_t* lbl = lv_label_create(sb);
    lv_label_set_recolor(lbl, true);
    lv_label_set_text(lbl, labelText ? labelText : "");
    lv_obj_align(lbl, LV_ALIGN_OUT_LEFT_TOP, -2, -20);
    lv_obj_add_style(lbl, &styleLabel_, LV_STATE_DEFAULT);

    // User data
    auto* data = static_cast<SpinboxData*>(lv_mem_alloc(sizeof(SpinboxData)));
    if (data) {
        data->cursor_pos = 0;
        data->digit_count = (digit_count > 0) ? (digit_count - 1) : 0;
        data->id = id;
        lv_obj_set_user_data(sb, data);
    }

    // Events (pass this via user_data of each cb)
    lv_obj_add_event_cb(sb, &SpinboxPro::onPressedThunk, LV_EVENT_SHORT_CLICKED, this);
    lv_obj_add_event_cb(sb, &SpinboxPro::onSelectThunk,  LV_EVENT_PRESSED,       this);
    lv_obj_add_event_cb(sb, &SpinboxPro::onDeleteThunk,  LV_EVENT_DELETE,        this);

    // No scrollbar
    lv_obj_set_scrollbar_mode(sb, LV_SCROLLBAR_MODE_OFF);

    return sb;
}

// ---------------- public: queries ----------------
lv_obj_t* SpinboxPro::findById(lv_obj_t* parent, uint32_t id) const {
    uint32_t child_cnt = lv_obj_get_child_cnt(parent);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(parent, i);
        auto* data = static_cast<SpinboxData*>(lv_obj_get_user_data(child));
        if (data && data->id == id) return child;
    }
    return nullptr;
}

int32_t SpinboxPro::valueById(lv_obj_t* parent, uint32_t id) const {
    if (lv_obj_t* obj = findById(parent, id)) return lv_spinbox_get_value(obj);
    return 0;
}

void SpinboxPro::setValueById(lv_obj_t* parent, uint32_t id, int32_t value) {
    if (lv_obj_t* obj = findById(parent, id)) {
        if (lv_obj_is_visible(obj)) lv_spinbox_set_value(obj, value);
    }
}

// ---------------- public: selection ----------------
void SpinboxPro::clearSelection() {
    if (selected_) {
        removeRedBorder(selected_);
        selected_ = nullptr;
    }
}

// ---------------- public: cursor/value helpers ----------------
void SpinboxPro::cursorLeft(lv_obj_t* sb)  { lv_spinbox_step_next(sb); } // higher digit
void SpinboxPro::cursorRight(lv_obj_t* sb) { lv_spinbox_step_prev(sb); } // lower digit
void SpinboxPro::increment(lv_obj_t* sb)   { if (lv_obj_is_visible(sb)) lv_spinbox_increment(sb); }
void SpinboxPro::decrement(lv_obj_t* sb)   { if (lv_obj_is_visible(sb)) lv_spinbox_decrement(sb); }

// ---------------- events: thunks ----------------
void SpinboxPro::onPressedThunk(lv_event_t* e) {
    auto* self = static_cast<SpinboxPro*>(lv_event_get_user_data(e));
    if (self) self->handlePressed(e);
}
void SpinboxPro::onSelectThunk(lv_event_t* e) {
    auto* self = static_cast<SpinboxPro*>(lv_event_get_user_data(e));
    if (self) self->handleSelect(e);
}
void SpinboxPro::onDeleteThunk(lv_event_t* e) {
    auto* self = static_cast<SpinboxPro*>(lv_event_get_user_data(e));
    if (self) self->handleDelete(e);
}

// ---------------- events: handlers ----------------
void SpinboxPro::handlePressed(lv_event_t* e) {
    if (cfg_.clickBeep && beep_) beep_();
}

void SpinboxPro::handleSelect(lv_event_t* e) {
    lv_obj_t* obj = lv_event_get_target(e);
    ensureStyles();
    if (selected_ && selected_ != obj) removeRedBorder(selected_);
    addRedBorder(obj);
    selected_ = obj;
}

void SpinboxPro::handleDelete(lv_event_t* e) {
    lv_obj_t* sb = lv_event_get_target(e);
    if (selected_ == sb) selected_ = nullptr;

    auto* data = static_cast<SpinboxData*>(lv_obj_get_user_data(sb));
    if (data) {
        lv_mem_free(data);
        lv_obj_set_user_data(sb, nullptr);
    }
}
