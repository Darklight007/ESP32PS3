#!/usr/bin/env python3
"""
Power Supply Remote Control GUI.
Close PIO monitor before running. Opens the port without resetting the board.
Logs (Logger tab) go to logs/ps_log_<date>.csv next to this script.
Usage: python3 ps_control.py [serial_port]
"""

import sys
import os
import json
import threading
import webbrowser
import serial
import time
import socketserver
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import unquote

PORT = 8788
BAUD = 115200
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Serial port: use argv[1], or auto-detect from ACM0-9 / USB0-9
def _find_serial_port():
    if len(sys.argv) > 1:
        return sys.argv[1]
    candidates = (
        [f"/dev/ttyACM{i}" for i in range(10)] +
        [f"/dev/ttyUSB{i}" for i in range(10)]
    )
    for port in candidates:
        if os.path.exists(port):
            return port
    return "/dev/ttyACM0"  # fallback

SERIAL_PORT = _find_serial_port()

ser = None
ser_lock = threading.Lock()


def _open_port(port, timeout=2):
    """Open without toggling DTR/RTS, so the board is NOT reset (a reset turns
    the output off). write_timeout turns a wedged USB CDC link into an error
    (-> reconnect) instead of a hang."""
    s = serial.Serial()
    s.port, s.baudrate, s.timeout, s.write_timeout = port, BAUD, timeout, 2
    s.dtr = False
    s.rts = False
    s.open()
    return s


def hard_reset_board():
    """ESP32-S3 USB-Serial/JTAG reset (as esptool does). Turns the output OFF.
    Recovers a wedged CDC link; only run when the user asks for it."""
    global ser
    with ser_lock:
        port = SERIAL_PORT
        try:
            if ser:
                ser.close()
        except Exception:
            pass
        ser = None
        try:
            s = _open_port(port)
            s.rts = True
            time.sleep(0.1)
            s.rts = False
            s.close()
        except Exception as e:
            return str(e)
    time.sleep(10)  # boot to setup() done
    return None if connect_serial() else "no response after reset"


def _try_port(port):
    """Try opening port and checking for device IDN response. Returns serial if found."""
    try:
        s = _open_port(port)
        for _ in range(6):
            time.sleep(0.5)
            try:
                while s.in_waiting:
                    s.readline()
                s.write(b"*IDN?\n")
                s.flush()
                raw = s.readline()
                if raw:
                    line = raw.decode(errors="ignore").strip()
                    if "ESP32S3PS" in line or "ESP32" in line:
                        return s, line
            except Exception:
                pass
        s.close()
    except Exception:
        pass
    return None, None


def connect_serial():
    global ser, SERIAL_PORT
    if ser and ser.is_open:
        return True

    # If explicit port given, try only that
    if len(sys.argv) > 1:
        ports = [sys.argv[1]]
    else:
        ports = (
            [f"/dev/ttyACM{i}" for i in range(10)] +
            [f"/dev/ttyUSB{i}" for i in range(10)]
        )
        ports = [p for p in ports if os.path.exists(p)]

    print("Scanning for device...")
    for port in ports:
        print(f"  Trying {port}...", end=" ", flush=True)
        s, idn = _try_port(port)
        if s:
            ser = s
            SERIAL_PORT = port
            print(f"found! [{idn}]")
            return True
        print("no response")

    # Last resort: open first available without IDN check
    for port in ports:
        try:
            print(f"  Opening {port} without IDN check...")
            ser = _open_port(port)
            SERIAL_PORT = port
            print("WARNING: No IDN response, using anyway")
            return True
        except Exception as e:
            print(f"  {e}")

    print("ERROR: No device found")
    ser = None
    return False


def scpi_query(cmd):
    with ser_lock:
        if not ser or not ser.is_open:
            pass  # fall through to reconnect
        else:
            return _scpi_query_locked(cmd)
    # Try reconnect outside lock, then retry
    if connect_serial():
        with ser_lock:
            return _scpi_query_locked(cmd)
    print(f"  [{cmd}] serial not open after reconnect")
    return None


