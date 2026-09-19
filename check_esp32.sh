#!/usr/bin/env bash
# Identify every attached ESP32 (chip, revision, flash size, PSRAM, MAC).
# Usage:  ./check_esp32.sh [port]
#   no args -> scans all /dev/serial/by-id/* and /dev/ttyACM* /dev/ttyUSB*

set -u

ESPTOOL="python3 -m esptool"

probe() {
    local port="$1"
    local out
    out=$($ESPTOOL --port "$port" flash_id 2>&1)

    if echo "$out" | grep -qi "fatal\|Failed to connect"; then
        echo "  no ESP32 response (may be running firmware - hold BOOT, tap RESET)"
        return 1
    fi

    local chip rev feat flash mac psram
    chip=$(echo "$out"  | grep -i "^Chip is"        | head -1 | sed 's/^Chip is //')
    feat=$(echo "$out"  | grep -i "^Features:"      | head -1 | sed 's/^Features: //')
    flash=$(echo "$out" | grep -i "Detected flash size:" | head -1 | sed 's/.*: //')
    mac=$(echo "$out"   | grep -i "^MAC:"           | head -1 | sed 's/^MAC: //')

    psram=$(echo "$feat" | grep -o "PSRAM [0-9]*MB" | head -1)
    [ -z "$psram" ] && psram="none"

    echo "  chip   : ${chip:-?}"
    echo "  flash  : ${flash:-?}"
    echo "  psram  : ${psram}"
    echo "  mac    : ${mac:-?}"

    # N16R8 = 16MB flash + 8MB PSRAM
    if [ "${flash:-}" = "16MB" ] && [ "$psram" = "PSRAM 8MB" ]; then
        echo "  verdict: N16R8  ✓"
    else
        echo "  verdict: NOT N16R8 (got ${flash:-?} flash / ${psram})"
    fi
    return 0
}

if [ $# -ge 1 ]; then
    echo "== $1"
    probe "$1"
    exit $?
fi

found=0
for p in /dev/serial/by-id/* /dev/ttyACM* /dev/ttyUSB*; do
    [ -e "$p" ] || continue
    case "$p" in *2303*) continue;; esac   # skip the PL2303 adapter
    # skip by-id symlinks we've already probed via their target
    echo "== $p"
    probe "$p" && found=1
    echo
done

[ "$found" -eq 0 ] && echo "No ESP32 found. If firmware is running: hold BOOT, tap RESET, rerun."
exit 0
