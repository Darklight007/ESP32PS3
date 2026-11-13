# SCPI Command Reference

## Overview

This ESP32-S3 power supply implements SCPI (Standard Commands for Programmable Instruments) following IEEE 488.2 and SCPI-1999 standards. Commands can be sent via USB Serial at 115200 baud.

**Command Format:**
- Commands are case-insensitive
- Commands can be in long or short form (e.g., `:SOURCE:VOLTAGE` or `:SOUR:VOLT`)
- Queries end with `?`
- Commands are terminated with newline (`\n`) or carriage return (`\r`)
- Multiple commands can be separated by semicolon (`;`)

**Example Session:**
```
*IDN?
Anthropic,ESP32-PS,348518908788,v0.0.2-138-g3c504a2

:SOUR:VOLT 5.0
:SOUR:CURR 1.5
:OUTP:STAT ON
:MEAS:VOLT?
5.001234
:MEAS:CURR?
0.512345
```

---

## IEEE 488.2 Common Commands

### *IDN? - Identification Query
**Description:** Returns instrument identification string
**Syntax:** `*IDN?`
**Response:** `Manufacturer,Model,SerialNumber,FirmwareVersion`
**Example:**
```
*IDN?
Anthropic,ESP32-PS,348518908788,v0.0.2-138-g3c504a2
```

### *RST - Reset
**Description:** Reset instrument to default state (0V, 0A)
**Syntax:** `*RST`
**Example:**
```
*RST
```

### *CLS - Clear Status
**Description:** Clear all status registers and error queue
**Syntax:** `*CLS`
**Example:**
```
*CLS
```

### *ESE - Event Status Enable
**Description:** Set/query event status enable register
**Syntax:** `*ESE <value>` or `*ESE?`
**Parameters:** `<value>` - Byte value (0-255)
**Example:**
```
*ESE 255
*ESE?
255
```

### *ESR? - Event Status Register Query
**Description:** Query and clear event status register
**Syntax:** `*ESR?`
**Response:** Byte value (0-255)
**Note:** Reading clears the register
**Example:**
```
*ESR?
0
```

### *OPC - Operation Complete
**Description:** Set operation complete bit when all operations finish
**Syntax:** `*OPC` or `*OPC?`
**Example:**
```
*OPC
*OPC?
1
```

### *SRE - Service Request Enable
**Description:** Set/query service request enable register
**Syntax:** `*SRE <value>` or `*SRE?`
**Parameters:** `<value>` - Byte value (0-255)
**Example:**
```
*SRE 32
*SRE?
32
```

### *STB? - Status Byte Query
**Description:** Query status byte register
**Syntax:** `*STB?`
**Response:** Byte value (0-255)
**Status Bits:**
- Bit 2: Error queue not empty
- Bit 4: Message available (MAV)
**Example:**
```
*STB?
0
```

### *TST? - Self-Test Query
**Description:** Perform self-test
**Syntax:** `*TST?`
**Response:** `0` = pass, `1` = fail
**Example:**
```
*TST?
0
```

### *WAI - Wait to Continue
**Description:** Wait for all pending operations to complete
**Syntax:** `*WAI`
**Example:**
```
*WAI
```

---

## SOURce Subsystem (Output Control)

### :SOURce:VOLTage - Set Output Voltage
**Description:** Set the output voltage setpoint
**Syntax:** `:SOUR:VOLT <value>` or `:SOUR:VOLT?`
**Short Form:** `:SOUR:V`
**Parameters:** `<value>` - Voltage in volts (0 to max)
**Units:** V, mV, uV, kV supported
**Example:**
```
:SOUR:VOLT 12.5
:SOUR:VOLT?
12.500000
:SOUR:VOLT 3.3V
:SOUR:VOLT 500mV
```

### :SOURce:CURRent - Set Output Current
**Description:** Set the output current setpoint
**Syntax:** `:SOUR:CURR <value>` or `:SOUR:CURR?`
**Short Form:** `:SOUR:C`
**Parameters:** `<value>` - Current in amps (0 to max)
**Units:** A, mA, uA, kA supported
**Example:**
```
:SOUR:CURR 2.5
:SOUR:CURR?
2.500000
:SOUR:CURR 500mA
:SOUR:CURR 100uA
```