def _scpi_query_locked(cmd):
    """Execute query while ser_lock is held."""
    global ser
    try:
        # drain
        ser.timeout = 0.05
        drained = 0
        while ser.readline():
            drained += 1

        ser.write((cmd + "\n").encode())
        ser.flush()
        ser.timeout = 0.3
        for _ in range(3):
            raw = ser.readline()
            if not raw:
                continue
            line = raw.decode(errors="ignore").strip()
            if not line:
                continue
            if line.startswith("[") or line.startswith("E (") or line.startswith("I (") or line.startswith("W ("):
                continue
            return line
        return None
    except Exception as e:
        print(f"ERR {cmd}: {e}")
        ser = None
        return None


def scpi_terminal(cmd):
    """Send any line and return every line that comes back (terminal box).
    None if the port is not open."""
    global ser
    if not (ser and ser.is_open) and not connect_serial():
        return None
    with ser_lock:
        try:
            ser.timeout = 0.05
            while ser.readline():
                pass
            ser.write((cmd + "\n").encode())
            ser.flush()
            lines, quiet, t0 = [], 0, time.time()
            ser.timeout = 0.15
            # until 0.45 s of silence (0.6 s if nothing came at all), 8 s at most
            while quiet < (3 if lines else 4) and time.time() - t0 < 8:
                raw = ser.readline()
                if raw:
                    quiet = 0
                    line = raw.decode(errors="ignore").rstrip()
                    if line:
                        lines.append(line)
                else:
                    quiet += 1
            return lines
        except Exception as e:
            print(f"  ERR {cmd}: {e}")
            ser = None
            return None


def scpi_command(cmd):
    with ser_lock:
        if not ser or not ser.is_open:
            pass  # fall through to reconnect
        else:
            return _scpi_command_locked(cmd)
    # Try reconnect outside lock, then retry
    if connect_serial():
        with ser_lock:
            return _scpi_command_locked(cmd)
    return False


def _scpi_command_locked(cmd):
    """Execute command while ser_lock is held."""
    global ser
    try:
        ser.timeout = 0.05
        while ser.readline():
            pass
        ser.write((cmd + "\n").encode())
        ser.flush()
        time.sleep(0.05)
        return True
    except Exception as e:
        print(f"  ERR {cmd}: {e}")
        ser = None
        return False


def fetch_status_or_none():
    """DATA:STAT? parsed to numbers, current in amps; None if no reply."""
    resp = scpi_query("DATA:STAT?")
    if not resp:
        return None
    p = resp.split(",")
    try:
        mA = len(p) >= 7 and p[6] == "1"
        i = float(p[1])
        return {
            "v": float(p[0]), "i": i / 1000.0 if mA else i, "p": float(p[5]),
            "set_v": float(p[2]), "set_i": float(p[3]),
            "output": int(p[4]), "mA": int(mA),
        }
    except (ValueError, IndexError):
        return None


def fetch_status():
    # Logger running: reuse its newest sample instead of a second serial query
    s = logger.latest(max_age=0.8)
    if s:
        return {
            "meas_v": str(s["v"]), "meas_i": str(s["i"] * 1000.0 if s["mA"] else s["i"]),
            "set_v": str(s["set_v"]), "set_i": str(s["set_i"]),
            "output": str(s["output"]), "power": str(s["p"]), "mA_active": str(s["mA"]),
            "ok": True, "port": SERIAL_PORT,
        }
    # Single query returns: meas_v,meas_i,set_v,set_i,output,power,mA_active
    resp = scpi_query("DATA:STAT?")
    if resp:
        parts = resp.split(",")
        if len(parts) >= 6:
            return {
                "meas_v": parts[0],
                "meas_i": parts[1],
                "set_v": parts[2],
                "set_i": parts[3],
                "output": parts[4],
                "power": parts[5],
                "mA_active": parts[6] if len(parts) >= 7 else "0",
                "ok": True, "port": SERIAL_PORT,
            }
    return {
        "meas_v": "0", "meas_i": "0", "set_v": "0",
        "set_i": "0", "output": "0", "power": "0", "mA_active": "0",
        "ok": False, "port": SERIAL_PORT,  # no reply from the PSU
    }


