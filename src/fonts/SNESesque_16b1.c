/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef SNESESQUE_16B1
#define SNESESQUE_16B1 1
#endif

#if SNESESQUE_16B1

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xf3,

    /* U+0022 "\"" */
    0xb6, 0x80,

    /* U+0023 "#" */
    0x52, 0xbe, 0xa5, 0x7d, 0x4a,

    /* U+0024 "$" */
    0x23, 0xaa, 0xc3, 0x55, 0xc4,

    /* U+0025 "%" */
    0xe1, 0xa2, 0xe4, 0x8, 0x10, 0x27, 0x45, 0x87,

    /* U+0026 "&" */
    0x73, 0x4c, 0x18, 0xdf, 0x27, 0x0,

    /* U+0027 "'" */
    0xf6,

    /* U+0028 "(" */
    0x2a, 0x49, 0x11,

    /* U+0029 ")" */
    0x88, 0x92, 0x54,

    /* U+002A "*" */
    0xaa, 0x80,

    /* U+002B "+" */
    0x21, 0x3e, 0x42, 0x0,

    /* U+002C "," */
    0xf6,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x11, 0x22, 0x44, 0x88,

    /* U+0030 "0" */
    0x7b, 0x3d, 0xf7, 0xef, 0xbc, 0xde,

    /* U+0031 "1" */
    0x33, 0xc3, 0xc, 0x30, 0xc3, 0x3f,

    /* U+0032 "2" */
    0x7b, 0x30, 0xc6, 0x31, 0x8c, 0x3f,

    /* U+0033 "3" */
    0x7a, 0x30, 0xce, 0xc, 0x38, 0xde,

    /* U+0034 "4" */
    0x18, 0xe5, 0xa6, 0xfc, 0x61, 0x86,

    /* U+0035 "5" */
    0xff, 0xc, 0x3e, 0xc, 0x38, 0xde,

    /* U+0036 "6" */
    0x7b, 0x1c, 0x3e, 0xcf, 0x3c, 0xde,

    /* U+0037 "7" */
    0xfc, 0x31, 0x86, 0x30, 0xc3, 0xc,

    /* U+0038 "8" */
    0x7b, 0x3c, 0xde, 0xcf, 0x3c, 0xde,

    /* U+0039 "9" */
    0x7b, 0x3c, 0xf3, 0x7c, 0x38, 0xde,

    /* U+003A ":" */
    0xf0, 0xf0,

    /* U+003B ";" */
    0xf0, 0xf6,

    /* U+003C "<" */
    0xc, 0xcc, 0x30, 0x30, 0x30,

    /* U+003D "=" */
    0xf0, 0xf,

    /* U+003E ">" */
    0xc0, 0xc0, 0xc3, 0x33, 0x0,

    /* U+003F "?" */
    0x7a, 0x31, 0x8c, 0x30, 0x3, 0xc,

    /* U+0040 "@" */
    0x3c, 0x42, 0x9d, 0xa5, 0xa5, 0xbb, 0x40, 0x3e,

    /* U+0041 "A" */
    0x7b, 0x3c, 0xf3, 0xff, 0x3c, 0xf3,

    /* U+0042 "B" */
    0xfb, 0x3c, 0xfe, 0xcf, 0x3c, 0xfe,

    /* U+0043 "C" */
    0x7b, 0x1c, 0x30, 0xc3, 0xc, 0x5e,

    /* U+0044 "D" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3c, 0xfe,

    /* U+0045 "E" */
    0xff, 0xc, 0x3e, 0xc3, 0xc, 0x3f,

    /* U+0046 "F" */
    0xff, 0xc, 0x3e, 0xc3, 0xc, 0x30,

    /* U+0047 "G" */
    0x7b, 0x1c, 0x30, 0xdf, 0x3c, 0xde,

    /* U+0048 "H" */
    0xcf, 0x3c, 0xff, 0xcf, 0x3c, 0xf3,

    /* U+0049 "I" */
    0xf6, 0x66, 0x66, 0x6f,

    /* U+004A "J" */
    0x18, 0xc6, 0x31, 0x8e, 0x6e,

    /* U+004B "K" */
    0xcf, 0x6f, 0x38, 0xe3, 0xcd, 0xb3,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xc, 0x3f,

    /* U+004D "M" */
    0xc7, 0xdf, 0xfe, 0xbc, 0x78, 0xf1, 0xe3,

    /* U+004E "N" */
    0xc7, 0x8f, 0x9f, 0xbd, 0xf9, 0xf1, 0xe3,

    /* U+004F "O" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x3c, 0xde,

    /* U+0050 "P" */
    0xfb, 0x3c, 0xf3, 0xfb, 0xc, 0x30,

    /* U+0051 "Q" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x3c, 0xde, 0xc,

    /* U+0052 "R" */
    0xfb, 0x3c, 0xf3, 0xfb, 0xcd, 0xb3,

    /* U+0053 "S" */
    0x7b, 0x1c, 0x3e, 0x7c, 0x38, 0xde,

    /* U+0054 "T" */
    0xfc, 0xc3, 0xc, 0x30, 0xc3, 0xc,

    /* U+0055 "U" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xde,

    /* U+0056 "V" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xbc,

    /* U+0057 "W" */
    0xcc, 0xf3, 0x3c, 0xcf, 0x33, 0xcc, 0xf3, 0x3c,
    0xcb, 0xfc,

    /* U+0058 "X" */
    0xcf, 0x3c, 0xde, 0xcf, 0x3c, 0xf3,

    /* U+0059 "Y" */
    0xcf, 0x3c, 0xf3, 0x78, 0xc3, 0xc,

    /* U+005A "Z" */
    0xfc, 0x31, 0x8c, 0x63, 0xc, 0x3f,

    /* U+005B "[" */
    0xea, 0xab,

    /* U+005C "\\" */
    0x88, 0x44, 0x22, 0x11,

    /* U+005D "]" */
    0xd5, 0x57,

    /* U+005E "^" */
    0x22, 0xa2,

    /* U+005F "_" */
    0xf0,

    /* U+0060 "`" */
    0x90,

    /* U+0061 "a" */
    0x78, 0x37, 0xf3, 0xcd, 0xd0,

    /* U+0062 "b" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xfe,

    /* U+0063 "c" */
    0x7b, 0x1c, 0x30, 0xc5, 0xe0,

    /* U+0064 "d" */
    0xc, 0x37, 0xf3, 0xcf, 0x3d, 0xdb,

    /* U+0065 "e" */
    0x7b, 0x3f, 0xf0, 0xc5, 0xe0,

    /* U+0066 "f" */
    0x3b, 0x19, 0xf6, 0x31, 0x8c,

    /* U+0067 "g" */
    0x7f, 0x3c, 0xf3, 0xcd, 0xf0, 0xe3, 0x78,

    /* U+0068 "h" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xf3,

    /* U+0069 "i" */
    0x63, 0xb6, 0xdb,

    /* U+006A "j" */
    0x63, 0xb6, 0xdb, 0x6f, 0x0,

    /* U+006B "k" */
    0xc3, 0xc, 0xf6, 0xf3, 0xcd, 0xb3,

    /* U+006C "l" */
    0xed, 0xb6, 0xdb,

    /* U+006D "m" */
    0xff, 0xb3, 0x3c, 0xcf, 0x33, 0xcc, 0xf3, 0x30,

    /* U+006E "n" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x30,

    /* U+006F "o" */
    0x7b, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+0070 "p" */
    0xfb, 0x3c, 0xf3, 0xcf, 0xec, 0x30, 0xc0,

    /* U+0071 "q" */
    0x7f, 0x3c, 0xf3, 0xcd, 0xf0, 0xc3, 0xc,

    /* U+0072 "r" */
    0xdb, 0x9c, 0x30, 0xc3, 0x0,

    /* U+0073 "s" */
    0x7b, 0x1f, 0xf, 0x8d, 0xe0,

    /* U+0074 "t" */
    0x21, 0x8f, 0x18, 0x61, 0x86, 0x4e,

    /* U+0075 "u" */
    0xcf, 0x3c, 0xf3, 0xcd, 0xd0,

    /* U+0076 "v" */
    0xcf, 0x3c, 0xf3, 0xcb, 0xc0,

    /* U+0077 "w" */
    0xcc, 0xf3, 0x3c, 0xcf, 0x33, 0xcc, 0xbd, 0xc0,

    /* U+0078 "x" */
    0xcd, 0xe3, 0xc, 0x7b, 0x30,

    /* U+0079 "y" */
    0xcf, 0x3c, 0xf3, 0xcd, 0xf0, 0xe3, 0x78,

    /* U+007A "z" */
    0xfc, 0x31, 0x8c, 0x63, 0xf0,

    /* U+007B "{" */
    0x29, 0x28, 0x92, 0x20,

    /* U+007C "|" */
    0xff, 0x80,

    /* U+007D "}" */
    0x89, 0x22, 0x92, 0x80,

    /* U+007E "~" */
    0x66, 0x60
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 80, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 64, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 5, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 10, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 15, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 23, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 29, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 30, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 36, .adv_w = 64, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 38, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 42, .adv_w = 64, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 43, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 44, .adv_w = 64, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 45, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 49, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 67, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 73, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 85, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 91, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 64, .box_w = 2, .box_h = 6, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 111, .adv_w = 64, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 113, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 118, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 120, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 125, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 131, .adv_w = 144, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 145, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 151, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 169, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 181, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 191, .adv_w = 112, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 202, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 215, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 222, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 228, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 241, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 253, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 265, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 271, .adv_w = 176, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 281, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 287, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 299, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 301, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 305, .adv_w = 48, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 307, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 309, .adv_w = 80, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 310, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 311, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 316, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 327, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 338, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 343, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 350, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 356, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 359, .adv_w = 64, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 364, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 370, .adv_w = 64, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 373, .adv_w = 176, .box_w = 10, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 386, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 391, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 398, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 405, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 410, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 415, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 421, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 426, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 431, .adv_w = 176, .box_w = 10, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 439, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 444, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 451, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 456, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 460, .adv_w = 32, .box_w = 1, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 462, .adv_w = 64, .box_w = 3, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 466, .adv_w = 112, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 3}
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
const lv_font_t SNESesque_16b1 = {
#else
lv_font_t SNESesque_16b1 = {
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



#endif /*#if SNESESQUE_16B1*/

