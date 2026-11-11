# Drift at High Current, and Calibration Improvements

Hardware assumed (from the code): A range = 10 mΩ shunt + INA190, mA range =
own shunt + INA190, mA key only switches which INA190 feeds the ADS1219 (the
CC loop always senses the 10 mΩ shunt). ADS1219 on an external reference,
LTC2655 DAC on its internal reference, LM70880 pre-regulator. There's no
temperature sensor.

## 1. What can drift during a 6 A load (most likely first)

| # | Source | Mechanism | Size estimate | How to tell |
|---|---|---|---|---|
| 1 | **The load itself** | resistive load heats, R rises, real current falls (CV mode) | %-level | external DMM in series drifts too → not the PSU |
| 2 | **10 mΩ shunt self-heating** | 6² × 0.01 = 0.36 W; TCR 50–200 ppm/°C × ΔT 30–60 °C | 0.15–1.2 % (9–70 mA) | drift follows a ~1–5 min exponential after load-on, recovers the same way |
| 3 | **Copper in the sense path** | if the INA190 inputs aren't true Kelvin at the shunt pads, trace/solder joint copper (+3900 ppm/°C) is in series | 1 mΩ of copper = 10 % of shunt, × 0.39 %/°C | touch the trace with freeze spray / hot air while loaded |
| 4 | **Ground IR drop** | 6 A return shares ground with ADC/INA190 reference → offset that grows as copper warms | mV-level → mA-level | measure V between ADC GND and output − terminal under load |
| 5 | **Voltage sense location** | V measured before shunt/output traces: reads high by I × R_path, and R_path heats | 6 A × 20 mΩ = 120 mV | DMM at output terminals vs display, at 0 A and 6 A |
| 6 | **ADC reference temperature** | board warms from pass transistor/heatsink; ref tempco (e.g. 10–50 ppm/°C) | 0.01–0.1 % on both V and I | V reading drifts with the same time constant as heatsink |
| 7 | **INA190 offset/gain drift** | ±0.08 µV/°C offset, 7 ppm/°C gain | tiny at 6 A | only matters in mA range |
| 8 | **DAC reference (CC setpoint)** | LTC2655 internal ref ~10 ppm/°C | small | CC current drifts, measured agrees with DMM |
| 9 | **Leakage compensation** | uses V; at 6 A V changes in CC → comp shifts | ≤ 0.9 mA | turn leakage comp off (R=0) and compare |
| 10 | **Thermal EMF** | dissimilar metals at shunt with temperature gradient | µV → 0.1–1 mA on 10 mΩ | reverse-current test isn't possible; watch offset right after load off |

**Test to separate them (one 20 min run):** constant 6 A in CC into a load,
log PSU V/I (PC app CSV log) + external DMM on a separate shunt + output
terminal voltage, every 1 s. Then turn the load off and keep logging 5 min.
- Display and DMM drift together → load/real current (1, 8).
- Display drifts, DMM steady, slow exponential → shunt/copper heating (2, 3).
- Step at load-off that decays → thermal EMF / ground (4, 10).
- Voltage error ∝ current → sense location (5).

## 2. Calibration improvements

### A. Match the A and mA ranges (new, no extra hardware)
Both INA190s see the same current at the same moment; the mA key only
switches the ADC input, without interrupting the output. So the precise mA
range can calibrate the A range's **zero** at real, non-zero current, away
from the 0 V SMPS pulses that make zero calibration noisy:

```
fixed resistor (e.g. 1 kΩ) on output, CC limit high, rate 0
for V in 1, 2, 3, 4, 5, 6 V:          →  I ≈ 1..6 mA
    read mA range (n samples)  → I_ref
    switch to A range, settle, read → I_A
fit I_A = I_ref + offset_A (+ slope)   → corrects A-range code_1
switch back to user's range
```
- Gives A-range zero to the mA range's accuracy (~µA) instead of op-amp offset.
- Same data checks the gain ratio between ranges; a mismatch > 0.1 % flags a
  bad gain cal.
- SCPI `CAL:MATCH` + a Settings button; reuses `calib_low_noise_begin/end()`.

### B. Thermal model for the shunt (no sensor needed)
`ΔT(t)` follows first-order heating driven by `P = I² × R_shunt`:
`T' = (k·P − T)/τ`, correction `I_corr = I × (1 − α·T)`.
Calibrate `k·α` and `τ` from the 20 min 6 A run above (fit display vs DMM).
Applied as a pipeline stage on Core 0, so it costs a few float ops per sample.
Better: add an NTC on a spare ADS1219 channel (AIN2/AIN3) taped to the shunt
and fit `α` directly.

### C. Voltage drop compensation
Measure output-terminal voltage with a DMM at 0 A and 5 A: `R_drop = ΔV/ΔI`.
Apply `V_out = V_meas − I × R_drop` (stage, calibratable `CAL:VDROP`). If
the board has a spare pair, true remote sense is the hardware fix.

### D. Current INL (multi-point)
Only voltage has INL. Run the CC-limit sweep (`calib_cc_limit.cpp` already
steps 0.1–5 A against a short) with a DMM and store a monotone table for
current too. The INL interpolator code is reusable.

### E. Leakage factor 0.8
`DACUpdate()` uses an unverified `0.8 × V/R`. Measure: CV at 32 V, no load,
read DMM current while toggling compensation; set the factor from data.

### F. Process improvements
- After any calibration, run an automatic **verify pass** (3 points per
  range) and show pass/fail against spec.
- Store **calibration date + temperature/warm-up minutes** with the data.
- **Warm-up rule:** calibrate after 20 min powered with output on at mid load.
- Keep the last 3 calibrations (ring) so a bad cal can be rolled back.
