#!/usr/bin/env python3
"""Read the boot-time DAC diagnostic log off the PS over USB.

The -0.2V-at-boot bug only reproduces after a full power cycle with USB
unplugged, so the boot log can't be watched live on a serial monitor. The
firmware captures the first DACUpdate() calls into RAM instead; this sends
the DEBUG:DACLOG? query and prints just that reply, filtering out the
[SECDBG] timing spam that otherwise buries it.

Usage:  python3 read_daclog.py [port]
"""
import sys
import time

try:
    import serial
except ImportError:
    sys.exit("pyserial missing - install with: pip install pyserial")

port = sys.argv[1] if len(sys.argv) > 1 else "/dev/ttyACM0"

with serial.Serial(port, 115200, timeout=0.3) as ser:
    time.sleep(0.3)
    ser.reset_input_buffer()
    ser.write(b"DEBUG:DACLOG?\n")
    ser.flush()

    deadline = time.time() + 3.0
    lines = []
    while time.time() < deadline:
        raw = ser.readline()
        if not raw:
            continue
        line = raw.decode("utf-8", errors="replace").rstrip()
        if not line or "[SECDBG]" in line:
            continue
        lines.append(line)

if lines:
    print("\n".join(lines))
else:
    print("No reply. Check the port, and that the firmware with DEBUG:DACLOG? is flashed.")
