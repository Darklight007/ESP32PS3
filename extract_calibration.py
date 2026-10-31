#!/usr/bin/env python3
"""Extract calibration data from the PSU over USB serial and save it to a file.

Sends CAL:LOAD (pull the saved bank out of NVS into RAM) then DATA:CAL? and
captures the CSV dump. The output file is in exactly the format DATA:CAL:SET
accepts, so it can be restored later onto repaired/replacement hardware with
restore_calibration.py.

Usage:  python3 extract_calibration.py [outfile] [port]
"""
import sys
import time

try:
    import serial
except ImportError:
    sys.exit("pyserial missing - install with: pip install pyserial")

outfile = sys.argv[1] if len(sys.argv) > 1 else "calibration_backup.csv"
port = sys.argv[2] if len(sys.argv) > 2 else \
    "/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_34:85:18:90:84:A0-if00"

WANTED = ("MAC,", "VCAL,", "ICAL0,", "ICAL1,", "LEAK,", "INLM,", "INLI,")

with serial.Serial(port, 115200, timeout=0.5) as ser:
    time.sleep(0.5)
    ser.reset_input_buffer()

    # Pull the persisted bank from NVS into RAM first - DATA:CAL? dumps the
    # in-memory CalBank, which may differ from what's actually saved.
    ser.write(b"CAL:LOAD\n")
    ser.flush()
    time.sleep(1.5)
    ser.reset_input_buffer()

    ser.write(b"DATA:CAL?\n")
    ser.flush()

    lines = []
    deadline = time.time() + 15.0
    while time.time() < deadline:
        raw = ser.readline()
        if not raw:
            continue
        line = raw.decode("utf-8", errors="replace").strip()
        if not line or "[SECDBG]" in line:
            continue
        if line == "END":
            break
        if line.startswith(WANTED):
            lines.append(line)

if not lines:
    sys.exit("No calibration data received. Check the port and that the PSU is powered.")

with open(outfile, "w") as f:
    f.write("\n".join(lines) + "\n")

print(f"Saved {len(lines)} lines to {outfile}\n")
for line in lines:
    print(line[:120] + ("..." if len(line) > 120 else ""))

missing = [k.rstrip(",") for k in WANTED
           if not any(l.startswith(k) for l in lines)]
if missing:
    print(f"\nWARNING: missing sections: {', '.join(missing)}")