def fetch_graph():
    global ser
    with ser_lock:
        if not ser or not ser.is_open:
            pass  # fall through to reconnect
        else:
            return _fetch_graph_locked()
    if connect_serial():
        with ser_lock:
            return _fetch_graph_locked()
    return None, "Not connected"


def _fetch_graph_locked():
    global ser
    try:
        ser.timeout = 0.05
        while ser.readline():
            pass
        ser.write(b"DATA:GRAPH?\n")
        ser.flush()
        ser.timeout = 2
        indices, voltages, currents = [], [], []
        started = False
        mA_active = 0
        for _ in range(1300):
            raw = ser.readline()
            if not raw:
                if started:
                    break
                continue
            line = raw.decode(errors="ignore").strip()
            if not line:
                if started:
                    break
                continue
            # Parse metadata line: META,mA_Active=0/1
            if line.startswith("META,"):
                if "mA_Active=1" in line:
                    mA_active = 1
                continue
            parts = line.split(",")
            if len(parts) >= 3:
                try:
                    indices.append(int(parts[0]))
                    voltages.append(float(parts[1]) / 1000.0)
                    cur_raw = float(parts[2])
                    # In mA mode, graph stores value*1000 (mA*1000); convert to mA
                    currents.append(cur_raw / 1000.0 if mA_active else cur_raw)
                    started = True
                    if len(indices) >= 1200:
                        break
                except ValueError:
                    continue
            elif started:
                break
        if not indices:
            return None, "No data received"
        return {"idx": indices, "voltage": voltages, "current": currents,
                "mA_active": mA_active}, None
    except Exception as e:
        ser = None
        return None, str(e)


def fetch_stats():
    """Query DATA:STATS? — returns V and I stats lines."""
    global ser
    with ser_lock:
        if not ser or not ser.is_open:
            pass
        else:
            return _fetch_stats_locked()
    if connect_serial():
        with ser_lock:
            return _fetch_stats_locked()
    return None, "Not connected"


def _fetch_stats_locked():
    global ser
    try:
        ser.timeout = 0.05
        while ser.readline():
            pass
        ser.write(b"DATA:STATS?\n")
        ser.flush()
        ser.timeout = 1
        result = {}
        for _ in range(6):
            raw = ser.readline()
            if not raw:
                if result:
                    break
                continue
            line = raw.decode(errors="ignore").strip()
            if not line:
                if result:
                    break
                continue
            if line.startswith("[") or line.startswith("E (") or line.startswith("I (") or line.startswith("W ("):
                continue
            parts = line.split(",")
            if len(parts) >= 7:
                key = parts[0]
                try:
                    result[key] = {
                        "mean": float(parts[1]),
                        "std":  float(parts[2]),
                        "min":  float(parts[3]),
                        "max":  float(parts[4]),
                        "rms":  float(parts[5]),
                        "er":   float(parts[6]),
                    }
                except ValueError:
                    continue
        if not result:
            return None, "No stats received"
        return result, None
    except Exception as e:
        ser = None
        return None, str(e)


def fetch_mem():
    """Query DATA:MEM? — returns 10 lines: idx,voltage,current"""
    global ser
    with ser_lock:
        if not ser or not ser.is_open:
            pass
        else:
            return _fetch_mem_locked()
    if connect_serial():
        with ser_lock:
            return _fetch_mem_locked()
    return []


