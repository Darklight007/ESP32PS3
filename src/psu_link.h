#pragma once
#include <Arduino.h>

// PSU-to-PSU link over ESP-NOW (psu_link.cpp).
// Enable the link on both units, Scan on one, pick the other from the list
// and confirm on both: the one that asked becomes master. The master sends
// V-Set, I-Limit and output state; the slave follows them x ratio.
enum class LinkRole : uint8_t { OFF = 0, MASTER = 1, SLAVE = 2, READY = 3 }; // READY: radio on, not linked

void linkBegin();                 // setup(): load NVS, start the radio if enabled
void linkLoop();                  // main loop (Core 1)
void linkEnable(bool on);         // radio on (READY or the saved link) / off
bool linkEnabled();
LinkRole linkRole();
void linkScan();                  // 3 s: ask every enabled PSU nearby to answer
bool linkScanning();
int linkFound(char names[][12], int max); // units found by the last scan
uint32_t linkFoundVersion();      // changes when the list does
void linkRequest(int foundIndex); // ask that unit to follow this one
bool linkIncoming(char *name);    // another unit asks this one to follow it
void linkAnswer(bool accept);
void linkUnpair();
void linkSetRatio(float pct);     // master: slave value = master x pct/100 (1..100)
float linkRatio();
bool linkConnected();
const char *linkSelfName();       // "PSU-A1B2C3" (MAC end)
String linkStatus();              // short, comma separated (SCPI LINK:STAT?, menu)
