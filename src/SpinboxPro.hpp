#pragma once
#include <functional>
#include <utility>
#include "lvgl.h"

class SpinboxPro
{
public:
    struct Config
    {
        const lv_font_t *spinboxFont = nullptr;
        const lv_font_t *labelFont = &lv_font_montserrat_12;
        bool clickBeep = true;
    };
    using BeepFn = std::function<void()>;

    SpinboxPro();                                           // default ctor
    explicit SpinboxPro(Config cfg, BeepFn beep = nullptr); // pass-by-value (OK with {})

    ~SpinboxPro() = default;

    // Create and wire a spinbox with label; returns the spinbox lv_obj_t*
    lv_obj_t *create(lv_obj_t *parent, const char *labelText,
                     int32_t range_min, int32_t range_max,
                     uint8_t digit_count, uint8_t separator_position,
                     lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs,
                     lv_coord_t width, uint8_t id);
    struct CreateInfo
    {
        lv_obj_t *parent = nullptr;
        const char *label = "";
        int32_t rangeMin = 0;
        int32_t rangeMax = 0;
        uint8_t digitCnt = 0;
        uint8_t sepPos = 0;
        lv_align_t align = LV_ALIGN_DEFAULT;
        lv_coord_t xOfs = 0;
        lv_coord_t yOfs = 0;
        lv_coord_t width = 0;
        uint8_t id = 0;
    };

    // Factory using CreateInfo (overload)
    lv_obj_t *create(const CreateInfo &ci);

    // -------- Fluent Builder --------
    class Builder
    {
    public:
        explicit Builder(SpinboxPro &spx) : spx_(spx) {}

        Builder &parent(lv_obj_t *p)
        {
            ci_.parent = p;
            return *this;
        }
        Builder &label(const char *txt)
        {
            ci_.label = txt;
            return *this;
        }
        Builder &range(int32_t minV, int32_t maxV)
        {
            ci_.rangeMin = minV;
            ci_.rangeMax = maxV;
            return *this;
        }
        Builder &digits(uint8_t count, uint8_t sep)
        {
            ci_.digitCnt = count;
            ci_.sepPos = sep;
            return *this;
        }
        Builder &align(lv_align_t a, lv_coord_t x, lv_coord_t y)
        {
            ci_.align = a;
            ci_.xOfs = x;
            ci_.yOfs = y;
            return *this;
        }
        Builder &width(lv_coord_t w)
        {
            ci_.width = w;
            return *this;
        }
        Builder &id(uint8_t v)
        {
            ci_.id = v;
            return *this;
        }

        lv_obj_t *create() { return spx_.create(ci_); }

    private:
        SpinboxPro &spx_;
        CreateInfo ci_;
    };

    // Query helpers
    lv_obj_t *findById(lv_obj_t *parent, uint32_t id) const;
    int32_t valueById(lv_obj_t *parent, uint32_t id) const;
    void setValueById(lv_obj_t *parent, uint32_t id, int32_t value);

    // Selection helpers
    lv_obj_t *selected() const { return selected_; }
    void clearSelection();

    // Cursor/value helpers (map to enc2/enc1)
    void cursorLeft(lv_obj_t *sb);  // select higher digit
    void cursorRight(lv_obj_t *sb); // select lower digit
    void increment(lv_obj_t *sb);
    void decrement(lv_obj_t *sb);

private:
    struct SpinboxData
    {
        uint8_t cursor_pos{0};
        uint8_t digit_count{0}; // last index (digit_count-1)
        uint8_t id{0};
    };

    // Event thunks
    static void onPressedThunk(lv_event_t *e);
    static void onSelectThunk(lv_event_t *e);
    static void onDeleteThunk(lv_event_t *e);

    // Instance handlers
    void handlePressed(lv_event_t *e);
    void handleSelect(lv_event_t *e);
    void handleDelete(lv_event_t *e);

    // Style helpers
    void ensureStyles();
    void addRedBorder(lv_obj_t *sb);
    void removeRedBorder(lv_obj_t *sb);

    // Config / DI
    Config cfg_;
    BeepFn beep_;

    // Styles (owned by this manager, live as long as it)
    bool stylesInited_{false};
    lv_style_t styleRedBorder_{};
    lv_style_t styleSpinbox_{};
    lv_style_t styleLabel_{};

    // Current selection
    lv_obj_t *selected_{nullptr};
};
