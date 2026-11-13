#pragma once

#include <lvgl.h>
#include <Arduino.h>

// Load analyser (Utility page, "Load" tab).
//  Sweep: ramps the output 0 -> V-Set -> 0 and fits I = I0 + V/R + C*dV/dt,
//         plus a knee voltage for diodes/LEDs/batteries; plots I against V.
//  FFT:   captures the current at the present setpoint at the full ADC rate
//         and plots its spectrum (ripple, oscillation, load noise).
//  Zero:  a sweep with nothing connected; its R/C/I0 (the PSU's own output
//         capacitor and bleeders) is subtracted from later results.

void createLoadTab(lv_obj_t *parent);

// Task_ADC (Core 0), after each fresh V+I pair. volts, amps.
void loadAnalyzerAdcHook(double volts, double amps);

// Start from UI or SCPI (Core 1). false = refused (output off, FUN, busy).
bool loadSweepStart(bool zero = false);
bool loadFftStart();
bool loadImpStart(); // impedance vs frequency (lock-in), bias V-Set/2
bool loadAnalyzerBusy();
bool loadImpBusy(); // Z(f)/LCR running: Task_ADC uses Clean Wave timing
String loadAnalyzerResult(); // last result text, one line, ';' separated
String loadAnalyzerDetails(); // probe details, ';' separated
void loadAnalyzerSetImax(float amps); // analysis current limit; 0 = keep I-Set
