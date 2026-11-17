import serial, time, re, threading
P = '/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_34:85:18:8F:21:2C-if00'
S = '/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_34:85:18:96:01:88-if00'
p = serial.Serial(P, 115200, timeout=0.1); s = serial.Serial(S, 115200, timeout=0.1)
time.sleep(0.3); p.read(1 << 20); s.read(1 << 20)
simlog = []
def simread():
    while True:
        d = s.read(4096)
        if d: simlog.append(d.decode(errors='ignore'))
threading.Thread(target=simread, daemon=True).start()
def cmd(c, wait=1.5, pat=None):
    p.write((c + '\n').encode()); t = time.time(); o = b''
    while time.time() - t < wait:
        o += p.read(65536)
        if pat and re.search(pat, o.decode(errors='ignore'), re.M): break
    return o.decode(errors='ignore')
def sim(c, w=1.0): s.write((c + '\n').encode()); time.sleep(w)
def offs(): return ''.join(simlog).count('slave output OFF')
def lost(): return ''.join(simlog).count('link lost')
sim('auto'); cmd('SOUR:FUNC CV'); cmd('SOUR:VOLT 3'); cmd('SOUR:CURR 0.3'); cmd('SETT:ADC:RATE 4')
cmd('LINK:ENAB 1', 2); cmd('LINK:SCAN', 4); cmd('LINK:REQ 0', 3); cmd('OUTP:STAT ON'); time.sleep(1.5)
print('start', cmd('LINK:STAT?').strip()[-80:])
tests = [('rate0 + page switches', ['SETT:ADC:RATE 0'] + ['DISP:PAGE %d' % k for k in (0, 1, 2, 3, 4, 2, 1, 2)] + ['SETT:ADC:RATE 4']),
         ('FUN start/stop', ['SOUR:FUNC FGEN', 'SOUR:FUNC CV']),
         ('splash', ['DISP:SPLASH:SHOT? 1500']),
         ('screenshot', ['DISP:SHOT?']),
         ('hang 1000 ms', ['DIAG:HANG 1000']),
         ('hang 3000 ms (must drop)', ['DIAG:HANG 3000'])]
for name, cmds in tests:
    simlog.clear()
    for c in cmds:
        cmd(c, 40 if 'SHOT' in c else 3.5 if 'HANG' in c else 1.2, r'^END' if 'SHOT' in c else None)
    time.sleep(3.5 if 'must' in name else 1)
    print(f'{name:26s} sim link lost: {lost()} sim-off: {offs()}  psu out={cmd("OUTP:STAT?").strip()[-1:]}  {cmd("LINK:STAT?").strip()[-45:]}')
    if lost(): cmd('OUTP:STAT ON'); time.sleep(1)
cmd('OUTP:STAT OFF'); cmd('LINK:ENAB 0', 2)
