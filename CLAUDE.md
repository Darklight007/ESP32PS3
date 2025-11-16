# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an ESP32-S3 based programmable power supply with advanced features including function generation, calibration, FFT analysis, and LVGL-based touchscreen UI. The firmware runs on ESP32-S3-DevKitC-1 with 16MB flash and 8MB PSRAM.

## Build Commands

**Platform**: PlatformIO (not Arduino IDE)

```bash
# Build firmware (use full path to pio)
~/.platformio/penv/bin/pio run

# Upload to device
~/.platformio/penv/bin/pio run --target upload

# Clean build
~/.platformio/penv/bin/pio run --target clean

# Monitor serial output
~/.platformio/penv/bin/pio device monitor
```

**Important Build Notes**:
- Uses C++20 standard (`-std=gnu++2a`)
- Optimized with `-O3` flag
- Custom partition table: `par/default_16MB_2.csv`
- Pre-build script `update_version.py` auto-updates version from git tags
- Upload speed: 921600 baud
- Monitor speed: 115200 baud

## Architecture Overview

### Core Components

**Device Class (`device.hpp`)**: Central power supply control
- Manages voltage/current setpoints and measurements
- Handles ADC readings (ADS1219) and DAC outputs (LTC2655)
- Contains calibration data and state machine (OFF/ON/CV/CC/OVP/OCP)
- Stores function generator configuration and memory presets
- Instance: `PowerSupply` (global singleton in `config.hpp`)

**Display System**:
- **LVGL 8.4**: UI framework with hardware-accelerated DMA rendering
- **TFT_eSPI**: 320x240 ILI9341 display driver
- **DispObject.h**: Custom display objects (labels, charts, histograms, statistics)
- DMA double-buffering via `TFT_eSprite` for flicker-free rendering
- Touch input via resistive touchscreen

**Task Architecture (FreeRTOS)**:
- `Task_ADC` (`tasks.cpp`): High-priority ADC reading, measurements, charts, encoder input, DAC updates
- `Task_BarGraph` (`tasks.cpp`): Updates bar graph displays on page 2
- Watchdog timer: 120 second timeout
- Runs on dual-core ESP32-S3
- Tasks communicate via flags: `adcDataReady`, `lvglIsBusy`, `lvglChartIsBusy`, `blockAll`

### Code Organization (Recently Refactored)

**Core headers** (read these first):
- `config.hpp`: Global configuration, extern declarations, pin definitions
- `device.hpp`: Device class, enums (DEVICE states), calibration structures
- `globals.h/cpp`: Global variables (chart data, UI objects, encoders, dropdown_active, util charts)
- `functions.h`: Forward declarations to avoid circular includes
- `waveform_generator.h/cpp`: All waveform generation functions (19 waveforms)
- `ui_helpers.h/cpp`: UI chart/graph drawing functions (extracted from globalFunctions.h)
- `tasks.h/cpp`: FreeRTOS task implementations (Task_ADC, Task_BarGraph)
- `input_handler.h/cpp`: Touch calibration and input handlers (touch_calibrate, my_touchpad_read, init_touch)
- `intervals.h/cpp`: Scheduling functions (schedule overloads for periodic execution)
- `globalFunctions.h`: Large file (~3865 lines, being progressively reduced) with remaining UI implementations and callbacks

**Important constants** (defined across various headers):
- `CHART_SIZE`: 240 × 5 = 1200 points (graph chart buffer)
- `BUCKET_COUNT`: 100 (histogram buckets)
- `CHART_POINTS`: 20 (chart display points, defined in `ui_helpers.h`)
- `NUM_LABELS`: 7 (chart axis labels)
- `VOLTAGE`: 1 (ADC channel)
- `CURRENT`: 3 (ADC channel)
- `DAC_VOLTAGE`: CHANNEL_D (DAC channel)
- `DAC_CURRENT`: CHANNEL_B (DAC channel)

**Setup flow** (`main.cpp`):
1. `initializeSerial()`, `initialMemory()`
2. `initializeI2C()`, `initializeDisplay()`, `initializeTouch()`
3. `setupLVGL()`, `setupDMA()`
4. `setupPowerSupply()`, `setupPreferences()`, `setupADC()`, `setupDAC()`
5. `setupCalibPage()`, `createTasks()`

### Key Subsystems

**ADC System**:
- ADS1219: 24-bit delta-sigma ADC for voltage/current measurement
- Multi-channel: voltage (CH1), current (CH3)
- Configurable averaging (up to 128 samples)
- Data-ready interrupt (DRDY pin) triggers ISR
- INL (Integral Non-Linearity) calibration using monotone cubic interpolation

