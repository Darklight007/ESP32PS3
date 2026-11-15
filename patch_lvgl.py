"""Pre-build patch for the vendored LVGL submodule (same idea as patch_tft_espi.py).

WHY THIS EXISTS
---------------
lib/lvgl is a git submodule: a local edit inside it is not in this repo's
history and is wiped by `git submodule update` or a fresh clone. This script
keeps the patch in this repo and re-applies it on every build.

WHAT IT PATCHES
---------------
LVGL 8.4's lv_obj_get_transformed_area() always ends with

    lv_area_increase(area, 5, 5);

even when neither the object nor its parents are transformed (rotated or
zoomed). lv_obj_invalidate_area() calls it for every invalidation, so every
dirty area grew by 5 px on each side: a 46x54 digit strip became 56x64 (+46%
pixels) and a 5 px bar strip became 15 px tall (3x). Nothing here uses
transforms, so the padding only cost render time.

The patch returns early, area untouched, when no transform applies (checked
the same way lv_obj_transform_point() does). Transformed objects still get
the padding.

Idempotent: detects its own marker and does nothing if already applied.
"""

import os

TARGET = os.path.join("lib", "lvgl", "src", "core", "lv_obj_pos.c")

ANCHOR = """void lv_obj_get_transformed_area(const lv_obj_t * obj, lv_area_t * area, bool recursive,
                                 bool inv)
{
"""
MARKER = "PATCHED by patch_lvgl.py"

INSERT = """    /* PATCHED by patch_lvgl.py (pre-build) - do not edit here, change the script.
     * Nothing transformed on obj (or, if recursive, its parents): the area is
     * unchanged. Upstream fell through and padded it by 5 px regardless. */
    {
        const lv_obj_t * t = obj;
        while(t && _lv_obj_get_layer_type(t) != LV_LAYER_TYPE_TRANSFORM)
            t = recursive ? lv_obj_get_parent(t) : NULL;
        if(t == NULL) return;
    }

"""


def main():
    if not os.path.isfile(TARGET):
        print("patch_lvgl: WARNING %s not found - submodule not checked out?" % TARGET)
        return

    with open(TARGET, "r", encoding="utf-8", errors="surrogateescape") as fh:
        text = fh.read()

    if MARKER in text:
        print("patch_lvgl: already patched, nothing to do")
        return

    if ANCHOR not in text:
        print("patch_lvgl: WARNING lv_obj_get_transformed_area() not found - NOT patched")
        print("patch_lvgl: WARNING every invalidated area stays 5 px larger per side")
        return

    text = text.replace(ANCHOR, ANCHOR + INSERT, 1)
    with open(TARGET, "w", encoding="utf-8", errors="surrogateescape") as fh:
        fh.write(text)

    print("patch_lvgl: patched %s (no 5 px padding without a transform)" % TARGET)


# ---------------------------------------------------------------------------
# Core guard: LVGL may only run on Core 1 (main loop). lv_core_guard()
# (src/lv_core_guard.cpp) records the backtrace of the first call from any
# other core in RTC memory (survives the crash it causes); DIAG:LVCORE? reads
# it. Added after a heap-corruption crash in layout_update_core().
GUARD_MARK = "lv_core_guard(); /* PATCHED core guard */"
GUARDS = [
    (os.path.join("lib", "lvgl", "src", "core", "lv_obj_pos.c"),
     ["void lv_obj_invalidate_area(const lv_obj_t * obj, const lv_area_t * area)\n{\n",
      "void lv_obj_invalidate(const lv_obj_t * obj)\n{\n"]),
    (os.path.join("lib", "lvgl", "src", "core", "lv_obj_tree.c"),
     ["void lv_obj_del(lv_obj_t * obj)\n{\n", "void lv_obj_clean(lv_obj_t * obj)\n{\n"]),
    (os.path.join("lib", "lvgl", "src", "core", "lv_obj_class.c"),
     ["lv_obj_t * lv_obj_class_create_obj(const lv_obj_class_t * class_p, lv_obj_t * parent)\n{\n"]),
]


def guard():
    for path, anchors in GUARDS:
        if not os.path.isfile(path):
            print("patch_lvgl: WARNING %s missing" % path)
            continue
        with open(path, "r", encoding="utf-8", errors="surrogateescape") as fh:
            text = fh.read()
        if GUARD_MARK in text:
            continue
        n = 0
        for a in anchors:
            if a in text:
                text = text.replace(a, a + "    " + GUARD_MARK + "\n", 1)
                n += 1
            else:
                print("patch_lvgl: WARNING guard anchor not found in %s" % path)
        text = "void lv_core_guard(void);\n" + text
        with open(path, "w", encoding="utf-8", errors="surrogateescape") as fh:
            fh.write(text)
        print("patch_lvgl: core guard in %s (%d)" % (path, n))


main()
guard()
