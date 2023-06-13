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

#ifndef WINDYCITY_12
#define WINDYCITY_12 1
#endif

#if WINDYCITY_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xf3, 0xc0,

    /* U+0022 "\"" */
    0xb6, 0x80,

    /* U+0023 "#" */
    0x44, 0x8b, 0xfa, 0x24, 0x5f, 0xd1, 0x22,

    /* U+0024 "$" */
    0x23, 0xaf, 0x47, 0x1c, 0xb7, 0x71, 0x0,

    /* U+0025 "%" */
    0x62, 0x49, 0x25, 0xd, 0x80, 0x80, 0x58, 0x52,
    0x29, 0x23, 0x0,

    /* U+0026 "&" */
    0x7c, 0xcc, 0xc1, 0x61, 0x61, 0xce, 0xcc, 0xcc,
    0x78,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x7b, 0x6d, 0xb6, 0xcc,

    /* U+0029 ")" */
    0xcd, 0xb6, 0xdb, 0x78,

    /* U+002A "*" */
    0x25, 0x4c, 0xea, 0x90,

    /* U+002B "+" */
    0x10, 0x23, 0xf8, 0x81, 0x0,

    /* U+002C "," */
    0xfd, 0xe0,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x18, 0x8c, 0x42, 0x31, 0x8, 0x80,

    /* U+0030 "0" */
    0xff, 0x3d, 0xf7, 0xff, 0xbc, 0xf3, 0x78,

    /* U+0031 "1" */
    0x7f, 0xff, 0xff, 0xe0,

    /* U+0032 "2" */
    0xfc, 0x30, 0xc2, 0x39, 0xcc, 0x30, 0xfc,

    /* U+0033 "3" */
    0xfc, 0x63, 0x3f, 0xc, 0x30, 0xe3, 0xf8,

    /* U+0034 "4" */
    0xc, 0x79, 0xf6, 0x6c, 0xdf, 0xc3, 0x6, 0xc,

    /* U+0035 "5" */
    0xff, 0xc, 0x3f, 0xc, 0x30, 0xe3, 0xf8,

    /* U+0036 "6" */
    0x39, 0x8c, 0x3f, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+0037 "7" */
    0xfe, 0xc, 0x18, 0x30, 0x63, 0x6, 0xc, 0x18,

    /* U+0038 "8" */
    0xff, 0x3c, 0xd4, 0x73, 0x3c, 0xf3, 0xf8,

    /* U+0039 "9" */
    0x7b, 0x3c, 0xf3, 0xcd, 0xf0, 0xc6, 0x70,

    /* U+003A ":" */
    0xf0, 0x3c,

    /* U+003B ";" */
    0xf0, 0x3d, 0x80,

    /* U+003C "<" */
    0x1b, 0x99, 0x84, 0x38, 0xe0,

    /* U+003D "=" */
    0xfc, 0x0, 0x3f,

    /* U+003E ">" */
    0x86, 0x63, 0x26, 0xc0,

    /* U+003F "?" */
    0x7a, 0x30, 0xce, 0x38, 0x80, 0x8, 0x20,

    /* U+0040 "@" */
    0x3e, 0x20, 0xa7, 0x34, 0x9a, 0x4d, 0x26, 0x7e,
    0x80, 0x3e, 0x0,

    /* U+0041 "A" */
    0x7b, 0x3c, 0xf3, 0xff, 0x3c, 0xf3, 0xcc,

    /* U+0042 "B" */
    0xff, 0x3c, 0xf2, 0xfb, 0x3c, 0xf3, 0xf8,

    /* U+0043 "C" */
    0x7b, 0x3c, 0x30, 0xc3, 0xc, 0x33, 0x78,

    /* U+0044 "D" */
    0xff, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0xf8,

    /* U+0045 "E" */
    0xff, 0xc, 0x30, 0xf3, 0xc, 0x30, 0xfc,

    /* U+0046 "F" */
    0xff, 0xc, 0x30, 0xf3, 0xc, 0x30, 0xc0,

    /* U+0047 "G" */
    0xff, 0xc, 0x33, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+0048 "H" */
    0xcf, 0x3c, 0xf3, 0xff, 0x3c, 0xf3, 0xcc,

    /* U+0049 "I" */
    0xff, 0xff, 0xc0,

    /* U+004A "J" */
    0xc, 0x30, 0xc3, 0xf, 0x3c, 0xf3, 0x78,

    /* U+004B "K" */
    0xc7, 0x2d, 0x38, 0xe3, 0x8d, 0x32, 0xc4,

    /* U+004C "L" */
    0xc6, 0x31, 0x8c, 0x63, 0x18, 0xf8,

    /* U+004D "M" */
    0xe1, 0xf8, 0x7e, 0x1f, 0xcf, 0x9e, 0x67, 0x98,
    0xc6, 0x1, 0x80, 0x40,

    /* U+004E "N" */
    0xc3, 0x87, 0x8d, 0x9b, 0xb3, 0xe3, 0xc3, 0x82,

    /* U+004F "O" */
    0xff, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+0050 "P" */
    0xff, 0x3c, 0xf2, 0xfb, 0xc, 0x30, 0xc0,

    /* U+0051 "Q" */
    0xff, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0x78, 0x30,

    /* U+0052 "R" */
    0xff, 0x3c, 0xf2, 0xfb, 0x3c, 0xf3, 0xcc,

    /* U+0053 "S" */
    0xfe, 0x30, 0xe7, 0x9c, 0x73, 0x70,

    /* U+0054 "T" */
    0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18,

    /* U+0055 "U" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+0056 "V" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xf2, 0xf0,

    /* U+0057 "W" */
    0xcc, 0x73, 0x1c, 0xc7, 0x31, 0xcc, 0x73, 0x1c,
    0xc7, 0x32, 0xff, 0x80,

    /* U+0058 "X" */
    0xcf, 0x3c, 0xf3, 0x73, 0x3c, 0xf3, 0xcc,

    /* U+0059 "Y" */
    0xcf, 0x3c, 0xf3, 0xf8, 0x41, 0x4, 0x10,

    /* U+005A "Z" */
    0xfc, 0x71, 0x8e, 0x71, 0x86, 0x30, 0xfc,

    /* U+005B "[" */
    0xfc, 0xcc, 0xcc, 0xcc, 0xcf,

    /* U+005C "\\" */
    0x92, 0x24, 0x89, 0x20,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0x33, 0x3f,

    /* U+005E "^" */
    0x10, 0xfb, 0xb8,

    /* U+005F "_" */
    0xff,

    /* U+0060 "`" */
    0x80,

    /* U+0061 "a" */
    0x7a, 0x30, 0xdf, 0xcf, 0x37, 0xc0,

    /* U+0062 "b" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xf3, 0xf8,

    /* U+0063 "c" */
    0x76, 0x71, 0x8c, 0x65, 0xc0,

    /* U+0064 "d" */
    0xc, 0x37, 0xf3, 0xcf, 0x3c, 0xf3, 0x7c,

    /* U+0065 "e" */
    0x7f, 0x3c, 0xff, 0xc3, 0x17, 0xc0,

    /* U+0066 "f" */
    0x3b, 0x3c, 0xc6, 0x31, 0x8c, 0x60,

    /* U+0067 "g" */
    0x7f, 0x3c, 0xf3, 0xcf, 0x37, 0xc3, 0x8d, 0xe0,

    /* U+0068 "h" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xf3, 0xcc,

    /* U+0069 "i" */
    0xcf, 0xff, 0xc0,

    /* U+006A "j" */
    0x18, 0x6, 0x31, 0x8c, 0x63, 0x1e, 0xdc,

    /* U+006B "k" */
    0xc3, 0xc, 0xf6, 0xf3, 0xf, 0x36, 0xcc,

    /* U+006C "l" */
    0xff, 0xff, 0xc0,

    /* U+006D "m" */
    0xfb, 0xb3, 0x3c, 0xcf, 0x33, 0xcc, 0xf3, 0x3c,
    0xcc,

    /* U+006E "n" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3c, 0xc0,

    /* U+006F "o" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x37, 0x80,

    /* U+0070 "p" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3f, 0xb0, 0xc0,

    /* U+0071 "q" */
    0x7f, 0x3c, 0xf3, 0xcf, 0x37, 0xc3, 0xc,

    /* U+0072 "r" */
    0xdf, 0x8c, 0x30, 0xc3, 0xc, 0x0,

    /* U+0073 "s" */
    0x7b, 0x1f, 0x1e, 0x3e, 0x77, 0x0,

    /* U+0074 "t" */
    0x66, 0xf6, 0x66, 0x66, 0x30,

    /* U+0075 "u" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x37, 0xc0,

    /* U+0076 "v" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x2f, 0x0,

    /* U+0077 "w" */
    0xcc, 0x73, 0x1c, 0xc7, 0x31, 0xcc, 0x73, 0x2f,
    0xf8,

    /* U+0078 "x" */
    0xcf, 0x3c, 0xcc, 0xcf, 0x3c, 0xc0,

    /* U+0079 "y" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x37, 0xc3, 0x8d, 0xe0,

    /* U+007A "z" */
    0xfc, 0x33, 0x8c, 0xc3, 0xf, 0xc0,

    /* U+007B "{" */
    0x69, 0x48, 0x92, 0x44,

    /* U+007C "|" */
    0xff, 0xc0,

    /* U+007D "}" */
    0xc9, 0x22, 0x92, 0x50,

    /* U+007E "~" */
    0x66, 0x60
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 60, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 60, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 6, .adv_w = 120, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 13, .adv_w = 108, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 20, .adv_w = 156, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 31, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 40, .adv_w = 24, .box_w = 1, .box_h = 3, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 41, .adv_w = 60, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 45, .adv_w = 60, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 49, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 53, .adv_w = 120, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 58, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 60, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 61, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 62, .adv_w = 84, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 75, .adv_w = 48, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 120, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 101, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 115, .adv_w = 120, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 137, .adv_w = 48, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 142, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 147, .adv_w = 108, .box_w = 6, .box_h = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 150, .adv_w = 96, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 154, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 161, .adv_w = 156, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 172, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 179, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 186, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 200, .adv_w = 96, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 96, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 214, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 228, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 231, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 238, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 245, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 251, .adv_w = 168, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 263, .adv_w = 120, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 271, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 278, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 285, .adv_w = 108, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 293, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 300, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 315, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 329, .adv_w = 168, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 341, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 348, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 355, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 362, .adv_w = 72, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 367, .adv_w = 72, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 371, .adv_w = 72, .box_w = 4, .box_h = 10, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 376, .adv_w = 108, .box_w = 7, .box_h = 3, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 379, .adv_w = 144, .box_w = 8, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 380, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 381, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 387, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 394, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 399, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 406, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 412, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 418, .adv_w = 108, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 426, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 433, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 96, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 443, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 450, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 453, .adv_w = 168, .box_w = 10, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 462, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 468, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 474, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 481, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 488, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 494, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 500, .adv_w = 72, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 505, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 511, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 517, .adv_w = 168, .box_w = 10, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 526, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 532, .adv_w = 108, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 540, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 546, .adv_w = 60, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 550, .adv_w = 24, .box_w = 1, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 552, .adv_w = 60, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 556, .adv_w = 108, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 5}
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
const lv_font_t WindyCity_12 = {
#else
lv_font_t WindyCity_12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 12,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if WINDYCITY_12*/

