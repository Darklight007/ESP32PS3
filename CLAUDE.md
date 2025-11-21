# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP32-S3 based precision power supply with touchscreen GUI, dual rotary encoders, and keypad input. The firmware is built with PlatformIO using the Arduino framework and LVGL 8.4 for graphics.

**Hardware:**
- ESP32-S3-N16R8 (16MB Flash, 8MB PSRAM)
- 320x240 ILI9341 touchscreen (TFT_eSPI driver)
- ADS1219 24-bit ADC for precision voltage/current measurement
- LTC2655 16-bit DAC for voltage/current control
- Dual rotary encoders (ESP32Encoder library)
- Matrix keypad (Keypad_MC17 via MCP23017 I2C expander)

## Build Commands

**Compile the project:**
```bash
~/.platformio/penv/bin/pio run
# or
pio run
```

**Upload to device:**
```bash
~/.platformio/penv/bin/pio run --target upload
```

**Clean build:**
```bash
~/.platformio/penv/bin/pio run --target clean
```

**Monitor serial output:**
```bash
~/.platformio/penv/bin/pio device monitor
# Baud rate: 115200
```

**Build flags:**
- Compiler: `-O3` optimization, `-std=gnu++2a` (C++20)
- `-w` suppresses warnings
- `-D RELEASE` for release builds

## Architecture

### Core Design Pattern

The codebase uses a **centralized Device object** (`PowerSupply`) that coordinates all hardware and UI components. This is a global singleton defined in `device.hpp` and instantiated in `globals.cpp`.

```cpp
Device PowerSupply;  // The central controller object
```

All major subsystems interact through this object:
- `PowerSupply.Voltage` - Voltage channel (ADC, DAC, display, calibration)
- `PowerSupply.Current` - Current channel (ADC, DAC, display, calibration)
- `PowerSupply.gui` - All LVGL UI object references
- `PowerSupply.graph` / `PowerSupply.stats` - Chart objects
- `PowerSupply.page[0-4]` - Tab pages (Histogram, Graph, Main, Utility, Calibration)

### Task Architecture (FreeRTOS)

Two main tasks run on separate cores:
- **Task_ADC** (Core 0) - High-priority ADC sampling and measurement processing
- **Task_BarGraph** (Core 1) - LVGL rendering, UI updates, and user input handling

Task creation happens in `createTasks()` (tasks.cpp). The GUI task must run on Core 1 because LVGL is not thread-safe across cores.

### Module Organization (Post-Refactoring)

The codebase was recently refactored (91.7% reduction) from a 4,015-line monolithic `globalFunctions.h` into focused modules:

**Core Modules:**
- `device.hpp/.cpp` - Device class definition and hardware abstraction
- `DispObject.h/.cpp` - Display object wrapper for voltage/current channels
- `globals.h/.cpp` - Global variables and shared state
- `config.hpp/.cpp` - Hardware pin definitions and configuration

**UI Modules:**
- `ui_creation.h/.cpp` - UI initialization and widget creation
- `ui_helpers.h/.cpp` - UI event callbacks (draw_event_stat_chart_cb, draw_event_cb2, btn_function_gen_event_cb)
- `input_handler.h/.cpp` - All input handling (touch, encoders, keyboard)
  - Page handlers: `handleGraphPage()`, `handleHistogramPage()`, `handleCalibrationPage()`, `handleUtilityPage()`, `handleUtility_function_Page()`
  - `keyCheckLoop()` - Main keyboard input dispatcher with `keyMenus()` and `keyMenusPage()` lambdas
  - `managePageEncoderInteraction()` - Routes encoder events to active page
  - `getSettingEncoder()` - Encoder input processing
- `table_pro.h/.cpp` - Enhanced table widget with selection/scrolling
- `spinbox_pro.h/.cpp` - Enhanced spinbox widget

**Functionality Modules:**
- `waveform_generator.h/.cpp` - Function generator with 18 waveform types plus functionGenerator() and functionGenerator_demo()
- `functions.h/.cpp` - Utility functions (scaleVoltage, scaleCurrent, print_obj_type, trackLoopExecution, monitorMinChanges, autoScrollY)
- `intervals.h/.cpp` - Periodic update functions (LvglUpdatesInterval, FFTUpdateInterval, statisticUpdateInterval, etc.)
- `memory.h/.cpp` - Preset memory save/load functionality
- `setting_menu.h/.cpp` - Settings and calibration menus

**Hardware Modules:**
- `input_device.h/.cpp` - Touch calibration (touch_calibrate, my_touchpad_read, init_touch)
- `LTC2655.h/.cpp` - DAC driver
- `FFTHandler.h/.cpp` - FFT processing for waveform analysis
- `buzzer.h/.cpp` - Tone generation
- `power_management.h/.cpp` - OVP/OCP protection, timer, energy counter, power-on duration tracking

