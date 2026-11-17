import serial, time, re, random, sys
P = '/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_34:85:18:8F:21:2C-if00'
p = serial.Serial(P, 115200, timeout=0.1); time.sleep(0.3); p.read(1 << 20)
log = open('loadrep.log', 'w')
def cmd(c, wait=1.0, pat=None):
    p.write((c + '\n').encode()); t = time.time(); o = b''
    while time.time() - t < wait:
        o += p.read(65536)
        if pat and re.search(pat, o.decode(errors='ignore'), re.M): break
    s = o.decode(errors='ignore'); log.write(f'>>> {c}\n{s}\n'); log.flush(); return s
def heap(tag):
    o = cmd('DIAG:HEAP?', 3, r'^[01]\s*$')
    if re.search(r'^0\s*$', o, re.M) or 'Guru' in o or 'rst:' in o:
        print('CORRUPT/CRASH after', tag, o[-1500:]); sys.exit(1)
    if 'ESP32' not in cmd('*IDN?', 3, 'ESP32'): print('no reply after', tag); sys.exit(1)
random.seed(int(sys.argv[2]) if len(sys.argv) > 2 else 1)
cmd('LINK:ENAB 1', 2); cmd('LINK:SCAN', 4); cmd('LINK:REQ 0', 3)
t0 = time.time(); n = 0
while time.time() - t0 < float(sys.argv[1]):
    c = random.choice(['LOAD:SWE', 'LOAD:SWE', 'LOAD:IMP', 'LOAD:FFT'])
    cmd('SOUR:FUNC CV', 0.3); cmd('SOUR:VOLT 3', 0.2); cmd('SOUR:CURR 0.3', 0.2); cmd('OUTP:STAT ON', 0.5)
    cmd(random.choice(['DISP:PAGE 3,1', 'DISP:PAGE 2', 'DISP:PAGE 1']), 0.3); cmd(c, 1)
    t = time.time()
    while time.time() - t < 60:
        cmd(f'DISP:PAGE {random.choice([0,1,2,3,3,4])}' + (',1' if random.random() < .3 else ''), random.uniform(0.1, 0.6))
        if random.random() < 0.15: cmd('DISP:MENU %d' % random.randint(0, 14), 0.3)
        if re.search(r'^0\s*$', cmd('LOAD:BUSY?', 0.6, r'^[01]\s*$'), re.M): break
    cmd('LOAD:RES?', 1); heap(c); n += 1
    print(n, c, 'ok', round(time.time() - t0)); sys.stdout.flush()
cmd('OUTP:STAT OFF'); cmd('LINK:ENAB 0', 2); print('done', n)
