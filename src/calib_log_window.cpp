#include "calib_log_window.h"
#include "buzzer.h"
#include <cstring>
#include <cstdio>
#include <cstdarg>

// External references
extern bool lvglIsBusy;

// Logging state
static char s_logbuf[2048];
static size_t s_len = 0;
static bool s_pending = false; // true after _begin(), cleared by _done()

static lv_obj_t *log_win;
static lv_obj_t *log_label;

void close_log_cb(lv_timer_t *t)
{
    if (log_win)
    {
        lv_obj_del(log_win);
        log_win = NULL;
        log_label = NULL;
    }
    lv_timer_del(t);
}

static inline void log_update_label()
{
    s_logbuf[s_len] = '\0';
    lv_label_set_text(log_label, s_logbuf);
    lv_obj_scroll_to_view(log_label, LV_ANIM_OFF);
    myTone(NOTE_A3, 50, false);
}

// 1) Print: "1.  Setting volt to 0v ...\n"
void log_step_begin(const char *fmt, ...)
{
    // auto-close any previous pending step
    if (s_pending)
    {
        // replace the previous " ...\n" with " done!\n"
        // (safe even if buffer tight; see _done code)
        // You can call log_step_done(); but inline is faster.
        const char tail[] = " ...\n";
        const size_t tail_len = sizeof(tail) - 1;
        if (s_len >= tail_len && std::memcmp(s_logbuf + s_len - tail_len, tail, tail_len) == 0)
        {
            s_len -= tail_len;
            const char done[] = " done!\n";
            size_t add = strlen(done);
            if (s_len + add >= sizeof(s_logbuf))
                add = sizeof(s_logbuf) - s_len - 1;
            std::memcpy(s_logbuf + s_len, done, add);
            s_len += add;
        }
        s_pending = false;
    }

    // format the new line head: "1.  Setting volt to 0v"
    char line[256];
    va_list args;
    va_start(args, fmt);
    const int n = vsnprintf(line, sizeof(line), fmt, args);
    va_end(args);

    // append to the big buffer + " ...\n"
    const char ell[] = " ...\n";
    const size_t need = (size_t)((n > 0 ? n : 0)) + sizeof(ell) - 1;

    // truncate if needed
    size_t avail = sizeof(s_logbuf) - s_len - 1;
    size_t to_copy = avail > need ? (size_t)(n > 0 ? n : 0) : (avail > (sizeof(ell) - 1) ? avail - (sizeof(ell) - 1) : 0);

    if (to_copy > 0)
    {
        std::memcpy(s_logbuf + s_len, line, to_copy);
        s_len += to_copy;
    }
    // add " ...\n" if there is room
    size_t add = sizeof(ell) - 1;
    if (s_len + add >= sizeof(s_logbuf))
        add = sizeof(s_logbuf) - s_len - 1;
    std::memcpy(s_logbuf + s_len, ell, add);
    s_len += add;

    s_pending = true;
    log_update_label();
    myTone(NOTE_A5, 100, false);
}

// 2) Replace trailing " ...\n" with " done!\n" on the same line
void log_step_done()
{
    if (!s_pending)
        return;

    const char tail[] = " ...\n";
    const size_t tail_len = sizeof(tail) - 1;

    if (s_len >= tail_len && std::memcmp(s_logbuf + s_len - tail_len, tail, tail_len) == 0)
    {
        // remove " ...\n"
        s_len -= tail_len;

        // append " done!\n"
        const char done[] = " done!\n";
        size_t add = sizeof(done) - 1;
        if (s_len + add >= sizeof(s_logbuf))
            add = sizeof(s_logbuf) - s_len - 1;
        std::memcpy(s_logbuf + s_len, done, add);
        s_len += add;

        log_update_label();
    }
    s_pending = false;
    myTone(NOTE_A3, 50, false);
}

void log_step(const char *fmt, ...)
{
    if (!lvglIsBusy)
    {
        char buf[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        const char *old = lv_label_get_text(log_label);
        static char new_txt[1024];
        snprintf(new_txt, sizeof(new_txt), "%s%s\n", old, buf);

        lv_label_set_text(log_label, new_txt);
        lv_obj_scroll_to_view(log_label, LV_ANIM_OFF);
    }
}

void log_clear()
{
    if (log_label)
    {
        lv_label_set_text(log_label, "");              // wipe text
        lv_obj_scroll_to_y(log_label, 0, LV_ANIM_OFF); // scroll back to top
    }
}

void log_reset()
{
    s_len = 0;
    s_logbuf[0] = '\0';
    s_pending = false;
}

void create_log_window()
{
    log_win = lv_win_create(lv_scr_act(), 40);
    lv_win_add_title(log_win, "Measuring total internal resistor");

    lv_obj_t *cont = lv_win_get_content(log_win);
    log_label = lv_label_create(cont);
    lv_label_set_text(log_label, "");
    lv_label_set_long_mode(log_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(log_label, lv_pct(100));
}
