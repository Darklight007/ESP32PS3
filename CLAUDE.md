# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Development Commands

```bash
pio run                    # Build project
pio run -t upload          # Upload to ESP32-S3
pio device monitor         # Serial monitor (115200 baud)
pio run -t clean           # Clean build artifacts
```

**Hardware Target:** ESP32-S3-N16R8 (240MHz dual-core, 16MB Flash, 8MB PSRAM)

## Project Overview

Precision laboratory power supply with 24-bit ADC measurement (ADS1219), 16-bit DAC control (LTC2655), touchscreen UI (ILI9341 + LVGL 8.4), rotary encoders, matrix keypad, and SCPI remote control via USB Serial. Includes 18+ waveform function generator.

**Ranges:** 0-32.768V, 0-5.000A (switchable mA mode), ADC: 20/90/330/1000 SPS

## Critical Architecture Concepts

### Dual-Core FreeRTOS Task Assignment

**Core 0 (ADC/DAC critical path):**
- `Task_ADC` (Priority 1): ADC interrupt handling, measurement processing, INL calibration, DAC updates, input polling
- `Task_BarGraph` (Priority 0): Bar graph rendering (main page only)

**Core 1 (UI/SCPI):**
- Main loop: LVGL timer handler (UI rendering), SCPI command processing, status bar updates, power management, statistics calculation

**CRITICAL:** LVGL is NOT thread-safe. All LVGL calls MUST happen on Core 1 (main loop). Never call LVGL functions from Task_ADC or Task_BarGraph without proper synchronization. Use `lvglIsBusy` and `blockAll` flags when necessary.

### Hardware I2C Bus Separation

Two independent I2C buses to prevent conflicts:
- **Wire** (SDA=15, SCL=16 @ 1.7MHz): MCP23017 keypad expander (0x20), LTC2655 DAC (0x41)
- **Wire1** (SDA=17, SCL=18 @ 1.7MHz): ADS1219 ADC (0x40)

**Why:** ADC runs on interrupt-driven cycle; separating buses prevents DAC writes from blocking ADC reads.

### Central Device Class Pattern

`Device` class (`device.hpp/cpp`) is the single source of truth for all power supply state:
- Contains two `DispObject` instances (Voltage, Current) for measurement/display
- Manages device states: `OFF, ON, VC, CC, FUN, INFO, OVP, OCP, OTP, WARNING, ERROR, FAILURE, SAFEMODE`
- Handles mode transitions, calibration data, settings persistence
- All other modules interact through `PowerSupply` global instance

### Measurement Data Flow

```
ADS1219 ADC → GPIO9 DRDY interrupt → Task_ADC
  → Raw ADC codes → INL calibration correction → DispObject updates
  → Statistics (histogram, mean, stddev) → LVGL display updates (Core 1)
  → DAC output adjustment (CC/CV feedback loop)
```

### DAC Update Timing

- **Normal mode:** 100ms interval (10 Hz)
- **Function Generator mode:** 5ms interval (200 Hz) - optimized for waveform generation with minimal jitter
- Updates happen in Task_ADC (Core 0) for deterministic timing

### FUN Only Mode

Special optimization when function generator needs minimal UI overhead:
- Disables bar graph task updates
- Reduces graph refresh rate (500ms vs 125ms)
- Keyboard polling goes to ultra-fast (2ms when typing, otherwise 10ms)
- Allows maximum waveform generation performance

## Code Structure