### :SOURce:VOLTage:LIMit - Voltage Limit Query
**Description:** Query maximum voltage capability
**Syntax:** `:SOUR:VOLT:LIM?`
**Response:** Maximum voltage in volts
**Example:**
```
:SOUR:VOLT:LIM?
32.768000
```

### :SOURce:CURRent:LIMit - Current Limit Query
**Description:** Query maximum current capability
**Syntax:** `:SOUR:CURR:LIM?`
**Response:** Maximum current in amps
**Example:**
```
:SOUR:CURR:LIM?
5.000000
```

### :SOURce:FUNCtion - Operating Mode
**Description:** Set/query operating mode
**Syntax:** `:SOUR:FUNC <mode>` or `:SOUR:FUNC?`
**Parameters:**
- `CV` or `VOLT` - Constant Voltage mode
- `FGEN` or `FUNC` - Function Generator mode
**Example:**
```
:SOUR:FUNC CV
:SOUR:FUNC?
CV
:SOUR:FUNC FGEN
:SOUR:FUNC?
FGEN
```

---

## MEASure Subsystem (Measurements)

### :MEASure:VOLTage? - Measure Output Voltage
**Description:** Query measured output voltage
**Syntax:** `:MEAS:VOLT?`
**Response:** Voltage in volts (6 decimal places)
**Example:**
```
:MEAS:VOLT?
12.504523
```

### :MEASure:CURRent? - Measure Output Current
**Description:** Query measured output current
**Syntax:** `:MEAS:CURR?`
**Response:** Current in amps (6 decimal places)
**Example:**
```
:MEAS:CURR?
1.523412
```

### :MEASure:POWer? - Measure Output Power
**Description:** Query calculated output power (V × I)
**Syntax:** `:MEAS:POW?`
**Response:** Power in watts (6 decimal places)
**Example:**
```
:MEAS:POW?
19.045623
```

### :MEASure:VOLTage:MIN? - Minimum Voltage
**Description:** Query minimum measured voltage since last reset
**Syntax:** `:MEAS:VOLT:MIN?`
**Response:** Minimum voltage in volts
**Example:**
```
:MEAS:VOLT:MIN?
12.498234
```

### :MEASure:VOLTage:MAX? - Maximum Voltage
**Description:** Query maximum measured voltage since last reset
**Syntax:** `:MEAS:VOLT:MAX?`
**Response:** Maximum voltage in volts
**Example:**
```
:MEAS:VOLT:MAX?
12.512456
```

### :MEASure:VOLTage:MEAN? - Mean Voltage
**Description:** Query average measured voltage
**Syntax:** `:MEAS:VOLT:MEAN?`
**Response:** Mean voltage in volts
**Example:**
```
:MEAS:VOLT:MEAN?
12.504234
```

### :MEASure:CURRent:MIN? - Minimum Current
**Description:** Query minimum measured current since last reset
**Syntax:** `:MEAS:CURR:MIN?`
**Response:** Minimum current in amps
**Example:**
```
:MEAS:CURR:MIN?
1.521234
```

### :MEASure:CURRent:MAX? - Maximum Current
**Description:** Query maximum measured current since last reset
**Syntax:** `:MEAS:CURR:MAX?`
**Response:** Maximum current in amps
**Example:**
```
:MEAS:CURR:MAX?
1.525678
```

### :MEASure:CURRent:MEAN? - Mean Current
**Description:** Query average measured current
**Syntax:** `:MEAS:CURR:MEAN?`
**Response:** Mean current in amps
**Example:**
```
:MEAS:CURR:MEAN?
1.523456
```

---

### :SENSe:CURRent:RANGe - Current Measurement Range
**Description:** Select the A or mA current range, same as the mA key
**Syntax:** `:SENS:CURR:RANG <A|MA>` / `:SENS:CURR:RANG?`
**Response:** `OK`; query returns `A` or `MA`

---

## OUTPut Subsystem (Output Control)