**Calibration System (Modular):**
- `calib_adc.h/.cpp` - ADC voltage/current calibration windows and procedures
- `calib_dac.h/.cpp` - DAC calibration windows and procedures
- `calib_inl.h/.cpp` - INL (Integral Non-Linearity) calibration for enhanced accuracy
- `calib_internal_leakage.h/.cpp` - Internal leakage current calibration
- `calib_sequencer.h/.cpp` - Automated calibration sequence controller
- `calib_log_window.h/.cpp` - Calibration logging and status display

**Navigation:**
- `tabs.h/.cpp` - Static tab/page management class with `Tabs::getCurrentPage()`, `Tabs::nextPage()`, etc.

**Remaining:**
- `globalFunctions.h` - Now only 333 lines (91.7% reduced), contains remaining event callbacks and helper functions

### Key Data Flow

1. **ADC Sampling** (Task_ADC, Core 0):
   - ADS1219 reads voltage/current → stores in `PowerSupply.Voltage.measured.value`
   - Statistics updated: `Mean()`, `StandardDeviation()`, `Min()`, `Max()`
   - Histogram bins populated: `PowerSupply.Voltage.hist.data[]`

2. **DAC Control**:
   - User input → `PowerSupply.Voltage.SetUpdate(value)` → LTC2655 DAC
   - Encoder changes call `SetEncoderUpdate()` with rotary step size

3. **UI Updates** (Task_BarGraph, Core 1):
   - Interval functions called periodically (defined in intervals.cpp)
   - `LvglUpdatesInterval()` - Fast UI updates (values, bars)
   - `LvglFullUpdates()` - Slow updates (charts, statistics)
   - `ChartUpdate()` - Adds new points to voltage/current charts

4. **Input Handling**:
   - Touch events → `my_touchpad_read()` callback
   - Encoder events → `getSettingEncoder()` → `managePageEncoderInteraction()` → page-specific handlers
   - Keyboard events → `getKeys()` → `keyCheckLoop()` with lambda handlers

### LVGL Integration

LVGL 8.4 runs in buffered mode with DMA transfers. Key points:

- Display buffer: 320x30 pixels (one-tenth screen height)
- Tick handler: `lv_tick_inc(1)` called every 1ms
- Rendering: `lv_timer_handler()` called in main loop
- Thread safety: All LVGL calls must be on Core 1 (Task_BarGraph)
- Custom widgets use LVGL's object-oriented API (classes, inheritance, events)

### Calibration System

Uses `MonotoneCubicCalibrator` for non-linear ADC/DAC calibration:
- Multi-point calibration with cubic interpolation
- Separate calibration for voltage/current, ADC/DAC
- Data stored in Preferences (NVS flash)
- INL (Integral Non-Linearity) calibration for enhanced accuracy

### Settings System

User-configurable settings are stored in `PowerSupply.settingParameters` (defined in `device.hpp`):

```cpp
struct SettingParameters {
    // Graph settings
    bool graphXaxisTimeMode = false;     // false = points mode, true = time mode
    uint16_t graphTimeSpanSeconds = 60;  // X-axis time span (5s to 12h)

    // Protection settings
    bool ovpEnabled = false;             // Over-voltage protection enable
    float voltageLimitMax = 32.0;        // OVP threshold
    bool ocpEnabled = false;             // Over-current protection enable
    float currentLimitMax = 6.0;         // OCP threshold

    // Timer and energy tracking
    bool timerEnabled = false;
    uint32_t timerDurationSeconds = 0;
    // ... beeper settings, display preferences, etc.
};
```

Settings are persisted via `PowerSupply.SaveSetting()` / `LoadSetting()` using NVS.

### Important Globals

These are defined in `globals.h/.cpp`:
```cpp
extern Device PowerSupply;          // Main device object
extern lv_obj_t *slider_x;          // Graph horizontal slider
extern lv_obj_t *label_legend1/2;   // Chart legends
extern int32_t encoder1_value;      // Encoder 1 count
extern int32_t encoder2_value;      // Encoder 2 count
extern char keyChar;                // Last key pressed
extern String msg;                  // Key state (" RELEASED.", " HOLD.", " IDLE.")
extern bool blockAll;               // Global input blocking flag
extern Waveform waveforms[];        // Waveform function array (in waveform_generator.cpp)
extern bool lvglChartIsBusy;        // Chart update lock flag
extern unsigned long encoderTimeStamp; // Last encoder activity timestamp
```

## Common Development Tasks

**Adding a new UI page:**
1. Add page object to `PowerSupply.page[]` array
2. Create page in `ui_creation.cpp` using `lv_tabview_add_tab()`
3. Add handler function in `input_handler.cpp` (follow pattern of existing handlers)
4. Register handler in `managePageEncoderInteraction()` switch statement
5. Add keyboard shortcuts in `keyCheckLoop()` if needed

