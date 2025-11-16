#pragma once

#include <functional>

// Scheduling function overloads
void schedule(std::function<void(void)> func, unsigned long &&interval, unsigned long &startTime);
void schedule(std::function<void(void)> func, unsigned long &interval, unsigned long &startTime);
