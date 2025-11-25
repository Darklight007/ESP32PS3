# State Machines and Control Flow Documentation

This document describes the key state machines and control flow in the ESP32-S3 Power Supply firmware.

## Table of Contents
1. [Device State Machine](#device-state-machine)
2. [Power Management Flow](#power-management-flow)
3. [ADC/DAC Update Cycle](#adcdac-update-cycle)
4. [Calibration State Flow](#calibration-state-flow)
5. [SCPI Command Processing](#scpi-command-processing)

---

## Device State Machine

The power supply has 5 primary operating states managed by the `DEVICE` enum:

```
┌─────────────────────────────────────────────────────────┐
│                   Device State Machine                  │
└─────────────────────────────────────────────────────────┘

                         ┌─────┐
                         │ OFF │◄───────────────┐
                         └──┬──┘                │
                            │                   │
                   Power ON │                   │ Power OFF
                     │      │                   │ OVP/OCP Trigger
                     │      ▼                   │ Timer Expire
                     │   ┌────┐                 │
                     └──►│ ON │─────────────────┤
                         └─┬──┘                 │
                           │                    │
         ┌─────────────────┼─────────────┐      │
         │                 │             │      │
         ▼                 ▼             ▼      │
     ┌──────┐          ┌──────┐      ┌─────┐   │
     │  VC  │          │  CC  │      │ FUN │   │
     │(CV)  │◄────────►│(CI)  │      │     │   │
     └──────┘          └──────┘      └─────┘   │
         │                 │             │      │
         └─────────────────┴─────────────┴──────┘
```

### State Definitions

| State | Description | Entry Condition |
|-------|-------------|-----------------|
| **OFF** | Output disabled, DAC at offset values | Power switch OFF, startup, OVP/OCP trigger |
| **ON** | Output enabled, transitioning to VC/CC | Power switch ON (brief) |
| **VC** | Voltage Control (CV) mode | Output voltage < setpoint, current available |
| **CC** | Current Control (CI) mode | Output current limited by setpoint |
| **FUN** | Function generator active | Function generator enabled |

### State Transitions

**Device.cpp:641-664** (`Device::turn()`, `Device::toggle()`)

```cpp
OFF ──[Power ON]──> ON ──[Auto]──> VC/CC (based on CCCV pin)
VC/CC ──[Power OFF]──> OFF
VC/CC ──[OVP/OCP]──> OFF (power_management.cpp:149-201)
VC/CC ──[Timer]──> OFF (power_management.cpp:80-92)
```

**Key Functions:**
- `Device::turn(SWITCH onOff)` - Direct state change
- `Device::toggle()` - Toggle between ON/OFF
- `Device::setStatus(DEVICE state)` - Update state and UI (device.cpp:747-864)

---

## Power Management Flow

The power management system handles timers, energy counting, and protection.

```
┌──────────────────────────────────────────────────────────┐
│              Power Management Flow Chart                 │
└──────────────────────────────────────────────────────────┘

    Power Supply State: ON
           │
           ▼
    ┌──────────────────┐
    │ Initialize PM    │ (power_management.cpp:229-244)
    │ - Start timer    │
    │ - Reset energy   │
    │ - Start duration │
    └────────┬─────────┘
             │
             ▼
    ┌────────────────────────┐
    │   Main Loop Tasks      │
    ├────────────────────────┤
    │  1. Integrate Energy   │──► Power × Time → Wh
    │  2. Update Displays    │──► Show counter/time
    │  3. Check Timer        │──► Expire → OFF
    │  4. Enforce Limits     │──► OVP/OCP check
    │  5. Auto-Save Check    │──► Periodic save
    └────────┬───────────────┘
             │
             │ Every loop iteration
             ▼
    ┌──────────────────┐
    │  integrateEnergy │ (power_management.cpp:102-135)
    └──────────────────┘
             │
             ▼
    ┌──────────────────┐
    │  updateTimerDisplay │ (power_management.cpp:60-99)
    └──────────────────┘
             │
             ▼
    ┌──────────────────┐
    │ enforceSoftwareLimits │ (power_management.cpp:138-202)
    │  - Check OVP      │
    │  - Check OCP      │
    │  - Trigger OFF if │
    │    limit exceeded │
    └──────────────────┘
             │
             ▼
    ┌──────────────────┐
    │  autoSaveCheck   │ (power_management.cpp:205-226)
    └──────────────────┘
```

### Energy Integration Algorithm

**File:** `power_management.cpp:102-135`

```
Energy (Wh) += Power (W) × ΔTime (hours)

Where:
  Power = Voltage × Current
  ΔTime = (currentMillis - lastUpdateMillis) / 3600000

Current conversion: If mA mode → Current / 1000
```

### Timer Operation

**File:** `power_management.cpp:60-99`

```
IF timer enabled AND timer started:
    elapsed = (millis() - timerStartTime) / 1000
    remaining = timerDurationSeconds - elapsed

    IF remaining > 0:
        Display countdown
    ELSE:
        Turn output OFF
        Reset timer
        Beep (if enabled)
```

---

## ADC/DAC Update Cycle

The ADC/DAC system runs in a tight loop on Core 0 with interrupt-driven sampling.

```
┌──────────────────────────────────────────────────────────┐
│              ADC/DAC Update Cycle (Core 0)               │
└──────────────────────────────────────────────────────────┘

  ADS1219 ADC            ESP32-S3           LTC2655 DAC
  ────────────           ────────           ────────────

     [Voltage]              │                    │
        │                   │                    │
        │  Conversion       │                    │
        │  Complete         │                    │
        │  (DRDY LOW)       │                    │
        │                   │                    │
        └──INTERRUPT────────┤                    │
                            │                    │
                    ┌───────▼────────┐           │
                    │  ISR Handler   │           │
                    │  Set flag      │           │
                    │  Notify task   │           │
                    └───────┬────────┘           │
                            │                    │
                    ┌───────▼─────────────┐      │
                    │  Task_ADC awakens   │      │
                    │  readVoltage()      │      │
                    │  - Read raw value   │      │
                    │  - Apply calib      │      │
                    │  - Apply INL        │      │
                    │  - Update stats     │      │
                    │  - Update histogram │      │
                    └───────┬─────────────┘      │
                            │                    │
                            │  Start current     │
                            │  conversion        │
                            │                    │
                            ▼                    │
                      [Current]                  │
                         │                       │
                         │  Conversion           │
                         │  Complete             │
                         └──INTERRUPT────────┐   │
                                             │   │
                            ┌────────────────▼───┤
                            │  ISR Handler       │
                            │  Set flag          │
                            │  Notify task       │
                            └────────┬───────────┘
                                     │
                            ┌────────▼────────────┐
                            │  Task_ADC awakens   │
                            │  readCurrent()      │
                            │  - Read raw value   │
                            │  - Apply calib      │
                            │  - Compensate leak  │
                            │  - Update stats     │
                            │  - Update histogram │
                            └────────┬────────────┘
                                     │
                            ┌────────▼────────────┐
                            │  Calculate Power    │
                            │  P = V × I          │
                            └────────┬────────────┘
                                     │
                            ┌────────▼────────────┐
                            │   DAC Update        │
                            │   (100Hz normal)    │
                            │   (200Hz fgen)      │
                            │                     │
                            │  Write voltage  ────┼──► [CH_A]
                            │  Write current  ────┼──► [CH_B]
                            └─────────────────────┘
                                     │
                                     │  Loop back
                                     └──► Start voltage conversion
```

### ADC Processing Details

**Voltage Path** (device.cpp:351-414):
1. Read raw ADC value (24-bit)
2. Apply linear calibration: `v_ideal = (raw - b) × (1/m)`
3. Apply rate compensation (depends on SPS setting)
4. Apply INL correction (if calibrated): `v_corrected = INL(v_ideal)`
5. Update running statistics (mean, stddev, min, max)
6. Update histogram bin
7. Update bar graph value

**Current Path** (device.cpp:416-474):
1. Read raw ADC value (24-bit)
2. Apply linear calibration: `c_ideal = (raw - b) × (1/m)`
3. Compensate internal leakage: `c = c_ideal - (V / R_internal)`
4. Apply mA/A scaling if needed
5. Update running statistics
6. Update histogram bin
7. Update bar graph value

### Task Timing

**File:** `tasks.cpp:86-178`

| Interval | Function | Rate |
|----------|----------|------|
| ADC Interrupt | 20-1000 SPS | Configurable |
| DAC Update | 100ms | Normal mode |
| DAC Update | 5ms | Function generator |
| Graph Refresh | 125ms | Page 1 only |
| Histogram | Immediate | Page 0 only |
| Keyboard | 10-105ms | Context-dependent |

---

## Calibration State Flow

Calibration proceeds through multiple stages with validation at each step.

```
┌──────────────────────────────────────────────────────────┐
│                  Calibration State Flow                  │
└──────────────────────────────────────────────────────────┘

     [User Action]
           │
           ▼
    ┌─────────────┐
    │  Select     │
    │  Calibration│
    │  Type       │
    └──────┬──────┘
           │
           ├───► ADC Calibration ───► calib_adc.cpp
           │
           ├───► DAC Calibration ───► calib_dac.cpp
           │
           └───► INL Calibration ───► calib_inl.cpp
                      │
                      ▼
            ┌─────────────────┐
            │  INL Wizard     │
            │  State Machine  │
            └─────────┬───────┘
                      │
        ┌─────────────┼─────────────┬──────────────┐
        │             │             │              │
        ▼             ▼             ▼              ▼
    [IDLE]      [MEASURING]   [COMPUTING]    [COMPLETE]
        │             │             │              │
        │             │             │              │
        │  Start ────►│             │              │
        │             │             │              │
        │             │  Done ─────►│              │
        │             │             │              │
        │             │             │  Success ───►│
        │             │             │              │
        │◄────────────┴─────────────┴──────────────┘
        │               Cancel/Error
        │
        ▼
    [Validate]
        │
        ├───► Check for NaN/Inf
        ├───► Check monotonicity
        └───► Save if valid
                │
                └───► LoadCalibrationData()
                          │
                          ▼
                   [Apply to measurements]
```

### INL Calibration States

**File:** `calib_inl.cpp:47-563`

| State | Description | Actions |
|-------|-------------|---------|
| **IDLE** | Waiting to start | Show start button |
| **MEASURING** | Taking measurements | Sweep voltages, collect ADC data |
| **COMPUTING** | Processing data | Fit monotone cubic spline |
| **COMPLETE** | Calibration done | Save calibration, enable INL |
| **ERROR** | Calibration failed | Show error, allow retry |

### Calibration Validation

**File:** `device.cpp:127-156`

```
ON LOAD:
  IF any calibration coefficient is NaN or Inf:
    Log error
    Load factory defaults
    Save factory calibration

  ELSE:
    Apply loaded calibration
```

---

## SCPI Command Processing

SCPI commands are parsed and executed with full validation and error reporting.

```
┌──────────────────────────────────────────────────────────┐
│              SCPI Command Processing Flow                │
└──────────────────────────────────────────────────────────┘

    Serial Input
         │
         ▼
  ┌─────────────┐
  │ Receive char│
  │ Buffer cmd  │
  └──────┬──────┘
         │
         │  Terminator (\n or \r)
         ▼
  ┌──────────────┐
  │ Parse command│──► Split into command + parameter
  │ Trim spaces  │
  └──────┬───────┘
         │
         ▼
  ┌──────────────────────┐
  │  Command Router      │
  ├──────────────────────┤
  │  *IDN? ────► IEEE488 │
  │  *RST  ────► IEEE488 │
  │  :SOUR ────► Source  │
  │  :MEAS ────► Measure │
  │  :OUTP ────► Output  │
  │  :SYST ────► System  │
  └──────┬───────────────┘
         │
         ▼
  ┌──────────────────┐
  │  Validate Params │
  ├──────────────────┤
  │  • Parse numeric │
  │  • Check range   │
  │  • Check type    │
  └──────┬───────────┘
         │
         ├──► VALID ──► Execute command ──► Send response
         │
         └──► INVALID ──► Add error to queue
                             │
                             ▼
                      ┌──────────────┐
                      │ Error Queue  │
                      │ (16 entries) │
                      └──────────────┘
                             │
                             │  Query with :SYST:ERR?
                             ▼
                      Return oldest error
```

### SCPI Error Handling

**File:** `scpi_parser.cpp:1-900+`

Error codes follow IEEE 488.2 standard:

| Code | Category | Example |
|------|----------|---------|
| -100 to -199 | Command errors | Undefined header |
| -200 to -299 | Execution errors | Data out of range |
| -300 to -399 | Device errors | Hardware failure |
| -400 to -499 | Query errors | Query unterminated |

### Command Validation Flow

**Enhanced validation** (scpi_parser.cpp:464-540):

```
SOUR:VOLT <value>
    │
    ├───► Parse numeric ──► FAIL ──► ERR_DATA_TYPE_ERROR
    │                                  + Log validation error
    │
    └───► Check range (0 to max) ──► FAIL ──► ERR_DATA_OUT_OF_RANGE
                                                + Log expected range
                 │
                 └───► PASS ──► Apply to DAC
```

---

## Integration Points

### Main Loop (Core 1)

**File:** `main.cpp:108-132`

```cpp
void loop() {
    // Power management
    integrateEnergy();
    updateEnergyAndTimeDisplays();
    updateTimerDisplay();
    enforceSoftwareLimits();
    autoSaveCheck();

    // SCPI processing
    scpiParser.process();

    // LVGL updates
    lv_timer_handler();

    // Yield to other tasks
    delay(1);
}
```

### Task_ADC (Core 0)

**File:** `tasks.cpp:86-178`

```cpp
void Task_ADC(void *pvParameters) {
    for (;;) {
        // DAC updates
        schedule(dacUpdate, interval, timer);

        // Input handling
        getKeyboard();
        getEncoder();

        // Wait for ADC
        ulTaskNotifyTake(timeout);

        // Process measurements
        if (adcDataReady) {
            readVoltage();
            readCurrent();
            calculatePower();

            // Update visualizations
            updateHistogram();
            updateGraph();
        }
    }
}
```

---

## Performance Characteristics

### Typical Timings

| Operation | Time | Notes |
|-----------|------|-------|
| ADC Conversion | 1-50ms | Depends on SPS (20-1000) |
| DAC Update | ~2ms | I2C transaction |
| LVGL Refresh | 5-20ms | Depends on complexity |
| INL Correction | ~10µs | Cubic interpolation |
| Energy Integration | <1µs | Floating point math |
| SCPI Parse | 50-200µs | String operations |

### Memory Usage

| Component | RAM | Flash |
|-----------|-----|-------|
| LVGL Buffers | ~38KB | - |
| Graph Data | 8KB | - |
| Histogram Data | 2KB | - |
| Task Stacks | ~16KB | - |
| Error Handler | <1KB | 2KB |
| Total Application | 61KB | 1.5MB |

---

## Safety and Reliability

### Protection Mechanisms

1. **OVP (Over-Voltage Protection)**
   - Software limit check every loop
   - Immediate shutdown on trigger
   - Beep + status flag

2. **OCP (Over-Current Protection)**
   - Software limit check every loop
   - Immediate shutdown on trigger
   - Beep + status flag

3. **Timer-Based Shutdown**
   - Countdown from user-set duration
   - Automatic OFF when expired
   - Persistent across resets (if saved)

4. **Watchdog Timer**
   - 120-second timeout
   - Prevents system hang
   - Reset on timeout

### Error Recovery

1. **Invalid Calibration**
   - Auto-revert to factory defaults
   - Log error via ErrorHandler
   - Save known-good values

2. **Settings Corruption**
   - Magic number validation (314 = π)
   - Load defaults if invalid
   - Clamp ADC parameters to valid range

3. **SCPI Errors**
   - Queued for retrieval
   - Non-blocking operation
   - Standard error codes

---

## Summary

The ESP32-S3 Power Supply uses well-defined state machines and control flows to ensure:

✅ **Deterministic behavior** - Predictable state transitions
✅ **Safety** - Multiple protection mechanisms
✅ **Reliability** - Error recovery and validation
✅ **Performance** - Optimized hot paths (ADC/DAC)
✅ **Maintainability** - Clear separation of concerns

This documentation serves as a reference for understanding system behavior, debugging issues, and implementing new features.
