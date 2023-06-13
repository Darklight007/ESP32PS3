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

#ifndef TAURI_R_16
#define TAURI_R_16 1
#endif

#if TAURI_R_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0x6d, 0xb6, 0xdb, 0x3, 0xfe,

    /* U+0022 "\"" */
    0xde, 0xf7, 0xbd, 0xec,

    /* U+0023 "#" */
    0x4, 0x60, 0x23, 0x3, 0x10, 0x18, 0x80, 0xcc,
    0x3f, 0xfc, 0x63, 0x3, 0x10, 0x19, 0x87, 0xff,
    0x84, 0x60, 0x62, 0x3, 0x10, 0x19, 0x80,

    /* U+0024 "$" */
    0x30, 0xc3, 0x1e, 0xc3, 0xc, 0x18, 0x78, 0x60,
    0xc3, 0x8f, 0xe7, 0xc, 0x30,

    /* U+0025 "%" */
    0x78, 0x61, 0x98, 0xc3, 0x33, 0x6, 0x66, 0xc,
    0xc8, 0x19, 0xb0, 0x1e, 0x6f, 0x1, 0xb3, 0x3,
    0x66, 0xc, 0xcc, 0x19, 0x98, 0x63, 0x30, 0xc3,
    0xc0,

    /* U+0026 "&" */
    0x3e, 0x18, 0x86, 0x1, 0x86, 0x31, 0x8f, 0xf6,
    0x1b, 0x6, 0xc1, 0xb0, 0x66, 0x18, 0xff,

    /* U+0027 "'" */
    0xff, 0xf0,

    /* U+0028 "(" */
    0x19, 0xdc, 0xcc, 0x63, 0x18, 0xc6, 0x30, 0xc6,
    0x18, 0x61,

    /* U+0029 ")" */
    0xc7, 0x1c, 0x63, 0x8c, 0x63, 0x18, 0xc6, 0x63,
    0x33, 0x10,

    /* U+002A "*" */
    0x18, 0x99, 0xff, 0x3c, 0xff, 0x99, 0x18,

    /* U+002B "+" */
    0x18, 0x30, 0x67, 0xf1, 0x83, 0x6, 0x0,

    /* U+002C "," */
    0xff, 0xbd, 0x0,

    /* U+002D "-" */
    0xfc,

    /* U+002E "." */
    0xff, 0x80,

    /* U+002F "/" */
    0x3, 0x3, 0x2, 0x6, 0x4, 0xc, 0xc, 0x18,
    0x18, 0x10, 0x30, 0x20, 0x60, 0x60, 0x40, 0xc0,

    /* U+0030 "0" */
    0x3e, 0x31, 0x90, 0xd8, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0xc1, 0xb1, 0x98, 0xc7, 0xc0,

    /* U+0031 "1" */
    0x3f, 0xb3, 0x33, 0x33, 0x33, 0x33,

    /* U+0032 "2" */
    0xf9, 0x18, 0x18, 0x30, 0x61, 0x87, 0xc, 0x30,
    0xc3, 0xff, 0xf0,

    /* U+0033 "3" */
    0xfc, 0x86, 0x6, 0x6, 0xc, 0x38, 0x3e, 0x7,
    0x3, 0x3, 0x86, 0xfc,

    /* U+0034 "4" */
    0x6, 0x7, 0x3, 0x82, 0xc3, 0x61, 0x31, 0x99,
    0x8c, 0xff, 0x83, 0x1, 0x80, 0xc0,

    /* U+0035 "5" */
    0x7e, 0x60, 0x60, 0x60, 0x7c, 0x46, 0x3, 0x3,
    0x3, 0x3, 0x86, 0xfc,

    /* U+0036 "6" */
    0x1e, 0x60, 0x60, 0xc0, 0xdc, 0xe6, 0xc3, 0xc3,
    0xc3, 0x43, 0x66, 0x3c,

    /* U+0037 "7" */
    0xff, 0x6, 0x6, 0xe, 0xc, 0xc, 0x18, 0x18,
    0x18, 0x30, 0x30, 0x30,

    /* U+0038 "8" */
    0x3e, 0x63, 0x63, 0x63, 0x3e, 0x7e, 0xe7, 0xc3,
    0xc3, 0xc3, 0x66, 0x3c,

    /* U+0039 "9" */
    0x3c, 0x66, 0xc2, 0xc3, 0xc3, 0xe7, 0x7b, 0x3,
    0x3, 0x6, 0x46, 0x78,

    /* U+003A ":" */
    0xff, 0x80, 0x3f, 0xe0,

    /* U+003B ";" */
    0xff, 0x80, 0x3f, 0xef, 0x40,

    /* U+003C "<" */
    0x6, 0xc, 0x38, 0x70, 0xc0, 0x70, 0x38, 0xc,
    0x6,

    /* U+003D "=" */
    0xfe, 0x0, 0x0, 0xf, 0xe0,

    /* U+003E ">" */
    0xe0, 0x60, 0x60, 0x60, 0x61, 0x86, 0x18, 0xe0,

    /* U+003F "?" */
    0xf8, 0x30, 0xc3, 0x19, 0xc6, 0x18, 0x0, 0x7,
    0x1c, 0x70,

    /* U+0040 "@" */
    0x7, 0xf0, 0x38, 0x30, 0xc0, 0x33, 0x1f, 0x36,
    0x66, 0x79, 0x8c, 0xf3, 0x19, 0xe6, 0x23, 0xcc,
    0x4d, 0x99, 0x99, 0x9d, 0xe3, 0x0, 0x3, 0x82,
    0x1, 0xf8, 0x0,

    /* U+0041 "A" */
    0x1c, 0x7, 0x81, 0xe0, 0x58, 0x33, 0xc, 0xc3,
    0x31, 0x8c, 0x7f, 0x98, 0x66, 0x1b, 0x3,

    /* U+0042 "B" */
    0xfc, 0xce, 0xc6, 0xc6, 0xce, 0xfc, 0xc6, 0xc3,
    0xc3, 0xc3, 0xc6, 0xfc,

    /* U+0043 "C" */
    0x1f, 0x71, 0x60, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
    0xc0, 0x60, 0x71, 0x1f,

    /* U+0044 "D" */
    0xf8, 0xc6, 0xc6, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0xc3, 0xc6, 0xce, 0xf8,

    /* U+0045 "E" */
    0xff, 0x83, 0x6, 0xc, 0x1f, 0xb0, 0x60, 0xc1,
    0x83, 0x7, 0xf0,

    /* U+0046 "F" */
    0xff, 0x83, 0x6, 0xc, 0x1f, 0xb0, 0x60, 0xc1,
    0x83, 0x6, 0x0,

    /* U+0047 "G" */
    0x3e, 0x62, 0x60, 0xc0, 0xc0, 0xcf, 0xc3, 0xc3,
    0xc3, 0x63, 0x63, 0x1f,

    /* U+0048 "H" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3,
    0xc3, 0xc3, 0xc3, 0xc3,

    /* U+0049 "I" */
    0xff, 0xff, 0xff,

    /* U+004A "J" */
    0x33, 0x33, 0x33, 0x33, 0x33, 0x3e,

    /* U+004B "K" */
    0xc2, 0xc6, 0xcc, 0xcc, 0xd8, 0xf8, 0xf8, 0xec,
    0xcc, 0xc6, 0xc6, 0xc3,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xc,
    0x3f,

    /* U+004D "M" */
    0xe0, 0xfc, 0x1f, 0xc7, 0xe8, 0xbd, 0xb7, 0xb6,
    0xf3, 0x9e, 0x73, 0xc4, 0x78, 0xf, 0x1, 0xe0,
    0x30,

    /* U+004E "N" */
    0xc3, 0xe3, 0xe3, 0xf3, 0xf3, 0xdb, 0xdb, 0xcf,
    0xcf, 0xc7, 0xc7, 0xc3,

    /* U+004F "O" */
    0x3e, 0x31, 0x98, 0xd8, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0xc1, 0xb1, 0x98, 0xc7, 0xc0,

    /* U+0050 "P" */
    0xfc, 0xc6, 0xc3, 0xc3, 0xc3, 0xc6, 0xfc, 0xc0,
    0xc0, 0xc0, 0xc0, 0xc0,

    /* U+0051 "Q" */
    0x3e, 0x31, 0x98, 0xd8, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0xc1, 0xb1, 0x9f, 0xc7, 0xc1, 0x80, 0xc0,
    0x38,

    /* U+0052 "R" */
    0xfc, 0xc6, 0xc3, 0xc3, 0xc3, 0xc6, 0xfc, 0xcc,
    0xcc, 0xc6, 0xc6, 0xc3,

    /* U+0053 "S" */
    0x7b, 0xc, 0x30, 0xe1, 0xc3, 0x87, 0xc, 0x38,
    0xfe,

    /* U+0054 "T" */
    0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18,

    /* U+0055 "U" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0xc3, 0xc3, 0x66, 0x3c,

    /* U+0056 "V" */
    0xc1, 0xb0, 0xd8, 0x6c, 0x66, 0x31, 0x98, 0xcc,
    0x64, 0x16, 0xf, 0x7, 0x83, 0x80,

    /* U+0057 "W" */
    0xe3, 0x8c, 0xc7, 0x19, 0x8e, 0x33, 0x14, 0x66,
    0x28, 0xcc, 0xd9, 0xd, 0xb6, 0x1b, 0x6c, 0x34,
    0x58, 0x78, 0xf0, 0x71, 0xc0, 0xe3, 0x80,

    /* U+0058 "X" */
    0x61, 0x98, 0xc3, 0x30, 0xd8, 0x1e, 0x3, 0x1,
    0xc0, 0x78, 0x36, 0xc, 0xc6, 0x1b, 0x86,

    /* U+0059 "Y" */
    0x61, 0xb0, 0xcc, 0xc6, 0x61, 0xe0, 0xf0, 0x30,
    0x18, 0xc, 0x6, 0x3, 0x1, 0x80,

    /* U+005A "Z" */
    0x7f, 0x7, 0x6, 0xe, 0xc, 0x1c, 0x18, 0x38,
    0x30, 0x30, 0x60, 0x7f,

    /* U+005B "[" */
    0xfe, 0x31, 0x8c, 0x63, 0x18, 0xc6, 0x31, 0x8c,
    0x63, 0x1f,

    /* U+005C "\\" */
    0xc0, 0x40, 0x60, 0x60, 0x20, 0x30, 0x10, 0x18,
    0x18, 0xc, 0xc, 0x4, 0x6, 0x2, 0x3, 0x3,

    /* U+005D "]" */
    0xf8, 0xc6, 0x31, 0x8c, 0x63, 0x18, 0xc6, 0x31,
    0x8c, 0x7f,

    /* U+005E "^" */
    0x18, 0x1c, 0x3c, 0x34, 0x24, 0x66, 0x66, 0x42,
    0xc3,

    /* U+005F "_" */
    0xff, 0xf0,

    /* U+0060 "`" */
    0xc6, 0x63,

    /* U+0061 "a" */
    0x7c, 0x9c, 0x18, 0x37, 0xf8, 0xf1, 0xe3, 0x7e,

    /* U+0062 "b" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc6, 0xc3, 0xc3,
    0xc3, 0xc3, 0xc3, 0xc6, 0xfc,

    /* U+0063 "c" */
    0x3d, 0x8c, 0x30, 0xc3, 0xc, 0x18, 0x3c,

    /* U+0064 "d" */
    0x3, 0x3, 0x3, 0x3, 0x3f, 0x63, 0xc3, 0xc3,
    0xc3, 0xc3, 0xc3, 0x63, 0x3f,

    /* U+0065 "e" */
    0x3c, 0xcf, 0x1e, 0x3f, 0xf8, 0x30, 0x31, 0x3e,

    /* U+0066 "f" */
    0x36, 0x66, 0xf6, 0x66, 0x66, 0x66, 0x60,

    /* U+0067 "g" */
    0x7f, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0xc0, 0xc0,
    0x7e, 0xc3, 0x83, 0xc7, 0x7e,

    /* U+0068 "h" */
    0xc1, 0x83, 0x6, 0xf, 0xdc, 0xf1, 0xe3, 0xc7,
    0x8f, 0x1e, 0x3c, 0x60,

    /* U+0069 "i" */
    0x6c, 0x6, 0xdb, 0x6d, 0xb6,

    /* U+006A "j" */
    0x6c, 0x6, 0xdb, 0x6d, 0xb6, 0xdf, 0xc0,

    /* U+006B "k" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc6, 0xcc, 0xd8, 0xf0,
    0xf8, 0xd8, 0xcc, 0xc6, 0xc6,

    /* U+006C "l" */
    0xff, 0xff, 0xff, 0xc0,

    /* U+006D "m" */
    0xfd, 0xec, 0x63, 0xc6, 0x3c, 0x63, 0xc6, 0x3c,
    0x63, 0xc6, 0x3c, 0x63, 0xc6, 0x30,

    /* U+006E "n" */
    0xfd, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc6,

    /* U+006F "o" */
    0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66,
    0x3c,

    /* U+0070 "p" */
    0xf9, 0x9b, 0x1e, 0x3c, 0x78, 0xf1, 0xe6, 0xf9,
    0x83, 0x6, 0xc, 0x0,

    /* U+0071 "q" */
    0x3f, 0x63, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x63,
    0x3f, 0x3, 0x3, 0x3, 0x3,

    /* U+0072 "r" */
    0xff, 0xf1, 0x8c, 0x63, 0x18, 0xc0,

    /* U+0073 "s" */
    0x39, 0xc, 0x1c, 0x78, 0x70, 0xe3, 0xf8,

    /* U+0074 "t" */
    0x63, 0x19, 0xe6, 0x31, 0x8c, 0x63, 0x18, 0x70,

    /* U+0075 "u" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0x7e,

    /* U+0076 "v" */
    0xc3, 0x43, 0x66, 0x66, 0x26, 0x34, 0x3c, 0x1c,
    0x18,

    /* U+0077 "w" */
    0xc7, 0x1e, 0x38, 0xd9, 0x44, 0xcb, 0x66, 0xdb,
    0x16, 0xd8, 0xe2, 0x87, 0x1c, 0x38, 0xe0,

    /* U+0078 "x" */
    0x66, 0x66, 0x3c, 0x1c, 0x18, 0x3c, 0x3c, 0x66,
    0x63,

    /* U+0079 "y" */
    0xc3, 0x31, 0x98, 0xcc, 0x43, 0x61, 0xb0, 0x58,
    0x38, 0x1c, 0x6, 0x2, 0x3, 0x3, 0x0,

    /* U+007A "z" */
    0xfc, 0x71, 0x8e, 0x31, 0xc6, 0x38, 0xfc,

    /* U+007B "{" */
    0x1c, 0xc3, 0xc, 0x30, 0xc3, 0x38, 0xe0, 0xc3,
    0xc, 0x30, 0xc3, 0x7,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xff, 0xf0,

    /* U+007D "}" */
    0xe0, 0xc3, 0xc, 0x30, 0xc3, 0x7, 0x1c, 0xc3,
    0xc, 0x30, 0xc3, 0x38,

    /* U+007E "~" */
    0x72, 0x8e,

    /* U+007F "" */
    0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 76, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 92, .box_w = 3, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 6, .adv_w = 125, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 10, .adv_w = 239, .box_w = 13, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 134, .box_w = 6, .box_h = 17, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 46, .adv_w = 279, .box_w = 15, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 200, .box_w = 10, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 72, .box_w = 2, .box_h = 6, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 88, .adv_w = 141, .box_w = 5, .box_h = 16, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 98, .adv_w = 141, .box_w = 5, .box_h = 16, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 108, .adv_w = 165, .box_w = 8, .box_h = 7, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 115, .adv_w = 168, .box_w = 7, .box_h = 7, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 122, .adv_w = 93, .box_w = 3, .box_h = 6, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 125, .adv_w = 140, .box_w = 6, .box_h = 1, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 126, .adv_w = 91, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 195, .box_w = 8, .box_h = 16, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 144, .adv_w = 168, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 158, .adv_w = 112, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 164, .adv_w = 145, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 149, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 168, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 155, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 213, .adv_w = 162, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 141, .box_w = 8, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 237, .adv_w = 161, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 162, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 92, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 265, .adv_w = 96, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 270, .adv_w = 152, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 279, .adv_w = 168, .box_w = 7, .box_h = 5, .ofs_x = 2, .ofs_y = 4},
    {.bitmap_index = 284, .adv_w = 152, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 292, .adv_w = 126, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 302, .adv_w = 286, .box_w = 15, .box_h = 14, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 329, .adv_w = 159, .box_w = 10, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 344, .adv_w = 168, .box_w = 8, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 356, .adv_w = 141, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 368, .adv_w = 170, .box_w = 8, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 380, .adv_w = 144, .box_w = 7, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 391, .adv_w = 136, .box_w = 7, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 402, .adv_w = 167, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 414, .adv_w = 181, .box_w = 8, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 426, .adv_w = 80, .box_w = 2, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 429, .adv_w = 92, .box_w = 4, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 435, .adv_w = 161, .box_w = 8, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 447, .adv_w = 124, .box_w = 6, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 456, .adv_w = 222, .box_w = 11, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 473, .adv_w = 187, .box_w = 8, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 485, .adv_w = 168, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 499, .adv_w = 155, .box_w = 8, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 511, .adv_w = 168, .box_w = 9, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 528, .adv_w = 160, .box_w = 8, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 540, .adv_w = 132, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 549, .adv_w = 140, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 561, .adv_w = 181, .box_w = 8, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 573, .adv_w = 150, .box_w = 9, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 587, .adv_w = 238, .box_w = 15, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 610, .adv_w = 156, .box_w = 10, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 625, .adv_w = 150, .box_w = 9, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 639, .adv_w = 146, .box_w = 8, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 651, .adv_w = 118, .box_w = 5, .box_h = 16, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 661, .adv_w = 195, .box_w = 8, .box_h = 16, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 677, .adv_w = 118, .box_w = 5, .box_h = 16, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 687, .adv_w = 128, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 696, .adv_w = 168, .box_w = 12, .box_h = 1, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 698, .adv_w = 13, .box_w = 4, .box_h = 4, .ofs_x = -3, .ofs_y = 10},
    {.bitmap_index = 700, .adv_w = 142, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 708, .adv_w = 149, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 721, .adv_w = 120, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 728, .adv_w = 153, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 741, .adv_w = 139, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 749, .adv_w = 87, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 756, .adv_w = 151, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 769, .adv_w = 157, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 781, .adv_w = 76, .box_w = 3, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 786, .adv_w = 76, .box_w = 3, .box_h = 17, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 793, .adv_w = 142, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 806, .adv_w = 72, .box_w = 2, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 810, .adv_w = 244, .box_w = 12, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 824, .adv_w = 159, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 832, .adv_w = 149, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 841, .adv_w = 148, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 853, .adv_w = 148, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 866, .adv_w = 101, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 872, .adv_w = 118, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 879, .adv_w = 96, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 887, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 895, .adv_w = 128, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 904, .adv_w = 209, .box_w = 13, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 919, .adv_w = 133, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 928, .adv_w = 130, .box_w = 9, .box_h = 13, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 943, .adv_w = 128, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 950, .adv_w = 130, .box_w = 6, .box_h = 16, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 962, .adv_w = 82, .box_w = 2, .box_h = 18, .ofs_x = 2, .ofs_y = -3},
    {.bitmap_index = 967, .adv_w = 130, .box_w = 6, .box_h = 16, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 979, .adv_w = 165, .box_w = 8, .box_h = 2, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 981, .adv_w = 0, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0}
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
const lv_font_t Tauri_R_16 = {
#else
lv_font_t Tauri_R_16 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 19,          /*The maximum line height required by the font*/
    .base_line = 4,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if TAURI_R_16*/