def _fetch_mem_locked():
    global ser
    try:
        ser.timeout = 0.05
        while ser.readline():
            pass
        ser.write(b"DATA:MEM?\n")
        ser.flush()
        ser.timeout = 1
        slots = []
        for _ in range(12):
            raw = ser.readline()
            if not raw:
                if slots:
                    break
                continue
            line = raw.decode(errors="ignore").strip()
            if not line:
                if slots:
                    break
                continue
            parts = line.split(",")
            if len(parts) >= 3:
                try:
                    v = float(parts[1])
                    i = float(parts[2])
                    slots.append({"v": v, "i": i})
                except ValueError:
                    continue
            elif slots:
                break
        return slots
    except Exception as e:
        print(f"  ERR DATA:MEM?: {e}")
        ser = None
        return []


def fetch_settings():
    """Query DATA:SETT? — returns one CSV line with all settings."""
    resp = scpi_query("DATA:SETT?")
    if not resp:
        return {}
    parts = resp.split(",")
    if len(parts) < 18:
        return {}
    try:
        d = {
            "adcRate": int(parts[0]),
            "adcAvg": int(parts[1]),
            "adcDig": int(parts[2]),
            "buzzer": parts[3] == "1",
            "beepPwr": parts[4] == "1",
            "beepErr": parts[5] == "1",
            "timerEn": parts[7] == "1",
            "timerDur": int(parts[8]),
            "ovpEn": parts[9] == "1",
            "ovpLvl": float(parts[10]),
            "ocpEn": parts[11] == "1",
            "ocpLvl": float(parts[12]),
            "delay": int(parts[13]),
            "asave": int(parts[14]),
            "graphTime": parts[15] == "1",
            "graphSpan": int(parts[16]),
            "graphStop": parts[17] == "1",
        }
        if len(parts) >= 20:
            d["startup"] = int(parts[19])
        if len(parts) >= 22:  # newer firmware
            d["adcDisc"] = parts[20] == "1"
            d["faultProt"] = parts[21] == "1"
        return d
    except (ValueError, IndexError) as e:
        print(f"  ERR parsing settings: {e}")
        return {}


def fetch_fgen():
    """Query DATA:FGEN? — returns freq,ampl,offset,duty.
    Also query SOUR:FGEN:WAVE? for waveform index,name."""
    resp = scpi_query("DATA:FGEN?")
    if not resp:
        return {}
    parts = resp.split(",")
    if len(parts) < 4:
        return {}
    try:
        d = {
            "freq": float(parts[0]),
            "ampl": float(parts[1]),
            "offset": float(parts[2]),
            "duty": float(parts[3]),
        }
    except ValueError:
        return {}
    # Get waveform selection
    wave = scpi_query("SOUR:FGEN:WAVE?")
    if wave:
        wp = wave.split(",", 1)
        try:
            d["waveIdx"] = int(wp[0])
            d["waveName"] = wp[1].strip() if len(wp) > 1 else ""
        except ValueError:
            d["waveIdx"] = 0
            d["waveName"] = "Sine"
    # Get mode (CV/FGEN)
    mode = scpi_query("SOUR:FUNC?")
    d["mode"] = mode if mode else "CV"
    return d


def fetch_arbt(bank=0):
    """Query DATA:ARBT? <bank> — returns 40 lines: idx,value."""
    resp_lines = []
    with ser_lock:
        if not ser or not ser.is_open:
            return {"points": [70] * 40}
        try:
            ser.timeout = 0.05
            while ser.readline():
                pass
            ser.write(f"DATA:ARBT? {bank}\n".encode())
            ser.flush()
            ser.timeout = 1
            for _ in range(45):
                raw = ser.readline()
                if not raw:
                    if resp_lines:
                        break
                    continue
                line = raw.decode(errors="ignore").strip()
                if not line:
                    if resp_lines:
                        break
                    continue
                if line.startswith("[") or line.startswith("E (") or line.startswith("I (") or line.startswith("W ("):
                    continue
                parts = line.split(",")
                if len(parts) >= 2:
                    try:
                        resp_lines.append(float(parts[1]))
                    except ValueError:
                        continue
                elif resp_lines:
                    break
        except Exception as e:
            print(f"  ERR DATA:ARBT?: {e}")
    if not resp_lines:
        return {"points": [70] * 40}
    return {"points": resp_lines}


