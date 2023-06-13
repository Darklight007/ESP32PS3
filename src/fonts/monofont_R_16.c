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

#ifndef MONOFONT_R_16
#define MONOFONT_R_16 1
#endif

#if MONOFONT_R_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xfc, 0xc,

    /* U+0022 "\"" */
    0xde, 0xf7, 0xb0,

    /* U+0023 "#" */
    0x12, 0x12, 0x16, 0x7f, 0x24, 0x24, 0x24, 0xfe,
    0x68, 0x48, 0x48,

    /* U+0024 "$" */
    0x30, 0xc7, 0xf3, 0xc3, 0x8f, 0x1e, 0x3c, 0x3c,
    0xf3, 0xf8, 0xc3, 0x0,

    /* U+0025 "%" */
    0xe1, 0xa2, 0xa4, 0xa4, 0xa8, 0xff, 0x15, 0x25,
    0x25, 0x45, 0x87,

    /* U+0026 "&" */
    0x38, 0x6c, 0x6c, 0x6c, 0x38, 0x70, 0xf9, 0xcf,
    0xce, 0xce, 0x7b,

    /* U+0027 "'" */
    0xff,

    /* U+0028 "(" */
    0x22, 0x66, 0xcc, 0xcc, 0xcc, 0xcc, 0x66, 0x23,

    /* U+0029 ")" */
    0x44, 0x66, 0x23, 0x33, 0x33, 0x33, 0x66, 0x4c,

    /* U+002A "*" */
    0x18, 0x18, 0xdb, 0xff, 0x18, 0x34, 0x66,

    /* U+002B "+" */
    0x18, 0x30, 0x67, 0xf1, 0x83, 0x6, 0x0,

    /* U+002C "," */
    0xfa,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0xc0,

    /* U+002F "/" */
    0x3, 0x2, 0x6, 0x4, 0x4, 0xc, 0x8, 0x18,
    0x10, 0x30, 0x20, 0x20, 0x60, 0x40, 0xc0,

    /* U+0030 "0" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0xcf, 0x37,
    0x80,

    /* U+0031 "1" */
    0x3, 0xff, 0x33, 0x33, 0x33, 0x33,

    /* U+0032 "2" */
    0x7b, 0x3c, 0xc3, 0x1c, 0x73, 0x8e, 0x71, 0x8f,
    0xc0,

    /* U+0033 "3" */
    0x7d, 0x8c, 0x18, 0x31, 0xc1, 0xc1, 0x83, 0xc7,
    0x99, 0xe0,

    /* U+0034 "4" */
    0x1c, 0x1c, 0x3c, 0x3c, 0x6c, 0x6c, 0xcc, 0xff,
    0xc, 0xc, 0xc,

    /* U+0035 "5" */
    0xff, 0xc, 0x30, 0xfb, 0x30, 0xc3, 0xcf, 0x37,
    0x80,

    /* U+0036 "6" */
    0x1c, 0x30, 0xc1, 0x87, 0xdd, 0xf1, 0xe3, 0xc7,
    0xd9, 0xe0,

    /* U+0037 "7" */
    0xfe, 0x18, 0x30, 0xe1, 0x83, 0xe, 0x18, 0x70,
    0xe1, 0x80,

    /* U+0038 "8" */
    0x7d, 0x8f, 0x1e, 0x37, 0xdd, 0xf1, 0xe3, 0xc6,
    0xd8, 0xf0,

    /* U+0039 "9" */
    0x7d, 0x8f, 0x1e, 0x3c, 0x6f, 0xc7, 0xc, 0x38,
    0x61, 0xc0,

    /* U+003A ":" */
    0xc0, 0xc,

    /* U+003B ";" */
    0xc2, 0xea,

    /* U+003C "<" */
    0x2, 0x1c, 0xe7, 0xc, 0xe, 0x7, 0x3, 0x0,

    /* U+003D "=" */
    0xfe, 0x0, 0x7, 0xf0,

    /* U+003E ">" */
    0x81, 0xc0, 0xe0, 0x70, 0x63, 0x9c, 0x60, 0x0,

    /* U+003F "?" */
    0x7b, 0x3c, 0xc3, 0x1c, 0xe3, 0xc, 0x0, 0x3,
    0x0,

    /* U+0040 "@" */
    0x3e, 0x63, 0xc3, 0xdf, 0xf3, 0xf3, 0xf3, 0xdf,
    0xc0, 0xc0, 0x60, 0x3f,

    /* U+0041 "A" */
    0x18, 0x18, 0x3c, 0x3c, 0x3c, 0x3c, 0x6e, 0x66,
    0x7e, 0x66, 0xc7,

    /* U+0042 "B" */
    0xfb, 0x3c, 0xf3, 0xcf, 0xec, 0xf3, 0xcf, 0x3f,
    0x80,

    /* U+0043 "C" */
    0x79, 0x9b, 0x6, 0xc, 0x18, 0x30, 0x60, 0xc1,
    0x99, 0xe0,

    /* U+0044 "D" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0xcf, 0x3f,
    0x80,

    /* U+0045 "E" */
    0xff, 0xc, 0x30, 0xc3, 0xfc, 0x30, 0xc3, 0xf,
    0xc0,

    /* U+0046 "F" */
    0xff, 0x83, 0x6, 0xc, 0x1f, 0xb0, 0x60, 0xc1,
    0x83, 0x0,

    /* U+0047 "G" */
    0x7b, 0x3c, 0xf0, 0xc3, 0x7c, 0xf3, 0xcf, 0x37,
    0xc0,

    /* U+0048 "H" */
    0xcf, 0x3c, 0xf3, 0xcf, 0xfc, 0xf3, 0xcf, 0x3c,
    0xc0,

    /* U+0049 "I" */
    0xf9, 0x8c, 0x63, 0x18, 0xc6, 0x31, 0xbe,

    /* U+004A "J" */
    0x1c, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x2f,
    0x80,

    /* U+004B "K" */
    0xcd, 0x9b, 0x66, 0xcf, 0x1e, 0x3c, 0x6c, 0xdd,
    0x9b, 0x38,

    /* U+004C "L" */
    0xc1, 0x83, 0x6, 0xc, 0x18, 0x30, 0x60, 0xc1,
    0x83, 0xf8,

    /* U+004D "M" */
    0xc7, 0xdf, 0xbf, 0x7f, 0xff, 0xf5, 0xeb, 0xd7,
    0x8f, 0x18,

    /* U+004E "N" */
    0xef, 0xbe, 0xfb, 0xff, 0xfd, 0xf7, 0xdf, 0x7c,
    0xc0,

    /* U+004F "O" */
    0x7d, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7,
    0x8d, 0xf0,

    /* U+0050 "P" */
    0xf9, 0x9b, 0x1e, 0x3c, 0x79, 0xff, 0x60, 0xc1,
    0x83, 0x0,

    /* U+0051 "Q" */
    0x7d, 0x8b, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7,
    0x8d, 0xf0, 0x60, 0xc0,

    /* U+0052 "R" */
    0xfd, 0x8f, 0x1e, 0x3c, 0xff, 0xbe, 0x6e, 0xdd,
    0x9f, 0x18,

    /* U+0053 "S" */
    0x7b, 0x3c, 0x38, 0xf1, 0xe3, 0xc7, 0xcf, 0x37,
    0x80,

    /* U+0054 "T" */
    0xfe, 0x30, 0x60, 0xc1, 0x83, 0x6, 0xc, 0x18,
    0x30, 0x60,

    /* U+0055 "U" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7,
    0x8d, 0xf0,

    /* U+0056 "V" */
    0xe3, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x3c, 0x3c,
    0x3c, 0x18, 0x18,

    /* U+0057 "W" */
    0xc3, 0xc3, 0xc3, 0xee, 0x7e, 0x7e, 0x7e, 0x7e,
    0x7e, 0x66, 0x66,

    /* U+0058 "X" */
    0x66, 0x66, 0x7e, 0x3c, 0x3c, 0x18, 0x3c, 0x3c,
    0x7e, 0x66, 0x66,

    /* U+0059 "Y" */
    0xe7, 0x66, 0x76, 0x3c, 0x3c, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18,

    /* U+005A "Z" */
    0xfc, 0x18, 0x70, 0xc3, 0x87, 0xc, 0x38, 0x61,
    0xc3, 0xf8,

    /* U+005B "[" */
    0xfc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcf,

    /* U+005C "\\" */
    0xc0, 0x40, 0x60, 0x20, 0x20, 0x30, 0x10, 0x18,
    0x8, 0xc, 0x4, 0x4, 0x6, 0x2, 0x3,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3f,

    /* U+005E "^" */
    0x31, 0xe4, 0xb3, 0x84,

    /* U+005F "_" */
    0xff,

    /* U+0060 "`" */
    0xe6, 0x30,

    /* U+0061 "a" */
    0x7c, 0x8c, 0x1b, 0xfc, 0x78, 0xf3, 0xbf,

    /* U+0062 "b" */
    0xc1, 0x83, 0x7, 0xee, 0xf8, 0xf1, 0xe3, 0xc7,
    0xcf, 0xf0,

    /* U+0063 "c" */
    0x3c, 0xcb, 0x6, 0xc, 0x18, 0x19, 0x1e,

    /* U+0064 "d" */
    0x6, 0xc, 0x1b, 0xfc, 0xf8, 0xf1, 0xe3, 0xc7,
    0x9d, 0xf8,

    /* U+0065 "e" */
    0x3c, 0x8f, 0x1f, 0xfc, 0x18, 0x19, 0x9e,

    /* U+0066 "f" */
    0x1e, 0x60, 0xc7, 0xc3, 0x6, 0xc, 0x18, 0x30,
    0x60, 0xc0,

    /* U+0067 "g" */
    0x7f, 0xdf, 0x1e, 0x3c, 0x78, 0xf3, 0xbf, 0x7,
    0x8f, 0xf0,

    /* U+0068 "h" */
    0xc1, 0x83, 0x7, 0xee, 0x78, 0xf1, 0xe3, 0xc7,
    0x8f, 0x18,

    /* U+0069 "i" */
    0x60, 0x1, 0xc6, 0x31, 0x8c, 0x63, 0x3e,

    /* U+006A "j" */
    0x18, 0x0, 0xf1, 0x8c, 0x63, 0x18, 0xc6, 0x31,
    0xf8,

    /* U+006B "k" */
    0xc1, 0x83, 0x6, 0x6d, 0x9b, 0x3c, 0x78, 0xd9,
    0xbb, 0x30,

    /* U+006C "l" */
    0xf3, 0x33, 0x33, 0x33, 0x33, 0x30,

    /* U+006D "m" */
    0xff, 0xbf, 0x7e, 0xfd, 0xfb, 0xf7, 0xef,

    /* U+006E "n" */
    0xfd, 0xcf, 0x1e, 0x3c, 0x78, 0xf1, 0xe3,

    /* U+006F "o" */
    0x38, 0xdb, 0x1e, 0x3c, 0x78, 0xdb, 0x1c,

    /* U+0070 "p" */
    0xfd, 0xdf, 0x1e, 0x3c, 0x78, 0xf9, 0xfe, 0xc1,
    0x83, 0x0,

    /* U+0071 "q" */
    0x7f, 0x9f, 0x1e, 0x3c, 0x78, 0xf3, 0xbf, 0x6,
    0xc, 0x18,

    /* U+0072 "r" */
    0xff, 0xbc, 0x30, 0xc3, 0xc, 0x30,

    /* U+0073 "s" */
    0x7b, 0x3e, 0x3e, 0x3c, 0x7c, 0xde,

    /* U+0074 "t" */
    0x30, 0xcf, 0x8c, 0x30, 0xc3, 0xc, 0x30, 0x70,

    /* U+0075 "u" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xbe,

    /* U+0076 "v" */
    0x62, 0x66, 0x66, 0x76, 0x3c, 0x3c, 0x3c, 0x18,

    /* U+0077 "w" */
    0xc3, 0xf3, 0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x24,

    /* U+0078 "x" */
    0xcd, 0xe7, 0x8c, 0x79, 0xec, 0xf3,

    /* U+0079 "y" */
    0x62, 0x66, 0x66, 0x36, 0x3c, 0x3c, 0x3c, 0x18,
    0x18, 0x18, 0x30,

    /* U+007A "z" */
    0xfc, 0x63, 0x8c, 0x71, 0x8e, 0x3f,

    /* U+007B "{" */
    0x19, 0x8c, 0x63, 0x18, 0xc6, 0xc1, 0x8c, 0x63,
    0x18, 0xc3,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xfc,

    /* U+007D "}" */
    0xc6, 0x66, 0x66, 0x66, 0x16, 0x66, 0x66, 0x6c,

    /* U+007E "~" */
    0xf0, 0x3c
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 128, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 128, .box_w = 2, .box_h = 11, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 128, .box_w = 5, .box_h = 4, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 7, .adv_w = 128, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 18, .adv_w = 128, .box_w = 6, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 30, .adv_w = 128, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 41, .adv_w = 128, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 52, .adv_w = 128, .box_w = 2, .box_h = 4, .ofs_x = 3, .ofs_y = 7},
    {.bitmap_index = 53, .adv_w = 128, .box_w = 4, .box_h = 16, .ofs_x = 3, .ofs_y = -3},
    {.bitmap_index = 61, .adv_w = 128, .box_w = 4, .box_h = 16, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 69, .adv_w = 128, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 76, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 83, .adv_w = 128, .box_w = 2, .box_h = 4, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 84, .adv_w = 128, .box_w = 5, .box_h = 1, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 85, .adv_w = 128, .box_w = 2, .box_h = 1, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 128, .box_w = 8, .box_h = 15, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 101, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 110, .adv_w = 128, .box_w = 4, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 116, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 135, .adv_w = 128, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 155, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 165, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 185, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 195, .adv_w = 128, .box_w = 2, .box_h = 7, .ofs_x = 3, .ofs_y = 2},
    {.bitmap_index = 197, .adv_w = 128, .box_w = 2, .box_h = 8, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 199, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 207, .adv_w = 128, .box_w = 7, .box_h = 4, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 211, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 219, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 228, .adv_w = 128, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 240, .adv_w = 128, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 251, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 260, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 270, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 288, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 298, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 307, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 316, .adv_w = 128, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 323, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 332, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 342, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 352, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 362, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 371, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 391, .adv_w = 128, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 403, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 413, .adv_w = 128, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 422, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 432, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 442, .adv_w = 128, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 453, .adv_w = 128, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 464, .adv_w = 128, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 475, .adv_w = 128, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 486, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 496, .adv_w = 128, .box_w = 4, .box_h = 16, .ofs_x = 4, .ofs_y = -3},
    {.bitmap_index = 504, .adv_w = 128, .box_w = 8, .box_h = 15, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 519, .adv_w = 128, .box_w = 4, .box_h = 16, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 527, .adv_w = 128, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 531, .adv_w = 128, .box_w = 8, .box_h = 1, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 532, .adv_w = 128, .box_w = 4, .box_h = 3, .ofs_x = 2, .ofs_y = 9},
    {.bitmap_index = 534, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 541, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 551, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 558, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 568, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 575, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 585, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 595, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 605, .adv_w = 128, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 612, .adv_w = 128, .box_w = 5, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 621, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 631, .adv_w = 128, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 637, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 644, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 651, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 658, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 668, .adv_w = 128, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 678, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 684, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 690, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 698, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 705, .adv_w = 128, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 713, .adv_w = 128, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 721, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 727, .adv_w = 128, .box_w = 8, .box_h = 11, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 738, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 744, .adv_w = 128, .box_w = 5, .box_h = 16, .ofs_x = 2, .ofs_y = -3},
    {.bitmap_index = 754, .adv_w = 128, .box_w = 2, .box_h = 15, .ofs_x = 3, .ofs_y = -3},
    {.bitmap_index = 758, .adv_w = 128, .box_w = 4, .box_h = 16, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 766, .adv_w = 128, .box_w = 7, .box_h = 2, .ofs_x = 1, .ofs_y = 5}
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
const lv_font_t monofont_R_16 = {
#else
lv_font_t monofont_R_16 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if MONOFONT_R_16*/

