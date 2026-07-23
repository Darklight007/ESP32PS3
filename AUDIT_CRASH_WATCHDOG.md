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