def send_arbt(bank, points):
    """Send arbitrary waveform points: DATA:ARBT <bank> <idx> <value>."""
    for i, val in enumerate(points[:40]):
        scpi_command(f"DATA:ARBT {bank} {i} {int(val)}")


def fetch_tabl():
    """Query DATA:TABL? — returns LEN line then idx,value lines."""
    length = 100
    points = []
    with ser_lock:
        if not ser or not ser.is_open:
            return {"points": [0.0] * 100, "length": 100}
        try:
            ser.timeout = 0.05
            while ser.readline():
                pass
            ser.write(b"DATA:TABL?\n")
            ser.flush()
            ser.timeout = 2
            for _ in range(260):
                raw = ser.readline()
                if not raw:
                    if points:
                        break
                    continue
                line = raw.decode(errors="ignore").strip()
                if not line:
                    if points:
                        break
                    continue
                if line.startswith("[") or line.startswith("E (") or line.startswith("I (") or line.startswith("W ("):
                    continue
                if line.startswith("LEN,"):
                    try:
                        length = int(line.split(",")[1])
                    except ValueError:
                        pass
                    continue
                parts = line.split(",")
                if len(parts) >= 2:
                    try:
                        points.append(float(parts[1]))
                    except ValueError:
                        continue
                elif points:
                    break
        except Exception as e:
            print(f"  ERR DATA:TABL?: {e}")
    if not points:
        return {"points": [0.0] * 100, "length": 100}
    return {"points": points, "length": length}


def fetch_cal():
    """Query DATA:CAL? — returns calibration data as a dict for PC download."""
    global ser
    with ser_lock:
        if not ser or not ser.is_open:
            pass
        else:
            return _fetch_cal_locked()
    if connect_serial():
        with ser_lock:
            return _fetch_cal_locked()
    return None, "Not connected"


def _fetch_cal_locked():
    global ser
    try:
        ser.timeout = 0.05
        while ser.readline():
            pass
        ser.write(b"DATA:CAL?\n")
        ser.flush()
        ser.timeout = 2
        result = {}
        started = False
        for _ in range(50):
            raw = ser.readline()
            if not raw:
                if started:
                    break
                continue
            line = raw.decode(errors="ignore").strip()
            if not line:
                if started:
                    break
                continue
            if line.startswith("[") or line.startswith("E (") or line.startswith("I (") or line.startswith("W ("):
                continue
            if line == "END":
                break
            started = True
            parts = line.split(",")
            key = parts[0]
            vals = parts[1:]
            if key == "MAC":
                result["mac"] = vals[0] if vals else ""
            elif key == "VCAL":
                result["vCal"] = {"v1": float(vals[0]), "c1": int(vals[1]), "v2": float(vals[2]), "c2": int(vals[3])}
            elif key == "ICAL0":
                result["iCal0"] = {"v1": float(vals[0]), "c1": int(vals[1]), "v2": float(vals[2]), "c2": int(vals[3])}
            elif key == "ICAL1":
                result["iCal1"] = {"v1": float(vals[0]), "c1": int(vals[1]), "v2": float(vals[2]), "c2": int(vals[3])}
            elif key == "LEAK":
                result["leak"] = [float(vals[0]), float(vals[1])]
            elif key == "INLM":
                result["inlMeasure"] = [float(v) for v in vals]
            elif key == "INLI":
                result["inlIdeal"] = [float(v) for v in vals]
            elif key == "INLZCM":
                result["inlZcMeasure"] = [float(v) for v in vals]
            elif key == "INLZCI":
                result["inlZcIdeal"] = [float(v) for v in vals]
        if not result:
            return None, "No calibration data received"
        return result, None
    except Exception as e:
        ser = None
        return None, str(e)