**DAC System**:
- LTC2655: 16-bit quad DAC for voltage/current control
- Channels: DAC_VOLTAGE (D), DAC_CURRENT (B)
- I2C interface
- Two-point calibration per channel

**Function Generator**:
- 19 waveform types (sine, square, triangle, sawtooth, PWM, exponential, gaussian, chirp, arbitrary, etc.)
- Configurable frequency, amplitude, offset, duty cycle
- Arbitrary waveform support (100-point table + 2 banks of custom points)
- See `waveform_generator.cpp` for implementations

**Calibration System** (`Calibration` class):
- Stores per-device calibration (MAC address-based)
- Two-point calibration: voltage, current (2 ranges), internal leakage
- ADC INL calibration: 36-point lookup table
- Persisted to EEPROM/Preferences

**Input Devices**:
- ESP32Encoder: Dual rotary encoders for voltage/current adjustment
- Keypad_MC17: I2C 6x6 keypad matrix (MCP23017 expander)
- Touchscreen: Resistive touch via TFT_eSPI

## Critical Development Guidelines

### Incremental Changes & Testing
**This is paramount**: The power supply hardware is connected and must remain functional.

1. **Always compile after each change**:
   ```bash
   ~/.platformio/penv/bin/pio run
   ```

2. **For critical changes, upload and test on hardware**:
   ```bash
   ~/.platformio/penv/bin/pio run --target upload
   # User will verify: "ok" means working, otherwise debug
   ```

3. **Make small, testable changes**:
   - Split large refactorings into multiple commits
   - Test after each step (compile + upload + verify)
   - Never batch multiple risky changes together

4. **Recent Example**: When splitting `globalFunctions.h` (originally 4878 lines → now ~3500 lines), changes were made in multiple steps with compilation and hardware testing after each:
   - Step 1: Extract globals to `globals.h/cpp` → compile → upload → test → ✓
   - Step 2: Extract UI helpers to `ui_helpers.h/cpp` → compile → upload → test → ✓
   - Step 3: Extract tasks to `tasks.h/cpp` → compile → upload → test → ✓
   - Step 4: Extract input handlers to `input_handler.h/cpp` → compile → fix errors → upload → test → ✓
   - Each step reduced file by ~400-500 lines, with systematic error fixing (linkage, multiple definitions, missing declarations)

### Refactoring Pattern for Extracting from globalFunctions.h

When extracting functions to a new module, follow this systematic approach:

1. **Create header and implementation files**:
   ```bash
   touch src/module_name.h src/module_name.cpp
   ```

2. **In the `.h` file**:
   - Add `#pragma once`
   - Include necessary dependencies (`<lvgl.h>`, `<TFT_eSPI.h>`, etc.)
   - Add function declarations (prototypes only)
   - Declare any `extern` variables needed

3. **In the `.cpp` file**:
   - Include the header file
   - Include `device.hpp`, `globals.h`, `functions.h` as needed
   - Copy function implementations from `globalFunctions.h`
   - Define any variables (not `extern`, actual definitions)
   - Add forward declarations for functions still in `globalFunctions.h`

4. **Update globalFunctions.h**:
   - Add `#include "module_name.h"` at the top
   - Remove the extracted functions
   - Remove any structs/variables moved to the new module

5. **Fix linkage issues**:
   - If getting "undefined reference" errors, the function may be `static` in `globalFunctions.h`
   - Remove `static` keyword to allow external linkage, OR
   - Add forward declaration to `functions.h`
   - For callback functions used by LVGL, add declarations to `functions.h`

6. **Compile and fix errors incrementally**:
   ```bash
   ~/.platformio/penv/bin/pio run
   ```
   - Fix "multiple definition" errors by moving variables to `globals.cpp` or making them `static` in `.cpp` files
   - Fix "undefined reference" errors by adding forward declarations or removing `static`
   - Fix "not declared in scope" by adding necessary includes or `#define` constants

7. **Upload and test**:
   ```bash
   ~/.platformio/penv/bin/pio run --target upload
   ```
   Wait for user to confirm "ok" before proceeding

### Code Style & Patterns

**Header organization**:
- Declarations in `.h` files, definitions in `.cpp` files
- Exception: `globalFunctions.h` contains implementations (legacy, being refactored)
- Use `#pragma once` for include guards
- Forward declarations in `functions.h` to avoid circular dependencies

**Global state**:
- Singletons declared `extern` in `config.hpp`, defined in `config.cpp`
- Example: `extern Device PowerSupply;`
- New globals go in `globals.h/cpp`

**LVGL patterns**:
- Event callbacks: `static void my_event_cb(lv_event_t *e)`
- Always check null pointers in callbacks
- Use `lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN)` to hide objects
- Chart data stored in `graph_data_V[]`, `graph_data_I[]` arrays

