#pragma once

#include <lvgl.h>
#include <functional>
#include <cstddef>

// Auto-measurement sequencer for calibration procedures
// Provides step-by-step execution with timing control and logging

// Measurement context to share data between steps
struct measure_ctx_t
{
    int step = 0; // unused by sequencer, keep if you want
    double v0 = 0.0;
    double v1 = 0.0;
};

// Single step in a calibration sequence
struct SeqStep
{
    const char *label;             // e.g. "Setting voltage to 0V"
    uint32_t wait_ms_begin_to_end; // delay before we call end()
    uint32_t wait_ms_after_end;    // cooldown before next step
    std::function<void()> begin;   // optional action at step start
    std::function<void()> end;     // optional action at step end
};

// Internal sequencer state (opaque to users)
struct SeqRunner
{
    lv_timer_t *timer = nullptr;
    const SeqStep *steps = nullptr;
    size_t count = 0;
    size_t index = 0;     // current step
    bool in_begin = true; // phase: begin or end
    measure_ctx_t *mctx = nullptr;
};

// Start a sequence of calibration steps
// Returns the runner instance (for internal use)
SeqRunner *seq_start(lv_timer_t *t, const SeqStep *steps, size_t count, measure_ctx_t *mctx);

// Internal timer callback (public for technical reasons)
void seq_cb(lv_timer_t *t);
