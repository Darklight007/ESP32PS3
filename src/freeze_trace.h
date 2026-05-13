#pragma once

// ============================================================================
// Freeze-trace instrumentation — set to 0 to compile away to NOTHING.
//   1 → TRACE() writes a per-core last-op pointer + bumps a counter (~10 ns)
//        and freeze_trace_periodic_print() prints every 250 ms.
//   0 → TRACE() becomes a no-op, the print function becomes an empty inline.
//        Zero runtime cost; no symbols left in the binary.
// ============================================================================
#ifndef FREEZE_TRACE_ENABLED
#define FREEZE_TRACE_ENABLED 1
#endif

#if FREEZE_TRACE_ENABLED

#include <Arduino.h>
#include <esp32-hal.h>  // xPortGetCoreID

extern volatile uint32_t g_traceCore0Counter;
extern volatile uint32_t g_traceCore1Counter;
extern const char *volatile g_traceCore0LastOp;
extern const char *volatile g_traceCore1LastOp;

#define TRACE(name)                                                  \
    do {                                                             \
        if (xPortGetCoreID() == 0) {                                 \
            g_traceCore0LastOp = (name);                             \
            g_traceCore0Counter++;                                   \
        } else {                                                     \
            g_traceCore1LastOp = (name);                             \
            g_traceCore1Counter++;                                   \
        }                                                            \
    } while (0)

void freeze_trace_periodic_print();

#else  // FREEZE_TRACE_ENABLED == 0  — compile to nothing

#define TRACE(name) ((void)0)
static inline void freeze_trace_periodic_print() {}

#endif  // FREEZE_TRACE_ENABLED
