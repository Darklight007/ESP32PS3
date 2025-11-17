#include "calib_sequencer.h"
#include "calib_log_window.h"
#include <Arduino.h>

// Create and start the sequencer runner
SeqRunner *seq_start(lv_timer_t *t, const SeqStep *steps, size_t count, measure_ctx_t *mctx)
{
    auto *r = static_cast<SeqRunner *>(lv_mem_alloc(sizeof(SeqRunner)));
    if (!r)
    {
        Serial.println("ERROR: Failed to allocate memory!");
        lv_timer_del(t);
        return nullptr;
    }
    r->timer = t;
    r->steps = steps;
    r->count = count;
    r->index = 0;
    r->in_begin = true;
    r->mctx = mctx;
    t->user_data = r; // bind runner to timer

    // run first tick "now"
    lv_timer_set_repeat_count(t, -1);
    lv_timer_set_period(t, 1);
    return r;
}

// The LVGL timer callback drives the sequence
void seq_cb(lv_timer_t *t)
{
    auto *r = static_cast<SeqRunner *>(t->user_data);
    if (!r || r->index >= r->count)
    {
        // done
        lv_timer_del(t);
        if (r)
            lv_mem_free(r);
        return;
    }

    const SeqStep &s = r->steps[r->index];

    if (r->in_begin)
    {
        // BEGIN PHASE
        log_step_begin("%u.  %s", (unsigned)(r->index + 1), s.label);
        if (s.begin)
            s.begin(); // user begin action
        lv_timer_set_period(t, s.wait_ms_begin_to_end);
        r->in_begin = false; // next time: end phase
        return;
    }
    else
    {
        // END PHASE
        log_step_done();
        if (s.end)
            s.end(); // user end action
        lv_timer_set_period(t, s.wait_ms_after_end);
        r->in_begin = true; // next time: begin of next step
        r->index += 1;

        if (r->index >= r->count)
        {
            // finished whole table; stop timer
            lv_timer_del(t);
            lv_mem_free(r);
        }
        return;
    }
}
