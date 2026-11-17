#!/usr/bin/env python3
"""Random stress / crash test for the PSU over SCPI.
usage: stress.py <seconds> [seed] [mode]
mode: all (default) | ui | heavy | fuzz
Logs everything to stress_<seed>.log; prints events (crash, hang, low memory)."""
import serial, time, sys, re, random, threading, subprocess, collections

PSU = '/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_34:85:18:8F:21:2C-if00'
SIM = '/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_34:85:18:96:01:88-if00'
ELF = '/home/h/Documents/PlatformIO/Projects/S3_PS_lvgl_8.4/.pio/build/esp32-s3-devkitc-1/firmware.elf'
A2L = '/home/h/.platformio/packages/toolchain-xtensa-esp32s3/bin/xtensa-esp32s3-elf-addr2line'
dur = float(sys.argv[1]); seed = int(sys.argv[2]) if len(sys.argv) > 2 else int(time.time())
mode = sys.argv[3] if len(sys.argv) > 3 else 'all'
HEAPCHK = '--heap' in sys.argv
random.seed(seed)
log = open(f'stress_{seed}.log', 'w')
CRASH = re.compile(r'Guru Meditation|Backtrace:|abort\(\) was called|assert failed|rst:0x|Stack canary|stack overflow|task_wdt|Brownout|CORRUPT HEAP|LoadProhibited|StoreProhibited|IllegalInstruction')

class Port:
    def __init__(self, path):
        self.path = path; self.buf = ''; self.lock = threading.Lock(); self.s = None
        self.crashes = []; self.alive = True
        threading.Thread(target=self.reader, daemon=True).start()
    def reader(self):
        while self.alive:
            try:
                if self.s is None:
                    self.s = serial.Serial(self.path, 115200, timeout=0.2)
                d = self.s.read(65536)
            except Exception:
                self.s = None; time.sleep(0.3); continue
            if d:
                t = d.decode(errors='replace')
                log.write(t); log.flush()
                with self.lock:
                    self.buf += t
                    if len(self.buf) > 2_000_000: self.buf = self.buf[-1_000_000:]
                for m in CRASH.finditer(t):
                    self.crashes.append((time.time(), m.group(0)))
    def write(self, c):
        if self is globals().get('psu'): log.write(f'\n>w {c[:160]!r}\n')
        for _ in range(20):
            try:
                if self.s: self.s.write(c.encode() + b'\n'); return
            except Exception: self.s = None
            time.sleep(0.2)
    def mark(self):
        with self.lock: return len(self.buf)
    def since(self, m):
        with self.lock: return self.buf[m:]

psu = Port(PSU)
sim = Port(SIM) if mode in ('all', 'heavy') else None
time.sleep(1)
recent = collections.deque(maxlen=40)

def cmd(c, wait=2.0, pat=None):
    m = psu.mark(); psu.write(c); recent.append(c[:80]); t = time.time()
    log.write(f'\n>>> {c[:200]}\n')
    while time.time() - t < wait:
        time.sleep(0.02)
        o = psu.since(m)
        if pat and re.search(pat, o, re.M): return o
    return psu.since(m)

def simc(c):
    if sim: sim.write(c)

stats = dict(actions=0, hangs=0, crashes=0, minFree=1 << 30, minBig=1 << 30, minStack=1 << 30, minFps=999, maxGap=0, reboots=0)
counts = collections.Counter()
t0 = time.time()

def event(msg):
    s = f'[{time.strftime("%H:%M:%S")} +{time.time()-t0:6.0f}s] {msg}'
    print(s); sys.stdout.flush(); log.write('\n#### ' + s + '\n')

def decode_crash():
    o = psu.buf[-20000:]
    i = max(0, o.rfind('Guru') if 'Guru' in o else o.rfind('Backtrace') - 1500)
    snippet = o[i:i + 3000]
    m = re.search(r'Backtrace:(.*)', snippet)
    dec = ''
    if m:
        pcs = re.findall(r'(0x4[0-9a-f]{7}):0x', m.group(1))
        dec = subprocess.run([A2L, '-pfiaC', '-e', ELF] + pcs, capture_output=True, text=True).stdout
    return snippet, dec

