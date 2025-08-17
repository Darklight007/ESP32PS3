#include "input_device.h"

// Put heavy/third-party headers here (not in the .h)
#include <Wire.h>
#include "tabs.h"
#include "Key.h"        // Keypad_MC17, LIST_MAX, PRESSED/HOLD/RELEASED/IDLE
#include "config.hpp"
#include "buzzer.h"     // myTone(), NOTE_A4/NOTE_B5/NOTE_A3

// These live elsewhere; we just reference them here.
extern Keypad_MC17 kpd;
extern bool ismyTextHiddenChange;

// ===== Definitions of shared objects =====
volatile unsigned long encoder1Flag = true;
volatile unsigned long encoder2Flag = true;
long long encoderTimeStamp = 0;

String msg = "";
char   keyChar = ' ';

// ===== ISRs =====
void IRAM_ATTR VoltageEnc(void *) { encoder1Flag++; }
void IRAM_ATTR CurrentEnc(void *) { encoder2Flag++; }

// ===== Keypad processing =====
void getKeys() {
    if (kpd.getKeys()) {
        for (int i = 0; i < LIST_MAX; i++) {
            if (kpd.key[i].stateChanged) {
                switch (kpd.key[i].kstate) {
                    case PRESSED:
                        myTone(NOTE_A4, 50);
                        msg = " PRESSED.";
                        ismyTextHiddenChange = true;
                        break;
                    case HOLD:
                        myTone(NOTE_B5, 500);
                        msg = " HOLD.";
                        break;
                    case RELEASED:
                        myTone(NOTE_A3, 50);
                        msg = " RELEASED.";
                        break;
                    case IDLE:
                        msg = " IDLE.";
                        break;
                }
                keyChar = kpd.key[i].kchar;
            }
        }
    }
}

// Menu helpers
void keyMenus(char&& item, String&& str, int x, std::function<void(int)> func) {
    if ((keyChar == item) && (msg == str)) {
        func(x);
        msg = "done!";   // break the loop
    }
}

void keyMenus(char&& item, String&& str, std::function<void(void)> func) {
    if ((keyChar == item) && (msg == str)) {
        static String oldMsg = "";
        // Ignore RELEASE after HOLD
        if (oldMsg == " HOLD." && str == " RELEASED.") {
            msg = "done!";
            oldMsg = msg;
            return;
        }
        oldMsg = msg;
        func();
        msg = "done!";
    }
}

void keyMenusPage(char&& item, String&& str, int page, std::function<void(void)> func) {
    if (Tabs::getCurrentPage() == page) {
        if ((keyChar == item) && (msg == str)) {
            func();
        }
    }
}

// MCP23017 helper (base address 0x20, plus A2..A0)
void sendCommand(int command, uint16_t data) {
    Wire.beginTransmission(0x20);             // adjust if A2..A0 != 000
    Wire.write(command);
    Wire.write((uint8_t)(data >> 8));
    Wire.write((uint8_t)(data & 0xFF));
    Wire.endTransmission();
    __asm__("nop\n\t");
}
