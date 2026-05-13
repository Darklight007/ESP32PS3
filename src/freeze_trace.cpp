#include "freeze_trace.h"

#if FREEZE_TRACE_ENABLED

#include "tabs.h"
#include "ui_helpers.h"

volatile uint32_t g_traceCore0Counter = 0;
volatile uint32_t g_traceCore1Counter = 0;
const char *volatile g_traceCore0LastOp = "init";
const char *volatile g_traceCore1LastOp = "init";

void freeze_trace_periodic_print()
{
    static uint32_t last_ms = 0;
    const uint32_t now = millis();
    if (now - last_ms < 250) return;
    last_ms = now;

    // Snapshot volatiles to local stack to avoid mid-print updates.
    const uint32_t c0 = g_traceCore0Counter;
    const uint32_t c1 = g_traceCore1Counter;
    const char *op0 = g_traceCore0LastOp ? (const char *)g_traceCore0LastOp : "null";
    const char *op1 = g_traceCore1LastOp ? (const char *)g_traceCore1LastOp : "null";

    Serial.printf("[FRZ %u] c0=%u op0=%-24s | c1=%u op1=%-24s | page=%d gMode=%d hMode=%d\n",
                  now, c0, op0, c1, op1,
                  Tabs::getCurrentPage(),
                  (int)g_graphViewMode,
                  (int)g_histViewMode);
}

#endif  // FREEZE_TRACE_ENABLED