def health(tag):
    o = cmd('*IDN?', 4, r'ESP32S3PS')
    if 'ESP32S3PS' not in o:
        o = cmd('*IDN?', 25, r'ESP32S3PS')
        if 'ESP32S3PS' not in o:
            stats['hangs'] += 1
            event(f'HANG after {tag}; recent: {list(recent)[-8:]}')
            # wait more (boot), then continue
            o = cmd('*IDN?', 40, r'ESP32S3PS')
            if 'ESP32S3PS' not in o:
                event('still no reply; stopping'); return False
    if HEAPCHK:
        h = cmd('DIAG:HEAP?', 3, r'^[01]\s*$')
        if re.search(r'^0\s*$', h, re.M):
            stats['crashes'] += 1
            event(f'HEAP CORRUPT after {tag}; recent: {list(recent)[-12:]}\n{h[-1500:]}')
            return False
    if psu.crashes:
        stats['crashes'] += 1
        snip, dec = decode_crash()
        event(f'CRASH after {tag}: {psu.crashes[-1][1]}; recent: {list(recent)[-10:]}\n{snip[:1500]}\n{dec}')
        psu.crashes.clear()
        time.sleep(8)
        setup()
    return True

def mem():
    o = cmd('DISP:MEM?', 2, r'^\d+,\d+,\d+,\d+')
    m = re.search(r'^(\d+),(\d+),(\d+),(\d+)', o, re.M)
    if m:
        f, b, p, st = map(int, m.groups())
        stats['minFree'] = min(stats['minFree'], f); stats['minBig'] = min(stats['minBig'], b); stats['minStack'] = min(stats['minStack'], st)
        if f < 4000 or b < 1500 or st < 1500: event(f'LOW MEM free={f} big={b} psram={p} stack={st}')
        return f, b, p, st
    return None

def fps():
    o = cmd('DISP:FPS?', 2.5, r'^[\d.]+,\d+,')
    m = re.search(r'^([\d.]+),(\d+),([\d.]+),([\d.]+),(\d+),(\d+),?(\d*)', o, re.M)
    if m:
        f = float(m.group(1)); g = int(m.group(7) or 0)
        stats['maxGap'] = max(stats['maxGap'], g)
        return f, int(m.group(6)), g
    return None

def setup():
    cmd('SOUR:FUNC CV', 1); cmd('OUTP:STAT OFF', 1); cmd('SOUR:VOLT 3', 0.5); cmd('SOUR:CURR 0.5', 0.5)
    cmd('SOUR:FGEN:AMPL 2', 0.5); cmd('SOUR:FGEN:OFFS 1.5', 0.5)

def wait_busy(q, limit):
    t = time.time()
    while time.time() - t < limit:
        o = cmd(q, 1.5, r'^\s*[01]\s*$')
        if re.search(r'^\s*0\s*$', o, re.M): return True
        if random.random() < 0.3: cmd(f'DISP:PAGE {random.choice([2,2,1,0,3])}', 0.3)
    return False

linked = False
def a_page():   cmd(f'DISP:PAGE {random.randint(0,4)}', 0.4)
def a_main():   cmd('DISP:PAGE 2', 0.3)
def a_util():   cmd(f'DISP:PAGE 3,{random.randint(0,7)}', 0.4)
def a_menu():   cmd(f'DISP:MENU {random.randint(0,14)}' + (f',{random.randint(0,400)}' if random.random() < .4 else ''), 0.6)
def a_rate():   cmd(f'SETT:ADC:RATE {random.choice([0,1,2,3,4,4,4])}', 0.5)
def a_out():    cmd(random.choice(['OUTP:STAT ON', 'OUTP:STAT OFF']), 0.4)
def a_set():    cmd(f'SOUR:VOLT {random.uniform(0,4):.3f}', 0.2); cmd(f'SOUR:CURR {random.uniform(0.01,0.5):.3f}', 0.2)
def a_burst():
    for _ in range(random.randint(20, 120)):
        psu.write(random.choice([f'SOUR:VOLT {random.uniform(0,4):.4f}', 'MEAS:VOLT?', 'MEAS:CURR?', f'DISP:PAGE {random.randint(0,4)}', 'MEAS:POW?', 'SOUR:VOLT?']))
        time.sleep(random.choice([0, 0.001, 0.005]))
    recent.append('BURST'); time.sleep(0.5)