```
src/
├── main.cpp              # Setup, main loop (Core 1)
├── SetupHandlers.cpp     # Hardware initialization sequence
├── device.hpp/cpp        # Central Device class (state management)
├── DispObject.h/cpp      # Measurement display objects (Voltage/Current)
├── tasks.cpp             # FreeRTOS tasks (Task_ADC, Task_BarGraph)
├── intervals.cpp         # Scheduled periodic functions
├── ui_creation.cpp       # LVGL UI layout (5 tabs: Stats/Graph/Main/Utility/Settings)
├── ui_helpers.cpp        # UI event callbacks
├── input_handler.cpp     # Encoder/keypad/touch input routing
├── waveform_generator.cpp # Function generator (18 waveforms)
├── scpi_parser.cpp       # SCPI IEEE 488.2 command parser
├── power_management.cpp  # Timer, energy counter, OVP/OCP
├── memory_monitor.cpp    # Heap tracking
├── calib_*.cpp           # Calibration modules (ADC, DAC, INL)
├── setting_menu.cpp      # Settings page UI
├── memory.cpp            # Preferences/SPIFFS save/load
└── LTC2655.cpp           # DAC driver
```

## Conventions

- **Classes:** PascalCase (`Device`, `DispObject`)
- **Functions:** camelCase (`keyCheckLoop`, `handleGraphPage`)
- **Files:** snake_case (`input_handler.cpp`, `waveform_generator.cpp`)
- **Indentation:** 4 spaces
- **Standard:** C++20 (`-std=gnu++2a`)

## Common Modification Patterns

### Adding a New Setting
1. Add field to `SettingParameters` struct in `device.hpp`
2. Create UI control in `setting_menu.cpp`
3. Add save/load handlers in `memory.cpp` (use Preferences API for small data, SPIFFS for >1984 bytes)

### Adding a SCPI Command
1. Add command handler in `scpi_parser.cpp` (follow IEEE 488.2/SCPI-1999 conventions)
2. Update error queue handling if needed
3. Document in `docs/SCPI_Commands.md`

### Adding a Waveform
1. Define waveform function in `waveform_generator.cpp` (signature: `double func(double t)` where t ∈ [0,1])
2. Add to `waveforms[]` array with name
3. UI table auto-populates from array

### Adding a UI Page
1. Add page object to `PowerSupply.page[]` array
2. Create layout in `ui_creation.cpp` using `lv_tabview_add_tab()`
3. Add input handler in `input_handler.cpp::managePageEncoderInteraction()` switch
4. Add keyboard shortcuts in `keyCheckLoop()` if needed

## Critical Gotchas

### LVGL Thread Safety
NEVER call LVGL functions from Core 0 tasks. LVGL runs exclusively on Core 1 main loop. Violating this causes crashes, memory corruption, and display artifacts.

### I2C Recovery
If I2C bus hangs (devices stop responding), check `i2c_recovery.cpp` for recovery routine. Common causes: noise, wire length, improper pull-ups.

### Calibration Data Storage Limits
- **Preferences (NVS):** Max ~1984 bytes per namespace. Use for ADC/DAC coefficients.
- **SPIFFS:** Use for large arrays (INL calibration tables with 100+ points).

### ADC Timing Sensitivity
ADC DRDY interrupt (GPIO9) drives the entire measurement loop. Don't block this interrupt. Keep ISR handlers minimal.

### Display DMA Alignment
TFT_eSPI uses DMA for fast transfers. Buffer alignment matters. If you see display artifacts, verify DMA buffer allocation.

### INL Calibration Accuracy
For <0.01% accuracy, INL calibration is required. Standard ADC calibration gives ~±0.05%. INL correction uses monotone cubic interpolation across 100+ reference points.

### Version Auto-Generation
`src/version.h` is auto-generated by `update_version.py` (runs pre-build). Don't edit manually. Version format: `0.0.2-<commit_count>`.

## Documentation References

- `README.md` - Comprehensive project overview with hardware specs, architecture diagrams, SCPI examples
- `docs/SCPI_Commands.md` - Complete SCPI command reference
- `docs/STATE_MACHINES.md` - Device state machine and control flow diagrams
- `platformio.ini` - Build flags, partition table, optimization settings

## Development Workflow Notes

- Build optimization: `-O3` for release builds
- Logging: Adjust `CORE_DEBUG_LEVEL` in platformio.ini (currently 4 = INFO)
- Partitions: Using `par/default_16MB_2.csv` (huge_app configuration)
- Upload speed: 921600 baud
- Monitor speed: 115200 baud
