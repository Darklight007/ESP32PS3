# Audit Summary

## Original audit
68 findings across 5 subsystems, found via 5 parallel Explore agents earlier in this
session. Focused on memory-safety/crash-class bugs: struct serialization issues,
exception safety gaps, cross-core races, undefined behavior, and a large Core-0→LVGL
threading violation in the keypad handler. Approved to fix all of it ("fix all possible
issue that would cause memory issue or restarts the cpu").

## Fixes from that pass still committed / in place
- Calibration NaN bug (`calib_1m` never checked for finiteness) — fixed and
  camera-verified.
- Various `DispObject.cpp` / `calib_inl.cpp` / `calib_adc.cpp` bounds checks,
  exception safety, and UB fixes.
- Several items visible in git log as actual commits, e.g.:
  - `9380135` Defer 'O'-key power toggle to Core 1; NAN-bust stale mA display
  - `942d335` Fix INL window sometimes buried at Full Auto step 6/6 (z-order)
  These survived because they were committed, not just sitting in the working tree.

## Fixes made in a later session that got wiped out by a full revert
(Reverted because they were never committed, after "used 50% of weekly limit and
things don't work" prompted a full revert to last commit.)

- SCPI-dead root cause: `boards/esp32-s3-devkitc-1-n16r8.json` had
  `ARDUINO_USB_CDC_ON_BOOT` / `ARDUINO_USB_MODE` binding `Serial` to an unwired
  native-USB peripheral instead of the actual UART bridge chip.
- FUN-Only-mode gate in `main.cpp` blocking SCPI/input drains.
- Watchdog never fed: `esp_task_wdt_reset()` missing from `loop()`.
- Real cross-core LVGL race: `lvglIsBusy` guard commented out around
  `lv_timer_handler()`, letting Task_BarGraph (Core 0) corrupt LVGL's invalid-area
  list mid-render.
- Watchdog `panic=false` → a hang would freeze the supply forever instead of
  auto-recovering.

These are **not** currently in the running firmware. Redoing them requires
re-applying each fix (with more care about batching changes / verification cost
this time).
