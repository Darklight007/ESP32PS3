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
