# S3_PS_lvgl_8.4 - ESP32-S3 Precision Laboratory Power Supply

## Quick Reference

**Build & Upload:**
```bash
pio run                    # Build
pio run -t upload          # Upload
pio device monitor         # Serial monitor (115200 baud)
```

**Hardware Target:** ESP32-S3-N16R8 (240MHz dual-core, 16MB Flash, 8MB PSRAM)

## Project Overview

Programmable laboratory power supply with:
- 24-bit ADC measurement (ADS1219)
- 16-bit DAC output control (LTC2655)
- Touchscreen UI (ILI9341 + LVGL 8.4)
- Rotary encoders + matrix keypad
- SCPI remote control via USB Serial
- Function generator (18+ waveforms)

### Capability Ranges
- Voltage: 0 - 32.768V
- Current: 0 - 5.000A (switchable mA mode)
- ADC: 20/90/330/1000 SPS

## Hardware Architecture

### I2C Buses
| Bus | Pins | Devices |
|-----|------|---------|
| Wire | SDA=15, SCL=16 | MCP23017 (0x20), LTC2655 (0x41) |
| Wire1 | SDA=17, SCL=18 | ADS1219 (0x40) |

### Key Pins
- SPI Display: MOSI=11, MISO=13, CLK=12, CS=10, DC=14
- Encoder1 (V): A=5, B=4
- Encoder2 (I): A=7, B=6
- ADC DRDY IRQ: GPIO9
- Backlight PWM: GPIO26

## Code Structure

```
src/
├── main.cpp              # Setup + main loop
├── SetupHandlers.cpp     # Hardware init
├── device.hpp/cpp        # Central Device class (state management)
├── DispObject.h/cpp      # Measurement objects (Voltage/Current/Power)
├── tasks.cpp             # FreeRTOS tasks
├── ui_creation.cpp       # LVGL UI layout (5 tabs)
├── ui_helpers.cpp        # UI callbacks
├── input_handler.cpp     # Encoder/keypad/touch
├── waveform_generator.cpp # Function generator
├── scpi_parser.cpp       # SCPI commands
├── calib_*.cpp           # Calibration (ADC, DAC, INL)
└── LTC2655.cpp           # DAC driver
```

### FreeRTOS Tasks
- **Task_ADC** (Core 0, Priority 1): ADC sampling, DAC updates, input polling
- **Task_BarGraph** (Core 0, Priority 0): Bar graph rendering
- **Main Loop** (Core 1): LVGL, SCPI, UI, power management

## Conventions

- **Classes**: PascalCase
- **Functions**: camelCase
- **Files**: snake_case
- **Indent**: 4 spaces, C++20

### Device States
`OFF, ON, VC, CC, FUN, INFO, OVP, OCP, OTP, WARNING, ERROR, FAILURE, SAFEMODE`

## Current Development Focus

Recent work on **FUN Only mode** optimization:
- Waveform generation with minimal jitter
- DAC update rate tuning (50 Hz)
- Touch/keyboard control while in FUN mode

## Common Tasks

### Adding a new setting
1. Add to `SettingParameters` struct in `device.hpp`
2. Add UI in `setting_menu.cpp`
3. Add save/load in `memory.cpp`

### Adding SCPI command
1. Add handler in `scpi_parser.cpp`
2. Document in `docs/SCPI_Commands.md`

### Calibration data
- Small data: Preferences API (NVS)
- Large data (>1984 bytes): SPIFFS

## Troubleshooting

- **I2C hang**: Check `i2c_recovery.cpp` - recovery routine available
- **Display artifacts**: Verify DMA buffer alignment
- **ADC noise**: Check INL calibration in `calib_inl.cpp`

## Documentation
- `README.md` - Full project overview
- `docs/SCPI_Commands.md` - SCPI reference
- `docs/STATE_MACHINES.md` - Mode transitions

# my Request:
Always use git before any major changes, so for any request, git commit first and then edit the code
Also dont forget to push the git