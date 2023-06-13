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

#ifndef SULLYVERGE_12
#define SULLYVERGE_12 1
#endif

#if SULLYVERGE_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfc, 0x30,

    /* U+0022 "\"" */
    0xda, 0x64,

    /* U+0023 "#" */
    0x33, 0xf3, 0x3f, 0x30,

    /* U+0024 "$" */
    0x44, 0x4f, 0xc4, 0x3b, 0x64,

    /* U+0025 "%" */
    0x7f, 0xb9, 0xc0, 0xe7, 0x79, 0x80,

    /* U+0026 "&" */
    0xf9, 0x9d, 0xb6, 0xd9, 0xc0,

    /* U+0027 "'" */
    0xd8,

    /* U+0028 "(" */
    0x7e, 0xcc, 0x63,

    /* U+0029 ")" */
    0xe7, 0x33, 0x6c,

    /* U+002A "*" */
    0x32, 0xdf, 0xff, 0xb4, 0xc0,

    /* U+002B "+" */
    0x33, 0xf3, 0x0,

    /* U+002C "," */
    0xf0,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0xc0,

    /* U+002F "/" */
    0x19, 0x8c, 0x4c, 0x63, 0x0,

    /* U+0030 "0" */
    0xfe, 0xff, 0xbd, 0xb8,

    /* U+0031 "1" */
    0x7f, 0x33, 0x33,

    /* U+0032 "2" */
    0xf8, 0xc4, 0xcc, 0x7c,

    /* U+0033 "3" */
    0xfb, 0x9e, 0x39, 0xb8,

    /* U+0034 "4" */
    0x39, 0x69, 0xbf, 0x18, 0x60,

    /* U+0035 "5" */
    0xfe, 0x3e, 0x39, 0xb8,

    /* U+0036 "6" */
    0x37, 0x77, 0xb5, 0x38,

    /* U+0037 "7" */
    0xf8, 0x46, 0x63, 0x18,

    /* U+0038 "8" */
    0xfe, 0xdd, 0xbd, 0xb8,

    /* U+0039 "9" */
    0xfe, 0xf6, 0xe3, 0x30,

    /* U+003A ":" */
    0xc0, 0xc0,

    /* U+003B ";" */
    0xc0, 0xf0,

    /* U+003C "<" */
    0x3a, 0xb0, 0x83, 0xc,

    /* U+003D "=" */
    0xff, 0xc0,

    /* U+003E ">" */
    0xc3, 0x6, 0x64, 0x60,

    /* U+003F "?" */
    0xf8, 0x46, 0x60, 0x18,

    /* U+0040 "@" */
    0x7d, 0xaf, 0xaf, 0x5d, 0x58, 0xdf, 0x0,

    /* U+0041 "A" */
    0x76, 0xff, 0xbd, 0xec,

    /* U+0042 "B" */
    0xfe, 0xfd, 0xbd, 0xf8,

    /* U+0043 "C" */
    0xfe, 0x31, 0x8c, 0xb8,

    /* U+0044 "D" */
    0xfe, 0xf7, 0xbd, 0xf8,

    /* U+0045 "E" */
    0xfc, 0xfc, 0xcf,

    /* U+0046 "F" */
    0xfc, 0xfc, 0xcc,

    /* U+0047 "G" */
    0xfe, 0x3f, 0xbd, 0xb8,

    /* U+0048 "H" */
    0xde, 0xff, 0xbd, 0xec,

    /* U+0049 "I" */
    0xff, 0xf0,

    /* U+004A "J" */
    0x18, 0xc7, 0xbd, 0xb8,

    /* U+004B "K" */
    0xdf, 0x6e, 0x3c, 0xdb, 0x30,

    /* U+004C "L" */
    0xcc, 0xcc, 0xcf,

    /* U+004D "M" */
    0xc7, 0x8e, 0xed, 0xd8, 0x30, 0x40,

    /* U+004E "N" */
    0xce, 0x6b, 0x79, 0xc4,

    /* U+004F "O" */
    0xfe, 0xf7, 0xbd, 0xb8,

    /* U+0050 "P" */
    0xfe, 0xfd, 0x8c, 0x60,

    /* U+0051 "Q" */
    0xfe, 0xf7, 0xbd, 0xb8, 0x60,

    /* U+0052 "R" */
    0xfe, 0xfd, 0xbd, 0xec,

    /* U+0053 "S" */
    0xfc, 0x63, 0xb6,

    /* U+0054 "T" */
    0xfc, 0xc3, 0xc, 0x30, 0xc0,

    /* U+0055 "U" */
    0xde, 0xf7, 0xbd, 0xb8,

    /* U+0056 "V" */
    0xde, 0xf7, 0xbd, 0xf8,

    /* U+0057 "W" */
    0xb7, 0x6e, 0xdd, 0xbb, 0x7f, 0x80,

    /* U+0058 "X" */
    0xde, 0xff, 0xbd, 0xec,

    /* U+0059 "Y" */
    0xcf, 0x3c, 0xde, 0x20, 0x80,

    /* U+005A "Z" */
    0xf8, 0xcc, 0xcc, 0x7c,

    /* U+005B "[" */
    0xfc, 0xcc, 0xcc, 0xcf,

    /* U+005C "\\" */
    0xc6, 0x30, 0x43, 0x18, 0x60,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0x3f,

    /* U+005E "^" */
    0x76, 0xc0,

    /* U+005F "_" */
    0xf8,

    /* U+0060 "`" */
    0xe4,

    /* U+0061 "a" */
    0x77, 0xf7, 0xb7, 0x80,

    /* U+0062 "b" */
    0xc7, 0xb7, 0xbd, 0xf8,

    /* U+0063 "c" */
    0x7d, 0xcd, 0x70,

    /* U+0064 "d" */
    0x1b, 0xf7, 0xbd, 0xbc,

    /* U+0065 "e" */
    0x76, 0xff, 0x97, 0x0,

    /* U+0066 "f" */
    0x3f, 0x98, 0xc6, 0x30,

    /* U+0067 "g" */
    0xfe, 0xf6, 0xf9, 0xb8,

    /* U+0068 "h" */
    0xc7, 0xb5, 0xbd, 0xec,

    /* U+0069 "i" */
    0xff, 0xf0,

    /* U+006A "j" */
    0x33, 0x33, 0x3b, 0xe0,

    /* U+006B "k" */
    0xc6, 0xfd, 0xcd, 0x6c,

    /* U+006C "l" */
    0xff, 0xf0,

    /* U+006D "m" */
    0xfd, 0x6a, 0xdd, 0xbb, 0x60,

    /* U+006E "n" */
    0xf6, 0xb7, 0xbd, 0x80,

    /* U+006F "o" */
    0x76, 0xf7, 0xb7, 0x0,

    /* U+0070 "p" */
    0xf6, 0xf7, 0xbf, 0x60,

    /* U+0071 "q" */
    0x7e, 0xf7, 0xb7, 0x8c,

    /* U+0072 "r" */
    0xde, 0xcc, 0xc0,

    /* U+0073 "s" */
    0x6d, 0x6b, 0x60,

    /* U+0074 "t" */
    0x6f, 0x66, 0x63,

    /* U+0075 "u" */
    0xde, 0xf6, 0xb7, 0x80,

    /* U+0076 "v" */
    0xde, 0xf7, 0xae, 0x0,

    /* U+0077 "w" */
    0xdb, 0xdb, 0xdb, 0xda, 0xfc,

    /* U+0078 "x" */
    0xde, 0xdd, 0xbd, 0x80,

    /* U+0079 "y" */
    0xde, 0xf6, 0xb7, 0xcd, 0xc0,

    /* U+007A "z" */
    0xf8, 0xd9, 0x8f, 0x80,

    /* U+007B "{" */
    0x7b, 0x30, 0xc6, 0x1c,

    /* U+007C "|" */
    0xff, 0xf0,

    /* U+007D "}" */
    0xf1, 0x86, 0x63, 0x70,

    /* U+007E "~" */
    0x6f, 0x60
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 60, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 36, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 72, .box_w = 5, .box_h = 3, .ofs_x = -1, .ofs_y = 3},
    {.bitmap_index = 5, .adv_w = 96, .box_w = 6, .box_h = 5, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 9, .adv_w = 72, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 14, .adv_w = 120, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 20, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 25, .adv_w = 36, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 26, .adv_w = 60, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 29, .adv_w = 60, .box_w = 4, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 32, .adv_w = 108, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 37, .adv_w = 84, .box_w = 6, .box_h = 3, .ofs_x = -1, .ofs_y = 2},
    {.bitmap_index = 40, .adv_w = 36, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 41, .adv_w = 84, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 42, .adv_w = 36, .box_w = 2, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 43, .adv_w = 84, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 48, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 52, .adv_w = 60, .box_w = 4, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 59, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 63, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 72, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 76, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 80, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 84, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 88, .adv_w = 36, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 90, .adv_w = 36, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 92, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 96, .adv_w = 84, .box_w = 5, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 98, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 106, .adv_w = 120, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 113, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 72, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 72, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 135, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 143, .adv_w = 36, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 145, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 149, .adv_w = 96, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 72, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 120, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 96, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 167, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 171, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 84, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 180, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 184, .adv_w = 72, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 84, .box_w = 6, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 192, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 200, .adv_w = 120, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 210, .adv_w = 84, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 215, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 219, .adv_w = 72, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 223, .adv_w = 84, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 228, .adv_w = 72, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 232, .adv_w = 84, .box_w = 5, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 234, .adv_w = 84, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 235, .adv_w = 36, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 236, .adv_w = 84, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 240, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 244, .adv_w = 72, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 251, .adv_w = 84, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 255, .adv_w = 72, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 259, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 263, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 267, .adv_w = 36, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 269, .adv_w = 72, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 273, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 36, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 120, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 84, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 288, .adv_w = 84, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 296, .adv_w = 84, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 300, .adv_w = 72, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 303, .adv_w = 72, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 60, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 84, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 313, .adv_w = 84, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 317, .adv_w = 132, .box_w = 8, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 84, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 326, .adv_w = 84, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 331, .adv_w = 84, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 335, .adv_w = 72, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 339, .adv_w = 48, .box_w = 2, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 341, .adv_w = 72, .box_w = 5, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 345, .adv_w = 108, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 4}
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
const lv_font_t SullyVerge_12 = {
#else
lv_font_t SullyVerge_12 = {
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



#endif /*#if SULLYVERGE_12*/