def a_fun():
    cmd(f'SOUR:FGEN:WAVE {random.randint(0,19)}', 0.3); cmd(f'SOUR:FGEN:FREQ {random.choice([0.5,2,10,50,100])}', 0.3)
    cmd('SOUR:FUNC FGEN', 0.5); cmd('OUTP:STAT ON', 0.3)
    for _ in range(random.randint(1, 6)):
        random.choice([a_page, a_main, a_util, a_menu])(); time.sleep(random.uniform(0.1, 1.5))
    if random.random() < 0.7: cmd('SOUR:FUNC CV', 0.5)
def a_load():
    cmd('SOUR:FUNC CV', 0.4); cmd('SOUR:VOLT 3', 0.2); cmd('SOUR:CURR 0.3', 0.2); cmd('OUTP:STAT ON', 0.5)
    c = random.choice(['LOAD:SWE', 'LOAD:FFT', 'LOAD:IMP'])
    cmd('DISP:PAGE 3,1', 0.3); cmd(c, 1)
    if random.random() < 0.25: time.sleep(random.uniform(0.5, 4)); cmd('OUTP:STAT OFF', 0.5); recent.append('abort load by OUTP OFF')
    ok = wait_busy('LOAD:BUSY?', 60)
    o = cmd('LOAD:RES?', 1.5); counts['load_ok' if ok else 'load_timeout'] += 1
def a_trace():
    if linked and random.random() < 0.6:
        cmd('TRAC:CONF 3,0.05,0.15,0.0001,3,15,4,60', 0.4); cmd('TRAC:STAR', 1)
    else:
        cmd('TRAC:DEMO', 1)
    cmd('DISP:PAGE 3,' + str(random.randint(0, 7)), 0.3)
    if random.random() < 0.3: time.sleep(random.uniform(0.5, 3)); cmd('TRAC:STOP', 0.5)
    t = time.time()
    while time.time() - t < 90:
        o = cmd('TRAC:STAT?', 1.2)
        if not re.search(r'run|sweep|servo|busy', o, re.I): break
        random.choice([a_page, a_util, a_main, lambda: None])()
    cmd('TRAC:DATA?', 2)
def a_link():
    global linked
    r = random.random()
    if r < 0.15: cmd('LINK:ENAB 0', 1.5); linked = False
    elif r < 0.6:
        cmd('LINK:ENAB 1', 2); simc('auto'); cmd('LINK:SCAN', 4)
        o = cmd('LINK:LIST?', 1.5)
        if 'PSU-' in o:
            cmd('LINK:REQ 0', 3); o = cmd('LINK:STAT?', 1.5); linked = 'Master' in o or 'MASTER' in o
    elif r < 0.75: cmd('LINK:UNL', 1); linked = False
    elif r < 0.85: simc('mute'); time.sleep(random.uniform(0.2, 1.5)); simc('mute')
    else: cmd(f'LINK:RAT {random.randint(1,100)}', 0.3); cmd('LINK:STAT?', 1)
