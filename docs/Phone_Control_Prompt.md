# Prompt: control the PSU from a phone (paste into Claude)

Copy everything below the line into Claude (claude.ai on the phone, or Claude Code on the laptop).

---

I have a home-made precision lab power supply (ESP32-S3, 0-32.768 V, 0-6.5 A, 24-bit ADC). It is controlled over **SCPI on a USB serial port** (115200 baud, newline-terminated, case-insensitive). It sits next to a **Linux laptop** (Ubuntu) that is always on and has internet. I want to **control and watch it from my phone, from any network** (home Wi-Fi, mobile data, work).

Build me a complete, ready-to-run project:

## Architecture (please keep this)
1. **Bridge server** on the Linux laptop, Python 3 + FastAPI + uvicorn + pyserial.
   - Opens the PSU by its stable path: `/dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_34:85:18:8F:21:2C-if00` (make it a config value).
   - One background thread **owns the serial port**, reads continuously (if nobody reads, the PSU's USB prints block for up to 100 ms each), and serializes commands through a queue with a lock. Never two commands in flight.
   - The PSU also prints debug log lines, e.g. `[123456][D][WiFiGeneric.cpp:1039] ...` or `Table data saved to SPIFFS`. Ignore lines starting with `[` and unrelated text: match each reply to its query by pattern and by timeout (most replies arrive within 50-300 ms; set commands like `:SOUR:VOLT 5` send no reply).
   - Reconnects by itself if the USB is unplugged or the PSU reboots (it re-enumerates).
   - Polls measurements ~5 times/s and pushes them to browsers over a **WebSocket**.
   - REST + WebSocket API; log every command sent with a time stamp to a rotating file.
2. **Mobile-first web app** (a single-page PWA, plain HTML/JS or a small React build, served by the bridge). Dark theme, big touch targets, works one-handed, "Add to Home Screen" with an icon.
3. **Access from any network:** use **Tailscale** (preferred: private, no open ports, free; phone app + laptop) and give me exact install commands for Ubuntu and steps for the phone. Also describe the **Cloudflare Tunnel** alternative with Cloudflare Access login. Never expose the bridge directly on a public port.
4. **Security:** a login (password stored hashed in a config file, session cookie, rate-limited), HTTPS when via Cloudflare, and the Tailscale route needs no extra port. A **read-only mode** toggle in the UI.
5. **Safety:**
   - Turning the output ON needs a confirm tap (slide-to-confirm).
   - The UI has max V / max A guards (config) that the bridge enforces too.
   - A big red **OUTPUT OFF** button, always visible.
   - Optional "dead-man" setting: if no phone has been connected for N minutes while the output is on, the bridge turns the output off.
   - Show a banner when protection has tripped (OVP/OCP/OTP) with a "clear" button.
6. **systemd** user service so the bridge starts at boot; an `install.sh`; a README with every step.

## The UI (screens)
- **Main:** large live V and A readings (4-5 digits), W, mode (CV/CC), output state, V-set / I-set with +/- steppers and a numeric keypad, presets (3.3 V, 5 V, 12 V, custom), output on/off, mA-range switch.
- **Graph:** live V and I chart (last 60 s, zoom/pan, pause, export CSV).
- **Function generator:** waveform (index 0-19), frequency, amplitude, offset, duty, start/stop.
- **Tools:** Load analyser (sweep / FFT / impedance, show result text), BJT curve tracer (config, start/demo/stop, plot the CSV as Ic-Vce curves), PSU link status.
- **Screen:** a "show me the PSU's screen" button that fetches a screenshot and displays it (see `:DISP:SHOT?` below), plus buttons to switch its display page.
- **Log:** the PSU's event log (`:SYST:LOG?`) and the bridge's command log.

## SCPI commands (the PSU's real command set)
Common: `*IDN?` -> `Anthropic,ESP32S3PS,<serial>,v<version>`; `*RST`, `*CLS`, `:SYST:ERR?`, `:SYST:ERR:ALL?`.

Output and setpoints:
- `:SOUR:VOLT <v>` / `:SOUR:VOLT?` (units V, mV allowed), `:SOUR:CURR <a>` / `:SOUR:CURR?` (A, mA, uA)
- `:SOUR:VOLT:LIM?`, `:SOUR:CURR:LIM?` (maximums)
- `:OUTP:STAT ON|OFF` / `:OUTP:STAT?` -> `1|0`
- `:OUTP:PROT:TRIP?` -> `1|0`, `:OUTP:PROT:CLE`
- `:SOUR:FUNC CV|FGEN` / `:SOUR:FUNC?` (CV = normal, FGEN = function generator)
- `:SENS:CURR:RANG A|MA` / `?`

Measurements: `:MEAS:VOLT?`, `:MEAS:CURR?`, `:MEAS:POW?`, `:MEAS:VOLT:MIN?|MAX?|MEAN?`, `:MEAS:CURR:MIN?|MAX?|MEAN?` (plain numbers in V, A, W).

Function generator: `:SOUR:FGEN:WAVE <n>` / `?`, `:SOUR:FGEN:FREQ <Hz>`, `:SOUR:FGEN:AMPL <V>`, `:SOUR:FGEN:OFFS <V>`, `:SOUR:FGEN:DUTY <0-1>`.

Display: `:DISP:PAGE <0-4>[,<sub>]` (0 Stats, 1 Graph, 2 Main, 3 Utility, 4 Settings) / `:DISP:PAGE?`; `:DISP:FPS?` -> `fps,px,render_ms,flush_ms,busy%,adc_sps,max_gap_ms,areas`; `:DISP:MEM?` -> `internal_free,largest_block,psram_free,ui_stack_unused`.

Screenshot: `:DISP:SHOT?` replies with 240 lines of 1280 hex characters (320 pixels x 4 hex digits, RGB565 per pixel, most significant byte first), then a line `END`. It takes several seconds; show a progress bar. Decode to a PNG on the bridge.

Load analyser (output must be ON, not FGEN): `:LOAD:SWE`, `:LOAD:FFT`, `:LOAD:IMP`, `:LOAD:BUSY?` (poll until `0`), `:LOAD:RES?` (result lines joined by `;`), `:LOAD:DET?`.

Curve tracer: `TRAC:CONF vceMax,icMax,pMax,ibStep,curves[,points,vbMax,settleMs]`, `TRAC:CONF?`, `TRAC:STAR`, `TRAC:DEMO`, `TRAC:STOP`, `TRAC:STAT?`, `TRAC:DATA?` (CSV lines `curve,ib_A,vce_V,ic_A,ib_pt_A`).

PSU link: `LINK:STAT?`, `LINK:ENAB 0|1`, `LINK:SCAN`, `LINK:LIST?`, `LINK:REQ <n>`, `LINK:UNL`, `LINK:RAT <1-100>`.

Event log: `:SYST:LOG?` (lines `boot,uptime_s,event,value,arg`), `:SYST:LOG:CLE`.

## Deliverables
- The full source tree (bridge, web app, install script, systemd unit, config example, README).
- A **simulator mode** (`--sim`) that fakes the PSU (a resistive load with noise) so I can try the UI without hardware.
- Tests for the reply-matching and the safety guards.
- Keep it simple to install: `git clone`, `./install.sh`, open the Tailscale URL on the phone.

Ask me only what you truly cannot decide; otherwise choose sensible defaults and explain them in the README.