### :OUTPut:STATe - Output Enable/Disable
**Description:** Enable or disable the power supply output
**Syntax:** `:OUTP:STAT <state>` or `:OUTP:STAT?`
**Parameters:**
- `ON` or `1` - Enable output
- `OFF` or `0` - Disable output
**Example:**
```
:OUTP:STAT ON
:OUTP:STAT?
1
:OUTP:STAT OFF
:OUTP:STAT?
0
```

### :OUTPut:PROTection:CLEar - Clear Protection Trip
**Description:** Clear protection circuit trip condition
**Syntax:** `:OUTP:PROT:CLE`
**Example:**
```
:OUTP:PROT:CLE
```

### :OUTPut:PROTection:TRIPped? - Query Protection Status
**Description:** Query if protection circuit has tripped
**Syntax:** `:OUTP:PROT:TRIP?`
**Response:** `1` = tripped, `0` = normal
**Example:**
```
:OUTP:PROT:TRIP?
0
```

---

## SYSTem Subsystem (System Control)

### :SYSTem:ERRor? - Query Error
**Description:** Query and remove oldest error from error queue
**Syntax:** `:SYST:ERR?`
**Response:** `<code>,"<description>"`
**Example:**
```
:SYST:ERR?
0,"No error"
:SYST:ERR?
-113,"Undefined header"
```

### :SYSTem:ERRor:COUNt? - Query Error Count
**Description:** Query number of errors in error queue
**Syntax:** `:SYST:ERR:COUN?`
**Response:** Number of errors (0-10)
**Example:**
```
:SYST:ERR:COUN?
0
```

### :SYSTem:ERRor:ALL? - Query All Errors
**Description:** Query and remove all errors from error queue
**Syntax:** `:SYST:ERR:ALL?`
**Response:** All errors separated by semicolon
**Example:**
```
:SYST:ERR:ALL?
0,"No error"
```

### :SYSTem:VERSion? - Query SCPI Version
**Description:** Query SCPI version compliance
**Syntax:** `:SYST:VERS?`
**Response:** SCPI version string
**Example:**
```
:SYST:VERS?
1999.0
```

### :SYSTem:CAPability? - Query Capabilities
**Description:** Query instrument voltage/current capabilities
**Syntax:** `:SYST:CAP?`
**Response:** Capability string
**Example:**
```
:SYST:CAP?
VOLT:MIN 0;MAX 32.768;CURR:MIN 0;MAX 5.000
```

### :SYSTem:BEEPer - Beep
**Description:** Trigger instrument beep
**Syntax:** `:SYST:BEEP`
**Example:**
```
:SYST:BEEP
```

### :SYSTem:BEEPer:SIRen - Siren Demo
**Description:** Start the Beeper page's tornado-siren demo (two wails, ~22 s); sent while playing, stops it. Silent when the buzzer is off.
**Syntax:** `:SYST:BEEP:SIR`

### :SYSTem:LOCal - Return to Local Control
**Description:** Return to front panel local control mode
**Syntax:** `:SYST:LOC`
**Example:**
```
:SYST:LOC
```

### :SYSTem:REMote - Enter Remote Control Mode
**Description:** Enter remote control mode (may disable front panel)
**Syntax:** `:SYST:REM`
**Example:**
```
:SYST:REM
```

---

## DISPlay Subsystem (Display Performance)

### :SYSTem:LOG? - Event Log
**Description:** Fault/event history kept across restarts (last 40): boots with reset reason, output lost (power/thermal), OVP, OCP. One line per event: `boot,uptime_s,event,value,arg`. `:SYST:LOG:CLE` clears it.
**Syntax:** `:SYST:LOG?`, `:SYST:LOG:CLE`

### :SETTing:PROT:FAULt - Power Lost / Thermal Protection
**Description:** Turn the output-lost protection on or off (Protection page switch)
**Syntax:** `:SETT:PROT:FAUL {0|1}`, `:SETT:PROT:FAUL?`

### :DISPlay:BENCH? - Full-Screen Repaint Time
**Description:** Repaints the whole current page 10 times; returns ms per frame (render + flush)
**Syntax:** `:DISP:BENCH?`