def a_weather(): cmd('SYST:WEAT:FETC', 15, r'^(OK|ERR|fail|\d)')
def a_shot():   cmd('DISP:SHOT?', 30, r'^END')
def a_splash(): cmd(f'DISP:SPLASH:SHOT? {random.randint(300,2500)}', 40, r'^END')
def a_fpsov(): cmd(f'DISP:FPS {random.randint(0,1)}', 0.3)
def a_query():  cmd(random.choice(['SYST:LOG?', 'LINK:DIAG?', 'TRAC:CONF?', 'LOAD:LIB?', 'SYST:ERR:ALL?', 'MEAS:VOLT:MEAN?', 'SYST:WEAT?', 'DISP:PAGE?']), 1)
def a_fuzz():
    k = random.randint(0, 9)
    if k == 0: psu.write('X' * random.randint(300, 5000))
    elif k == 1: psu.write(''.join(chr(random.randint(1, 255)) for _ in range(random.randint(1, 200))))
    elif k == 2: psu.write(random.choice(['SOUR:VOLT', 'SOUR:VOLT abc', 'SOUR:VOLT -5', 'SOUR:VOLT 1e9', 'SOUR:CURR nan', 'SOUR:CURR inf', 'DISP:PAGE 99', 'DISP:PAGE -1', 'DISP:PAGE 3,99', 'DISP:MENU 999', 'DISP:MENU -3', 'LINK:REQ 50', 'LINK:RAT 0', 'LINK:RAT 5000', 'TRAC:CONF 0,0,0,0,0', 'TRAC:CONF 1e9,1e9,1e9,1,1000,100000,99,999999', 'TRAC:CONF -1,-1,-1,-1,-1,-1,-1,-1', 'SOUR:FGEN:WAVE 999', 'SOUR:FGEN:FREQ 0', 'SOUR:FGEN:FREQ 1e9', 'SOUR:FGEN:AMPL -9', 'SETT:ADC:RATE 99', 'DISP:AREA 0,0,9999,9999', 'DISP:AREA 300,200,10,10', 'LOAD:IMAX 999', 'LOAD:LEARN ' + 'n' * 300, 'SYST:WEAT:FORC 99', 'SYST:WEAT:FORC -9', 'DISP:SPLASH:SHOT? 0', 'DISP:SPLASH:SHOT? 99999999', 'LINK:ACC 1', ':::::', '*', '**IDN?', 'MEAS:VOLT?;MEAS:CURR?;' * 30]))
    elif k == 3: psu.write(';'.join(['SOUR:VOLT?'] * random.randint(2, 60)))
    elif k == 4: psu.write('\r\n' * 50)
    else: psu.write(random.choice(['FOO', 'SOUR:', 'MEAS', 'DISP:', 'LINK:', 'TRAC:', 'LOAD:']) + ''.join(random.choice('ABCDEFGHIJKLMNOPQRSTUVWXYZ:?0123456789 ,.-') for _ in range(random.randint(0, 40))))
    recent.append(f'FUZZ{k}'); log.write(f'\n>>> (fuzz {k} sent)\n'); time.sleep(0.3)

pools = {
 'all':   [(a_page,8),(a_main,8),(a_util,5),(a_menu,6),(a_rate,3),(a_out,3),(a_set,4),(a_burst,3),(a_fun,4),(a_load,3),(a_trace,3),(a_link,4),(a_weather,1),(a_shot,1),(a_splash,2),(a_fpsov,2),(a_query,4),(a_fuzz,4)],
 'ui':    [(a_page,8),(a_main,8),(a_util,6),(a_menu,8),(a_burst,3),(a_splash,2),(a_fpsov,2),(a_query,2)],
 'heavy': [(a_fun,4),(a_load,4),(a_trace,4),(a_link,4),(a_weather,2),(a_splash,2),(a_shot,1),(a_page,3),(a_main,3)],
 'fuzz':  [(a_fuzz,10),(a_query,2),(a_page,2)],
}
pool = [f for f, w in pools[mode] for _ in range(w)]

if not health('start'): sys.exit(1)
setup(); mem(); f = fps()
event(f'start mode={mode} seed={seed} mem={mem()} fps={f}')
lastReport = time.time()
while time.time() - t0 < dur:
    a = random.choice(pool); name = a.__name__
    try: a()
    except Exception as e: event(f'harness error in {name}: {e}')
    stats['actions'] += 1; counts[name] += 1
    if not health(name): break
    if stats['actions'] % 6 == 0:
        mem()
        cmd('DISP:PAGE 2', 0.4); time.sleep(2.3)
        r = fps()
        if r:
            stats['minFps'] = min(stats['minFps'], r[0])
            if r[0] < 30 and r[1] > 200: event(f'main page slow: fps={r} after {list(recent)[-5:]}')
    if time.time() - lastReport > 300:
        lastReport = time.time(); event(f'progress {stats}')
setup(); cmd('LINK:ENAB 0', 1); cmd('DISP:PAGE 2', 0.5)
event(f'DONE {stats} {dict(counts)}')
psu.alive = False