def upload_cal(d):
    """Send a calibration JSON (from downloadCal) back to the device line by line."""
    global ser
    if not ser:
        return "Not connected"
    try:
        # Build lines in the same format DATA:CAL? emits
        lines = []
        if "mac"   in d: lines.append("MAC,"   + str(d["mac"]))
        if "vCal"  in d:
            c = d["vCal"]
            lines.append(f"VCAL,{c['v1']:.6f},{int(c['c1'])},{c['v2']:.6f},{int(c['c2'])}")
        if "iCal0" in d:
            c = d["iCal0"]
            lines.append(f"ICAL0,{c['v1']:.6f},{int(c['c1'])},{c['v2']:.6f},{int(c['c2'])}")
        if "iCal1" in d:
            c = d["iCal1"]
            lines.append(f"ICAL1,{c['v1']:.6f},{int(c['c1'])},{c['v2']:.6f},{int(c['c2'])}")
        if "leak"  in d:
            lines.append(f"LEAK,{d['leak'][0]:.9f},{d['leak'][1]:.9f}")
        if "inlMeasure" in d:
            lines.append("INLM," + ",".join(f"{v:.6f}" for v in d["inlMeasure"]))
        if "inlIdeal"   in d:
            lines.append("INLI," + ",".join(f"{v:.6f}" for v in d["inlIdeal"]))

        with ser_lock:
            ser.timeout = 0.05
            while ser.readline():
                pass
            for line in lines:
                cmd = f"DATA:CAL:SET {line}\n"
                ser.write(cmd.encode())
                ser.flush()
                ser.timeout = 1.0
                resp = ser.readline().decode(errors="ignore").strip()
                if resp != "OK":
                    return f"Device rejected line: {line!r} → {resp!r}"
            # Persist to NVS
            ser.write(b"CAL:SAVE\n")
            ser.flush()
        return None  # success
    except Exception as e:
        ser = None
        return str(e)


def send_tabl(length, points):
    """Send table points: DATA:TABL LEN <n>, then DATA:TABL <idx> <value>."""
    scpi_command(f"DATA:TABL LEN {length}")
    for i, val in enumerate(points[:length]):
        scpi_command(f"DATA:TABL {i} {val:.6f}")


# ---------------------------------------------------------------- data logger
LOG_DIR = os.path.join(SCRIPT_DIR, "logs")


class Logger:
    """Background CSV logger: keeps running with the browser closed. Every
    sample goes to logs/ps_log_<date>.csv and to an in-memory list the
    Logger tab plots (current always in amps)."""

    def __init__(self):
        self.lock = threading.Lock()
        self.stop_ev = threading.Event()
        self.thread = None
        self.samples = []
        self.path = None
        self.interval = 1.0
        self.t0 = 0.0
        self.errors = 0

    def running(self):
        return self.thread is not None and self.thread.is_alive()

    def start(self, interval):
        self.stop()
        os.makedirs(LOG_DIR, exist_ok=True)
        self.interval = max(0.1, float(interval))
        self.path = os.path.join(LOG_DIR, time.strftime("ps_log_%Y%m%d_%H%M%S.csv"))
        with self.lock:
            self.samples = []
        self.errors = 0
        self.stop_ev.clear()
        self.t0 = time.time()
        self.thread = threading.Thread(target=self._run, daemon=True)
        self.thread.start()

    def stop(self):
        self.stop_ev.set()
        if self.thread:
            self.thread.join(3)
        self.thread = None

    def latest(self, max_age):
        with self.lock:
            if self.running() and self.samples and time.time() - self.samples[-1]["ts"] <= max_age:
                return self.samples[-1]
        return None

    def since(self, n):
        with self.lock:
            return self.samples[n:]

    def status(self):
        with self.lock:
            n = len(self.samples)
        return {"running": self.running(), "path": self.path, "count": n,
                "interval": self.interval, "elapsed": (time.time() - self.t0) if self.t0 else 0,
                "errors": self.errors}

    def _run(self):
        with open(self.path, "w", buffering=1) as f:
            f.write("time_iso,t_s,voltage_v,current_a,power_w,set_v,set_i,output,mA_range\n")
            nxt = time.time()
            while not self.stop_ev.is_set():
                st = fetch_status_or_none()
                now = time.time()
                if st:
                    st["ts"] = now
                    st["t"] = now - self.t0
                    f.write("%s,%.3f,%.6f,%.7f,%.6f,%.4f,%.4f,%d,%d\n" % (
                        time.strftime("%Y-%m-%dT%H:%M:%S", time.localtime(now)) + ".%03d" % int((now % 1) * 1000),
                        st["t"], st["v"], st["i"], st["p"], st["set_v"], st["set_i"], st["output"], st["mA"]))
                    with self.lock:
                        self.samples.append(st)
                else:
                    self.errors += 1
                nxt += self.interval
                delay = nxt - time.time()
                if delay < 0:          # fell behind (slow serial): don't burst
                    nxt = time.time()
                    delay = 0
                self.stop_ev.wait(delay)


