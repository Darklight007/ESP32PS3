// LVGL is single-threaded and lives on Core 1 (main loop). A call from any
// other core corrupts its object tree; the crash then comes much later in an
// innocent redraw (a coredump showed layout_update_core() reading a garbage
// child pointer). patch_lvgl.py calls lv_core_guard() at the top of object
// create/delete/clean/invalidate: the first offender's backtrace goes to RTC
// memory, which survives the reset, and DIAG:LVCORE? prints it.
#include <Arduino.h>
#include "esp_debug_helpers.h"
#include "esp_attr.h"

static constexpr uint32_t kMagic = 0x1C0E6A2D;
struct LvCoreHit
{
    uint32_t magic;
    uint32_t count; // calls from the wrong core (since it was cleared)
    uint32_t core, task;
    char name[16];
    uint32_t pc[10];
};
RTC_NOINIT_ATTR static LvCoreHit s_hit;

extern "C" IRAM_ATTR void lv_core_guard(void)
{
    if (xPortGetCoreID() == ARDUINO_RUNNING_CORE)
        return;
    if (s_hit.magic != kMagic)
    {
        memset(&s_hit, 0, sizeof s_hit);
        s_hit.magic = kMagic;
    }
    if (s_hit.count++ > 0)
        return; // keep the first one
    s_hit.core = xPortGetCoreID();
    TaskHandle_t t = xTaskGetCurrentTaskHandle();
    s_hit.task = (uint32_t)t;
    strncpy(s_hit.name, pcTaskGetName(t), sizeof s_hit.name - 1);
    esp_backtrace_frame_t f;
    esp_backtrace_get_start(&f.pc, &f.sp, &f.next_pc);
    for (int k = 0; k < 10; k++)
    {
        s_hit.pc[k] = esp_cpu_process_stack_pc(f.pc);
        if (!f.next_pc || !esp_backtrace_get_next_frame(&f))
            break;
    }
}

String lvCoreGuardReport()
{
    if (s_hit.magic != kMagic || !s_hit.count)
        return "none";
    String r = String(s_hit.count) + " calls; first core " + String(s_hit.core) + " task " + s_hit.name + "; bt";
    for (int k = 0; k < 10 && s_hit.pc[k]; k++)
        r += " 0x" + String(s_hit.pc[k], HEX);
    return r;
}
void lvCoreGuardClear() { memset(&s_hit, 0, sizeof s_hit); s_hit.magic = kMagic; }
