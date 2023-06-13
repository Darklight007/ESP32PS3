/*******************************************************************************
 * Size: 16 px
 * Bpp: 2
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef SNESESQUE_16B2
#define SNESESQUE_16B2 1
#endif

#if SNESESQUE_16B2

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */

    /* U+0021 "!" */
    0xff, 0xff, 0xff, 0xf,

    /* U+0022 "\"" */
    0xcf, 0x3c, 0xc0,

    /* U+0023 "#" */
    0x33, 0xc, 0xcf, 0xfc, 0xcc, 0x33, 0x3f, 0xf3,
    0x30, 0xcc,

    /* U+0024 "$" */
    0xc, 0xf, 0xcc, 0xcc, 0xf0, 0xf, 0x33, 0x33,
    0xf0, 0x30,

    /* U+0025 "%" */
    0xfc, 0x3, 0xcc, 0xc, 0xfc, 0x30, 0x0, 0xc0,
    0x3, 0x0, 0xc, 0x3f, 0x30, 0x33, 0xc0, 0x3f,

    /* U+0026 "&" */
    0x3f, 0xf, 0x30, 0xf0, 0x3, 0xc0, 0xf3, 0xff,
    0xc, 0x3f, 0x0,

    /* U+0027 "'" */
    0xff, 0x3c,

    /* U+0028 "(" */
    0xc, 0xcc, 0x30, 0xc3, 0x3, 0x3,

    /* U+0029 ")" */
    0xc0, 0xc0, 0xc3, 0xc, 0x33, 0x30,

    /* U+002A "*" */
    0xcc, 0xcc, 0xc0,

    /* U+002B "+" */
    0xc, 0x3, 0xf, 0xfc, 0x30, 0xc, 0x0,

    /* U+002C "," */
    0xff, 0x3c,

    /* U+002D "-" */
    0xff, 0xc0,

    /* U+002E "." */
    0xff,

    /* U+002F "/" */
    0x3, 0x3, 0xc, 0xc, 0x30, 0x30, 0xc0, 0xc0,

    /* U+0030 "0" */
    0x3f, 0xcf, 0xf, 0xf3, 0xff, 0x3f, 0xfc, 0xff,
    0xcf, 0xf0, 0xf3, 0xfc,

    /* U+0031 "1" */
    0xf, 0xf, 0xf0, 0xf, 0x0, 0xf0, 0xf, 0x0,
    0xf0, 0xf, 0xf, 0xff,

    /* U+0032 "2" */
    0x3f, 0xcf, 0xf, 0x0, 0xf0, 0x3c, 0xf, 0x3,
    0xc0, 0xf0, 0xf, 0xff,

    /* U+0033 "3" */
    0x3f, 0xcc, 0xf, 0x0, 0xf0, 0xfc, 0x0, 0xf0,
    0xf, 0xc0, 0xf3, 0xfc,

    /* U+0034 "4" */
    0x3, 0xc0, 0xfc, 0x33, 0xcc, 0x3c, 0xff, 0xf0,
    0x3c, 0x3, 0xc0, 0x3c,

    /* U+0035 "5" */
    0xff, 0xff, 0x0, 0xf0, 0xf, 0xfc, 0x0, 0xf0,
    0xf, 0xc0, 0xf3, 0xfc,

    /* U+0036 "6" */
    0x3f, 0xcf, 0x3, 0xf0, 0xf, 0xfc, 0xf0, 0xff,
    0xf, 0xf0, 0xf3, 0xfc,

    /* U+0037 "7" */
    0xff, 0xf0, 0xf, 0x3, 0xc0, 0x3c, 0xf, 0x0,
    0xf0, 0xf, 0x0, 0xf0,

    /* U+0038 "8" */
    0x3f, 0xcf, 0xf, 0xf0, 0xf3, 0xfc, 0xf0, 0xff,
    0xf, 0xf0, 0xf3, 0xfc,

    /* U+0039 "9" */
    0x3f, 0xcf, 0xf, 0xf0, 0xff, 0xf, 0x3f, 0xf0,
    0xf, 0xc0, 0xf3, 0xfc,

    /* U+003A ":" */
    0xff, 0x0, 0xff,

    /* U+003B ";" */
    0xff, 0x0, 0xff, 0x3c,

    /* U+003C "<" */
    0x0, 0xf0, 0xf0, 0xf0, 0xf, 0x0, 0xf, 0x0,
    0xf,

    /* U+003D "=" */
    0xff, 0x0, 0x0, 0xff,

    /* U+003E ">" */
    0xf0, 0x0, 0xf0, 0x0, 0xf0, 0xf, 0xf, 0xf,
    0x0,

    /* U+003F "?" */
    0x3f, 0xcc, 0xf, 0x3, 0xc0, 0xf0, 0xf, 0x0,
    0x0, 0xf, 0x0, 0xf0,

    /* U+0040 "@" */
    0xf, 0xf0, 0x30, 0xc, 0xc3, 0xf3, 0xcc, 0x33,
    0xcc, 0x33, 0xcf, 0xcf, 0x30, 0x0, 0xf, 0xfc,

    /* U+0041 "A" */
    0x3f, 0xcf, 0xf, 0xf0, 0xff, 0xf, 0xff, 0xff,
    0xf, 0xf0, 0xff, 0xf,

    /* U+0042 "B" */
    0xff, 0xcf, 0xf, 0xf0, 0xff, 0xfc, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xfc,

    /* U+0043 "C" */
    0x3f, 0xcf, 0x3, 0xf0, 0xf, 0x0, 0xf0, 0xf,
    0x0, 0xf0, 0x33, 0xfc,

    /* U+0044 "D" */
    0xff, 0xcf, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xfc,

    /* U+0045 "E" */
    0xff, 0xff, 0x0, 0xf0, 0xf, 0xfc, 0xf0, 0xf,
    0x0, 0xf0, 0xf, 0xff,

    /* U+0046 "F" */
    0xff, 0xff, 0x0, 0xf0, 0xf, 0xfc, 0xf0, 0xf,
    0x0, 0xf0, 0xf, 0x0,

    /* U+0047 "G" */
    0x3f, 0xcf, 0x3, 0xf0, 0xf, 0x0, 0xf3, 0xff,
    0xf, 0xf0, 0xf3, 0xfc,

    /* U+0048 "H" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xff, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xf,

    /* U+0049 "I" */
    0xff, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0xff,

    /* U+004A "J" */
    0x3, 0xc0, 0xf0, 0x3c, 0xf, 0x3, 0xc0, 0xfc,
    0x3c, 0xfc,

    /* U+004B "K" */
    0xf0, 0xff, 0x3c, 0xff, 0xf, 0xc0, 0xfc, 0xf,
    0xf0, 0xf3, 0xcf, 0xf,

    /* U+004C "L" */
    0xf0, 0xf, 0x0, 0xf0, 0xf, 0x0, 0xf0, 0xf,
    0x0, 0xf0, 0xf, 0xff,

    /* U+004D "M" */
    0xf0, 0x3f, 0xf3, 0xff, 0xff, 0xfc, 0xcf, 0xf0,
    0x3f, 0xc0, 0xff, 0x3, 0xfc, 0xf,

    /* U+004E "N" */
    0xf0, 0x3f, 0xc0, 0xff, 0xc3, 0xff, 0xcf, 0xf3,
    0xff, 0xc3, 0xff, 0x3, 0xfc, 0xf,

    /* U+004F "O" */
    0x3f, 0xcf, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xf3, 0xfc,

    /* U+0050 "P" */
    0xff, 0xcf, 0xf, 0xf0, 0xff, 0xf, 0xff, 0xcf,
    0x0, 0xf0, 0xf, 0x0,

    /* U+0051 "Q" */
    0x3f, 0xcf, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xf3, 0xfc, 0x0, 0xf0,

    /* U+0052 "R" */
    0xff, 0xcf, 0xf, 0xf0, 0xff, 0xf, 0xff, 0xcf,
    0xf0, 0xf3, 0xcf, 0xf,

    /* U+0053 "S" */
    0x3f, 0xcf, 0x3, 0xf0, 0xf, 0xfc, 0x3f, 0xf0,
    0xf, 0xc0, 0xf3, 0xfc,

    /* U+0054 "T" */
    0xff, 0xf0, 0xf0, 0xf, 0x0, 0xf0, 0xf, 0x0,
    0xf0, 0xf, 0x0, 0xf0,

    /* U+0055 "U" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xf3, 0xfc,

    /* U+0056 "V" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xcf, 0xf0,

    /* U+0057 "W" */
    0xf0, 0xf0, 0xff, 0xf, 0xf, 0xf0, 0xf0, 0xff,
    0xf, 0xf, 0xf0, 0xf0, 0xff, 0xf, 0xf, 0xf0,
    0xf0, 0xcf, 0xff, 0xf0,

    /* U+0058 "X" */
    0xf0, 0xff, 0xf, 0xf0, 0xf3, 0xfc, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xf,

    /* U+0059 "Y" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0x3f, 0xc0,
    0xf0, 0xf, 0x0, 0xf0,

    /* U+005A "Z" */
    0xff, 0xf0, 0xf, 0x3, 0xc0, 0xf0, 0x3c, 0xf,
    0x0, 0xf0, 0xf, 0xff,

    /* U+005B "[" */
    0xfc, 0xcc, 0xcc, 0xcf,

    /* U+005C "\\" */
    0xc0, 0xc0, 0x30, 0x30, 0xc, 0xc, 0x3, 0x3,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0x3f,

    /* U+005E "^" */
    0xc, 0xc, 0xcc, 0xc,

    /* U+005F "_" */
    0xff,

    /* U+0060 "`" */
    0xc3,

    /* U+0061 "a" */
    0x3f, 0xc0, 0xf, 0x3f, 0xff, 0xf, 0xf0, 0xf3,
    0xf3,

    /* U+0062 "b" */
    0xf0, 0xf, 0x0, 0xff, 0xcf, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xfc,

    /* U+0063 "c" */
    0x3f, 0xcf, 0x3, 0xf0, 0xf, 0x0, 0xf0, 0x33,
    0xfc,

    /* U+0064 "d" */
    0x0, 0xf0, 0xf, 0x3f, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf3, 0xf3, 0xcf,

    /* U+0065 "e" */
    0x3f, 0xcf, 0xf, 0xff, 0xff, 0x0, 0xf0, 0x33,
    0xfc,

    /* U+0066 "f" */
    0xf, 0xcf, 0x3, 0xc3, 0xff, 0x3c, 0xf, 0x3,
    0xc0, 0xf0,

    /* U+0067 "g" */
    0x3f, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xf3,
    0xff, 0x0, 0xfc, 0xf, 0x3f, 0xc0,

    /* U+0068 "h" */
    0xf0, 0xf, 0x0, 0xff, 0xcf, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0xf,

    /* U+0069 "i" */
    0x3c, 0xf, 0xcf, 0x3c, 0xf3, 0xcf,

    /* U+006A "j" */
    0x3c, 0xf, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xff,
    0x0,

    /* U+006B "k" */
    0xf0, 0xf, 0x0, 0xf0, 0xff, 0x3c, 0xff, 0xf,
    0xf0, 0xf3, 0xcf, 0xf,

    /* U+006C "l" */
    0xfc, 0xf3, 0xcf, 0x3c, 0xf3, 0xcf,

    /* U+006D "m" */
    0xff, 0xff, 0xcf, 0xf, 0xf, 0xf0, 0xf0, 0xff,
    0xf, 0xf, 0xf0, 0xf0, 0xff, 0xf, 0xf,

    /* U+006E "n" */
    0xff, 0xcf, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf,

    /* U+006F "o" */
    0x3f, 0xcf, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xf3,
    0xfc,

    /* U+0070 "p" */
    0xff, 0xcf, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xfc, 0xf0, 0xf, 0x0, 0xf0, 0x0,

    /* U+0071 "q" */
    0x3f, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xf3,
    0xff, 0x0, 0xf0, 0xf, 0x0, 0xf0,

    /* U+0072 "r" */
    0xf3, 0xcf, 0xc3, 0xf0, 0xf, 0x0, 0xf0, 0xf,
    0x0,

    /* U+0073 "s" */
    0x3f, 0xcf, 0x3, 0xff, 0x0, 0xff, 0xc0, 0xf3,
    0xfc,

    /* U+0074 "t" */
    0xc, 0x3, 0xc0, 0xff, 0x3, 0xc0, 0x3c, 0x3,
    0xc0, 0x3c, 0x30, 0xfc,

    /* U+0075 "u" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xf3,
    0xf3,

    /* U+0076 "v" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xcf,
    0xf0,

    /* U+0077 "w" */
    0xf0, 0xf0, 0xff, 0xf, 0xf, 0xf0, 0xf0, 0xff,
    0xf, 0xf, 0xf0, 0xf0, 0xcf, 0xf3, 0xf0,

    /* U+0078 "x" */
    0xf0, 0xf3, 0xfc, 0xf, 0x0, 0xf0, 0x3f, 0xcf,
    0xf,

    /* U+0079 "y" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xf3,
    0xff, 0x0, 0xfc, 0xf, 0x3f, 0xc0,

    /* U+007A "z" */
    0xff, 0xf0, 0xf, 0x3, 0xc0, 0xf0, 0x3c, 0xf,
    0xff,

    /* U+007B "{" */
    0xc, 0xc3, 0xc, 0xc0, 0xc3, 0xc, 0xc,

    /* U+007C "|" */
    0xff, 0xff, 0xc0,

    /* U+007D "}" */
    0xc0, 0xc3, 0xc, 0xc, 0xc3, 0xc, 0xc0,

    /* U+007E "~" */
    0x3c, 0x3c, 0x3c
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 80, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 64, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 7, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 17, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 27, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 43, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 54, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 56, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 62, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 64, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 71, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 78, .adv_w = 64, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 80, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 82, .adv_w = 64, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 115, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 127, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 151, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 199, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 64, .box_w = 2, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 214, .adv_w = 64, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 218, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 227, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 231, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 240, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 252, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 268, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 280, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 304, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 316, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 328, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 352, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 364, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 372, .adv_w = 112, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 382, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 394, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 406, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 420, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 434, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 446, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 458, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 472, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 484, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 496, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 508, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 520, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 532, .adv_w = 176, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 552, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 564, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 576, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 588, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 592, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 600, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 604, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 608, .adv_w = 80, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 609, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 610, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 619, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 631, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 640, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 652, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 661, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 671, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 685, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 697, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 703, .adv_w = 64, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 712, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 724, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 730, .adv_w = 176, .box_w = 10, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 745, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 754, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 763, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 777, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 791, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 800, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 809, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 821, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 830, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 839, .adv_w = 176, .box_w = 10, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 854, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 863, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 877, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 886, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 893, .adv_w = 32, .box_w = 1, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 896, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 903, .adv_w = 112, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
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
    .cmap_num = 1,
    .bpp = 2,
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
const lv_font_t SNESesque_16b2 = {
#else
lv_font_t SNESesque_16b2 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 11,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if SNESESQUE_16B2*/

