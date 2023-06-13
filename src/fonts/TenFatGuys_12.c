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

#ifndef TENFATGUYS_12
#define TENFATGUYS_12 1
#endif

#if TENFATGUYS_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xf0, 0x7,

    /* U+0022 "\"" */
    0xed, 0xa0,

    /* U+0023 "#" */
    0x39, 0xc7, 0x3b, 0xff, 0x9c, 0xe3, 0x9d, 0xff,
    0xce, 0x71, 0xce,

    /* U+0024 "$" */
    0x30, 0xcf, 0xec, 0xfc, 0xdf, 0xcc, 0x30,

    /* U+0025 "%" */
    0xe3, 0xf3, 0xbb, 0xc3, 0xc3, 0xc3, 0xdd, 0xef,
    0xc7,

    /* U+0026 "&" */
    0x67, 0xd8, 0xc0,

    /* U+0027 "'" */
    0xe8,

    /* U+0028 "(" */
    0x1c, 0xe7, 0x1c, 0xe1, 0xc3, 0x87,

    /* U+0029 ")" */
    0xc3, 0x1c, 0xe3, 0xb9, 0x98,

    /* U+002A "*" */
    0x83, 0x7, 0xb9, 0xc3, 0x87, 0x31, 0xc1,

    /* U+002B "+" */
    0x18, 0x18, 0x18, 0x18, 0xff, 0x18, 0x18, 0x18,

    /* U+002C "," */
    0xf8,

    /* U+002D "-" */
    0xfc,

    /* U+002E "." */
    0xc0,

    /* U+002F "/" */
    0x4, 0x3, 0xd3, 0xdd, 0x47, 0x81, 0xc0, 0x40,

    /* U+0030 "0" */
    0xff, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xff,

    /* U+0031 "1" */
    0xff, 0xff, 0xff,

    /* U+0032 "2" */
    0xff, 0x7, 0x7, 0x7, 0xff, 0xe0, 0xe0, 0xff,

    /* U+0033 "3" */
    0xff, 0x7, 0x7, 0x7, 0xff, 0x7, 0x7, 0xff,

    /* U+0034 "4" */
    0xe7, 0xe7, 0xe7, 0xe7, 0xff, 0x7, 0x7, 0x7,

    /* U+0035 "5" */
    0xff, 0xe0, 0xe0, 0xe0, 0xff, 0x7, 0x7, 0xff,

    /* U+0036 "6" */
    0xff, 0xe0, 0xe0, 0xe0, 0xff, 0xe7, 0xe7, 0xff,

    /* U+0037 "7" */
    0xff, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0x7,

    /* U+0038 "8" */
    0xff, 0xe7, 0xe7, 0xe7, 0xff, 0xe7, 0xe7, 0xff,

    /* U+0039 "9" */
    0xff, 0xe7, 0xe7, 0xe7, 0xff, 0x7, 0x7, 0xff,

    /* U+003A ":" */
    0xc0, 0x30,

    /* U+003B ";" */
    0x60, 0x0, 0xf0,

    /* U+003C "<" */
    0x19, 0xde, 0xff, 0xbc, 0xe3,

    /* U+003D "=" */
    0xfc, 0x0, 0x3f,

    /* U+003E ">" */
    0xc6, 0x39, 0xef, 0xfb, 0x18,

    /* U+003F "?" */
    0x7b, 0xb0, 0xc7, 0x38, 0x0, 0xc,

    /* U+0040 "@" */
    0xff, 0xb7, 0xc6, 0x6c, 0xdf, 0x80,

    /* U+0041 "A" */
    0x7e, 0xe7, 0xe7, 0xe7, 0xff, 0xe7, 0xe7, 0xe7,

    /* U+0042 "B" */
    0xff, 0xe7, 0xe7, 0xe7, 0xff, 0xe7, 0xe7, 0xff,

    /* U+0043 "C" */
    0x7f, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0x7f,

    /* U+0044 "D" */
    0xfe, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xfe,

    /* U+0045 "E" */
    0xff, 0xe0, 0xe0, 0xe0, 0xfc, 0xe0, 0xe0, 0xff,

    /* U+0046 "F" */
    0xff, 0xe0, 0xe0, 0xe0, 0xfc, 0xe0, 0xe0, 0xe0,

    /* U+0047 "G" */
    0x7f, 0xe0, 0xe0, 0xe7, 0xe7, 0xe7, 0xe7, 0x7f,

    /* U+0048 "H" */
    0xe7, 0xe7, 0xe7, 0xe7, 0xff, 0xe7, 0xe7, 0xe7,

    /* U+0049 "I" */
    0xfc, 0xe3, 0x8e, 0x38, 0xe3, 0xbf,

    /* U+004A "J" */
    0x7, 0x7, 0x7, 0x7, 0x7, 0x7, 0xe7, 0x7f,

    /* U+004B "K" */
    0xe7, 0xe6, 0xe6, 0xee, 0xfe, 0xee, 0xe6, 0xe7,

    /* U+004C "L" */
    0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xff,

    /* U+004D "M" */
    0xe0, 0xfe, 0x3f, 0xef, 0xff, 0xfe, 0xef, 0xc9,
    0xf8, 0x3f, 0x7,

    /* U+004E "N" */
    0xe7, 0xf7, 0xff, 0xff, 0xef, 0xe7, 0xe7, 0xe7,

    /* U+004F "O" */
    0x7e, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0x7e,

    /* U+0050 "P" */
    0xfe, 0xe7, 0xe7, 0xe7, 0xfe, 0xe0, 0xe0, 0xe0,

    /* U+0051 "Q" */
    0x7e, 0xe7, 0xe7, 0xe7, 0xe7, 0xe6, 0xe6, 0x7f,

    /* U+0052 "R" */
    0xfe, 0xe6, 0xe7, 0xe6, 0xfe, 0xe6, 0xe6, 0xe7,

    /* U+0053 "S" */
    0x7f, 0xe0, 0xe0, 0xe0, 0x7f, 0x7, 0x7, 0xfe,

    /* U+0054 "T" */
    0xff, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38,

    /* U+0055 "U" */
    0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0x7e,

    /* U+0056 "V" */
    0xe7, 0xe7, 0xe7, 0xe7, 0xe6, 0xe6, 0xee, 0xfe,

    /* U+0057 "W" */
    0xe0, 0xfc, 0x1f, 0x83, 0xf7, 0x7f, 0xff, 0xff,
    0xfc, 0x7f, 0x7,

    /* U+0058 "X" */
    0xe7, 0xe6, 0x64, 0x7c, 0x7c, 0x7c, 0xe6, 0xe7,

    /* U+0059 "Y" */
    0xe7, 0xe7, 0xe7, 0x66, 0x7c, 0x38, 0x38, 0x38,

    /* U+005A "Z" */
    0xff, 0xf, 0x1e, 0x3c, 0x78, 0x78, 0xf0, 0xff,

    /* U+005B "[" */
    0xff, 0x39, 0xce, 0x73, 0x9f,

    /* U+005C "\\" */
    0x1c, 0xe, 0x7, 0x1f, 0xff, 0xff, 0xfc, 0x70,
    0x38, 0x1c, 0x0,

    /* U+005D "]" */
    0xf9, 0xce, 0x73, 0x9c, 0xff,

    /* U+005E "^" */
    0x73, 0xef, 0xc0,

    /* U+005F "_" */
    0xff,

    /* U+0060 "`" */
    0xf, 0x83, 0xfd, 0xf8, 0xfc, 0xce, 0xcb, 0xd,
    0x86, 0xff, 0x0,

    /* U+0061 "a" */
    0x7e, 0xff, 0x7f, 0x6f, 0xef, 0x7f,

    /* U+0062 "b" */
    0xe0, 0xe0, 0xfe, 0xe7, 0xe7, 0xe7, 0xe7, 0xfe,

    /* U+0063 "c" */
    0x7f, 0xe0, 0xe0, 0xe0, 0xe0, 0x7f,

    /* U+0064 "d" */
    0x7, 0x7, 0x7f, 0xe7, 0xe7, 0xe7, 0xe7, 0x7f,

    /* U+0065 "e" */
    0x7e, 0xef, 0xff, 0xe0, 0xff, 0x7e,

    /* U+0066 "f" */
    0x3b, 0x9c, 0xef, 0xb9, 0xce,

    /* U+0067 "g" */
    0xff, 0xe7, 0xe7, 0xe7, 0x67, 0x7f, 0x7f,

    /* U+0068 "h" */
    0xe0, 0xe0, 0xfe, 0xe6, 0xe7, 0xe7, 0xe7, 0xe7,

    /* U+0069 "i" */
    0xe0, 0xf, 0xff,

    /* U+006A "j" */
    0x38, 0xe, 0x73, 0x9c, 0xe7, 0xf0,

    /* U+006B "k" */
    0xe0, 0xe0, 0xe7, 0xee, 0xfe, 0xee, 0xe6, 0xe7,

    /* U+006C "l" */
    0xff, 0xff, 0xff,

    /* U+006D "m" */
    0xff, 0xdd, 0xdb, 0xbb, 0xf7, 0x7e, 0xef, 0xdd,
    0xc0,

    /* U+006E "n" */
    0xfe, 0xe6, 0xe7, 0xe7, 0xe7, 0xe7,

    /* U+006F "o" */
    0x7e, 0xe7, 0xe7, 0xe7, 0xe7, 0x7e,

    /* U+0070 "p" */
    0xff, 0xe7, 0xe7, 0xe7, 0xe6, 0xfe, 0xe0,

    /* U+0071 "q" */
    0xff, 0xe7, 0xe7, 0xe7, 0x67, 0x7f, 0x7,

    /* U+0072 "r" */
    0x7f, 0x60, 0xe0, 0xe0, 0xe0, 0xe0,

    /* U+0073 "s" */
    0x7f, 0xe0, 0x7e, 0xe, 0xf, 0xfe,

    /* U+0074 "t" */
    0x73, 0xbe, 0xe7, 0x39, 0xc7,

    /* U+0075 "u" */
    0xe7, 0xe7, 0xe7, 0xe7, 0x67, 0x7f,

    /* U+0076 "v" */
    0xe7, 0xe7, 0xe7, 0xe7, 0xee, 0xfe,

    /* U+0077 "w" */
    0xee, 0xfd, 0xdf, 0xbb, 0xf7, 0x76, 0xec, 0xff,
    0x80,

    /* U+0078 "x" */
    0xe7, 0xfe, 0x7c, 0x7c, 0xe6, 0xe7,

    /* U+0079 "y" */
    0xe7, 0xe7, 0xe7, 0xe7, 0x67, 0x7f, 0x7f,

    /* U+007A "z" */
    0xff, 0x1e, 0x3c, 0x3c, 0x78, 0xff,

    /* U+007B "{" */
    0x18, 0x3c, 0x7e, 0xff,

    /* U+007C "|" */
    0xff, 0xff, 0xf0,

    /* U+007D "}" */
    0xff, 0x7e, 0x3c, 0x18,

    /* U+007E "~" */
    0x6e, 0xfb, 0x6e
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 60, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 60, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 108, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 6, .adv_w = 180, .box_w = 11, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 17, .adv_w = 120, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 24, .adv_w = 156, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 108, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 36, .adv_w = 60, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 37, .adv_w = 96, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 43, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 48, .adv_w = 132, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 63, .adv_w = 48, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 64, .adv_w = 108, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 65, .adv_w = 48, .box_w = 2, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 66, .adv_w = 156, .box_w = 9, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 82, .adv_w = 60, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 85, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 101, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 133, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 141, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 149, .adv_w = 48, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 151, .adv_w = 48, .box_w = 3, .box_h = 7, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 154, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 159, .adv_w = 108, .box_w = 6, .box_h = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 162, .adv_w = 120, .box_w = 5, .box_h = 8, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 167, .adv_w = 120, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 120, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 179, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 195, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 203, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 219, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 235, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 120, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 257, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 265, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 273, .adv_w = 180, .box_w = 11, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 300, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 308, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 316, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 324, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 332, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 348, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 356, .adv_w = 180, .box_w = 11, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 367, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 375, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 383, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 391, .adv_w = 84, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 396, .adv_w = 156, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 407, .adv_w = 84, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 412, .adv_w = 120, .box_w = 6, .box_h = 3, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 415, .adv_w = 132, .box_w = 8, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 416, .adv_w = 156, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 427, .adv_w = 132, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 433, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 441, .adv_w = 132, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 447, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 455, .adv_w = 132, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 461, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 466, .adv_w = 132, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 473, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 481, .adv_w = 72, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 484, .adv_w = 84, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 490, .adv_w = 132, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 498, .adv_w = 60, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 501, .adv_w = 180, .box_w = 11, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 510, .adv_w = 132, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 516, .adv_w = 132, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 522, .adv_w = 132, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 529, .adv_w = 132, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 536, .adv_w = 132, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 542, .adv_w = 132, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 548, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 553, .adv_w = 132, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 559, .adv_w = 132, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 565, .adv_w = 180, .box_w = 11, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 574, .adv_w = 132, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 580, .adv_w = 132, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 587, .adv_w = 132, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 593, .adv_w = 132, .box_w = 8, .box_h = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 597, .adv_w = 36, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 600, .adv_w = 132, .box_w = 8, .box_h = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 604, .adv_w = 132, .box_w = 8, .box_h = 3, .ofs_x = 0, .ofs_y = 2}
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
const lv_font_t TenFatGuys_12 = {
#else
lv_font_t TenFatGuys_12 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 11,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if TENFATGUYS_12*/