**FreeRTOS/ISR**:
- ISRs marked `IRAM_ATTR` for fast execution from RAM
- Minimal work in ISRs (set flags, trigger tasks)
- Use `portYIELD_FROM_ISR()` when waking tasks

### Common Pitfalls

1. **Don't use `pio` directly** - it may not be in PATH. Use `~/.platformio/penv/bin/pio`

2. **Static functions and linkage issues** - When extracting functions from headers:
   - Functions marked `static` have internal linkage (file-scope only)
   - To call from another module, remove `static` keyword
   - Variables marked `static` in headers cause multiple definition errors
   - Move static variables to `globals.h/cpp` and declare with `extern` in header
   - Add forward declarations to `functions.h` for callbacks used across modules

3. **Multiple definition errors** - Common during refactoring:
   - Variables: Declare with `extern` in `.h` file, define in `.cpp` file (only once)
   - Structs defined in headers are now in implementation files (e.g., `TouchAttr` in `input_handler.cpp`)
   - Arrays like `dataBuckets[]` should be declared `extern` in header, defined in `.cpp`

4. **Circular dependencies** - Use forward declarations and include order:
   ```cpp
   // In header:
   class Device; // forward decl

   // In .cpp:
   #include "device.hpp" // full definition
   ```
   - `functions.h` exists specifically to provide forward declarations and break circular includes

5. **LVGL threading** - LVGL calls must be from main thread or protected by `lv_lock()`/`lv_unlock()`
   - Check `lvglIsBusy` and `blockAll` flags before LVGL operations from tasks
   - Charts use `lvglChartIsBusy` flag for additional protection

6. **Memory constraints** - ESP32-S3 has limited RAM; use PSRAM for large buffers, avoid heap fragmentation

7. **Schedule function overloads** - Two signatures exist for different interval parameter types:
   ```cpp
   void schedule(std::function<void(void)> func, unsigned long &&interval, unsigned long &startTime);
   void schedule(std::function<void(void)> func, unsigned long &interval, unsigned long &startTime);
   ```
   Both must be forward-declared if used from another module

## Hardware-Specific Notes

**Pin Configuration** (see `config.hpp`):
- LCD Backlight: GPIO 26 (PWM channel 1)
- ADC DRDY: Interrupt-driven
- I2C: Wire (ADC, DAC), Wire1 (Keypad expander at 0x20)
- Encoders: Interrupt-driven via ESP32Encoder library

**Partition Table**:
- Custom 16MB layout in `par/default_16MB_2.csv`
- Allows OTA updates and large LVGL assets

## Version Management

- Git tags drive version numbering
- `update_version.py` auto-updates `src/version.h` before each build
- Use: `git tag vX.Y.Z` to set version
- Check current: `git describe --tags`

## File Structure Highlights

**Must-read for understanding**:
- `src/device.hpp` - Device class (central control)
- `src/config.hpp` - Global configuration
- `src/main.cpp` - Initialization flow
- `platformio.ini` - Build configuration

**UI Implementation**:
- `src/tabs.h/cpp` - Tabbed interface
- `src/setting_menu.h/cpp` - Settings UI
- `src/DispObject.h` - Display objects (charts, labels, histograms)

**Hardware Abstraction**:
- `src/LTC2655.h/cpp` - DAC driver
- `lib/ADS1219/` - ADC library
- `src/input_device.h/cpp` - Encoder and keypad handling

## Testing & Verification

After making changes:
1. Compilation must succeed
2. For hardware-affecting changes, user tests on actual power supply
3. User confirms with "ok" if power supply works correctly
4. If issues occur, be prepared to rollback via `git reset --hard`

## Current Refactoring Status

**Ongoing Work**: Extracting functions from `globalFunctions.h` to improve maintainability

**Progress**:
- ✅ `globals.h/cpp` - Global variables centralized
- ✅ `functions.h` - Forward declarations for circular dependency resolution
- ✅ `ui_helpers.h/cpp` - Chart and graph UI functions (~400 lines extracted)
- ✅ `tasks.h/cpp` - FreeRTOS task implementations (~200 lines extracted)
- ✅ `input_handler.h/cpp` - Touch calibration and handlers (~100 lines extracted)
- ✅ `intervals.h/cpp` - Schedule function overloads (~20 lines extracted)
- ⏳ Future: Additional modularization of remaining ~3800 lines in `globalFunctions.h`

**File size reduction**: 4,878 lines → 3,865 lines (21% reduction so far)

**Known Issues Being Fixed**:
- Static function linkage errors when calling from new modules
- Multiple definition errors for variables declared in headers
- Missing forward declarations for callback functions
- `CHART_POINTS` constant availability across modules
