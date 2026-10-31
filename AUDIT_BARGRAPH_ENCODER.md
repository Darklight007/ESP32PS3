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
| 2 | Bar graph | Leftover `Serial.printf` in `Flush()` runs unthrottled during setpoint changes | Removable overhead | No — **fixed 2026-07-23** |
| 3 | Bar graph | Marker `lv_obj_set_x()` calls do a style lookup every 1ms tick even when unchanged; faster version already exists commented out | Redundant work, small | No — **fixed 2026-07-23** |
| 4 | Bar graph | Task priorities / forced-refresh split already well-tuned | No action needed | Adjacent (shares render pipeline with indev) |
| 5 | Encoder | ISR, PCNT config, 1ms poll cadence already near the practical floor | No action possible without risk | Yes — do not touch |
| 6 | Encoder | Adaptive VCCC/stat/flush throttling already correctly tuned | No action needed | No |
| 7 | Encoder | `EncoderRestartInterval()` has a narrow race that can drop a count | Correctness risk, not performance | Yes — sensitive, needs strong justification |

## Update — 2026-07-23: full render-request audit (whole codebase, not just this file)

Grepped every `lv_refr_now`, `lv_obj_invalidate(lv_scr_act())`, and `lv_timer_handler`
call site in `src/` to find redundant/unnecessary render work, per direct request.

### 8. Fixed: `StatusBar()` re-wrote 3 labels every 300ms regardless of change

`StatusBar()` (`ui_creation.cpp`, called via `StatusBarUpdateInterval(300)` from
`main.cpp` — Core 1 only, no race risk) unconditionally called, every single
tick:
- `lv_label_set_text(statusLabel_wifi, protStatus)` — OVP/OCP string, which only
  changes when a limit or trip-state changes (rare).
- `lv_obj_set_style_text_font(statusLabel_wifi, &lv_font_montserrat_10, ...)` —
  **re-applied the font every 300ms, forever**, not just once at widget
  creation. Style application walks the style list and invalidates the object;
  doing that ~3x/sec for a font that never changes was pure waste.
- `lv_label_set_text_fmt(statusLabel_time, ...)` — the clock string only
  changes once/sec (seconds ticking), so 2 of every 3 calls (300ms cadence)
  reformatted and invalidated the label for an identical string.
