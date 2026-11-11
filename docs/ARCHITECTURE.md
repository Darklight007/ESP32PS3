# Software Architecture

Where the firmware is today, what hurts, and the target structure to move to
in small, testable steps. Module names refer to `src/`.

## 1. Today

```
                 Core 0 (Task_ADC, prio 1)                    Core 1 (loop)
 ┌──────────────────────────────────────────────┐   ┌──────────────────────────────┐
 │ DRDY ISR ─► getAdcValues()                   │   │ lv_timer_handler (UI)        │
 │   raw ─► PGA undo ─► linear cal ─► INL ─►    │   │ intervals.cpp schedule()     │
 │   leakage ─► REL ─► DispObject.measured      │   │ SCPI parser                  │
 │ DACUpdate() (CC/CV, FUN gen, cc-limit table) │   │ calibration sequencer        │
 │ keypad scan + key handlers                   │   │ drain g_*Pending flags       │
 │ GraphPush accumulators                       │   │ power mgmt, stats, status    │
 └───────────────┬──────────────────────────────┘   └──────────────┬───────────────┘
                 │   shared globals: PowerSupply, DispObject,      │
                 └──► volatile g_*Pending flags, blockAll, ... ◄───┘
 persist task (Core 0, prio 0): SPIFFS/NVS saves via persistRequest()
```

What works well: single `Device` owner of state, deterministic ADC/DAC on
Core 0, deferred LVGL work, background flash saves, `schedule()` intervals.

## 2. Pain points (seen in this codebase)

| Problem | Example | Cost |
|---|---|---|
| Cross-core state is ad-hoc | ~15 `volatile bool g_*Pending` flags, `blockAll`, LVGL reads from Core 0 | races, hard to reason about |
| Measurement pipeline is one long function | `Device::getAdcValues()` does PGA, cal, leakage, REL, stats | each correction hard to test or reorder |
| Raw-byte persistence of C++ objects | `Calibration` (has a `String`) saved with `putBytes` | heap pointer saved; fixed 2026-11-02 |
| No versioning of stored structs | any field change silently misreads old NVS blobs | calibration loss after updates |
| Two sources for one number | `DATA:STATS?` read `measured`, Stats page read `Statistics` | std = 0 over SCPI; fixed 2026-11-02 |
| UI and logic mixed | key handlers in `input_handler.cpp` touch device + UI | Core 0 handlers need flags to reach LVGL |
| No host tests | cal math, INL, graph labels only testable on hardware | regressions (x-labels) |

## 3. Target structure

```
 ┌───────────────────────────── Core 0: real time ─────────────────────────────┐
 │  hal/      ads1219, ltc2655, mcp23017, gpio      (drivers only, no state)   │
 │  measure/  Pipeline: Stage[] = {PgaUndo, Linear, Inl, Leakage, Thermal, Rel}│
 │            each Stage: double apply(double x, const Ctx&)   ← unit-testable │
 │  control/  Setpoint ─► corrections (leakage, cc-limit) ─► DAC               │
 │  input/    key/encoder scan ─► InputEvent ─────────────────────┐            │
 │                                                                │            │
 │  publishes  Snapshot{V,I,P,state,flags,seq}  (seqlock, 1 writer)            │
 └───────────────────────────────┬────────────────────────────────┼────────────┘
                                 │ read any time, never blocks    │ queue
                                 ▼                                ▼
 ┌───────────────────────────── Core 1: application ───────────────────────────┐
 │  app/      Device state machine: consumes Command + InputEvent queues       │
 │  ui/       LVGL views; read Snapshot, emit Command (never touch hal/)       │
 │  scpi/     parser ─► Command; queries read Snapshot / Stats                 │
 │  cal/      sequencer + procedures; emit Command, read Snapshot              │
 │  store/    versioned POD blobs + CRC  ─► persist task                       │
 └──────────────────────────────────────────────────────────────────────────────┘
          Command = {SetV, SetI, Output, Range, PageGoto, ...}  (FreeRTOS queue)
```

Rules:
1. **One writer per datum.** Core 0 owns measurements and DAC; Core 1 owns UI
   and settings. Everything crossing cores goes through the `Snapshot` (Core 0
   to 1) or a `Command`/`InputEvent` queue (either way). This replaces the
   `g_*Pending` flags and most `blockAll` uses.
2. **Measurement is a pipeline of small stages.** Each correction (PGA, linear,
   INL, leakage, thermal, REL) is a pure function with its own parameters, so
   it can be unit-tested on the host, switched off for diagnosis
   (`CAL:STAGE:DIS LEAK`), and new ones (thermal, A/mA match) slot in.
3. **Persistence = versioned plain structs.** `struct CalV3 { uint16_t ver,
   size; ...; uint32_t crc; }` with only POD members (`char mac[18]`, not
   `String`). On load: check ver/size/crc, migrate older versions explicitly.
4. **UI is a view.** Pages read the Snapshot at frame time and post Commands;
   key handlers on Core 0 post `InputEvent`s instead of touching LVGL.
5. **One source per number.** SCPI, UI and graph read the same `Stats`
   object; there's no per-consumer computation.
6. **Host tests** (`pio test -e native`): pipeline stages, INL interpolation,
   cc-limit inversion, graph axis divisions, SCPI parsing.

## 4. Migration (each step ships on its own, nothing breaks)

| # | Step | Risk |
|---|---|---|
| 1 | Add `native` test env; tests for `graphXDivisions`, INL, cc-limit math | none |
| 2 | `Snapshot` + seqlock written at end of `getAdcValues()`; SCPI queries read it | low |
| 3 | `Command` queue; move key handlers from `g_*Pending` to commands one by one | low per key |
| 4 | Split `getAdcValues()` into pipeline stages (same math, same order) | medium, verify readings identical |
| 5 | `CalV3` POD + CRC, migrate from current blob on first boot, keep old key as backup | medium, test with a copy of NVS |
| 6 | Thermal and A/mA-match stages (see `DRIFT_AND_CALIBRATION.md`) | after 4 |
| 7 | Move UI pages to read Snapshot only; remove Core 0 `lv_obj_has_*` reads | low |