### :DISPlay:PROF? - Per-Widget Repaint Cost
**Description:** For each child of the current page: size, position and the ms it adds to a full repaint (hides it, times 3 frames, shows it again). Takes several seconds.
**Syntax:** `:DISP:PROF?`

### DIAG:ADC:FAST - ADC Cycle Speed-Ups
**Description:** Bit mask, default 7; set 0 to compare noise or speed against the old cycle. Not saved. Ignored parts in low-noise mode (rate 0).
- 1 - start the next conversion before reading the finished result
- 2 - channel select and START in one I2C transfer
- 4 - result read as one I2C transfer
**Syntax:** `DIAG:ADC:FAST <0-7>`, `DIAG:ADC:FAST?`
**Returns:** `mode,late_reads_redone,ready_glitches_ignored,stalls_recovered` (counts since boot)

### :DISPlay:PAGE - Show a Page
**Syntax:** `:DISP:PAGE <0-4>[,<sub-tab>]` (0 Stats, 1 Graph, 2 Main, 3 Utility, 4 Setting; the sub-tab applies to Utility), `:DISP:PAGE?`

### :DISPlay:HIST:FIT - Re-centre the Histogram
**Description:** Same as the stats-reset key on the Stats page: histogram window = mean ±6σ for V and I, statistics cleared.
**Syntax:** `:DISP:HIST:FIT`

### :DISPlay:GRAPH:FIT - Fit the Graph Vertically
**Description:** Same as the Y key on the Graph page: zooms and scrolls the Y axis so the visible part of the V and/or I trace fills the view (both shown together when both are on).
**Syntax:** `:DISP:GRAPH:FIT`

### :DISPlay:BUF - Draw Buffer Mode
**Description:** 1 = two internal strips with overlapped DMA (default); 0 = previous path (small frames in one strip, large ones in PSRAM). For A/B timing; not saved.
**Syntax:** `:DISP:BUF <0|1>`, `:DISP:BUF?`

### :DISPlay:SHOT? - Screenshot
**Description:** Repaints the screen and sends it: 240 lines, each 320 pixels as RGB565 hex (4 digits per pixel), then `END`. About 310 KB of text.
**Syntax:** `:DISP:SHOT?`

### :DISPlay:AREA - Redraw Time of One Rectangle
**Description:** Redraws one screen rectangle 100 times; returns ms per redraw (render + flush)
**Syntax:** `:DISP:AREA <x1>,<y1>,<x2>,<y2>`

### :DISPlay:MEM? - Memory Headroom
**Syntax:** `:DISP:MEM?`
**Returns:** `internal_free,internal_largest_block,psram_free,ui_stack_unused` (bytes)

### :DISPlay:SAMPRUN / :DISPlay:SAMP? - Sampling Profiler
**Description:** `SAMPRUN` records 6000 program-counter samples of the UI core (about 2 s); `SAMP?` prints them as hex, one per line, then `END`. `SAMP <x1>,<y1>,<x2>,<y2>` samples while redrawing one rectangle. Resolve with `addr2line -f -e firmware.elf`.
**Syntax:** `:DISP:SAMPRUN`, `:DISP:SAMP?`

### :DISPlay:FPS? - Frame-Rate Meter
**Description:** Display statistics over the last second
**Syntax:** `:DISP:FPS?`
**Returns:** `fps,px_per_frame,render_ms,flush_ms,busy_pct,adc_sps,max_gap_ms,areas_per_frame`
- `max_gap_ms` - longest wait between two frames (stutter)
- `areas_per_frame` - separate rectangles drawn and sent per frame
- `render_ms` - LVGL drawing per frame (in low-noise mode, rate 0, includes waiting for the ADC)
- `flush_ms` - SPI/DMA transfer per frame
- `busy_pct` - share of Core 1 spent on frames
- `adc_sps` - ADC conversions per second per channel (as on the status bar)
**Example:**
```
:DISP:FPS?
→ 63.8,3854,6.60,1.82,53,310
```

