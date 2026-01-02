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
void Page2RightSideCleanup(unsigned long interval);  // Clean dirty pixels on right side of page 2
void VCCCInterval(unsigned long interval);
void LvglUpdatesInterval(unsigned long interval);
void LvglUpdatesInterval(unsigned long interval, bool forceUpdate);
void FFTUpdateInterval(unsigned long interval);
void PowerManagementInterval(unsigned long interval);  // New: Timer, Energy, Auto-save, Limits
void MemoryMonitorInterval(unsigned long interval);    // New: Heap and stack monitoring
void RecordingPlaybackInterval();                      // New: Voltage recording and playback
