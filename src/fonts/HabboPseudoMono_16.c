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

#ifndef HABBOPSEUDOMONO_16
#define HABBOPSEUDOMONO_16 1
#endif

#if HABBOPSEUDOMONO_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfa,

    /* U+0022 "\"" */
    0xb4,

    /* U+0023 "#" */
    0x4b, 0xf4, 0x92, 0xfd, 0x20,

    /* U+0024 "$" */
    0x28, 0xfe, 0xa5, 0x47, 0xc5, 0x4a, 0xfe, 0x28,

    /* U+0025 "%" */
    0x43, 0x49, 0x20, 0x82, 0x49, 0x61, 0x0,

    /* U+0026 "&" */
    0x31, 0x24, 0x9c, 0x96, 0x27, 0x40,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x72, 0x49, 0x18,

    /* U+0029 ")" */
    0xc4, 0x92, 0x70,

    /* U+002A "*" */
    0x25, 0x5d, 0x52, 0x0,

    /* U+002B "+" */
    0x21, 0x3e, 0x42, 0x0,

    /* U+002C "," */
    0xf6,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x2, 0x8, 0x20, 0x82, 0x8, 0x20, 0x0,

    /* U+0030 "0" */
    0x74, 0x67, 0x5c, 0xc5, 0xc0,

    /* U+0031 "1" */
    0xe1, 0x8, 0x42, 0x13, 0xe0,

    /* U+0032 "2" */
    0xf0, 0x42, 0xe8, 0x43, 0xe0,

    /* U+0033 "3" */
    0xf0, 0x42, 0xe0, 0x87, 0xc0,

    /* U+0034 "4" */
    0x11, 0x95, 0x2f, 0x88, 0x40,

    /* U+0035 "5" */
    0xfc, 0x21, 0xe0, 0x87, 0xc0,

    /* U+0036 "6" */
    0x74, 0x21, 0xe8, 0xc5, 0xc0,

    /* U+0037 "7" */
    0xf8, 0x44, 0x22, 0x11, 0x0,

    /* U+0038 "8" */
    0x74, 0x62, 0xe8, 0xc5, 0xc0,

    /* U+0039 "9" */
    0x74, 0x62, 0xf0, 0x85, 0xc0,

    /* U+003A ":" */
    0xf3, 0xc0,

    /* U+003B ";" */
    0xf3, 0xd8,

    /* U+003C "<" */
    0x12, 0x48, 0x42, 0x10,

    /* U+003D "=" */
    0xf8, 0x3e,

    /* U+003E ">" */
    0x84, 0x21, 0x24, 0x80,

    /* U+003F "?" */
    0x74, 0x42, 0x22, 0x0, 0x80,

    /* U+0040 "@" */
    0x7d, 0x6, 0xed, 0x5b, 0xf0, 0x1f, 0x0,

    /* U+0041 "A" */
    0x74, 0x63, 0xf8, 0xc6, 0x20,

    /* U+0042 "B" */
    0xf4, 0x63, 0xe8, 0xc7, 0xc0,

    /* U+0043 "C" */
    0x74, 0x61, 0x8, 0x45, 0xc0,

    /* U+0044 "D" */
    0xf4, 0x63, 0x18, 0xc7, 0xc0,

    /* U+0045 "E" */
    0xfc, 0x21, 0xe8, 0x43, 0xe0,

    /* U+0046 "F" */
    0xfc, 0x21, 0xe8, 0x42, 0x0,

    /* U+0047 "G" */
    0x74, 0x61, 0x78, 0xc5, 0xc0,

    /* U+0048 "H" */
    0x8c, 0x63, 0xf8, 0xc6, 0x20,

    /* U+0049 "I" */
    0xe9, 0x24, 0xb8,

    /* U+004A "J" */
    0x8, 0x42, 0x10, 0xc5, 0xc0,

    /* U+004B "K" */
    0x8c, 0xa9, 0x8a, 0x4a, 0x20,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x43, 0xe0,

    /* U+004D "M" */
    0x87, 0x3b, 0x61, 0x86, 0x18, 0x40,

    /* U+004E "N" */
    0x8c, 0x73, 0x59, 0xc6, 0x20,

    /* U+004F "O" */
    0x74, 0x63, 0x18, 0xc5, 0xc0,

    /* U+0050 "P" */
    0xf4, 0x63, 0x1f, 0x42, 0x0,

    /* U+0051 "Q" */
    0x72, 0x28, 0xa2, 0x8a, 0x67, 0x81,

    /* U+0052 "R" */
    0xf4, 0x63, 0x1f, 0x4a, 0x20,

    /* U+0053 "S" */
    0x74, 0x60, 0xe0, 0xc5, 0xc0,

    /* U+0054 "T" */
    0xf9, 0x8, 0x42, 0x10, 0x80,

    /* U+0055 "U" */
    0x8c, 0x63, 0x18, 0xc5, 0xc0,

    /* U+0056 "V" */
    0x8c, 0x63, 0x15, 0x28, 0x80,

    /* U+0057 "W" */
    0x83, 0x6, 0x4c, 0x95, 0x4a, 0x8a, 0x0,

    /* U+0058 "X" */
    0x8c, 0x54, 0x45, 0x46, 0x20,

    /* U+0059 "Y" */
    0x8c, 0x62, 0xa2, 0x10, 0x80,

    /* U+005A "Z" */
    0xf8, 0x44, 0x44, 0x43, 0xe0,

    /* U+005B "[" */
    0xf2, 0x49, 0x38,

    /* U+005C "\\" */
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,

    /* U+005D "]" */
    0xe4, 0x92, 0x78,

    /* U+005E "^" */
    0x54,

    /* U+005F "_" */
    0xf8,

    /* U+0060 "`" */
    0x90,

    /* U+0061 "a" */
    0x70, 0x5f, 0x17, 0x80,

    /* U+0062 "b" */
    0x84, 0x3d, 0x18, 0xc7, 0xc0,

    /* U+0063 "c" */
    0x74, 0x61, 0x17, 0x0,

    /* U+0064 "d" */
    0x8, 0x5f, 0x18, 0xc5, 0xe0,

    /* U+0065 "e" */
    0x74, 0x7f, 0x7, 0x80,

    /* U+0066 "f" */
    0x34, 0xe4, 0x44, 0x40,

    /* U+0067 "g" */
    0x7c, 0x63, 0x17, 0x85, 0xc0,

    /* U+0068 "h" */
    0x84, 0x3d, 0x18, 0xc6, 0x20,

    /* U+0069 "i" */
    0xbe,

    /* U+006A "j" */
    0x45, 0x55, 0x80,

    /* U+006B "k" */
    0x88, 0x9a, 0xca, 0x90,

    /* U+006C "l" */
    0xfe,

    /* U+006D "m" */
    0xfd, 0x26, 0x4c, 0x99, 0x20,

    /* U+006E "n" */
    0xf4, 0x63, 0x18, 0x80,

    /* U+006F "o" */
    0x74, 0x63, 0x17, 0x0,

    /* U+0070 "p" */
    0xf4, 0x63, 0x1f, 0x42, 0x0,

    /* U+0071 "q" */
    0x7c, 0x63, 0x17, 0x84, 0x20,

    /* U+0072 "r" */
    0xbc, 0x88, 0x80,

    /* U+0073 "s" */
    0x7c, 0x1c, 0x1f, 0x0,

    /* U+0074 "t" */
    0x44, 0xf4, 0x44, 0x30,

    /* U+0075 "u" */
    0x8c, 0x63, 0x17, 0x0,

    /* U+0076 "v" */
    0x8c, 0x54, 0xa2, 0x0,

    /* U+0077 "w" */
    0x93, 0x26, 0x4c, 0x96, 0xc0,

    /* U+0078 "x" */
    0x8a, 0x88, 0xa8, 0x80,

    /* U+0079 "y" */
    0x8c, 0x63, 0x17, 0x85, 0xc0,

    /* U+007A "z" */
    0xf8, 0x88, 0x8f, 0x80,

    /* U+007B "{" */
    0x34, 0x48, 0x44, 0x30,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xf0,

    /* U+007D "}" */
    0xc2, 0x21, 0x22, 0xc0,

    /* U+007E "~" */
    0x6d, 0x80
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 80, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 32, .box_w = 1, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 3, .adv_w = 112, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 8, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 16, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 23, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 29, .adv_w = 32, .box_w = 1, .box_h = 3, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 30, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 33, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 36, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 40, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 44, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 45, .adv_w = 80, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 46, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 47, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 54, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 59, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 64, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 84, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 89, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 94, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 104, .adv_w = 48, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 106, .adv_w = 48, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 108, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 112, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 114, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 118, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 135, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 140, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 145, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 155, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 165, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 170, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 183, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 188, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 194, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 199, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 204, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 209, .adv_w = 112, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 215, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 220, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 230, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 235, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 240, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 252, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 257, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 262, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 265, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 64, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 275, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 276, .adv_w = 96, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 278, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 282, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 287, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 291, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 300, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 304, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 309, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 314, .adv_w = 32, .box_w = 1, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 315, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 318, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 322, .adv_w = 32, .box_w = 1, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 323, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 328, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 332, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 336, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 341, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 346, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 349, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 353, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 357, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 361, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 365, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 370, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 374, .adv_w = 96, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 379, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 383, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 387, .adv_w = 80, .box_w = 2, .box_h = 14, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 391, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 395, .adv_w = 96, .box_w = 5, .box_h = 2, .ofs_x = 0, .ofs_y = 5}
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
const lv_font_t HabboPseudoMono_16 = {
#else
lv_font_t HabboPseudoMono_16 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 14,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if HABBOPSEUDOMONO_16*/