### :DISPlay:FPS - Frame-Rate Overlay
**Description:** Show/hide the same numbers in the bottom-left corner of the screen. Saved; same as Settings → Display → Show FPS
**Syntax:** `:DISP:FPS <0|1|ON|OFF>`
**Example:**
```
:DISP:FPS 1
```

---

## CALibration Subsystem

### :CALibration:STATe? - Query Calibration Status
**Description:** Query if instrument is calibrated
**Syntax:** `:CAL:STAT?`
**Response:** `1` = calibrated, `0` = not calibrated
**Example:**
```
:CAL:STAT?
1
```

### :CALibration:SAVE - Save Calibration
**Description:** Save current calibration data to flash
**Syntax:** `:CAL:SAVE`
**Example:**
```
:CAL:SAVE
```

### :CALibration:LOAD - Load Calibration
**Description:** Load calibration data from flash
**Syntax:** `:CAL:LOAD`
**Example:**
```
:CAL:LOAD
```

### :CALibration:LEAKage - Zero + Leakage Calibration
**Description:** Same as the Int. Resistance button: for the active current range
(A or mA), measures the current with no load at 1, 8, 16, 24 and 32 V and fits
`I = I0 + V/R`. Sets the zero code (`code_1`) and the leakage resistance. Runs at
ADC rate 0 (restored afterwards), about 2.5 min. Output must be on with nothing
connected. Not saved until `:CAL:SAVE`.
**Syntax:** `:CAL:LEAK`
**Response:** `OK` (error -200 if a calibration is already running)

### :CALibration:BUSY? - Calibration Running
**Syntax:** `:CAL:BUSY?`
**Response:** `1` while a calibration routine runs, else `0`
**Example:**
```
:CAL:LEAK
OK
:CAL:BUSY?      (repeat until 0)
0
:CAL:SAVE
```

### :CALibration:CCLIM - Current-Limit Calibration (Shorted Output)
**Description:** With a thick wire across the output and the output ON, steps the current limit 0.1/1/2/3/4/5 A in the A range (ADC rate 0) and stores what actually flows. The current-limit DAC then uses that table, so the limit matches the setpoint. Runs ~2 min, restores the user's V/I setpoints and range, saves itself (no `CAL:SAVE` needed). Fails without changing anything if the output isn't shorted.
**Syntax:** `:CAL:CCLIM`, `:CAL:CCLIM?`, `:CAL:CCLIM:CLEar`
**Response:** `OK`; the query returns `set,actual;...` in A (empty = no table)
**Example:**
```
:CAL:CCLIM
OK
:CAL:BUSY?      (repeat until 0)
0
:CAL:CCLIM?
0.1000,0.10080;1.0000,1.00609;...;5.0000,5.02712
```

### :CALibration:RATE - Speed Match (ADC speeds vs Quiet)
**Description:** No load, output ON. Measures the voltage at every ADC speed (Quiet/Slow/Normal/Fast/Turbo) at 2 V and 29 V and fits a gain and offset per speed so each reads the same as Quiet. Runs ~2 min, restores the V setpoint and speed, saves itself. Fails without changing anything if a load (>5 mA) is connected. `:CAL:RATE:DEFault` goes back to the built-in trim.
**Syntax:** `:CAL:RATE`, `:CAL:RATE?`, `:CAL:RATE:DEFault`
**Response:** `OK`; the query returns `gain,offset_mV;...` for speeds 0-4
**Example:**
```
:CAL:RATE
OK
:CAL:BUSY?      (repeat until 0)
0
:CAL:RATE?
1.0000000,0.0000;0.9999998,0.0380;1.0000482,0.0029;1.0001857,-0.3176;1.0004289,-0.8397
```

---

## Error Codes

Standard SCPI error codes returned by `:SYST:ERR?`:

