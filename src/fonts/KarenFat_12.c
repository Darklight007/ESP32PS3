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

#ifndef KARENFAT_12
#define KARENFAT_12 1
#endif

#if KARENFAT_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xc,

    /* U+0022 "\"" */
    0xff, 0x80,

    /* U+0023 "#" */
    0x6c, 0xdb, 0xf3, 0x6f, 0xed, 0x9b, 0x0,

    /* U+0024 "$" */
    0x7f, 0xc7, 0x1e, 0x3f, 0xe3, 0x0,

    /* U+0025 "%" */
    0x30, 0x5f, 0x3e, 0x1c, 0x38, 0x38, 0x7c, 0xde,
    0xc,

    /* U+0026 "&" */
    0x70, 0x50, 0x50, 0x7e, 0x50, 0xd7, 0x7e,

    /* U+0027 "'" */
    0xec,

    /* U+0028 "(" */
    0x6b, 0x49, 0x93,

    /* U+0029 ")" */
    0xc9, 0x92, 0xd6,

    /* U+002A "*" */
    0xfc, 0xc3, 0x3f,

    /* U+002B "+" */
    0x37, 0xcc, 0x60,

    /* U+002C "," */
    0x6d, 0xe0,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0xc0,

    /* U+002F "/" */
    0x12, 0x26, 0x44, 0x80,

    /* U+0030 "0" */
    0xfe, 0xf7, 0xbd, 0xed, 0xc0,

    /* U+0031 "1" */
    0x2e, 0x66, 0x66, 0x6f,

    /* U+0032 "2" */
    0xf8, 0xc7, 0xec, 0x6f, 0xe0,

    /* U+0033 "3" */
    0xfb, 0x67, 0x1e, 0x1b, 0x7f, 0x80,

    /* U+0034 "4" */
    0x3b, 0x6d, 0xb6, 0xfc, 0x61, 0x80,

    /* U+0035 "5" */
    0xfe, 0x31, 0xf1, 0xed, 0xc0,

    /* U+0036 "6" */
    0xfe, 0x31, 0xfd, 0xed, 0xc0,

    /* U+0037 "7" */
    0xff, 0xb1, 0xc6, 0x38, 0xc3, 0x0,

    /* U+0038 "8" */
    0xfa, 0x95, 0xfd, 0xed, 0xc0,

    /* U+0039 "9" */
    0xfe, 0xf6, 0xf1, 0xed, 0xc0,

    /* U+003A ":" */
    0xc0, 0xc0,

    /* U+003B ";" */
    0x60, 0x0, 0xf0,

    /* U+003C "<" */
    0x7c, 0x47,

    /* U+003D "=" */
    0xf0, 0xf,

    /* U+003E ">" */
    0xe3, 0x2e,

    /* U+003F "?" */
    0x7d, 0xdc, 0x70, 0xc3, 0x0, 0x0, 0x18,

    /* U+0040 "@" */
    0x3c, 0x8e, 0xed, 0x5a, 0xb7, 0xd0, 0x1f,

    /* U+0041 "A" */
    0x7b, 0x3c, 0xf3, 0xff, 0x3c, 0xc0,

    /* U+0042 "B" */
    0xfb, 0x3c, 0xbe, 0xcb, 0x3f, 0x80,

    /* U+0043 "C" */
    0x7b, 0x3c, 0x30, 0xc3, 0x37, 0x80,

    /* U+0044 "D" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x2f, 0x80,

    /* U+0045 "E" */
    0xff, 0x3c, 0x3c, 0xc3, 0x3f, 0xc0,

    /* U+0046 "F" */
    0xff, 0x3c, 0x3e, 0xc3, 0xc, 0x0,

    /* U+0047 "G" */
    0x7b, 0x3c, 0x37, 0xcd, 0x27, 0x80,

    /* U+0048 "H" */
    0xcf, 0x3c, 0xff, 0xcf, 0x3c, 0xc0,

    /* U+0049 "I" */
    0xf6, 0x66, 0x66, 0xf0,

    /* U+004A "J" */
    0x7c, 0x30, 0xc3, 0xf, 0x37, 0x80,

    /* U+004B "K" */
    0xcf, 0x2d, 0xbe, 0xdb, 0x2c, 0xc0,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0x3f, 0xc0,

    /* U+004D "M" */
    0xc7, 0x8f, 0xbe, 0xbc, 0x78, 0xf1, 0x80,

    /* U+004E "N" */
    0xcf, 0x3e, 0xff, 0xff, 0x7c, 0xc0,

    /* U+004F "O" */
    0x7b, 0x3c, 0xf3, 0xcd, 0x27, 0x80,

    /* U+0050 "P" */
    0xfb, 0x3c, 0xf3, 0xff, 0xc, 0x0,

    /* U+0051 "Q" */
    0x7b, 0x3c, 0xf3, 0xcd, 0x27, 0x84, 0xc,

    /* U+0052 "R" */
    0xfb, 0x3d, 0xbe, 0xcb, 0x2c, 0xc0,

    /* U+0053 "S" */
    0x7f, 0x34, 0x9e, 0x4b, 0x3f, 0x80,

    /* U+0054 "T" */
    0xff, 0x6c, 0xc1, 0x83, 0x6, 0xc, 0x0,

    /* U+0055 "U" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x37, 0x80,

    /* U+0056 "V" */
    0xc6, 0xd9, 0xb3, 0x67, 0x8f, 0xc, 0x0,

    /* U+0057 "W" */
    0xc7, 0xaf, 0x5e, 0xb6, 0xcd, 0x9b, 0x0,

    /* U+0058 "X" */
    0xc6, 0x89, 0xf3, 0xe7, 0xcf, 0xb1, 0x80,

    /* U+0059 "Y" */
    0xcd, 0x24, 0x9e, 0x30, 0xc3, 0x0,

    /* U+005A "Z" */
    0xff, 0x98, 0x21, 0xc2, 0xc, 0xff, 0x80,

    /* U+005B "[" */
    0xea, 0xaa, 0xb0,

    /* U+005C "\\" */
    0x88, 0x46, 0x22, 0x10,

    /* U+005D "]" */
    0xd5, 0x55, 0x70,

    /* U+005E "^" */
    0x76, 0xf6,

    /* U+005F "_" */
    0xff,

    /* U+0060 "`" */
    0xcc,

    /* U+0061 "a" */
    0x7c, 0x3f, 0xf3, 0xcf, 0xf0,

    /* U+0062 "b" */
    0xc3, 0xd, 0xbf, 0xcf, 0x3c, 0xfe,

    /* U+0063 "c" */
    0x7f, 0x3c, 0xf0, 0xc1, 0xf0,

    /* U+0064 "d" */
    0xc, 0x36, 0xf7, 0xcf, 0x3c, 0xdf,

    /* U+0065 "e" */
    0x7f, 0x3f, 0xf0, 0xc1, 0xf0,

    /* U+0066 "f" */
    0x3b, 0x3e, 0xc6, 0x31, 0x8c,

    /* U+0067 "g" */
    0x77, 0x9b, 0x36, 0x6c, 0xcf, 0x83, 0x3e,

    /* U+0068 "h" */
    0xc3, 0x6e, 0xbb, 0xcf, 0x3c, 0xc0,

    /* U+0069 "i" */
    0x60, 0xe6, 0x66, 0x67,

    /* U+006A "j" */
    0x18, 0x0, 0xf1, 0x8c, 0x63, 0x1e, 0xdc,

    /* U+006B "k" */
    0xc3, 0xc, 0xf6, 0xfb, 0x2c, 0xc0,

    /* U+006C "l" */
    0xe6, 0x66, 0x66, 0xf0,

    /* U+006D "m" */
    0xf7, 0x3a, 0x9b, 0x6d, 0xb6, 0xdb, 0x6c,

    /* U+006E "n" */
    0xdc, 0xa9, 0x9b, 0x36, 0x6c, 0xc0,

    /* U+006F "o" */
    0x7b, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+0070 "p" */
    0xdb, 0xbc, 0xf3, 0xcf, 0xec, 0x30,

    /* U+0071 "q" */
    0x7b, 0x3c, 0xf3, 0xfd, 0xb0, 0xc3,

    /* U+0072 "r" */
    0xdb, 0xbc, 0x30, 0xc3, 0x0,

    /* U+0073 "s" */
    0x7f, 0x37, 0x86, 0x1f, 0xe0,

    /* U+0074 "t" */
    0x23, 0x3e, 0xc6, 0x31, 0x87,

    /* U+0075 "u" */
    0xcf, 0x3c, 0xf3, 0x55, 0xd0,

    /* U+0076 "v" */
    0xc7, 0x8f, 0xbb, 0x66, 0xcf, 0x80,

    /* U+0077 "w" */
    0xc3, 0xdb, 0xdb, 0xdb, 0x5a, 0x66,

    /* U+0078 "x" */
    0xdb, 0xcc, 0x65, 0xec,

    /* U+0079 "y" */
    0xcf, 0x3c, 0xf3, 0x4d, 0xf0, 0xde,

    /* U+007A "z" */
    0xff, 0x63, 0x9c, 0x6f, 0xf0,

    /* U+007B "{" */
    0x69, 0x28, 0x92, 0x4c,

    /* U+007C "|" */
    0xff, 0xc0,

    /* U+007D "}" */
    0xc9, 0x22, 0x92, 0x58,

    /* U+007E "~" */
    0x5f, 0x80
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 60, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 60, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 3, .adv_w = 96, .box_w = 5, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 5, .adv_w = 120, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 12, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 18, .adv_w = 132, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 27, .adv_w = 132, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 34, .adv_w = 60, .box_w = 3, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 35, .adv_w = 72, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 72, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 41, .adv_w = 108, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 44, .adv_w = 108, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 47, .adv_w = 72, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 49, .adv_w = 72, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 50, .adv_w = 60, .box_w = 2, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 51, .adv_w = 84, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 55, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 60, .adv_w = 84, .box_w = 4, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 64, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 75, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 81, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 96, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 107, .adv_w = 60, .box_w = 2, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 72, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 112, .adv_w = 84, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 114, .adv_w = 84, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 116, .adv_w = 84, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 118, .adv_w = 108, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 132, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 132, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 156, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 162, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 168, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 174, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 72, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 184, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 202, .adv_w = 132, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 209, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 215, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 234, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 240, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 246, .adv_w = 120, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 253, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 120, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 132, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 273, .adv_w = 120, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 280, .adv_w = 120, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 286, .adv_w = 120, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 60, .box_w = 2, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 296, .adv_w = 84, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 300, .adv_w = 60, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 303, .adv_w = 108, .box_w = 5, .box_h = 3, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 305, .adv_w = 132, .box_w = 8, .box_h = 1, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 306, .adv_w = 72, .box_w = 3, .box_h = 2, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 307, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 312, .adv_w = 108, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 318, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 323, .adv_w = 108, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 329, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 334, .adv_w = 84, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 339, .adv_w = 120, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 346, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 352, .adv_w = 72, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 356, .adv_w = 96, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 363, .adv_w = 108, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 369, .adv_w = 72, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 373, .adv_w = 156, .box_w = 9, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 380, .adv_w = 120, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 386, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 391, .adv_w = 108, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 397, .adv_w = 108, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 403, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 408, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 413, .adv_w = 84, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 418, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 423, .adv_w = 120, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 429, .adv_w = 144, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 435, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 439, .adv_w = 108, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 445, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 450, .adv_w = 72, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 454, .adv_w = 48, .box_w = 1, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 456, .adv_w = 72, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 460, .adv_w = 96, .box_w = 5, .box_h = 2, .ofs_x = 1, .ofs_y = 3}
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
const lv_font_t KarenFat_12 = {
#else
lv_font_t KarenFat_12 = {
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



#endif /*#if KARENFAT_12*/