logger = Logger()


# ---------------------------------------------------------------- sequencer
class Sequencer:
    """Runs a list of steps {v, i, t[, out]} on the PC, `repeat` times.
    Output is switched on at start; `off_at_end` turns it off when done."""

    def __init__(self):
        self.stop_ev = threading.Event()
        self.thread = None
        self.state = {"running": False, "step": -1, "rep": 0, "repeat": 0, "steps": 0, "msg": ""}

    def start(self, steps, repeat=1, off_at_end=True):
        self.stop()
        clean = []
        for st in steps:
            try:
                clean.append({"v": float(st["v"]), "i": float(st["i"]), "t": max(0.05, float(st["t"])),
                              "out": bool(st.get("out", True))})
            except (KeyError, TypeError, ValueError):
                return "bad step: %r" % (st,)
        if not clean:
            return "no steps"
        self.stop_ev.clear()
        self.state = {"running": True, "step": -1, "rep": 0, "repeat": int(repeat), "steps": len(clean), "msg": ""}
        self.thread = threading.Thread(target=self._run, args=(clean, int(repeat), off_at_end), daemon=True)
        self.thread.start()
        return None

    def stop(self):
        self.stop_ev.set()
        if self.thread:
            self.thread.join(3)
        self.thread = None

    def _run(self, steps, repeat, off_at_end):
        out = None
        rep = 0
        try:
            while not self.stop_ev.is_set() and (repeat <= 0 or rep < repeat):
                self.state["rep"] = rep + 1
                for k, st in enumerate(steps):
                    if self.stop_ev.is_set():
                        break
                    self.state["step"] = k
                    scpi_command("SOUR:VOLT %.4f" % st["v"])
                    scpi_command("SOUR:CURR %.4f" % st["i"])
                    if st["out"] != out:
                        scpi_command("OUTP:STAT ON" if st["out"] else "OUTP:STAT OFF")
                        out = st["out"]
                    self.stop_ev.wait(st["t"])
                rep += 1
            self.state["msg"] = "stopped" if self.stop_ev.is_set() else "done"
        finally:
            if off_at_end:
                scpi_command("OUTP:STAT OFF")
            self.state["running"] = False


sequencer = Sequencer()


def fetch_info():
    return {"port": SERIAL_PORT, "idn": scpi_query("*IDN?") or "",
            "driver": scpi_query("SYST:DISP?") or "", "version": scpi_query("SYST:VERS?") or ""}


# Read HTML file
html_path = os.path.join(SCRIPT_DIR, "ps_control.html")
with open(html_path, "r") as f:
    HTML_CONTENT = f.read().encode("utf-8")