- `lv_label_set_text_fmt(statusLabel_avg, ...)` — SPS/Avgs string, similar:
  `adc.realADCSpeed` itself only updates once/sec internally
  (`device.cpp`'s 1000ms window), so most 300ms ticks reformatted and
  invalidated this label for no new information.

**Fixed:** each of the three now compares against a cached last-applied
value/string first and only calls the LVGL setter when something actually
changed — same pattern already used for `barUpdate()`'s markers (#3 above).
The font-style call was moved into the one-time widget-creation block instead
of the per-tick path.

### Other call sites checked, no action needed
- `Device::FlushBars()` (`device.cpp:1180`) calls `lv_refr_now(NULL)` on bar
  change — **dead code, never called from anywhere**. Zero cost since it never
  executes; flagged for awareness only, not fixed (out of scope of a
  perf-focused pass).
- `HistogramChartRefreshInterval`/`GraphChartRefreshInterval` (125ms) already
  gate on tab visibility (page 0/1) — no waste.
- `updateStatChartSize()` (every loop, Core 1) already has its own
  change-detection guard (`s_lastAppliedVI`) with a comment describing a prior
  infinite-resize bug it was built to avoid — already correct.
- The dozen-plus `lv_obj_invalidate(lv_scr_act())` full-screen invalidations
  elsewhere (`tabs.cpp`, `setting_menu.cpp`, `calib_*.cpp`, numeric-entry
  confirmation in `ui_creation.cpp`) are all on rare, user-driven events (page
  change, calibration steps, confirming a typed value) — not per-frame,
  nothing to gate further.
- `Bar.changed` flag (`DispObject.cpp`) is set on every sample but the only
  reader is the dead `FlushBars()` above — vestigial, zero live cost.
- Doubling `LvglUpdatesInterval(0, true)` to twice per loop iteration
  (`main.cpp`, tested same day) — camera-tested against a fast FUN-mode sweep,
  no measurable improvement in bar lag. Left in (harmless, near-zero cost when
  nothing's dirty) but not treated as a real fix.

## Update — 2026-07-23 (later same day): live diagnosis of the actual "hiccup"

Everything above was static analysis plus one camera test. This round used
live instrumentation (per-section timing printed over serial, `[SECDBG]`) and
a webcam on the real display to separate real bugs from red herrings. Three
real, confirmed causes found — in order of how they were found, not severity.

### A. `keyCheckLoop()`'s LVGL calls — real, but not what was causing the hiccup

Confirmed via camera: with a real load connected and current actually
flowing, the bar tracked the digit correctly at every sampled point, even
through large fast swings (5.7V→23V in ~100ms). With no load connected, the
bar appeared to "lag" the digit by ~1.5V — but that's the physical output
capacitor failing to slew without a discharge path, not a rendering bug.
**Lesson for next time: always test bar-lag claims with a real load
connected**, or the output's own analog settling time gets misread as a
software problem.

### B. Big-label flush pileup during active FUN mode — real, confirmed with numbers

Reinstated flush/redraw timing (same technique as the display-performance
investigation) and captured live while FUN mode was actively sweeping:
`lv_refr_now()` spiked to **107-136ms** in a single call, vs. ~1.2ms normally.
Root cause: the big V/A label flush costs ~10-15ms (SPI/DMA-bound, confirmed
in the display-performance report), and during active sweeping several
widgets (label, bar, stat labels) go dirty in the same window and flush
sequentially. This is the same underlying mechanism the display-performance
report already flagged — the only complete fix (non-blocking DMA) is the one
change this session already proved unsafe (froze the display).

Made worse, then fixed, by a follow-up change: `FlushMeasures()` was changed
to run every loop gated only by `displayReady` (`NofAvgs`-driven, so avg=N
updates every Nth sample as requested) — but at avg=1 with continuous change,
that demanded up to ~1000 big-label repaints/sec against a ~65-100Hz physical
ceiling, and spiked to **133-136ms**. Fixed with a 20ms floor (~50Hz cap) on
top of the `displayReady` gate — confirmed live afterward: max dropped to
9.27ms (a single normal flush, not a pileup).

### C. Periodic ~16-19ms stall — `Page2RightSideCleanup(1000)`, confirmed and disabled

A second, smaller but clockwork-regular spike (~16-19ms, once/sec, distinct
from B) was isolated by disabling `Page2RightSideCleanup()` (which invalidates
~10 separate small widgets every 1000ms) and re-measuring: max dropped from
~19ms to ~1.9ms with it off. Currently **disabled** in `main.cpp` pending a
decision on whether its original purpose (clearing dirty pixels on page 2's
right side) still matters — watch for dirty/stale pixels there and re-enable
or fix properly if so.

### D. The real "freeze everything" cause — SPIFFS, not rendering at all

After A-C, the user still reported a global pause hitting settings, V/A
digits, the bar, *and* a visible notch in the graph trace, all simultaneously
— too broad to be a render-only issue. Instrumented every periodic function
in `loop()` individually and found it: **`SaveGraphDataIfDirty()`'s SPIFFS
write cost up to 638ms in one call**, entirely blocking Core 1 (which is
exactly why every UI element paused together, and why the graph itself
showed a gap). Broke it down further: `SPIFFS.open(path, "w")` on an existing
file cost ~196ms and `f.close()` cost ~137ms — the actual `f.write()` calls
were under 20ms combined. This is a known SPIFFS weakness (open/close
bookkeeping on an existing file), not a bug in this codebase's logic.

**Fixed (partial, documented as such):**
- Mounted SPIFFS once at boot (`main.cpp`) instead of `begin()`/`end()`
  around every save/load — cut the worst case from 638ms to ~305ms, and
  incidentally closes the independent-auto-save-timers mount race from
  `AUDIT_CRASH_WATCHDOG.md` Issue 2 (both timers now share one mount instead
  of racing to mount/unmount).
- Reduced how often the remaining ~300ms cost can be hit: FunGen autosave
  2s→15s, graph-trace autosave 30s→180s. Same per-hit cost, ~7-10x less
  frequent.
- **Not fixed:** the ~300ms open+close cost itself is inherent to SPIFFS.
  The real fix is migrating this storage to LittleFS (ESP32's recommended
  SPIFFS replacement specifically for this kind of overhead) — bigger change
  touching graph save/load and FunGen save/load together, needs its own
  scoped task with real testing, not a same-session patch.

### Net effect

All of B, C, and D were contributing to what the user experienced as one
generic "hiccup" — they were three separate, unrelated mechanisms (render
pileup, a redundant periodic invalidation, and filesystem latency) that
happened to produce a similar-looking symptom. Splitting them required live
timing data at each step, not code reading — several early hypotheses (CC/CV
beep flapping, low real ADC sample rate from I2C contention) were tested live
and ruled out before D was found. Current state: meaningfully better
(confirmed by the user), not completely eliminated — the SPIFFS latency and
the FUN-mode pileup ceiling both still exist, just far less frequently
triggered.
