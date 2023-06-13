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

#ifndef UNSCII_12
#define UNSCII_12 1
#endif

#if UNSCII_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xf0, 0xc0,

    /* U+0022 "\"" */
    0xde, 0xc0,

    /* U+0023 "#" */
    0x79, 0xe7, 0xbf, 0x79, 0xef, 0xde, 0x78,

    /* U+0024 "$" */
    0x21, 0xed, 0xd0, 0x20, 0xe0, 0xf2, 0x70, 0x80,

    /* U+0025 "%" */
    0xe, 0x76, 0x63, 0x11, 0x19, 0xce, 0x0,

    /* U+0026 "&" */
    0x33, 0x65, 0xc, 0xf3, 0xfd, 0xb6, 0xec,

    /* U+0027 "'" */
    0x6f, 0x0,

    /* U+0028 "(" */
    0x3b, 0x6d, 0xb6, 0xd8, 0x80,

    /* U+0029 ")" */
    0x8d, 0xb6, 0xdb, 0x6e, 0x0,

    /* U+002A "*" */
    0x6c, 0x53, 0xf9, 0xc6, 0xc0,

    /* U+002B "+" */
    0x30, 0xcf, 0xcc, 0x30,

    /* U+002C "," */
    0x73, 0x6c,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x4, 0x20, 0x86, 0x18, 0xc2, 0x8, 0xc3, 0xc,
    0x0,

    /* U+0030 "0" */
    0x22, 0xb7, 0xbf, 0xef, 0x6a, 0x20,

    /* U+0031 "1" */
    0x31, 0xcf, 0xc, 0x30, 0xc3, 0xc, 0xfc,

    /* U+0032 "2" */
    0x76, 0xc6, 0x31, 0x33, 0x18, 0xf8,

    /* U+0033 "3" */
    0x76, 0xc6, 0x33, 0xc, 0x7b, 0x70,

    /* U+0034 "4" */
    0x18, 0xe7, 0xb6, 0xdb, 0xf1, 0x86, 0x18,

    /* U+0035 "5" */
    0xfe, 0x31, 0xe1, 0x8c, 0x7b, 0x70,

    /* U+0036 "6" */
    0x33, 0x31, 0xed, 0xef, 0x7b, 0x70,

    /* U+0037 "7" */
    0xf8, 0x42, 0x63, 0x18, 0xc6, 0x30,

    /* U+0038 "8" */
    0x76, 0xf7, 0xf7, 0x7f, 0x7b, 0x70,

    /* U+0039 "9" */
    0x76, 0xf7, 0xb7, 0x8c, 0x66, 0x60,

    /* U+003A ":" */
    0xf0, 0xf,

    /* U+003B ";" */
    0x33, 0x0, 0x0, 0x73, 0x6c,

    /* U+003C "<" */
    0x13, 0x48, 0xc3, 0x10,

    /* U+003D "=" */
    0xf0, 0xf,

    /* U+003E ">" */
    0x8c, 0x21, 0x3c, 0x80,

    /* U+003F "?" */
    0x7b, 0x30, 0xce, 0x20, 0x80, 0x0, 0x0, 0x80,

    /* U+0040 "@" */
    0x76, 0xf7, 0xff, 0xff, 0x18, 0x70,

    /* U+0041 "A" */
    0x22, 0xb7, 0xbf, 0xef, 0x7b, 0xd8,

    /* U+0042 "B" */
    0xf6, 0xf7, 0xee, 0x6b, 0x7b, 0xf0,

    /* U+0043 "C" */
    0x76, 0xf1, 0x8c, 0x63, 0x1b, 0x70,

    /* U+0044 "D" */
    0xe7, 0xb7, 0xbd, 0xef, 0x7e, 0xe0,

    /* U+0045 "E" */
    0xfe, 0x31, 0x8f, 0x63, 0x18, 0xf8,

    /* U+0046 "F" */
    0xfe, 0x31, 0xec, 0x63, 0x18, 0xc0,

    /* U+0047 "G" */
    0x76, 0xf1, 0x8f, 0xef, 0x7b, 0x78,

    /* U+0048 "H" */
    0xde, 0xf7, 0xbf, 0xef, 0x7b, 0xd8,

    /* U+0049 "I" */
    0xfc, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0xfc,

    /* U+004A "J" */
    0x18, 0xc6, 0x31, 0x8c, 0x7b, 0x70,

    /* U+004B "K" */
    0xde, 0xf7, 0xce, 0x6f, 0x7b, 0xd8,

    /* U+004C "L" */
    0xc6, 0x31, 0x8c, 0x63, 0x18, 0xf8,

    /* U+004D "M" */
    0xdf, 0xff, 0xff, 0xef, 0x7b, 0xd8,

    /* U+004E "N" */
    0xde, 0xff, 0xbf, 0xff, 0xfb, 0xd8,

    /* U+004F "O" */
    0x76, 0xf7, 0xbd, 0xef, 0x7b, 0x70,

    /* U+0050 "P" */
    0xf6, 0xf7, 0xbf, 0x63, 0x18, 0xc0,

    /* U+0051 "Q" */
    0x76, 0xf7, 0xbd, 0xef, 0x7b, 0x71, 0x86,

    /* U+0052 "R" */
    0xf6, 0xf7, 0xbf, 0x7b, 0x7b, 0xd8,

    /* U+0053 "S" */
    0x76, 0xf0, 0xc7, 0x18, 0x7b, 0x70,

    /* U+0054 "T" */
    0xfc, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30,

    /* U+0055 "U" */
    0xde, 0xf7, 0xbd, 0xef, 0x7b, 0x70,

    /* U+0056 "V" */
    0xde, 0xf7, 0xbd, 0xa9, 0xce, 0x20,

    /* U+0057 "W" */
    0xde, 0xf7, 0xbf, 0xff, 0xff, 0xd8,

    /* U+0058 "X" */
    0xcd, 0x25, 0xc, 0x30, 0xc5, 0x12, 0xcc,

    /* U+0059 "Y" */
    0xcf, 0x2c, 0x8c, 0x30, 0xc3, 0xc, 0x30,

    /* U+005A "Z" */
    0xf8, 0xc6, 0x62, 0x63, 0x18, 0xf8,

    /* U+005B "[" */
    0xfc, 0xcc, 0xcc, 0xcc, 0xcc, 0xf0,

    /* U+005C "\\" */
    0xc3, 0xc, 0x8, 0x20, 0xc1, 0x86, 0x8, 0x20,
    0x40,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0x33, 0x33, 0xf0,

    /* U+005E "^" */
    0x26, 0xf7, 0xbd, 0x80,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0xc6, 0x30,

    /* U+0061 "a" */
    0x70, 0xdf, 0xbd, 0xbc,

    /* U+0062 "b" */
    0xc6, 0x31, 0xed, 0xef, 0x7b, 0xf0,

    /* U+0063 "c" */
    0x76, 0xf1, 0x8d, 0xb8,

    /* U+0064 "d" */
    0x18, 0xc6, 0xfd, 0xef, 0x7b, 0x78,

    /* U+0065 "e" */
    0x76, 0xf7, 0xfc, 0x38,

    /* U+0066 "f" */
    0x3b, 0x19, 0xf6, 0x31, 0x8c, 0x60,

    /* U+0067 "g" */
    0x7e, 0xf7, 0xbd, 0xbc, 0x7e,

    /* U+0068 "h" */
    0xc6, 0x31, 0xed, 0xef, 0x7b, 0xd8,

    /* U+0069 "i" */
    0x30, 0x0, 0x3c, 0x30, 0xc3, 0xc, 0x3c,

    /* U+006A "j" */
    0x30, 0x3, 0x33, 0x33, 0x33, 0xe0,

    /* U+006B "k" */
    0xc6, 0x31, 0xbd, 0x73, 0x9a, 0xd8,

    /* U+006C "l" */
    0xf0, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x3c,

    /* U+006D "m" */
    0xd7, 0xff, 0xff, 0xec,

    /* U+006E "n" */
    0xf6, 0xf7, 0xbd, 0xec,

    /* U+006F "o" */
    0x76, 0xf7, 0xbd, 0xb8,

    /* U+0070 "p" */
    0xf6, 0xf7, 0xbd, 0xfb, 0x18,

    /* U+0071 "q" */
    0x7e, 0xf7, 0xbd, 0xbc, 0x63,

    /* U+0072 "r" */
    0xf6, 0xb7, 0x8c, 0x60,

    /* U+0073 "s" */
    0x7a, 0x30, 0xe1, 0xf8,

    /* U+0074 "t" */
    0x63, 0x3e, 0xc6, 0x31, 0x87,

    /* U+0075 "u" */
    0xde, 0xf7, 0xbd, 0xbc,

    /* U+0076 "v" */
    0xde, 0xf7, 0xb5, 0x10,

    /* U+0077 "w" */
    0xde, 0xff, 0xf7, 0x28,

    /* U+0078 "x" */
    0xda, 0x98, 0x45, 0x6c,

    /* U+0079 "y" */
    0xde, 0xf7, 0xbd, 0xbc, 0x6e,

    /* U+007A "z" */
    0xf8, 0xcc, 0x8c, 0x7c,

    /* U+007B "{" */
    0x1c, 0xc3, 0xc, 0x33, 0x83, 0xc, 0x30, 0xc1,
    0xc0,

    /* U+007C "|" */
    0xff, 0xff, 0xfc,

    /* U+007D "}" */
    0xe0, 0xc3, 0xc, 0x30, 0x73, 0xc, 0x30, 0xce,
    0x0,

    /* U+007E "~" */
    0x65, 0x6c,

    /* U+007F "" */
    0xc6, 0x31, 0xbc, 0xe7, 0x21, 0x8, 0x42
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 96, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 12},
    {.bitmap_index = 1, .adv_w = 96, .box_w = 2, .box_h = 9, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 4, .adv_w = 96, .box_w = 5, .box_h = 2, .ofs_x = 1, .ofs_y = 10},
    {.bitmap_index = 6, .adv_w = 96, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 13, .adv_w = 96, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 21, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 28, .adv_w = 96, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 35, .adv_w = 96, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 9},
    {.bitmap_index = 37, .adv_w = 96, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 42, .adv_w = 96, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 47, .adv_w = 96, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 52, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 56, .adv_w = 96, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 58, .adv_w = 96, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 59, .adv_w = 96, .box_w = 2, .box_h = 2, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 60, .adv_w = 96, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 69, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 75, .adv_w = 96, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 82, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 88, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 94, .adv_w = 96, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 101, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 107, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 113, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 119, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 125, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 131, .adv_w = 96, .box_w = 2, .box_h = 8, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 133, .adv_w = 96, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 96, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 142, .adv_w = 96, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 144, .adv_w = 96, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 148, .adv_w = 96, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 156, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 162, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 168, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 174, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 180, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 186, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 192, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 198, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 204, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 210, .adv_w = 96, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 217, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 223, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 229, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 235, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 241, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 247, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 253, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 259, .adv_w = 96, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 272, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 278, .adv_w = 96, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 285, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 291, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 297, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 303, .adv_w = 96, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 310, .adv_w = 96, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 317, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 323, .adv_w = 96, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 329, .adv_w = 96, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 338, .adv_w = 96, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 344, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 8},
    {.bitmap_index = 348, .adv_w = 96, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 349, .adv_w = 96, .box_w = 4, .box_h = 3, .ofs_x = 2, .ofs_y = 9},
    {.bitmap_index = 351, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 355, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 361, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 365, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 371, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 375, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 381, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 386, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 392, .adv_w = 96, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 399, .adv_w = 96, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 405, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 411, .adv_w = 96, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 418, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 422, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 426, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 430, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 435, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 440, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 444, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 448, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 453, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 457, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 461, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 465, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 469, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 474, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 478, .adv_w = 96, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 487, .adv_w = 96, .box_w = 2, .box_h = 11, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 490, .adv_w = 96, .box_w = 6, .box_h = 11, .ofs_x = -1, .ofs_y = 1},
    {.bitmap_index = 499, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 10},
    {.bitmap_index = 501, .adv_w = 96, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 1}
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
const lv_font_t unscii_12 = {
#else
lv_font_t unscii_12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 13,          /*The maximum line height required by the font*/
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



#endif /*#if UNSCII_12*/