class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        path = self.path

        if path == "/status":
            data = fetch_status()
            self._json(data)

        elif path == "/graph":
            data, err = fetch_graph()
            self._json({"error": err} if err else data)

        elif path == "/stats":
            data, err = fetch_stats()
            self._json({"error": err} if err else data)

        elif path == "/mem":
            self._json(fetch_mem())

        elif path == "/settings":
            self._json(fetch_settings())

        elif path == "/fgen":
            self._json(fetch_fgen())

        elif path.startswith("/arbt"):
            bank = 0
            if "bank=" in path:
                try: bank = int(path.split("bank=")[1][:1])
                except ValueError: pass
            self._json(fetch_arbt(bank))

        elif path == "/tabl":
            self._json(fetch_tabl())

        elif path.startswith("/term?c="):
            lines = scpi_terminal(unquote(path[8:]))
            self._json({"ok": lines is not None, "lines": lines or []})

        elif path.startswith("/query?c="):
            cmd = unquote(path[9:])
            resp = scpi_query(cmd)
            self._json({"response": resp if resp else ""})

        elif path == "/cal":
            data, err = fetch_cal()
            self._json({"error": err} if err else data)

        elif path == "/info":
            self._json(fetch_info())

        elif path.startswith("/log/start"):
            iv = 1.0
            if "interval=" in path:
                try: iv = float(path.split("interval=")[1].split("&")[0])
                except ValueError: pass
            logger.start(iv)
            self._json(logger.status())

        elif path == "/log/stop":
            logger.stop()
            self._json(logger.status())

        elif path == "/log/status":
            self._json(logger.status())

        elif path.startswith("/log/data"):
            n = 0
            if "since=" in path:
                try: n = int(path.split("since=")[1].split("&")[0])
                except ValueError: pass
            self._json({"from": n, "samples": [{k: x[k] for k in ("t", "v", "i", "p", "output")} for x in logger.since(n)]})

        elif path == "/seq/stop":
            sequencer.stop()
            self._json(sequencer.state)

        elif path == "/seq/status":
            self._json(sequencer.state)

        elif path == "/reset_board":
            err = hard_reset_board()
            self._json({"error": err} if err else {"ok": True})

        elif path.startswith("/cmd?c="):
            cmd = unquote(path[7:])
            scpi_command(cmd)
            self._json({"ok": True})

        else:
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Cache-Control", "no-cache, no-store")
            self.end_headers()
            self.wfile.write(HTML_CONTENT)

    def do_POST(self):
        path = self.path
        content_len = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(content_len).decode("utf-8") if content_len else "{}"
        try:
            data = json.loads(body)
        except json.JSONDecodeError:
            data = {}

        if path.startswith("/arbt"):
            bank = 0
            if "bank=" in path:
                try: bank = int(path.split("bank=")[1][:1])
                except ValueError: pass
            pts = data.get("points", [])
            send_arbt(bank, pts)
            self._json({"ok": True})

        elif path == "/tabl":
            pts = data.get("points", [])
            length = data.get("length", len(pts))
            send_tabl(length, pts)
            self._json({"ok": True})

        elif path == "/upload_cal":
            err = upload_cal(data)
            self._json({"error": err} if err else {"ok": True})

        elif path == "/seq/start":
            err = sequencer.start(data.get("steps", []), data.get("repeat", 1), data.get("off_at_end", True))
            self._json({"error": err} if err else sequencer.state)

        else:
            self._json({"error": "unknown endpoint"})

    def _json(self, data):
        try:
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Cache-Control", "no-cache")
            self.end_headers()
            self.wfile.write(json.dumps(data).encode())
        except BrokenPipeError:
            pass

    def log_message(self, format, *args):
        pass


class ThreadedServer(socketserver.ThreadingMixIn, HTTPServer):
    daemon_threads = True

    def handle_error(self, request, client_address):
        pass  # suppress BrokenPipeError tracebacks


if __name__ == "__main__":
    print("=" * 50)
    print("  Power Supply Remote Control")
    print("=" * 50)
    print(f"  Port: {SERIAL_PORT}")
    print(f"  Close PIO monitor first!")
    print()
    connect_serial()
    print()

    server = ThreadedServer(("127.0.0.1", PORT), Handler)
    url = f"http://127.0.0.1:{PORT}"
    print(f"  Control panel: {url}")
    print("  Press Ctrl+C to stop")
    print()
    threading.Timer(0.5, lambda: webbrowser.open(url)).start()
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopped")
        sequencer.stop()
        logger.stop()
        if ser and ser.is_open:
            ser.close()
        server.server_close()
