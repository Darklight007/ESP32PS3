# Audit: Watchdog Log-Spam and Real Crash (2026-07-21)

Captured from `platformio device monitor` on `/dev/ttyACM0`, ~24-32 minutes of
uptime. Two distinct issues in the same session, analyzed read-only (no code
changes made as part of this audit).

## Issue 1 — Repeating task-watchdog trips (non-fatal, log-only)

Symptom: `E (...) task_wdt: Task watchdog got triggered ... loopTask (CPU 1)`
firing repeatedly, e.g. at uptimes 126.7s, 246.7s, 366.7s, 486.7s, 606.7s,
726.7s, 846.7s, 966.7s, 1086.7s, 1206.7s, 1326.7s, 1446.7s, 1806.7s, 1926.7s —
**every single trip is exactly 120.0 seconds after the previous one.**

**Root cause:** `esp_task_wdt_init(120, false)` in `src/main.cpp` arms a
120-second watchdog for the loop task, but nothing in `loop()` ever calls
`esp_task_wdt_reset()` to feed it. The exact, unvarying 120.0s spacing is the
signature of a countdown that is never reset — a mechanical certainty, not a
recurring real hang. `panic=false` on that same init call means these
particular trips don't reboot the device; they just print the dump and
continue (confirmed: several entries show *both* cores sitting in `IDLE` at
the moment of the "crash", and no ESP-ROM boot banner appears between any of
these — device uptime is continuous throughout).

**Status:** This was already found and fixed once earlier in this project's
history (`esp_task_wdt_reset()` added to `loop()`, plus `panic=true` so a
*genuine* hang would actually recover instead of freezing forever) — but that
fix was wiped out by a full revert-to-last-commit (done at the user's request
over a budget concern) and has not been reapplied since. Only the
battery-charger feature work was redone after that revert.

## Issue 2 — SPIFFS mount race between independent auto-save paths

Symptom: `[W][SPIFFS.cpp:71] begin(): SPIFFS Already Mounted!` immediately
followed by `[E][vfs_api.cpp:24] open(): File system is not mounted` — seen
at uptimes ~81.5s, ~721s, ~1072s.

**Root cause:** `SaveMemoryFgen`/`LoadMemoryFgen` (`src/device.cpp`) and the
graph-trace snapshot save/load (`src/ui_helpers.cpp`) each independently call
`SPIFFS.begin()` / `SPIFFS.end()` around their own file I/O, rather than
mounting once at boot and staying mounted. FunGen auto-save runs every 2s
while dirty; the graph auto-save runs every 30s. If two of these land close
together, one can tear down the mount (`SPIFFS.end()`) while the other still
believes it's open — producing exactly this contradictory warning/error pair.

**Impact:** separate from the watchdog issue. Risk is a failed or partial
flash save (lost FunGen table / graph trace data), and unnecessary flash
wear/latency from repeated mount cycles, not a crash by itself.

## Issue 3 — Real crash: Guru Meditation Error (LoadProhibited), actual reboot

Unlike Issue 1, this one is a genuine fault with a real device reboot
(ESP-ROM boot banner appears immediately after):

```
Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.
EXCCAUSE: 0x0000001c   EXCVADDR: 0x00000004
```

`EXCCAUSE 0x1C` = LoadProhibited (read from an address with no read
permission). `EXCVADDR 0x00000004` — an attempt to read from address 4, the
classic fingerprint of dereferencing a null (or near-null/garbage) pointer,
e.g. `somePointer->fieldAtOffset4` where `somePointer` is null.

**Leading evidence — recursion signature:** the CPU1 backtrace captured by
the watchdog print immediately before the panic shows two addresses
alternating repeatedly:

```
0x4202E5CD ... 0x4202E55D ... 0x4202E5CD ... 0x4202E55D ... (repeats ~7x)
```

This pattern — two frames calling back and forth — is the classic signature
of deep or infinite recursion, not a normal call chain.

**Most probable root cause (ranked):**

