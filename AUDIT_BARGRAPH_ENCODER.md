# Audit: Main-Page Bar-Graph Speed & Encoder Response

Read-only audit (single agent), 2026-07-20. No code changes made as part of this audit.
Hard constraint respected throughout: physical control responsiveness (keypad, touch,
encoder) must never be made slower — findings that would touch that path are flagged
and were not acted on.

## Bar graph (main page)

1. **The ADC sample rate is the real ceiling, not `Task_BarGraph`'s 1ms delay.**
   `Task_BarGraph` refreshes every 1ms, but new data only actually arrives when an ADC
   conversion completes. At 1000 SPS that's ~1ms (good match); at 20/90/330 SPS it's
   every 3–50ms, so the bar task is already spinning far faster than there's new data
   to show. Nothing to fix — already correct. Do not lower the delay further; it would
   burn Core 0 cycles shared with the ADC/DAC/keypad/encoder critical path for no
   visual gain.

2. **Leftover debug `Serial.printf` in `DispObjects::Flush()`** (`src/DispObject.cpp`
   ~line 269) fires unthrottled on every setpoint change (`adjValueChanged`), called
   every Core-1 `loop()` iteration — not through the project's normal `schedule()`
   interval pattern. Competes for the same Core-1 time slice as `lv_refr_now()` and
   `lv_timer_handler()`. Safe to remove — doesn't touch input handling. Small but
   real, unthrottled cost while a setpoint is being adjusted.

3. **Bar min/max markers always call `lv_obj_set_x()`**, even when unchanged
   (`src/DispObject.cpp` ~lines 134-135) — the "skip if unchanged" guard was
   deliberately removed per a comment there. LVGL's `lv_obj_set_x()` still does a
   style-property lookup before its internal short-circuit, so at 1ms cadence that's
   up to ~2000 style lookups/sec of pure overhead when the markers haven't moved. A
   faster direct-pointer-write version (matching the pattern already used for the bar
   value itself) is already sitting commented out directly below it in the code —
   low-risk swap-in, doesn't touch input handling.

4. **Task priorities and the render-timer split are already well-tuned** —
   `Task_bargraph` is pinned at the lowest priority on Core 0 in every SPS
   configuration, and the throttled `lv_timer_handler()` (100ms) plus the unthrottled
   forced `lv_refr_now()` (every loop pass) already deliver bar invalidations to the
   screen about as fast as Core 1 can cycle. No action needed; flagged as touching the
   same render pipeline the touchscreen indev relies on, so treat as input-adjacent.

## Rotary encoder response

1. **Already essentially optimal at the detection layer — nothing to gain without
   touching the input path (which is off-limits).** ISRs are minimal `IRAM_ATTR` flag
   increments; the PCNT hardware already fires on every single quadrature count
   (`always_interrupt=true`, ±1 threshold); the polling loop (`getSettingEncoder()`,
   called every `Task_ADC` iteration) is bounded by a 1ms FreeRTOS tick — the finest
   granularity available without busy-polling, which would steal cycles from the
   ADC-critical task. The glitch filter (`setFilter(500)`, ~6.25µs) is a standard
   debounce value; lowering it risks re-admitting contact bounce as spurious counts.

2. **The adaptive fast/slow scheme during encoder activity is already tuned in your
   favor.** While the encoder is active, non-essential display refreshes
   (`VCCCStatusUpdate`, `statUpdate`, `FlushMeasures`) are deliberately slowed to free
   Core-1 cycles for the forced `lv_refr_now()`, so the setpoint label/bar repaint
   immediately. This trades off non-essential redraws, not encoder detection — no
   upside identified in retuning it further.

3. **Flag only, not a speed item — `EncoderRestartInterval()`** (`src/intervals.cpp`
   ~lines 80-93, every 1000ms) does a clear-then-restore of the encoder count
   (`getCount()` → `clearCount()` → `setCount(temp)`). A pulse landing in that brief
   window between clear and restore could be silently dropped — an occasional missed
   step, most likely while spinning fast. The call site already carries a
   `"--> some bugs?"` comment, suggesting this was previously suspected. Not a speed
   lever — flagged because it directly manipulates encoder count state, which is
   squarely inside the "never regress input" constraint. Any fix here would need a
   very strong justification and careful testing before touching it.

## Ranked summary

| # | Area | Finding | Type | Touches input path? |
|---|------|---------|------|---------------------|
| 1 | Bar graph | ADC SPS setting (not the 1ms task delay) is the real ceiling | Informational | No |
| 2 | Bar graph | Leftover `Serial.printf` in `Flush()` runs unthrottled during setpoint changes | Removable overhead | No |
| 3 | Bar graph | Marker `lv_obj_set_x()` calls do a style lookup every 1ms tick even when unchanged; faster version already exists commented out | Redundant work, small | No |
| 4 | Bar graph | Task priorities / forced-refresh split already well-tuned | No action needed | Adjacent (shares render pipeline with indev) |
| 5 | Encoder | ISR, PCNT config, 1ms poll cadence already near the practical floor | No action possible without risk | Yes — do not touch |
| 6 | Encoder | Adaptive VCCC/stat/flush throttling already correctly tuned | No action needed | No |
| 7 | Encoder | `EncoderRestartInterval()` has a narrow race that can drop a count | Correctness risk, not performance | Yes — sensitive, needs strong justification |
