#pragma once

#include <functional>

// Scheduling function overloads
void schedule(std::function<void(void)> func, unsigned long &&interval, unsigned long &startTime);
void schedule(std::function<void(void)> func, unsigned long &interval, unsigned long &startTime);

// Interval functions
void StatusBarUpdateInterval(unsigned long interval);
void FlushMeasuresInterval(unsigned long interval);
void statisticUpdateInterval(unsigned long interval);
void EncoderRestartInterval(unsigned long interval);
void KeyCheckInterval(unsigned long interval);
void LvglFullUpdates(unsigned long interval);
void VCCCInterval(unsigned long interval);
