# Post-build-config script: keep the hot display code in internal RAM.
#
# The ESP32-S3 runs flash code through a 16 KB instruction cache (fixed in the
# prebuilt Arduino SDK). One LVGL redraw walks far more code than that, so
# every small repaint stalled on flash fetches - measured ~1.6 ms to redraw a
# single pixel. Code in internal RAM needs no cache. This copies the SDK's
# sections.ld, adds the object files below to .iram0.text, and links with it.
# Costs their code size in internal RAM (LVGL's heap lives in PSRAM instead,
# see src/lv_mem_track.cpp). Pick files with DISP:SAMPRUN / DISP:SAMP?.
Import("env")
import os

HOT = {
    "liblvgl.a": [
        "lv_draw_sw_letter", "lv_obj_style", "lv_obj", "lv_refr", "lv_obj_pos", "lv_event",
        "lv_txt", "lv_style", "lv_font_fmt_txt", "lv_label", "lv_obj_tree", "lv_area", "lv_mem",
        "lv_draw_sw_rect", "lv_obj_scroll", "lv_anim", "lv_font", "lv_obj_draw", "lv_hal_disp",
        "lv_timer", "lv_draw_label", "lv_draw_rect", "lv_bar", "lv_ll", "lv_obj_class", "lv_disp",
        "lv_utils", "lv_draw_sw_blend", "lv_hal_tick", "lv_obj_style_gen", "lv_draw_mask",
        # Graph page: plot lines (draw_series_line was ~19% of a graph frame)
        "lv_chart", "lv_draw_sw_line", "lv_draw_line",
    ],
}

def patch():
    src = None
    for d in env.get("LIBPATH", []):
        p = os.path.join(env.subst(str(d)), "sections.ld")
        if os.path.isfile(p):
            src = p
    if not src:
        print("ld_iram_hot: sections.ld not found, linking unchanged")
        return
    text = open(src).read()
    anchor = "    *(.iram1 .iram1.*)\n"
    if text.count(anchor) != 1:
        print("ld_iram_hot: unexpected sections.ld layout, linking unchanged")
        return
    lines = "".join("    *%s:%s.*(.literal .literal.* .text .text.*)\n" % (lib, f)
                    for lib, files in HOT.items() for f in files)
    out = os.path.join(env.subst("$BUILD_DIR"), "sections_hot.ld")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    new = text.replace(anchor, anchor + lines)
    if not os.path.isfile(out) or open(out).read() != new:
        open(out, "w").write(new)
    flags = [out if str(f) == "sections.ld" else f for f in env["LINKFLAGS"]]
    env.Replace(LINKFLAGS=flags)
    # SCons doesn't track linker scripts: relink when this one changes
    env.Depends("$BUILD_DIR/${PROGNAME}.elf", out)
    print("ld_iram_hot: %d object files placed in internal RAM" % sum(len(v) for v in HOT.values()))

patch()
