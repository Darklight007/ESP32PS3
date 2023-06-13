/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef DSEG_R_12
#define DSEG_R_12 1
#endif

#if DSEG_R_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0x0,

    /* U+002D "-" */
    0xfc,

    /* U+002E "." */
    0xc0,

    /* U+0030 "0" */
    0x7d, 0x6, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+0031 "1" */
    0xfb, 0xe0,

    /* U+0032 "2" */
    0x7c, 0x4, 0x8, 0x10, 0x3f, 0xe0, 0x40, 0x81,
    0x2, 0x3, 0xe0,

    /* U+0033 "3" */
    0xfc, 0x4, 0x8, 0x10, 0x3f, 0xc0, 0x81, 0x2,
    0x4, 0xf, 0xe0,

    /* U+0034 "4" */
    0x83, 0x6, 0xc, 0x18, 0x3f, 0xc0, 0x81, 0x2,
    0x4, 0x8,

    /* U+0035 "5" */
    0x7d, 0x2, 0x4, 0x8, 0x1f, 0xc0, 0x81, 0x2,
    0x4, 0xb, 0xe0,

    /* U+0036 "6" */
    0x7d, 0x2, 0x4, 0x8, 0x1f, 0xe0, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+0037 "7" */
    0x7d, 0x6, 0xc, 0x18, 0x30, 0x40, 0x1, 0x2,
    0x4, 0x8, 0x10,

    /* U+0038 "8" */
    0x7d, 0x6, 0xc, 0x18, 0x3f, 0xe0, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+0039 "9" */
    0x7d, 0x6, 0xc, 0x18, 0x3f, 0xc0, 0x81, 0x2,
    0x4, 0xb, 0xe0,

    /* U+003A ":" */
    0xc0, 0x30,

    /* U+0041 "A" */
    0x7d, 0x6, 0xc, 0x18, 0x30, 0x7f, 0xc1, 0x83,
    0x6, 0xc, 0x10,

    /* U+0042 "B" */
    0x81, 0x2, 0x4, 0x8, 0x1f, 0xe0, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+0043 "C" */
    0xff, 0x2, 0x4, 0x8, 0x10, 0x1f, 0x80,

    /* U+0044 "D" */
    0x2, 0x4, 0x8, 0x10, 0x3f, 0xe0, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+0045 "E" */
    0x7f, 0x2, 0x4, 0x8, 0x1f, 0xe0, 0x40, 0x81,
    0x2, 0x3, 0xf0,

    /* U+0046 "F" */
    0x7f, 0x2, 0x4, 0x8, 0x10, 0x3f, 0xc0, 0x81,
    0x2, 0x4, 0x0,

    /* U+0047 "G" */
    0x7d, 0x2, 0x4, 0x8, 0x10, 0x20, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+0048 "H" */
    0x81, 0x2, 0x4, 0x8, 0x1f, 0xe0, 0xc1, 0x83,
    0x6, 0x8,

    /* U+0049 "I" */
    0xf8,

    /* U+004A "J" */
    0x2, 0x4, 0x8, 0x10, 0x20, 0x20, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+004B "K" */
    0x7d, 0x2, 0x4, 0x8, 0x10, 0x3f, 0xc1, 0x83,
    0x6, 0xc, 0x10,

    /* U+004C "L" */
    0x81, 0x2, 0x4, 0x8, 0x0, 0x20, 0x40, 0x81,
    0x2, 0x3, 0xf0,

    /* U+004D "M" */
    0x7d, 0x6, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x83,
    0x6, 0x8, 0x0,

    /* U+004E "N" */
    0xff, 0x6, 0xc, 0x18, 0x30, 0x40,

    /* U+004F "O" */
    0xff, 0x6, 0xc, 0x18, 0x30, 0x5f, 0x0,

    /* U+0050 "P" */
    0x7d, 0x6, 0xc, 0x18, 0x30, 0x7f, 0xc0, 0x81,
    0x2, 0x4, 0x0,

    /* U+0051 "Q" */
    0x7d, 0x6, 0xc, 0x18, 0x30, 0x7f, 0x81, 0x2,
    0x4, 0x8, 0x10,

    /* U+0052 "R" */
    0xff, 0x2, 0x4, 0x8, 0x10, 0x0,

    /* U+0053 "S" */
    0x81, 0x2, 0x4, 0x8, 0x1f, 0xc0, 0x81, 0x2,
    0x4, 0xb, 0xe0,

    /* U+0054 "T" */
    0x81, 0x2, 0x4, 0x8, 0x1f, 0xe0, 0x40, 0x81,
    0x2, 0x3, 0xf0,

    /* U+0055 "U" */
    0x83, 0x6, 0xc, 0x18, 0x2f, 0x80,

    /* U+0056 "V" */
    0x83, 0x6, 0xc, 0x18, 0x20, 0x20, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+0057 "W" */
    0x83, 0x6, 0xc, 0x18, 0x3f, 0xe0, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+0058 "X" */
    0x83, 0x6, 0xc, 0x18, 0x3f, 0xe0, 0xc1, 0x83,
    0x6, 0x8,

    /* U+0059 "Y" */
    0x83, 0x6, 0xc, 0x18, 0x3f, 0xc0, 0x81, 0x2,
    0x4, 0xb, 0xe0,

    /* U+005A "Z" */
    0x7c, 0x4, 0x8, 0x10, 0x20, 0x60, 0x40, 0x81,
    0x2, 0x3, 0xe0,

    /* U+005F "_" */
    0xfc,

    /* U+0061 "a" */
    0x7d, 0x6, 0xc, 0x18, 0x30, 0x7f, 0xc1, 0x83,
    0x6, 0xc, 0x10,

    /* U+0062 "b" */
    0x81, 0x2, 0x4, 0x8, 0x1f, 0xe0, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+0063 "c" */
    0xff, 0x2, 0x4, 0x8, 0x10, 0x1f, 0x80,

    /* U+0064 "d" */
    0x2, 0x4, 0x8, 0x10, 0x3f, 0xe0, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+0065 "e" */
    0x7f, 0x2, 0x4, 0x8, 0x1f, 0xe0, 0x40, 0x81,
    0x2, 0x3, 0xf0,

    /* U+0066 "f" */
    0x7f, 0x2, 0x4, 0x8, 0x10, 0x3f, 0xc0, 0x81,
    0x2, 0x4, 0x0,

    /* U+0067 "g" */
    0x7d, 0x2, 0x4, 0x8, 0x10, 0x20, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+0068 "h" */
    0x81, 0x2, 0x4, 0x8, 0x1f, 0xe0, 0xc1, 0x83,
    0x6, 0x8,

    /* U+0069 "i" */
    0xf8,

    /* U+006A "j" */
    0x2, 0x4, 0x8, 0x10, 0x20, 0x20, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+006B "k" */
    0x7d, 0x2, 0x4, 0x8, 0x10, 0x3f, 0xc1, 0x83,
    0x6, 0xc, 0x10,

    /* U+006C "l" */
    0x81, 0x2, 0x4, 0x8, 0x0, 0x20, 0x40, 0x81,
    0x2, 0x3, 0xf0,

    /* U+006D "m" */
    0x7d, 0x6, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x83,
    0x6, 0x8, 0x0,

    /* U+006E "n" */
    0xff, 0x6, 0xc, 0x18, 0x30, 0x40,

    /* U+006F "o" */
    0xff, 0x6, 0xc, 0x18, 0x30, 0x5f, 0x0,

    /* U+0070 "p" */
    0x7d, 0x6, 0xc, 0x18, 0x30, 0x7f, 0xc0, 0x81,
    0x2, 0x4, 0x0,

    /* U+0071 "q" */
    0x7d, 0x6, 0xc, 0x18, 0x30, 0x7f, 0x81, 0x2,
    0x4, 0x8, 0x10,

    /* U+0072 "r" */
    0xff, 0x2, 0x4, 0x8, 0x10, 0x0,

    /* U+0073 "s" */
    0x81, 0x2, 0x4, 0x8, 0x1f, 0xc0, 0x81, 0x2,
    0x4, 0xb, 0xe0,

    /* U+0074 "t" */
    0x81, 0x2, 0x4, 0x8, 0x1f, 0xe0, 0x40, 0x81,
    0x2, 0x3, 0xf0,

    /* U+0075 "u" */
    0x83, 0x6, 0xc, 0x18, 0x2f, 0x80,

    /* U+0076 "v" */
    0x83, 0x6, 0xc, 0x18, 0x20, 0x20, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+0077 "w" */
    0x83, 0x6, 0xc, 0x18, 0x3f, 0xe0, 0xc1, 0x83,
    0x6, 0xb, 0xe0,

    /* U+0078 "x" */
    0x83, 0x6, 0xc, 0x18, 0x3f, 0xe0, 0xc1, 0x83,
    0x6, 0x8,

    /* U+0079 "y" */
    0x83, 0x6, 0xc, 0x18, 0x3f, 0xc0, 0x81, 0x2,
    0x4, 0xb, 0xe0,

    /* U+007A "z" */
    0x7c, 0x4, 0x8, 0x10, 0x20, 0x60, 0x40, 0x81,
    0x2, 0x3, 0xe0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 38, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 157, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 157, .box_w = 6, .box_h = 1, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 3, .adv_w = 0, .box_w = 2, .box_h = 1, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 15, .adv_w = 157, .box_w = 1, .box_h = 12, .ofs_x = 7, .ofs_y = 0},
    {.bitmap_index = 17, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 28, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 39, .adv_w = 157, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 49, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 60, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 82, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 104, .adv_w = 38, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 106, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 117, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 157, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 135, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 168, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 179, .adv_w = 157, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 189, .adv_w = 157, .box_w = 1, .box_h = 6, .ofs_x = 7, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 212, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 223, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 157, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 240, .adv_w = 157, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 258, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 269, .adv_w = 157, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 275, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 286, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 297, .adv_w = 157, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 303, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 314, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 325, .adv_w = 157, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 335, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 346, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 357, .adv_w = 157, .box_w = 6, .box_h = 1, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 358, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 369, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 380, .adv_w = 157, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 387, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 398, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 409, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 420, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 431, .adv_w = 157, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 441, .adv_w = 157, .box_w = 1, .box_h = 6, .ofs_x = 7, .ofs_y = 0},
    {.bitmap_index = 442, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 453, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 464, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 475, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 486, .adv_w = 157, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 492, .adv_w = 157, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 499, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 510, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 521, .adv_w = 157, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 527, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 538, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 549, .adv_w = 157, .box_w = 7, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 555, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 566, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 577, .adv_w = 157, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 587, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 598, .adv_w = 157, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x1, 0xd, 0xe
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 15, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 4, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    },
    {
        .range_start = 48, .range_length = 11, .glyph_id_start = 5,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 65, .range_length = 26, .glyph_id_start = 16,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 95, .range_length = 1, .glyph_id_start = 42,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 97, .range_length = 26, .glyph_id_start = 43,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 5,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t dseg_r_12 = {
#else
lv_font_t dseg_r_12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 13,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if DSEG_R_12*/

