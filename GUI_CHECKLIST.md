# Web GUI Feature Checklist

✅ Works   ⚠️ Partial   ❌ Not implemented

---

## Main Panel (always visible)

- ✅ Live voltage / current / power display
- ✅ mA / A label auto-switches based on device mode
- ✅ Set output voltage
- ✅ Set output current limit
- ✅ Output ON / OFF toggle
- ✅ CV / CC mode indicator
- ✅ FGen ON / OFF — triggers FUN mode + red theme on device
- ✅ Polling enable / disable

---

## Graph Tab

- ✅ Fetch full graph snapshot (1200 points)
- ✅ Voltage trace
- ✅ Current trace — auto-scaled mA or A
- ✅ X-axis time labels derived from graph span setting
- ✅ Pan mode (XY)
- ✅ Zoom mode (XY)
- ✅ Zoom X only (Y axis locked)
- ✅ Zoom Y only (X axis locked)
- ✅ X-axis range slider
- ✅ Export graph to CSV
- ❌ Live streaming graph (manual snapshot only)

---

## Utility → Memory

- ✅ 10 memory slots loaded from device NVS
- ✅ Layout matches device: 3-column grid, order 7 8 9 / 4 5 6 / 1 2 3 / 0
- ✅ Click slot → recall to output
- ✅ Hold slot 600 ms → save current setpoint
- ✅ Amber flash on save confirmation
- ✅ Values formatted to match device display (+08.4f)

---

## Utility → FGen

- ✅ Read / set frequency, amplitude, offset, duty cycle
- ✅ Waveform selector (18 waveforms)
- ✅ FGen ON / OFF synced with main panel button

---

## Utility → Arbt (Arbitrary Waveform)

- ✅ 20 points per bank (correct — matches firmware CHART_POINTS)
- ✅ Bank 0 / 1 selector
- ✅ Load from device flash (NVS)
- ✅ Send to device
- ✅ Canvas waveform preview
- ✅ Edit individual points via table
- ✅ Presets: Sine, Square, Triangle, Sawtooth, Zero

---

## Utility → Tabl (Table Points)

- ✅ Load table from device
- ✅ Send table to device
- ✅ Adjustable length (1 – 250 points)
- ✅ Canvas waveform preview
- ✅ Edit individual points via table

---

## Settings → Display

- ✅ Backlight brightness slider

---

## Settings → Measurement

- ✅ ADC SPS rate — applied to hardware immediately
- ✅ ADC averaging window size — applied to hardware
- ✅ ADC digital filter level

---

## Settings → Graph

- ✅ Graph time span
- ✅ Graph auto-stop toggle

---

## Settings → Beeper

- ✅ Beeper master enable
- ✅ Beep on power on/off
- ✅ Beep on error
- ✅ Beep on keypress
- ✅ Beep volume

---

## Settings → Timer

- ✅ Timer enable
- ✅ Timer duration

---

## Settings → Power Management

- ✅ OVP enable + level
- ✅ OCP enable + level
- ✅ Output turn-on delay
- ✅ Auto-save interval
- ✅ Power at startup (Always Off / Always On / Last State)

---

## Settings → Calibration

- ✅ Save calibration to device NVS
- ✅ Download calibration JSON to PC (DATA:CAL?)
- ✅ Load calibration from device NVS
- ❌ ADC / DAC / INL / Touch calibration wizards — device touchscreen only

---

## Settings → Load / Save

- ✅ Save all settings to NVS
- ✅ Load all settings from NVS

---

## Settings → About

- ✅ Query firmware version via *IDN? — displayed inline
- ✅ Current date and time (browser clock)

---

## Device Features Not Available in Web GUI

- ❌ Histogram / statistics tab (min, max, mean, stddev) — no SCPI read-back
- ❌ Live bar graph (VU meters) — real-time Core 0 only
- ❌ REL mode (relative offset) — no SCPI command
- ❌ Energy counter read-back — no SCPI command
- ❌ OTP / safe mode — hardware-triggered only
- ❌ All calibration wizards — device touchscreen only
- ❌ Encoder / keypad / touch input — physical only
