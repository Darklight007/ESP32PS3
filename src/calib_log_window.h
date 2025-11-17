#pragma once

#include <lvgl.h>

// Logging window system for calibration procedures
// Provides step-by-step progress logging with visual feedback

// Begin a new logging step (shows "..." while in progress)
void log_step_begin(const char *fmt, ...);

// Mark current step as complete (changes "..." to "done!")
void log_step_done();

// Add a simple log line without step tracking
void log_step(const char *fmt, ...);

// Clear all log text
void log_clear();

// Reset the internal log state (clears buffer and pending flag)
void log_reset();

// Create the logging window UI
void create_log_window();

// Timer callback to close the log window (for use with lv_timer_create)
void close_log_cb(lv_timer_t *t);
