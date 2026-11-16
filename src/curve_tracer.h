#pragma once
#include <lvgl.h>
#include <Arduino.h>

// BJT curve tracer (Utility page, "Trace" tab), two linked PSUs:
//  this PSU (master): + to collector, - to emitter; sweeps Vce, measures Ic
//  linked PSU (slave): + through a base resistor to the base, - to emitter;
//  its voltage is servoed so its measured current hits each Ib step.
// One curve per Ib step (Ib = step x 1..N). Each point is stopped at the
// power limit; Ic is limited by this PSU's current limit (Ic max).

void createTracerTab(lv_obj_t *parent);
void tracerLoop(); // main loop (Core 1)

struct TracerCfg
{
    float vceMax = 10;    // V
    float icMax = 0.1f;   // A
    float pMax = 1;       // W
    float ibStep = 20e-6f; // A
    int curves = 5;
    int points = 40;
    float vbMax = 12;     // slave voltage ceiling, V
    int settleMs = 200;
};
TracerCfg &tracerCfg();
bool tracerStart(); // false: not linked as master / busy
bool tracerDemo();  // simulated NPN, no outputs used
void tracerStop();
bool tracerBusy();
String tracerStatus(); // one line
String tracerData();   // CSV: curve,ib_A,vce_V,ic_A per line
