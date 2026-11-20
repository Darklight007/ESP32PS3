// Local weather for the splash: once per boot, in the background, WiFi on,
// location from the public IP (ip-api.com), current weather (open-meteo.com),
// cached in NVS, WiFi off again. See weather.h.
#include "weather.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <sys/time.h>

extern const char *ssid;     // OTA.h
extern const char *password; // OTA.h

static volatile int s_kind = -1; // -1 = not loaded
static int s_force = -1;         // SCPI override, -1 = live
static float s_temp = NAN;
static int s_code = -1;
static char s_city[24] = "";
static char s_status[48] = "idle";
static volatile bool s_busy = false;
static volatile bool s_abort = false; // boot deadline hit: stop, radio off
static float s_lat = NAN, s_lon = NAN; // weather location (sun position)
static int32_t s_utcOff = INT32_MIN;    // local time offset, s (open-meteo, DST included)
static float s_fAlt = NAN, s_fAz = NAN; // SYST:WEAT:SUN override

static WxKind kindOfCode(int c)
{
    if (c < 0)
        return WX_CLEAR;
    if (c <= 1)
        return WX_CLEAR;
    if (c <= 3)
        return WX_CLOUD;
    if (c == 45 || c == 48)
        return WX_FOG;
    if ((c >= 71 && c <= 77) || c == 85 || c == 86)
        return WX_SNOW;
    if (c >= 95)
        return WX_THUNDER;
    if ((c >= 51 && c <= 67) || (c >= 80 && c <= 82))
        return WX_RAIN;
    return WX_CLOUD;
}

static void load()
{
    Preferences p;
    p.begin("my-app", true);
    s_code = p.getInt("wx_code", -1);
    s_temp = p.getFloat("wx_temp", NAN);
    s_force = p.getInt("wx_force", -1);
    String c = p.getString("wx_city", "");
    // sun position: the last weather location (older saves: user or IP place)
    const char *la = p.isKey("wx_lat") ? "wx_lat" : p.isKey("wx_ulat") ? "wx_ulat" : p.isKey("wx_iplat") ? "wx_iplat" : nullptr;
    if (la)
        s_lat = p.getFloat(la, NAN), s_lon = p.getFloat(!strcmp(la, "wx_lat") ? "wx_lon" : !strcmp(la, "wx_ulat") ? "wx_ulon" : "wx_iplon", NAN);
    s_utcOff = p.getInt("wx_utco", INT32_MIN);
    p.end();
    strlcpy(s_city, c.c_str(), sizeof s_city);
    s_kind = kindOfCode(s_code);
}

WxKind weatherKind()
{
    if (s_kind < 0)
        load();
    return (WxKind)(s_force >= 0 ? s_force : s_kind);
}

static bool numAfter(const String &s, const char *key, float &v)
{
    const int i = s.indexOf(key);
    if (i < 0)
        return false;
    v = s.substring(i + strlen(key)).toFloat();
    return true;
}

