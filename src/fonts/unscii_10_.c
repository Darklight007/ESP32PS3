/*******************************************************************************
 * Size: 10 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef UNSCII_10_
#define UNSCII_10_ 1
#endif

#if UNSCII_10_

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xff, 0xc0, 0xe0,

    /* U+0022 "\"" */
    0xe7, 0xe7, 0xe7, 0xe7,

    /* U+0023 "#" */
    0x77, 0x3b, 0xbf, 0xee, 0xe7, 0x77, 0xfd, 0xdc,
    0xee, 0x77, 0x0,

    /* U+0024 "$" */
    0x18, 0x18, 0x7f, 0xe0, 0x7c, 0x4, 0x7, 0xfc,
    0x18, 0x18,

    /* U+0025 "%" */
    0xe1, 0xfc, 0x20, 0x1c, 0x7, 0x1, 0x80, 0xe3,
    0xd8, 0x7f, 0xf,

    /* U+0026 "&" */
    0x1c, 0x3b, 0x87, 0x3, 0x87, 0xbf, 0x7b, 0x9c,
    0xc6, 0x7b, 0x80,

    /* U+0027 "'" */
    0x77, 0x6e,

    /* U+0028 "(" */
    0x1b, 0xb9, 0xce, 0x71, 0xc2, 0x18,

    /* U+0029 ")" */
    0xe3, 0x86, 0x31, 0x8d, 0xcc, 0xe0,

    /* U+002A "*" */
    0x73, 0x87, 0xcf, 0xfc, 0x7c, 0x1f, 0x1c, 0xe0,

    /* U+002B "+" */
    0x38, 0x38, 0xff, 0x38, 0x38, 0x38,

    /* U+002C "," */
    0x77, 0x6e,

    /* U+002D "-" */
    0xff,

    /* U+002E "." */
    0xfc,

    /* U+002F "/" */
    0x0, 0xc0, 0xe0, 0x38, 0x10, 0xc, 0x6, 0x7,
    0x1, 0x80, 0xe0, 0x0,

    /* U+0030 "0" */
    0x7c, 0xe7, 0xef, 0xe7, 0xf7, 0xe7, 0xe7, 0x64,
    0x7c,

    /* U+0031 "1" */
    0x38, 0x38, 0x78, 0x38, 0x38, 0x38, 0x38, 0x38,
    0x38, 0xff,

    /* U+0032 "2" */
    0x7c, 0xe7, 0x4, 0xc, 0x18, 0x70, 0xe0, 0xe0,
    0xff,

    /* U+0033 "3" */
    0x7c, 0xe7, 0x7, 0x4, 0x1c, 0x7, 0xe7, 0x64,
    0x7c,

    /* U+0034 "4" */
    0xf, 0x87, 0xe7, 0x3b, 0x8e, 0xe3, 0xbf, 0xf0,
    0x38, 0xe, 0x3, 0x80,

    /* U+0035 "5" */
    0xff, 0xe0, 0xfc, 0x4, 0x7, 0x7, 0xe7, 0x67,
    0x7c,

    /* U+0036 "6" */
    0x1c, 0x70, 0xe0, 0xe0, 0xfc, 0xe7, 0xe7, 0x64,
    0x7c,

    /* U+0037 "7" */
    0xff, 0x3, 0x3, 0x3, 0xe, 0x1c, 0x1c, 0x1c,
    0x1c,

    /* U+0038 "8" */
    0x7c, 0xe7, 0xe7, 0x64, 0x7c, 0xe7, 0xe7, 0x64,
    0x7c,

    /* U+0039 "9" */
    0x7c, 0xe7, 0xe7, 0x67, 0x7f, 0x7, 0x1c, 0x1c,
    0x78,

    /* U+003A ":" */
    0xfc, 0x0, 0x3f,

    /* U+003B ";" */
    0x77, 0x0, 0x7, 0x76, 0xe0,

    /* U+003C "<" */
    0xc, 0x67, 0x18, 0xe1, 0xc1, 0x82, 0xc,

    /* U+003D "=" */
    0xff, 0x0, 0x0, 0xff,

    /* U+003E ">" */
    0xe1, 0xc1, 0x82, 0xc, 0x67, 0x18, 0xe0,

    /* U+003F "?" */
    0x7c, 0xe7, 0x7, 0x1c, 0x1c, 0x18, 0x0, 0x0,
    0x18,

    /* U+0040 "@" */
    0x7e, 0x71, 0xfb, 0xfd, 0xfe, 0xff, 0x7f, 0x80,
    0xc0, 0x7e, 0x0,

    /* U+0041 "A" */
    0x18, 0x7c, 0x64, 0xe7, 0xe7, 0xff, 0xe7, 0xe7,
    0xe7,

    /* U+0042 "B" */
    0xfe, 0xe7, 0xe7, 0xe6, 0xfe, 0xe7, 0xe7, 0xe6,
    0xfe,

    /* U+0043 "C" */
    0x7c, 0xe7, 0xe0, 0xe0, 0xe0, 0xe0, 0xe7, 0x64,
    0x7c,

    /* U+0044 "D" */
    0xf8, 0xfc, 0xe7, 0xe7, 0xe7, 0xe7, 0xfc, 0xf8,
    0xf8,

    /* U+0045 "E" */
    0xff, 0xe0, 0xe0, 0xe0, 0xfe, 0xe0, 0xe0, 0xe0,
    0xff,

    /* U+0046 "F" */
    0xff, 0xe0, 0xe0, 0xe0, 0xfe, 0xe0, 0xe0, 0xe0,
    0xe0,

    /* U+0047 "G" */
    0x7c, 0xe7, 0xe0, 0xe0, 0xef, 0xe7, 0xe7, 0x67,
    0x7f,

    /* U+0048 "H" */
    0xe7, 0xe7, 0xe7, 0xe7, 0xff, 0xe7, 0xe7, 0xe7,
    0xe7,

    /* U+0049 "I" */
    0xff, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38,
    0xff,

    /* U+004A "J" */
    0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0xe7, 0x67,
    0x7c,

    /* U+004B "K" */
    0xe3, 0xf3, 0xbf, 0x1f, 0xf, 0x87, 0xe3, 0x9d,
    0xc6, 0xe3, 0x80,

    /* U+004C "L" */
    0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
    0xe0, 0xff,

    /* U+004D "M" */
    0xe3, 0xfb, 0xfd, 0xfd, 0x7e, 0xbf, 0x1f, 0x8f,
    0xc7, 0xe3, 0x80,

    /* U+004E "N" */
    0xe3, 0xf9, 0xfc, 0xfd, 0x7e, 0xff, 0x3f, 0x8f,
    0xc7, 0xe3, 0x80,

    /* U+004F "O" */
    0x7c, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0x64,
    0x7c,

    /* U+0050 "P" */
    0xfe, 0xe7, 0xe7, 0xe6, 0xfe, 0xe0, 0xe0, 0xe0,
    0xe0,

    /* U+0051 "Q" */
    0x7c, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xec, 0x64,
    0x77,

    /* U+0052 "R" */
    0xfc, 0xe7, 0xe7, 0xe4, 0xfc, 0xfc, 0xfc, 0xe7,
    0xe7,

    /* U+0053 "S" */
    0x7c, 0xe7, 0xe0, 0x60, 0x7c, 0x7, 0xe7, 0x64,
    0x7c,

    /* U+0054 "T" */
    0xff, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38,
    0x38,

    /* U+0055 "U" */
    0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7,
    0x7c,

    /* U+0056 "V" */
    0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0x64, 0x7c,
    0x18,

    /* U+0057 "W" */
    0xe3, 0xf1, 0xf8, 0xfc, 0x7e, 0xbf, 0xff, 0xdf,
    0xc7, 0xe3, 0x80,

    /* U+0058 "X" */
    0xe0, 0xdc, 0xe1, 0xf0, 0x30, 0xc, 0x7, 0xc7,
    0x39, 0x82, 0xe0, 0xc0,

    /* U+0059 "Y" */
    0xe0, 0xdc, 0xe7, 0x38, 0x30, 0xc, 0x3, 0x0,
    0xc0, 0x30, 0xc, 0x0,

    /* U+005A "Z" */
    0xff, 0x7, 0x7, 0xc, 0x18, 0x70, 0xe0, 0xe0,
    0xff,

    /* U+005B "[" */
    0xff, 0x8e, 0x38, 0xe3, 0x8e, 0x38, 0xfc,

    /* U+005C "\\" */
    0xe0, 0x1c, 0x7, 0x0, 0x20, 0xc, 0x1, 0xc0,
    0x38, 0x2, 0x0, 0xc0,

    /* U+005D "]" */
    0xfc, 0x71, 0xc7, 0x1c, 0x71, 0xc7, 0xfc,

    /* U+005E "^" */
    0x8, 0x3, 0xc1, 0xdc, 0x30, 0x8e, 0x1e,

    /* U+005F "_" */
    0xff, 0xc0,

    /* U+0060 "`" */
    0xe3, 0x9c, 0x30,

    /* U+0061 "a" */
    0x7c, 0x7, 0x7, 0x7f, 0xe7, 0x67, 0x7f,

    /* U+0062 "b" */
    0xe0, 0xe0, 0xe0, 0xfe, 0xe6, 0xe7, 0xe7, 0xe7,
    0xe7, 0xfe,

    /* U+0063 "c" */
    0x7d, 0x8e, 0x38, 0xe3, 0x87, 0xc0,

    /* U+0064 "d" */
    0x7, 0x7, 0x7, 0x7f, 0x67, 0xe7, 0xe7, 0xe7,
    0xe7, 0x7f,

    /* U+0065 "e" */
    0x7c, 0x64, 0xe7, 0xff, 0xe0, 0xe0, 0x7c,

    /* U+0066 "f" */
    0x3d, 0xcf, 0xdc, 0x71, 0xc7, 0x1c, 0x70,

    /* U+0067 "g" */
    0x7f, 0x67, 0xe7, 0xe7, 0x7f, 0x7, 0x7, 0xfc,

    /* U+0068 "h" */
    0xe0, 0xe0, 0xfe, 0xe6, 0xe7, 0xe7, 0xe7, 0xe7,
    0xe7,

    /* U+0069 "i" */
    0x70, 0x3, 0xc3, 0x87, 0xe, 0x1c, 0x38, 0x7e,

    /* U+006A "j" */
    0x1c, 0x0, 0x7, 0x1c, 0x71, 0xc7, 0x1c, 0x6f,
    0x80,

    /* U+006B "k" */
    0xe0, 0xe0, 0xe7, 0xe4, 0xfc, 0xf8, 0xfc, 0xfc,
    0xe7,

    /* U+006C "l" */
    0xf0, 0xe1, 0xc3, 0x87, 0xe, 0x1c, 0x38, 0x7e,

    /* U+006D "m" */
    0xe7, 0x7f, 0xfa, 0xfd, 0x7e, 0xbf, 0x1c,

    /* U+006E "n" */
    0xfe, 0xe6, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7,

    /* U+006F "o" */
    0x7c, 0x64, 0xe7, 0xe7, 0xe7, 0xe7, 0x7c,

    /* U+0070 "p" */
    0xfe, 0xe7, 0xe7, 0xe6, 0xfe, 0xe0, 0xe0, 0xe0,

    /* U+0071 "q" */
    0x7f, 0xe7, 0xe7, 0x67, 0x7f, 0x7, 0x7, 0x7,

    /* U+0072 "r" */
    0xfe, 0xe6, 0xe7, 0xe0, 0xe0, 0xe0, 0xe0,

    /* U+0073 "s" */
    0x7f, 0x60, 0xe0, 0x7c, 0x7, 0x4, 0xfc,

    /* U+0074 "t" */
    0x70, 0x70, 0xff, 0x70, 0x70, 0x70, 0x70, 0x70,
    0x1f,

    /* U+0075 "u" */
    0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0x67, 0x7f,

    /* U+0076 "v" */
    0xe7, 0xe7, 0xe7, 0xe7, 0x64, 0x7c, 0x18,

    /* U+0077 "w" */
    0xe3, 0xf1, 0xf8, 0xfd, 0x76, 0xb3, 0xb9, 0xdc,

    /* U+0078 "x" */
    0xe3, 0xb1, 0x9d, 0xc3, 0x87, 0x73, 0xbb, 0x8e,

    /* U+0079 "y" */
    0xe7, 0xe7, 0xe7, 0xe7, 0x7f, 0x7, 0x4, 0x7c,

    /* U+007A "z" */
    0xff, 0xc, 0xc, 0x38, 0x70, 0x70, 0xff,

    /* U+007B "{" */
    0xf, 0x38, 0x38, 0x38, 0xf0, 0x38, 0x38, 0x8,
    0xf,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xe0,

    /* U+007D "}" */
    0xf0, 0x38, 0x38, 0x38, 0xf, 0x38, 0x38, 0x30,
    0xf0,

    /* U+007E "~" */
    0x7b, 0xb5, 0xbb, 0xc0,

    /* U+007F "" */
    0xe0, 0x50, 0x24, 0x12, 0xf9, 0x24, 0x93, 0x88,
    0x4, 0x2, 0x1, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 160, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 10},
    {.bitmap_index = 1, .adv_w = 160, .box_w = 3, .box_h = 9, .ofs_x = 4, .ofs_y = 2},
    {.bitmap_index = 5, .adv_w = 160, .box_w = 8, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 9, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 20, .adv_w = 160, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 30, .adv_w = 160, .box_w = 11, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 41, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 52, .adv_w = 160, .box_w = 4, .box_h = 4, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 54, .adv_w = 160, .box_w = 5, .box_h = 9, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 60, .adv_w = 160, .box_w = 5, .box_h = 9, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 66, .adv_w = 160, .box_w = 10, .box_h = 6, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 74, .adv_w = 160, .box_w = 8, .box_h = 6, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 80, .adv_w = 160, .box_w = 4, .box_h = 4, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 82, .adv_w = 160, .box_w = 8, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 83, .adv_w = 160, .box_w = 3, .box_h = 2, .ofs_x = 4, .ofs_y = 1},
    {.bitmap_index = 84, .adv_w = 160, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 96, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 105, .adv_w = 160, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 115, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 124, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 133, .adv_w = 160, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 145, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 154, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 163, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 172, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 181, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 190, .adv_w = 160, .box_w = 3, .box_h = 8, .ofs_x = 4, .ofs_y = 1},
    {.bitmap_index = 193, .adv_w = 160, .box_w = 4, .box_h = 9, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 160, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 205, .adv_w = 160, .box_w = 8, .box_h = 4, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 209, .adv_w = 160, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 216, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 225, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 236, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 245, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 254, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 263, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 272, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 281, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 290, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 299, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 308, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 317, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 326, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 337, .adv_w = 160, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 347, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 358, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 369, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 378, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 387, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 396, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 405, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 414, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 423, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 432, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 441, .adv_w = 160, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 452, .adv_w = 160, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 464, .adv_w = 160, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 476, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 485, .adv_w = 160, .box_w = 6, .box_h = 9, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 492, .adv_w = 160, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 504, .adv_w = 160, .box_w = 6, .box_h = 9, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 511, .adv_w = 160, .box_w = 11, .box_h = 5, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 518, .adv_w = 160, .box_w = 10, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 520, .adv_w = 160, .box_w = 5, .box_h = 4, .ofs_x = 4, .ofs_y = 7},
    {.bitmap_index = 523, .adv_w = 160, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 530, .adv_w = 160, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 540, .adv_w = 160, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 546, .adv_w = 160, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 556, .adv_w = 160, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 563, .adv_w = 160, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 570, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 578, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 587, .adv_w = 160, .box_w = 7, .box_h = 9, .ofs_x = 3, .ofs_y = 2},
    {.bitmap_index = 595, .adv_w = 160, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 604, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 613, .adv_w = 160, .box_w = 7, .box_h = 9, .ofs_x = 3, .ofs_y = 2},
    {.bitmap_index = 621, .adv_w = 160, .box_w = 9, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 628, .adv_w = 160, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 635, .adv_w = 160, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 642, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 650, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 658, .adv_w = 160, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 665, .adv_w = 160, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 672, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 681, .adv_w = 160, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 688, .adv_w = 160, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 695, .adv_w = 160, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 703, .adv_w = 160, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 711, .adv_w = 160, .box_w = 8, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 719, .adv_w = 160, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 726, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 735, .adv_w = 160, .box_w = 3, .box_h = 9, .ofs_x = 4, .ofs_y = 1},
    {.bitmap_index = 739, .adv_w = 160, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 748, .adv_w = 160, .box_w = 9, .box_h = 3, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 752, .adv_w = 160, .box_w = 9, .box_h = 10, .ofs_x = 0, .ofs_y = 1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 96, .glyph_id_start = 1,
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
const lv_font_t unscii_10_ = {
#else
lv_font_t unscii_10_ = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 12,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 0,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if UNSCII_10_*/