**Adding a new waveform:**
1. Define waveform function in `waveform_generator.cpp` with signature `double func(double t)` where t ∈ [0,1]
2. Add function declaration to `waveform_generator.h`
3. Add entry to `waveforms[]` array with name and function pointer
4. Table will auto-populate from array

**Modifying calibration:**
1. Edit calibration UI in `setupCalibPage()` (setting_menu.cpp)
2. Calibration data stored via `PowerSupply.SaveCalibration()` / `LoadCalibration()`
3. Preferences namespace: "CalibrationV", "CalibrationI", etc.

**Hardware testing after changes:**
Always compile, upload, and verify on actual hardware - this controls a real power supply and must function correctly to avoid damage.

## Refactoring Guidelines

### Pattern for Extracting from globalFunctions.h

When extracting functions to a new module:

1. **Identify cohesive group of functions** (e.g., all keyboard handlers, all waveform functions)

2. **Create or update module files:**
   ```bash
   # If creating new module:
   touch src/module_name.h src/module_name.cpp
   ```

3. **In the `.h` file:**
   - Add `#pragma once`
   - Include necessary dependencies (`<lvgl.h>`, `device.hpp`, etc.)
   - Add function declarations (prototypes only)
   - Declare any `extern` variables needed

4. **In the `.cpp` file:**
   - Include the header file first
   - Include dependencies: `device.hpp`, `globals.h`, `functions.h`
   - Copy/move function implementations from `globalFunctions.h`
   - Add any necessary `extern` references

5. **Update globalFunctions.h:**
   - Add `#include "module_name.h"` at the top if needed
   - Remove the extracted functions
   - Replace with comment: `// functionName() moved to module_name.cpp`

6. **Compile and fix errors incrementally:**
   ```bash
   ~/.platformio/penv/bin/pio run
   ```
   Common issues:
   - Missing includes → add to header
   - Undefined references → check extern declarations
   - Multiple definitions → ensure variables only defined once in .cpp

7. **Test on hardware:**
   ```bash
   ~/.platformio/penv/bin/pio run --target upload
   ```
   Wait for user to confirm "ok" before proceeding

8. **Commit with descriptive message:**
   ```bash
   git add src/module_name.h src/module_name.cpp src/globalFunctions.h
   git commit -m "Extract [feature] functions to module_name.cpp

   Moved [function1], [function2], etc. from globalFunctions.h.

   Progress: X → Y lines (Z lines extracted)
   Total reduction: A lines (B%)

   🤖 Generated with [Claude Code](https://claude.com/claude-code)
   Co-Authored-By: Claude <noreply@anthropic.com>"
   git push
   ```

### Recent Refactoring Success (Example)

**From 4,015 lines to 333 lines (91.7% reduction):**

1. ✅ Monitoring utilities → functions.cpp (69 lines)
2. ✅ Function generators → waveform_generator.cpp (72 lines)
3. ✅ Page handlers → input_handler.cpp (538 lines)
4. ✅ Encoder handlers → input_handler.cpp (113 lines)
5. ✅ UI event callbacks → ui_helpers.cpp (296 lines)
6. ✅ Keyboard input → input_handler.cpp (558 lines)
7. ✅ Auto scroll → functions.cpp (26 lines)
8. ✅ Cleanup: Removed 1,340 lines of commented-out dead code

Each extraction was compiled, tested on hardware, and committed separately.

## File Naming Conventions

- `*.hpp` - C++ class headers (Device, SpinboxPro, MonotoneCubicCalibrator)
- `*.h` - C-style headers or simple function declarations
- `*.cpp` - Implementation files
- Snake_case for files: `input_handler.cpp`, `waveform_generator.cpp`
- PascalCase for classes: `Device`, `DispObject`
- camelCase for functions: `keyCheckLoop()`, `handleGraphPage()`

## Git Workflow

The project uses descriptive commit messages with this format:
```
Subject line describing the change

Detailed description of what was changed and why.

Progress tracking (if applicable):
- Lines reduced/added
- Performance improvements
- Bug fixes

🤖 Generated with [Claude Code](https://claude.com/claude-code)
Co-Authored-By: Claude <noreply@anthropic.com>
```

All commits are pushed to the `main` branch on GitHub (ESP32PS3 repository).

## Critical Notes

1. **Always test on hardware** after UI changes - touchscreen calibration and input handling are hardware-dependent
2. **FreeRTOS tasks** must be properly configured - wrong core assignments cause crashes
3. **LVGL is not thread-safe** - only call from Core 1 or use lv_lock if absolutely necessary
4. **Watchdog timer** is configured for 120 seconds - long operations must call `esp_task_wdt_reset()`
5. **Global state** is intentional - the Device pattern centralizes hardware access for safety
6. **Calibration data** is precious - backup before major changes to calibration system
7. **Use full PlatformIO path** - `~/.platformio/penv/bin/pio` instead of just `pio`
8. **Incremental changes** - Compile → Upload → Test → Commit after each logical change
9. **User confirmation** - Wait for "ok" after hardware testing before continuing
