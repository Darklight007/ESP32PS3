/*******************************************************************************
 * Size: 14 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef UNSCII_14
#define UNSCII_14 1
#endif

#if UNSCII_14

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xfc, 0x3c,

    /* U+0022 "\"" */
    0xde, 0xc0,

    /* U+0023 "#" */
    0x6d, 0xb6, 0xff, 0x6d, 0xbf, 0xdb, 0x6d, 0xb0,

    /* U+0024 "$" */
    0x31, 0xb3, 0x86, 0x18, 0x41, 0xcb, 0x8c,

    /* U+0025 "%" */
    0x7, 0xc7, 0xc8, 0x8, 0x18, 0x10, 0x30, 0x20,
    0x67, 0xc7, 0xc0,

    /* U+0026 "&" */
    0x31, 0xa6, 0x8c, 0x31, 0xdc, 0xf2, 0xc9, 0xf0,

    /* U+0027 "'" */
    0x6f, 0x0,

    /* U+0028 "(" */
    0x36, 0x6c, 0xcc, 0xcc, 0xc6, 0x63,

    /* U+0029 ")" */
    0xc6, 0x63, 0x33, 0x33, 0x36, 0x6c,

    /* U+002A "*" */
    0x6c, 0xd9, 0xa7, 0xf7, 0x8d, 0x9b, 0x0,

    /* U+002B "+" */
    0x31, 0xbe, 0x63, 0x18,

    /* U+002C "," */
    0x73, 0x36, 0xc0,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x6, 0xc, 0x30, 0x60, 0x82, 0x4, 0x8, 0x30,
    0xc1, 0x86, 0xc, 0x0,

    /* U+0030 "0" */
    0x38, 0x68, 0xc7, 0xc7, 0xcf, 0xf7, 0xc7, 0xc7,
    0x68, 0x38,

    /* U+0031 "1" */
    0x33, 0xbc, 0x63, 0x18, 0xc6, 0x37, 0xc0,

    /* U+0032 "2" */
    0x76, 0x72, 0x11, 0x18, 0xd8, 0xc7, 0xc0,

    /* U+0033 "3" */
    0x7e, 0xf6, 0x31, 0x8c, 0x7b, 0xdb, 0xc0,

    /* U+0034 "4" */
    0xc, 0x38, 0xf3, 0x6c, 0xdf, 0xc3, 0x6, 0xc,
    0x18,

    /* U+0035 "5" */
    0xfe, 0x31, 0x8f, 0x8c, 0x7b, 0xdb, 0xc0,

    /* U+0036 "6" */
    0x33, 0x31, 0x8f, 0x6f, 0x7b, 0xdb, 0x80,

    /* U+0037 "7" */
    0xfc, 0x30, 0xc3, 0x18, 0x63, 0xc, 0x30, 0xc0,

    /* U+0038 "8" */
    0x7e, 0xf7, 0xb7, 0xbf, 0xfb, 0xdb, 0xc0,

    /* U+0039 "9" */
    0x76, 0xf7, 0xb5, 0xbc, 0x63, 0x33, 0x0,

    /* U+003A ":" */
    0xf0, 0x3, 0xc0,

    /* U+003B ";" */
    0x33, 0x0, 0x0, 0x73, 0x36, 0xc0,

    /* U+003C "<" */
    0x19, 0x89, 0x86, 0x18, 0x41,

    /* U+003D "=" */
    0xf8, 0x0, 0xf, 0x80,

    /* U+003E ">" */
    0xe3, 0x84, 0x11, 0x19, 0x98,

    /* U+003F "?" */
    0x7e, 0x72, 0x11, 0x98, 0xc0, 0x1, 0x8c,

    /* U+0040 "@" */
    0x7b, 0x2c, 0xf3, 0xdf, 0x7d, 0xb0, 0xc1, 0xe0,

    /* U+0041 "A" */
    0x33, 0xf3, 0x9c, 0xff, 0x39, 0xce, 0x40,

    /* U+0042 "B" */
    0xf6, 0xf7, 0xaf, 0x73, 0x5b, 0xdf, 0x80,

    /* U+0043 "C" */
    0x7e, 0xf7, 0x8c, 0x63, 0x1b, 0xdb, 0xc0,

    /* U+0044 "D" */
    0xe7, 0xb7, 0xbd, 0xef, 0x7b, 0xf7, 0x0,

    /* U+0045 "E" */
    0xff, 0xc, 0x30, 0xc3, 0xec, 0x30, 0xc3, 0xf0,

    /* U+0046 "F" */
    0xff, 0xc, 0x30, 0xfb, 0xc, 0x30, 0xc3, 0x0,

    /* U+0047 "G" */
    0x7e, 0xf7, 0x8c, 0x7f, 0x7b, 0xdb, 0xc0,

    /* U+0048 "H" */
    0xde, 0xf7, 0xbd, 0xff, 0x7b, 0xde, 0xc0,

    /* U+0049 "I" */
    0xf9, 0x8c, 0x63, 0x18, 0xc6, 0x37, 0xc0,

    /* U+004A "J" */
    0x18, 0xc6, 0x31, 0x8c, 0x7b, 0xdb, 0xc0,

    /* U+004B "K" */
    0xcf, 0x3d, 0xb6, 0xe3, 0x8d, 0xb6, 0xcf, 0x30,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xf0,

    /* U+004D "M" */
    0xcf, 0xff, 0xf7, 0xdf, 0x3c, 0xf3, 0xcf, 0x30,

    /* U+004E "N" */
    0xcf, 0x3e, 0xfb, 0xef, 0x7d, 0xf7, 0xcf, 0x30,

    /* U+004F "O" */
    0x7e, 0xf7, 0xbd, 0xef, 0x7b, 0xdb, 0xc0,

    /* U+0050 "P" */
    0xf6, 0xf7, 0xbd, 0x7b, 0x18, 0xc6, 0x0,

    /* U+0051 "Q" */
    0x7e, 0xf7, 0xbd, 0xef, 0x7b, 0xdb, 0xc6, 0x30,

    /* U+0052 "R" */
    0xf6, 0xf7, 0xbd, 0x7b, 0xdb, 0xde, 0xc0,

    /* U+0053 "S" */
    0x76, 0x72, 0x86, 0x18, 0x59, 0xcb, 0x80,

    /* U+0054 "T" */
    0xf9, 0x8c, 0x63, 0x18, 0xc6, 0x31, 0x80,

    /* U+0055 "U" */
    0xde, 0xf7, 0xbd, 0xef, 0x7b, 0xdb, 0xc0,

    /* U+0056 "V" */
    0xce, 0x73, 0x9c, 0xe5, 0x2f, 0x79, 0x80,

    /* U+0057 "W" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x7d, 0xff, 0xff, 0x30,

    /* U+0058 "X" */
    0xc7, 0x8d, 0xb1, 0xc1, 0x2, 0xe, 0x36, 0xc7,
    0x8c,

    /* U+0059 "Y" */
    0xc7, 0x8d, 0xb3, 0x61, 0x2, 0x4, 0x8, 0x10,
    0x20,

    /* U+005A "Z" */
    0xf8, 0x42, 0x21, 0x31, 0x98, 0xc7, 0xc0,

    /* U+005B "[" */
    0xfc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcf,

    /* U+005C "\\" */
    0xc1, 0x81, 0x83, 0x3, 0x2, 0x4, 0x8, 0x8,
    0x18, 0x30, 0x30, 0x60,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0x33, 0x33, 0x3f,

    /* U+005E "^" */
    0x10, 0x68, 0x68, 0xc7, 0xc7,

    /* U+005F "_" */
    0xfe,

    /* U+0060 "`" */
    0xcc, 0x63,

    /* U+0061 "a" */
    0x7b, 0xd7, 0xbd, 0xbc,

    /* U+0062 "b" */
    0xc6, 0x31, 0x8f, 0x6f, 0x7b, 0xdf, 0x80,

    /* U+0063 "c" */
    0x7e, 0xf1, 0x8d, 0xbc,

    /* U+0064 "d" */
    0x18, 0xc6, 0x37, 0xef, 0x7b, 0xdb, 0xc0,

    /* U+0065 "e" */
    0x7e, 0xff, 0x8c, 0x3c,

    /* U+0066 "f" */
    0x7b, 0x18, 0xcf, 0xb1, 0x8c, 0x63, 0x0,

    /* U+0067 "g" */
    0x7e, 0xf7, 0xbd, 0xbc, 0x63, 0xf0,

    /* U+0068 "h" */
    0xc6, 0x31, 0x8f, 0x6b, 0x7b, 0xde, 0xc0,

    /* U+0069 "i" */
    0x30, 0xc0, 0x0, 0xf0, 0xc3, 0xc, 0x30, 0xf0,

    /* U+006A "j" */
    0x33, 0x0, 0x33, 0x33, 0x33, 0x33, 0xf0,

    /* U+006B "k" */
    0xc6, 0x31, 0x8d, 0xef, 0xde, 0xde, 0xc0,

    /* U+006C "l" */
    0xf0, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xf0,

    /* U+006D "m" */
    0xcb, 0xed, 0xf7, 0xdf, 0x30,

    /* U+006E "n" */
    0xf6, 0xb7, 0xbd, 0xec,

    /* U+006F "o" */
    0x7e, 0xf7, 0xbd, 0xbc,

    /* U+0070 "p" */
    0xf6, 0xf7, 0xbd, 0xfb, 0x18, 0xc0,

    /* U+0071 "q" */
    0x7e, 0xf7, 0xbd, 0xbc, 0x63, 0x18,

    /* U+0072 "r" */
    0xf6, 0xf7, 0x8c, 0x60,

    /* U+0073 "s" */
    0x7e, 0x1e, 0x31, 0xfc,

    /* U+0074 "t" */
    0x63, 0x19, 0xf6, 0x31, 0x8c, 0x78,

    /* U+0075 "u" */
    0xde, 0xf7, 0xb5, 0xbc,

    /* U+0076 "v" */
    0xce, 0x73, 0x97, 0x98,

    /* U+0077 "w" */
    0xcf, 0x3d, 0xf7, 0x79, 0xa0,

    /* U+0078 "x" */
    0xcf, 0x37, 0x9e, 0xcf, 0x30,

    /* U+0079 "y" */
    0xde, 0xf7, 0xb5, 0xbc, 0x63, 0x78,

    /* U+007A "z" */
    0xf8, 0xcc, 0xcc, 0x7c,

    /* U+007B "{" */
    0xc, 0xc3, 0xc, 0x30, 0xcf, 0xc, 0x30, 0xc3,
    0x3,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xc0,

    /* U+007D "}" */
    0xc0, 0xc3, 0xc, 0x30, 0xc3, 0xcc, 0x30, 0xc3,
    0x30,

    /* U+007E "~" */
    0x73, 0xae, 0x70,

    /* U+007F "" */
    0xc3, 0xc, 0x2b, 0xae, 0xaa, 0xb2, 0xc8, 0x20,
    0x82, 0x8
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 112, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 14},
    {.bitmap_index = 1, .adv_w = 112, .box_w = 2, .box_h = 11, .ofs_x = 3, .ofs_y = 3},
    {.bitmap_index = 4, .adv_w = 112, .box_w = 5, .box_h = 2, .ofs_x = 1, .ofs_y = 11},
    {.bitmap_index = 6, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 14, .adv_w = 112, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 21, .adv_w = 112, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 32, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 40, .adv_w = 112, .box_w = 3, .box_h = 3, .ofs_x = 2, .ofs_y = 10},
    {.bitmap_index = 42, .adv_w = 112, .box_w = 4, .box_h = 12, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 48, .adv_w = 112, .box_w = 4, .box_h = 12, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 54, .adv_w = 112, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 61, .adv_w = 112, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 65, .adv_w = 112, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 68, .adv_w = 112, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 69, .adv_w = 112, .box_w = 2, .box_h = 2, .ofs_x = 3, .ofs_y = 3},
    {.bitmap_index = 70, .adv_w = 112, .box_w = 7, .box_h = 13, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 82, .adv_w = 112, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 92, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 99, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 106, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 113, .adv_w = 112, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 122, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 129, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 136, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 144, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 151, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 158, .adv_w = 112, .box_w = 2, .box_h = 9, .ofs_x = 3, .ofs_y = 3},
    {.bitmap_index = 161, .adv_w = 112, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 167, .adv_w = 112, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 172, .adv_w = 112, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 176, .adv_w = 112, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 181, .adv_w = 112, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 188, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 196, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 203, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 210, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 217, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 224, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 232, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 240, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 247, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 254, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 261, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 268, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 276, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 284, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 292, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 300, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 307, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 314, .adv_w = 112, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 322, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 329, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 336, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 343, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 350, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 357, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 365, .adv_w = 112, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 374, .adv_w = 112, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 383, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 390, .adv_w = 112, .box_w = 4, .box_h = 12, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 396, .adv_w = 112, .box_w = 7, .box_h = 13, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 408, .adv_w = 112, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 414, .adv_w = 112, .box_w = 8, .box_h = 5, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 419, .adv_w = 112, .box_w = 7, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 420, .adv_w = 112, .box_w = 4, .box_h = 4, .ofs_x = 3, .ofs_y = 10},
    {.bitmap_index = 422, .adv_w = 112, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 426, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 433, .adv_w = 112, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 437, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 444, .adv_w = 112, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 448, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 455, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 461, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 468, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 476, .adv_w = 112, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 483, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 490, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 498, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 503, .adv_w = 112, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 507, .adv_w = 112, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 511, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 517, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 523, .adv_w = 112, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 527, .adv_w = 112, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 531, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 537, .adv_w = 112, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 541, .adv_w = 112, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 545, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 550, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 555, .adv_w = 112, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 561, .adv_w = 112, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 565, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 574, .adv_w = 112, .box_w = 2, .box_h = 13, .ofs_x = 3, .ofs_y = 2},
    {.bitmap_index = 578, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 587, .adv_w = 112, .box_w = 7, .box_h = 3, .ofs_x = 0, .ofs_y = 11},
    {.bitmap_index = 590, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = 3}
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
const lv_font_t unscii_14 = {
#else
lv_font_t unscii_14 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
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



#endif /*#if UNSCII_14*/