| Rank | Cause | Reasoning |
|---|---|---|
| 1 (highest) | Cross-core LVGL corruption → recursive traversal → wild pointer read | The `lvglIsBusy` guard around `lv_timer_handler()` in `main.cpp` is still commented out (same gap noted in Issue 1's history). Without it, `Task_BarGraph` (Core 0) can mutate LVGL's widget/invalid-area list while Core 1 is mid-traversal. A corrupted linked structure (e.g. a parent/child cycle) would make LVGL's own redraw code recurse into itself — matching the alternating-address pattern — until it walks into unmapped memory at `EXCVADDR 0x4`. |
| 2 | Heap corruption from an unrelated out-of-bounds write, coincidentally landing on an LVGL structure | Same end symptom, different origin. Less likely than #1 — the specific repeating-pair pattern is a stronger fingerprint for recursion than for a random heap smash. |
| 3 (lowest) | Legitimate deep call chain exhausting the stack under rare timing | The UI/widget structure here isn't deep enough for this to happen without an already-abnormal condition — points back to #1 or #2. |

**Caveat:** `.pio/build/esp32-s3-devkitc-1/firmware.elf` was not present when
this was analyzed, so the backtrace addresses could not be resolved to exact
function/line symbols via `addr2line` — this ranking is based on the
backtrace *pattern* plus the known, previously-flagged code gap, not a
confirmed symbol trace. Rebuilding with a matching binary before the next
crash would allow exact confirmation.

## Summary

Three distinct, real issues, none currently fixed in the running firmware:

1. Watchdog never fed → harmless but constant log spam every 120s (was fixed
   before, reverted, not reapplied).
2. SPIFFS mount race between two independent auto-save timers → risk of lost
   saves, not crashes.
3. Cross-core LVGL race (disabled `lvglIsBusy` guard) → most likely cause of
   an actual, confirmed reboot via Guru Meditation Error (LoadProhibited).

Per user instruction, no code changes were made as part of this audit —
analysis and documentation only.

## Update — 2026-07-23: status re-verified against current code, not assumed

Re-checked all three issues directly against the running source (not just this
document's prior claims) before touching anything.

### Issue 3 (cross-core LVGL race) — partially fixed, and bigger than first scoped

**Fixed:** the `lvglIsBusy` guard around `lv_timer_handler()` in `main.cpp` was
restored (branch `fix-core0-lvgl-race`, commit `bc69dd1`):
```cpp
lvglIsBusy = true;
lv_timer_handler();
lvglIsBusy = false;
```
This closes the door for `DispObjects::barUpdate()` (`DispObject.cpp:118`),
which is the *only* call site in the whole codebase that actually checks
`lvglIsBusy` before touching LVGL — confirmed by grepping every reference to
the flag, not assumed.

**Still open, and much larger than originally scoped:** `keyCheckLoop()`
(`input_handler.cpp`, runs on **Core 0** via `Task_ADC` — confirmed via
`tasks.cpp:149`) makes dozens of direct, unguarded LVGL calls across many key
handlers, none deferred to Core 1 and none checking `lvglIsBusy`/`blockAll`:

| Key(s) | Call | What it touches |
|---|---|---|
| `H` (RELEASED) | `hide()` ×5, `Tabs::goToHomeTab()`, `lv_obj_invalidate(lv_scr_act())` | Calibration window flags; **`goToHomeTab()` is missing the same Core-1 deferral `Tabs::setCurrentPage()` already has right next to it** |
| `r` | `lv_obj_invalidate(lv_scr_act())`, **`lv_refr_now(NULL)`** | Forces a full synchronous redraw pass directly from Core 0 — the most serious of this group, since it's not a single widget touch, it's an entire render pass racing Core 1's own `lv_refr_now()` |
| `V`/`A` (histogram & graph pages) | `lv_chart_hide_series()`, `lv_obj_clear_flag`/`add_flag` on legend labels | Chart series visibility + legend labels |
| `X` (histogram & graph pages) | Already deferred via `DEFER_VIEW_MODE_TO_CORE1` / `g_pendingViewModeChange` | Safe — good existing pattern |
| `Z` | `lv_slider_set_value()`, `lv_event_send()` | Averaging-count slider |
| Digit keys (page 3) | `lv_obj_get_child()` chain, `loadMemory(btn)` | Memory-bank load button lookup |
| `T` (HOLD) | `lv_obj_clear_flag`/`add_flag` on `label_current_rel` | REL indicator visibility |

**Already safe, confirmed by inspection, not by assumption:**
- `Tabs::setCurrentPage()`/`nextPage()`/`previousPage()` — already deferred to
  Core 1 via `DEFER_PAGE_CHANGE_TO_CORE1` in `tabs.cpp`.
- `'O'` key (power toggle) — deferred via `g_powerTogglePending`.
- `'T'` key mA/A toggle — deferred via `mA_toggle_pending`.
- `lv_obj_has_flag()` reads and bare `lv_obj_invalidate()` calls — both
  explicitly documented in `CLAUDE.md` as accepted Core-0 exceptions.
- `managePageEncoderInteraction()` (encoder-driven graph/histogram/utility
  scrolling) — initially suspected as another Core-0 offender, but confirmed
  via `main.cpp:253` that it actually runs inside `loop()` on **Core 1**. Not
  part of this bug.

**Recommendation:** don't batch-fix all of `keyCheckLoop()` in one pass — too
many independent call sites, too much risk of dropping/delaying a keypress if
the gating is wrong on any one of them (input responsiveness is inviolable
here). `Tabs::goToHomeTab()` is a clean, small, low-risk next step (mirrors an
existing pattern already proven in the same file). The rest — chart
show/hide, the `Z`-key slider, `loadMemory()`, and especially the `'r'` key's
direct `lv_refr_now()` — is real, additional work that needs its own scoping
pass, not a rushed sweep.

### Issue 1 (watchdog never fed) — confirmed still not fixed, live

Directly observed in a serial capture during this session: a task-watchdog
trip on `loopTask` (CPU 1) at ~125.8s uptime, matching the predicted ~120s
timeout + `setup()` overhead. `panic=false` confirmed — no reboot followed,
device kept running normally. Grepped `main.cpp` for `esp_task_wdt_reset()`:
no calls found anywhere. Still exactly as described in Issue 1 above.

The backtrace captured at that exact trip happened to show Core 1 mid-redraw
of a table widget (`lv_table_event` → `draw_main` → `lv_draw_label` →
`draw_letter_normal`), correlated with a page-navigation keypress
(`Last key: l`) just before it — most likely coincidental timing (the
watchdog was going to fire regardless of what Core 1 was doing at that
moment), not evidence that the table redraw itself is unusually slow.

### Issue 2 (SPIFFS mount race) — partially fixed

The **cross-core** aspect of this was closed this session: `Tabs::setCurrentPage()`'s
FunGen tab-leave save now only runs after the existing Core-1-only deferral
gate (`tabs.cpp`, commit `f593191`, from an earlier part of this session — see
git log). That fixed a real, confirmed "CORRUPT HEAP" crash.

Still open: the original two-independent-timers race described above (FunGen
autosave timer vs. graph-trace snapshot timer, `device.cpp` vs.
`ui_helpers.cpp`, each independently calling `SPIFFS.begin()`/`end()`) is
structurally unchanged — confirmed both still call `SPIFFS.begin`/`end`
independently via direct grep. Lower severity (risk of a lost/partial save,
not a crash), not addressed this session.

### Related, separate finding (from AUDIT_SUMMARY.md, re-confirmed this session)

FUN-Only mode's early `return` in `loop()` (`main.cpp:165-169`) still precedes
`scpiParser.process()` and all `drainPending*()` calls — confirmed via direct
read of the current file. SCPI and queued page/view/power-toggle events are
still silently skipped while FUN-Only mode is active. Not touched this
session.
