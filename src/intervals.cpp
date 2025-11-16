#include "intervals.h"
#include <Arduino.h>

void schedule(std::function<void(void)> func, unsigned long &&interval, unsigned long &startTime)
{
    if ((millis() - startTime) < interval)
        return;

    startTime = millis();
    func();
}

void schedule(std::function<void(void)> func, unsigned long &interval, unsigned long &startTime)
{
    if ((millis() - startTime) < interval)
        return;
    startTime = millis();
    func();
}
