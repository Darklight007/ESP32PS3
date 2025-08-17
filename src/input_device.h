//input...h
#pragma once

#include <Arduino.h>      // IRAM_ATTR, String
#include <WString.h>
#include <stdint.h>
#include <functional>

// ===== ISR counters shared with main code =====
extern volatile unsigned long encoder1Flag;
extern volatile unsigned long encoder2Flag;
extern long long encoderTimeStamp;

// ===== key state shared with the rest of the app =====
extern String msg;     // " PRESSED.", " HOLD.", " RELEASED.", " IDLE."
extern char   keyChar; // last key char

// ===== ISRs (kept tiny; placed in IRAM on ESP32) =====
void IRAM_ATTR VoltageEnc(void *);
void IRAM_ATTR CurrentEnc(void *);

// ===== Public API =====
void getKeys();

void keyMenus(char&& item, String&& str, int x, std::function<void(int)> func);
void keyMenus(char&& item, String&& str,       std::function<void(void)> func);

void keyMenusPage(char&& item, String&& str, int page,
                  std::function<void(void)> func);

// MCP23017 helper (I2C write to 0x20..0x27 depending on A2..A0)
void sendCommand(int command, uint16_t data);



// #pragma once

// #ifndef INPUT_DEVICE_H
// #define INPUT_DEVICE_H

// #include <Arduino.h>
// #include <iostream>
// #include <ctime>
// #include <string>
// #include <cstdlib>
// #include <functional>

// #include <WString.h>
// #include <stdint.h>
// #include <stdio.h>
// #include "tabs.h"
// #include "Wire.h"
// #include "Key.h"
// #include "config.hpp"

// // IRAM_ATTR void VoltageEnc(void *arg);
// // IRAM_ATTR void CurrentEnc(void *arg);

// // void getKeys();
// // void pageNavigate(char item, String str, int x, Tabs pages);
// // void keyMenus(char item, String str, int x, std::function<void(int)> func);
// // void keyMenus(char item, String str, std::function<void(void)> func);

// volatile unsigned long encoder1Flag = true;
// volatile unsigned long encoder2Flag = true;
// long long encoderTimeStamp = 0;

// void IRAM_ATTR VoltageEnc(void *)
// {
//     // ESP32Encoder *enc = (ESP32Encoder *)arg;
//     // Serial.printf("VoltageEnc cb at: %d\n", millis());
//     encoder1Flag++;
// }

// void IRAM_ATTR CurrentEnc(void *)
// {
//     // ESP32Encoder *enc = (ESP32Encoder *)arg;
//     // Serial.printf("CurrentEnc cb: count: %d\n", enc->getCount());
//     encoder2Flag++;
// }

// String msg = "";
// char keyChar = ' ';
// // extern Keypad_MC17 kpd;

// void getKeys()
// {

//     if (kpd.getKeys())
//     {
//         for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
//         {
//             if (kpd.key[i].stateChanged) // Only find keys that have changed state.
//             {

//                 switch (kpd.key[i].kstate)
//                 { // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
//                 case PRESSED:
//                     // myTone(NOTE_A4, 3);
//                     myTone(NOTE_A4, 50);
//                     msg = " PRESSED.";
//                     ismyTextHiddenChange = true;
//                     break;
//                 case HOLD:
//                     myTone(NOTE_B5, 500);
//                     msg = " HOLD.";
//                     break;
//                 case RELEASED:
//                     myTone(NOTE_A3, 50);
//                     msg = " RELEASED.";
//                     break;
//                 case IDLE:
//                     msg = " IDLE.";
//                 }
//                 // myTone(NOTE_A3, 25);
//                 // Serial.print("Key ");
//                 // Serial.print(kpd.key[i].kchar);
//                 // Serial.println(msg);
//                 keyChar = kpd.key[i].kchar;
//             }
//         }
//     }
// }

// // void pageNavigate(char &&item, String &&str, int x, Tabs &&pages)
// // {
// //     if ((keyChar == item) && (msg == str))
// //     {
// //         pages.setCurrentPage(x);
// //         // msg = "";
// //         msg = "done!"; // Have to break the loop
// //     }
// // }

// void keyMenus(char &&item, String &&str, int x, std::function<void(int)> func) //  void (*func)(int))
// {
//     if ((keyChar == item) && (msg == str))
//     {
//         //  pages.setCurrentPage(x);
//         func(x);
//         msg = "done!"; // Have to break the loop
//     }
// }

// void keyMenus(char &&item, String &&str, std::function<void(void)> func) //  void (*func)(int))
// {
//     if ((keyChar == item) && (msg == str))
//     {
//         static String oldMsg = "";
//         // Ignore RELEASE after HOLD
//         if (oldMsg == " HOLD." && str == " RELEASED.")
//         {
//             msg = "done!";
//             oldMsg = msg;
//             return;
//         }
//         //  pages.setCurrentPage(x);
//         oldMsg = msg;
//         func();
//         msg = "done!"; // Have to break the loop
//     }
// }

// void keyMenusPage(char &&item, String &&str, int page, std::function<void(void)> func) //  void (*func)(int))
// {
//     if (Tabs::getCurrentPage() == page)
//         if ((keyChar == item) && (msg == str))
//         {
//             //  pages.setCurrentPage(x);
//             func();
//             // msg = "";
//             // keyChar = ' ';
//         }
// }

// // MCP23017
// void sendCommand(int command, uint16_t data)
// {
//     Wire.beginTransmission(0x20); // transmit to device #65, Table 2. Slave Address Map, page 20th data sheet (FLOAT FLOAT FLOAT)
//     Wire.write(command);
//     Wire.write(data >> 8);
//     Wire.write(data & 0xff);
//     Wire.endTransmission();
//     __asm__("nop\n\t");
// }

// #endif
// #pragma once