// "Mon, 17 Nov 2026 07:23:11 GMT" -> system clock (UTC). The RTC keeps it
// over soft resets; a power cycle loses it until the next fetch.
static void setClockFromHttpDate(const String &d)
{
    static const char *const mon[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char m[4] = "";
    int day, year, hh, mm, ss;
    const int c = d.indexOf(',');
    if (c < 0 || sscanf(d.c_str() + c + 1, " %d %3s %d %d:%d:%d", &day, m, &year, &hh, &mm, &ss) != 6)
        return;
    int mi = -1;
    for (int k = 0; k < 12; k++)
        if (!strcmp(m, mon[k]))
            mi = k;
    if (mi < 0 || year < 2024)
        return;
    // days since 1970-01-01 (civil from days, proleptic Gregorian)
    const int y = year - (mi < 2), era = y / 400, yoe = y - era * 400;
    const int mp = (mi + 10) % 12, doy = (153 * mp + 2) / 5 + day - 1, doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    const int64_t days = (int64_t)era * 146097 + doe - 719468;
    struct timeval tv = {(time_t)(days * 86400 + hh * 3600 + mm * 60 + ss), 0};
    settimeofday(&tv, nullptr);
}

static void fetchTask(void *)
{
    s_busy = true;
    s_abort = false;
    strlcpy(s_status, "connecting", sizeof s_status);
    const uint32_t t0 = millis();
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    // fast connect: channel + BSSID of the last good connection (no scan)
    uint8_t bssid[6];
    int ch = 0;
    {
        Preferences p;
        p.begin("my-app", true);
        ch = p.getInt("wx_ch", 0);
        if (p.getBytes("wx_bssid", bssid, 6) != 6)
            ch = 0;
        p.end();
    }
    uint32_t ipc[4] = {0, 0, 0, 0}; // ip, gateway, mask, dns of the last connection: no DHCP
    {
        Preferences p;
        p.begin("my-app", true);
        if (p.getBytes("wx_ip", ipc, sizeof ipc) != sizeof ipc)
            ipc[0] = 0;
        p.end();
    }
    if (ch > 0 && ipc[0])
        WiFi.config(IPAddress(ipc[0]), IPAddress(ipc[1]), IPAddress(ipc[2]), IPAddress(ipc[3]));
    if (ch > 0)
        WiFi.begin(ssid, password, ch, bssid);
    else
        WiFi.begin(ssid, password);
    while (!s_abort && WiFi.status() != WL_CONNECTED && millis() - t0 < (ch > 0 ? 2500u : 6000u))
        vTaskDelay(pdMS_TO_TICKS(50));
    if (!s_abort && WiFi.status() != WL_CONNECTED && ch > 0) // AP moved channel: one full scan, DHCP
    {
        WiFi.disconnect();
        WiFi.config(IPAddress((uint32_t)0), IPAddress((uint32_t)0), IPAddress((uint32_t)0));
        WiFi.begin(ssid, password);
        const uint32_t t1 = millis();
        while (!s_abort && WiFi.status() != WL_CONNECTED && millis() - t1 < 6000)
            vTaskDelay(pdMS_TO_TICKS(100));
    }
    if (!s_abort && WiFi.status() == WL_CONNECTED)
    {
        {
            Preferences p;
            p.begin("my-app", false);
            if (p.getInt("wx_ch", 0) != WiFi.channel())
            {
                p.putInt("wx_ch", WiFi.channel());
                p.putBytes("wx_bssid", WiFi.BSSID(), 6);
            }
            const uint32_t nip[4] = {(uint32_t)WiFi.localIP(), (uint32_t)WiFi.gatewayIP(), (uint32_t)WiFi.subnetMask(),
                                     (uint32_t)WiFi.dnsIP()};
            if (memcmp(nip, ipc, sizeof nip))
                p.putBytes("wx_ip", nip, sizeof nip);
            p.end();
        }
        HTTPClient http;
        http.setConnectTimeout(1500);
        http.setTimeout(2000);
        float lat = NAN, lon = NAN;
        String city;
        strlcpy(s_status, "locating", sizeof s_status);
        String ucity; // user-set city (SYST:WEAT:CITY): geocoded once, cached
        {
            Preferences p;
            p.begin("my-app", true);
            if (p.isKey("wx_ucity"))
                ucity = p.getString("wx_ucity", "");
            if (p.isKey("wx_ulat"))
                lat = p.getFloat("wx_ulat", NAN), lon = p.getFloat("wx_ulon", NAN);
            p.end();
        }
        if (ucity.length())
        {
            city = ucity;
            if (!s_abort && (isnan(lat) || isnan(lon)))
            {
                String q = ucity;
                q.replace(" ", "%20");
                if (http.begin("http://geocoding-api.open-meteo.com/v1/search?count=1&name=" + q) && http.GET() == 200)
                {
                    const String b = http.getString();
                    numAfter(b, "\"latitude\":", lat);
                    numAfter(b, "\"longitude\":", lon);
                }
                http.end();
                if (!isnan(lat) && !isnan(lon))
                {
                    Preferences p;
                    p.begin("my-app", false);
                    p.putFloat("wx_ulat", lat);
                    p.putFloat("wx_ulon", lon);
                    p.end();
                }
            }
        }
        else
        {
            // Location from the public IP: ~3 s, so it is saved and looked up
            // again only every 10th boot (the boot waits at most 4.5 s)
            Preferences p;
            p.begin("my-app", true);
            uint8_t age = p.getUChar("wx_ipage", 255);
            if (age < 10)
                lat = p.getFloat("wx_iplat", NAN), lon = p.getFloat("wx_iplon", NAN), city = p.getString("wx_ipcity", "");
            p.end();
            if (!isnan(lat) && !isnan(lon) && age < 10)
            {
                p.begin("my-app", false);
                p.putUChar("wx_ipage", age + 1);
                p.end();
            }
            else if (!s_abort && http.begin("http://ip-api.com/json/?fields=lat,lon,city") && http.GET() == 200)
            {
                const String b = http.getString();
                numAfter(b, "\"lat\":", lat);
                numAfter(b, "\"lon\":", lon);
                const int i = b.indexOf("\"city\":\"");
                if (i >= 0)
                    city = b.substring(i + 8, b.indexOf('"', i + 8));
                if (!isnan(lat) && !isnan(lon))
                {
                    p.begin("my-app", false);
                    p.putFloat("wx_iplat", lat);
                    p.putFloat("wx_iplon", lon);
                    p.putString("wx_ipcity", city);
                    p.putUChar("wx_ipage", 0);
                    p.end();
                }
            }
        }
        http.end();
        if (!s_abort && !isnan(lat) && !isnan(lon))
        {
            strlcpy(s_status, "weather", sizeof s_status);
            char url[160];
            snprintf(url, sizeof url,
                     "http://api.open-meteo.com/v1/forecast?latitude=%.2f&longitude=%.2f&current=weather_code,temperature_2m&timezone=auto",
                     lat, lon);
            float code = NAN, temp = NAN, off = NAN;
            static const char *hdrs[] = {"Date"};
            if (http.begin(url) && (http.collectHeaders(hdrs, 1), http.GET() == 200))
            {
                setClockFromHttpDate(http.header("Date"));
                const String b = http.getString();
                numAfter(b, "\"utc_offset_seconds\":", off);
                const int cur = b.indexOf("\"current\":{");
                if (cur >= 0)
                {
                    const String c = b.substring(cur);
                    numAfter(c, "\"weather_code\":", code);
                    numAfter(c, "\"temperature_2m\":", temp);
                }
            }
            http.end();
            if (!isnan(code))
            {
                s_code = (int)code;
                s_temp = temp;
                strlcpy(s_city, city.c_str(), sizeof s_city);
                s_kind = kindOfCode(s_code);
                s_lat = lat, s_lon = lon;
                Preferences p;
                p.begin("my-app", false);
                if (!p.isKey("wx_lat") || p.getFloat("wx_lat", NAN) != lat || p.getFloat("wx_lon", NAN) != lon)
                    p.putFloat("wx_lat", lat), p.putFloat("wx_lon", lon);
                p.putInt("wx_code", s_code);
                p.putFloat("wx_temp", s_temp);
                if (!isnan(off) && (int32_t)off != p.getInt("wx_utco", INT32_MIN))
                    p.putInt("wx_utco", s_utcOff = (int32_t)off);
                p.putString("wx_city", s_city);
                p.end();
                snprintf(s_status, sizeof s_status, "ok %lus", (unsigned long)((millis() - t0) / 1000));
            }
            else
                strlcpy(s_status, "weather failed", sizeof s_status);
        }
        else
        {
            strlcpy(s_status, "locate failed", sizeof s_status);
            Preferences p;
            p.begin("my-app", false);
            p.remove("wx_ip"); // stale static IP? DHCP next time
            p.end();
        }
    }
    else if (s_abort)
        strlcpy(s_status, "boot deadline", sizeof s_status);
    else
    {
        strlcpy(s_status, "no wifi", sizeof s_status);
        Preferences p; // out of range: don't slow the next 5 boots
        p.begin("my-app", false);
        p.putInt("wx_skip", 5);
        p.end();
    }
    // radio off: no RF next to the 24-bit ADC
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    snprintf(s_status + strlen(s_status), sizeof s_status - strlen(s_status), " %lums", (unsigned long)(millis() - t0));
    s_busy = false;
    vTaskDelete(nullptr);
}

void weatherFetchStart()
{
    if (s_busy)
        return;
    if (s_kind < 0)
        load();
    if (heap_caps_get_free_size(MALLOC_CAP_INTERNAL) < 50000)
    {
        strlcpy(s_status, "low RAM: fetched at boot only", sizeof s_status);
        return;
    }
    if (millis() < 10000) // at boot: skip while WiFi was recently out of range, or turned off
    {
        Preferences p;
        p.begin("my-app", false);
        if (!p.getBool("wx_auto", true))
        {
            p.end();
            strlcpy(s_status, "auto off", sizeof s_status);
            return;
        }
        const int skip = p.getInt("wx_skip", 0);
        if (skip > 0)
            p.putInt("wx_skip", skip - 1);
        p.end();
        if (skip > 0)
        {
            snprintf(s_status, sizeof s_status, "skipped (no wifi, %d more)", skip - 1);
            return;
        }
    }
    s_busy = true;
    if (xTaskCreatePinnedToCore(fetchTask, "weather", 6144, nullptr, 0, nullptr, 0) != pdPASS)
    {
        s_busy = false;
        strlcpy(s_status, "no memory", sizeof s_status);
    }
}

// Boot: the fetch runs while the display and splash start; setup() waits
// here before the big internal-RAM allocations (draw buffers, tasks) so the
// WiFi driver's RAM is free again by then.
void weatherWait(uint32_t untilMs)
{
    while (s_busy && millis() < untilMs)
        delay(20);
    if (!s_busy)
        return;
    // late (slow server): abort and wait for the radio to be off, or the
    // ADC task can't get its RAM (STARTUP FAILURE)
    s_abort = true;
    const uint32_t t = millis();
    while (s_busy && millis() - t < 5000)
        delay(20);
}
bool weatherBusy() { return s_busy; }

void weatherForce(int kind)
{
    s_force = kind < 0 || kind >= WX_COUNT ? -1 : kind;
    Preferences p;
    p.begin("my-app", false);
    p.putInt("wx_force", s_force);
    p.end();
}

String weatherInfo()
{
    if (s_kind < 0)
        load();
    static const char *const nm[WX_COUNT] = {"CLEAR", "CLOUD", "RAIN", "SNOW", "FOG", "THUNDER"};
    char b[160];
    snprintf(b, sizeof b, "%s,%d,%.1f,%s,%s,%s%s", nm[weatherKind()], s_code, s_temp, s_city[0] ? s_city : "?",
             s_status, s_force >= 0 ? "forced " : "", s_force >= 0 ? nm[s_force] : "live");
    return String(b);
}

bool weatherHasData()
{
    if (s_kind < 0)
        load();
    return s_code >= 0 || s_force >= 0;
}
float weatherTemp() { return s_temp; }
const char *weatherCity() { return s_city[0] ? s_city : ""; }

void weatherSetAuto(bool on)
{
    Preferences p;
    p.begin("my-app", false);
    p.putBool("wx_auto", on);
    p.end();
}
bool weatherAuto()
{
    Preferences p;
    p.begin("my-app", true);
    const bool on = p.getBool("wx_auto", true);
    p.end();
    return on;
}

// Fixed city instead of IP location ("" = back to IP). Geocoded at the next boot fetch.
void weatherSetCity(const String &name)
{
    Preferences p;
    p.begin("my-app", false);
    p.putString("wx_ucity", name);
    p.remove("wx_ulat");
    p.remove("wx_ulon");
    p.end();
}
String weatherUserCity()
{
    Preferences p;
    p.begin("my-app", true);
    String c = p.isKey("wx_ucity") ? p.getString("wx_ucity", "") : String();
    p.end();
    return c;
}

// Sun position (NOAA low-precision formulas, ~0.5 deg): altitude above the
// horizon and azimuth from north, clockwise
bool weatherSun(float &altDeg, float &azDeg)
{
    if (!isnan(s_fAlt))
    {
        altDeg = s_fAlt, azDeg = isnan(s_fAz) ? 200 : s_fAz;
        return true;
    }
    if (s_kind < 0)
        load();
    const time_t now = time(nullptr);
    if (now < 1700000000 || isnan(s_lat) || isnan(s_lon))
        return false;
    const double R = M_PI / 180, n = now / 86400.0 + 2440587.5 - 2451545.0;
    const double L = fmod(280.460 + 0.9856474 * n, 360), g = fmod(357.528 + 0.9856003 * n, 360) * R;
    const double lam = (L + 1.915 * sin(g) + 0.020 * sin(2 * g)) * R, eps = (23.439 - 0.0000004 * n) * R;
    const double ra = atan2(cos(eps) * sin(lam), cos(lam)), dec = asin(sin(eps) * sin(lam));
    const double gmst = fmod(280.46061837 + 360.98564736629 * n, 360);
    double H = fmod(gmst + s_lon - ra / R, 360);
    if (H > 180)
        H -= 360;
    if (H < -180)
        H += 360;
    H *= R;
    const double lat = s_lat * R;
    altDeg = asin(sin(lat) * sin(dec) + cos(lat) * cos(dec) * cos(H)) / R;
    azDeg = fmod(atan2(sin(H), cos(H) * sin(lat) - tan(dec) * cos(lat)) / R + 180 + 360, 360);
    return true;
}
// Local time: the clock (set from the weather server) plus the location's
// UTC offset; before an offset is known, one from the longitude (no DST)
bool weatherLocalTime(struct tm &t)
{
    if (s_kind < 0)
        load();
    time_t now = time(nullptr);
    if (now < 1700000000)
        return false;
    if (s_utcOff != INT32_MIN)
        now += s_utcOff;
    else if (!isnan(s_lon))
        now += (time_t)lroundf(s_lon / 15) * 3600;
    gmtime_r(&now, &t);
    return true;
}
void weatherForceSun(float altDeg, float azDeg) { s_fAlt = altDeg, s_fAz = azDeg; }
bool weatherSunForced() { return !isnan(s_fAlt); }
float weatherLat()
{
    if (s_kind < 0)
        load();
    return s_lat;
}
