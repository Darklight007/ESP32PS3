#pragma once

// Display performance meter: frames/s, pixels per frame, render and flush time.
// Read with SCPI DISP:FPS? ; DISP:FPS 1/0 shows/hides an on-screen overlay.

#include <Arduino.h>
#include "lvgl.h"

struct DispPerfStats
{
    float fps;        // frames rendered per second
    uint32_t pxPerFrame;
    float renderMs;   // per frame, LVGL drawing (frame time minus flush)
    float flushMs;    // per frame, SPI/DMA push
    float busyPct;    // share of Core 1 spent in frames
};

void dispPerfAttach(lv_disp_drv_t *drv); // call before lv_disp_drv_register()
void dispPerfFlushBegin();
void dispPerfFlushEnd();
void dispPerfUpdate(); // Core 1 main loop - rolls the 1 s window, refreshes the overlay
void dispPerfShow(bool on);
bool dispPerfShown();
const DispPerfStats &dispPerfStats();