| Code | Description |
|------|-------------|
| 0 | No error |
| -100 | Command error |
| -101 | Invalid character |
| -102 | Syntax error |
| -103 | Invalid separator |
| -104 | Data type error |
| -108 | Parameter not allowed |
| -109 | Missing parameter |
| -110 | Command header error |
| -111 | Header separator error |
| -112 | Program mnemonic too long |
| -113 | Undefined header |
| -114 | Header suffix out of range |
| -120 | Numeric data error |
| -121 | Invalid character in number |
| -123 | Exponent too large |
| -124 | Too many digits |
| -128 | Numeric data not allowed |
| -130 | Suffix error |
| -131 | Invalid suffix |
| -134 | Suffix too long |
| -138 | Suffix not allowed |
| -200 | Execution error |
| -221 | Settings conflict (e.g. voltage above the OVP level while OVP is on) |
| -222 | Data out of range |
| -223 | Too much data |
| -224 | Illegal parameter value |
| -310 | Settings lost due to power down |
| -311 | Calibration failed |
| -410 | Query interrupted |
| -420 | Query unterminated |
| -430 | Query deadlocked |
| -440 | Query unterminated after indefinite response |

---

## Usage Examples

### Example 1: Basic Voltage/Current Control
```python
import serial

# Open serial connection
ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

# Identify instrument
ser.write(b'*IDN?\n')
print(ser.readline().decode().strip())

# Set voltage to 5V
ser.write(b':SOUR:VOLT 5.0\n')

# Set current limit to 1A
ser.write(b':SOUR:CURR 1.0\n')

# Enable output
ser.write(b':OUTP:STAT ON\n')

# Measure voltage
ser.write(b':MEAS:VOLT?\n')
voltage = float(ser.readline().decode().strip())
print(f"Measured voltage: {voltage:.6f} V")

# Measure current
ser.write(b':MEAS:CURR?\n')
current = float(ser.readline().decode().strip())
print(f"Measured current: {current:.6f} A")

# Disable output
ser.write(b':OUTP:STAT OFF\n')

ser.close()
```

### Example 2: Voltage Sweep
```python
import serial
import time

ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

# Enable output
ser.write(b':OUTP:STAT ON\n')
time.sleep(0.1)

# Sweep voltage from 0V to 10V in 1V steps
for voltage in range(0, 11):
    # Set voltage
    ser.write(f':SOUR:VOLT {voltage}\n'.encode())
    time.sleep(0.5)  # Settle time

    # Measure actual voltage
    ser.write(b':MEAS:VOLT?\n')
    measured_v = float(ser.readline().decode().strip())

    # Measure current
    ser.write(b':MEAS:CURR?\n')
    measured_i = float(ser.readline().decode().strip())

    print(f"Set: {voltage}V, Measured: {measured_v:.4f}V, Current: {measured_i:.4f}A")

# Disable output
ser.write(b':OUTP:STAT OFF\n')
ser.close()
```

### Example 3: Error Checking
```python
import serial

ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)

# Send invalid command
ser.write(b':INVALID:COMMAND\n')

# Check for errors
ser.write(b':SYST:ERR?\n')
error = ser.readline().decode().strip()
print(f"Error: {error}")

# Clear all errors
ser.write(b'*CLS\n')

# Verify no errors
ser.write(b':SYST:ERR?\n')
error = ser.readline().decode().strip()
print(f"After clear: {error}")

ser.close()
```

---

## Notes

1. **Command Parsing:** Commands are parsed in real-time from the serial port. Sending commands too quickly may cause buffer overflow. Wait for responses to queries before sending new commands.

2. **Numeric Precision:** All numeric responses use 6 decimal places for consistency.

3. **Unit Suffixes:** The parser supports standard SCPI unit suffixes:
   - Voltage: V, mV, uV, kV
   - Current: A, mA, uA, kA

4. **Error Queue:** The error queue holds up to 10 errors. When full, the oldest error is discarded.

5. **Thread Safety:** SCPI commands execute in the main loop. Long-running operations may block command processing temporarily.

6. **Remote/Local Mode:** `:SYST:REM` and `:SYST:LOC` commands are available for compatibility but currently do not disable front panel controls.

---

## Compliance

This implementation complies with:
- **IEEE 488.2-1992** - Standard Digital Interface for Programmable Instrumentation
- **SCPI-1999** - Standard Commands for Programmable Instruments

All mandatory IEEE 488.2 common commands are implemented. SCPI subsystems follow the hierarchical command tree structure defined in the SCPI standard.
