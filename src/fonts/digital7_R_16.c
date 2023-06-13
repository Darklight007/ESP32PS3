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

#ifndef DIGITAL7_R_16
#define DIGITAL7_R_16 1
#endif

#if DIGITAL7_R_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfb, 0xa0,

    /* U+0022 "\"" */
    0xb6, 0x80,

    /* U+0023 "#" */
    0x52, 0xaa, 0xa5, 0x55, 0x4a,

    /* U+0024 "$" */
    0x10, 0x4f, 0xe0, 0x92, 0x49, 0x3f, 0x14, 0x51,
    0x41, 0xfc, 0x41, 0x0,

    /* U+0025 "%" */
    0xe7, 0x4a, 0x97, 0x40, 0x80, 0x8, 0x17, 0x4a,
    0x97, 0x38,

    /* U+0026 "&" */
    0x78, 0x91, 0x20, 0x7, 0x90, 0xa1, 0x42, 0x85,
    0xf8, 0x10, 0x20,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x72, 0x48, 0x24, 0x91, 0x80,

    /* U+0029 ")" */
    0xc4, 0x92, 0x9, 0x27, 0x0,

    /* U+002A "*" */
    0x27, 0xdd, 0xf2, 0x0,

    /* U+002B "+" */
    0x21, 0x3e, 0x42, 0x0,

    /* U+002C "," */
    0xe0,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x4, 0x10, 0xc2, 0x18, 0x2, 0x18, 0x41, 0xc,
    0x0,

    /* U+0030 "0" */
    0xfe, 0x18, 0x61, 0x84, 0x8, 0x61, 0x86, 0x1f,
    0xc0,

    /* U+0031 "1" */
    0xfb, 0xe0,

    /* U+0032 "2" */
    0xfc, 0x10, 0x41, 0x7, 0xf8, 0x20, 0x82, 0xf,
    0x80,

    /* U+0033 "3" */
    0xfc, 0x10, 0x41, 0x5, 0xf0, 0x41, 0x4, 0x1f,
    0xc0,

    /* U+0034 "4" */
    0x6, 0x18, 0x61, 0x87, 0xf0, 0x41, 0x4, 0x10,
    0x40,

    /* U+0035 "5" */
    0xfa, 0x8, 0x20, 0x83, 0xf0, 0x41, 0x4, 0x1f,
    0xc0,

    /* U+0036 "6" */
    0xfa, 0x8, 0x20, 0x83, 0xf8, 0x61, 0x86, 0x1f,
    0xc0,

    /* U+0037 "7" */
    0xfe, 0x18, 0x61, 0x84, 0x0, 0x41, 0x4, 0x10,
    0x40,

    /* U+0038 "8" */
    0xfe, 0x18, 0x61, 0x87, 0xf8, 0x61, 0x86, 0x1f,
    0xc0,

    /* U+0039 "9" */
    0xfe, 0x18, 0x61, 0x87, 0xf0, 0x41, 0x4, 0x1f,
    0xc0,

    /* U+003A ":" */
    0x84,

    /* U+003B ";" */
    0x81, 0xc0,

    /* U+003C "<" */
    0x12, 0x44, 0x21,

    /* U+003D "=" */
    0xf8, 0x1, 0xf0,

    /* U+003E ">" */
    0xc6, 0x33, 0x6c,

    /* U+003F "?" */
    0xfc, 0x10, 0x41, 0x7, 0xf8, 0x20, 0x80, 0x8,
    0x0,

    /* U+0040 "@" */
    0xfe, 0x18, 0x6f, 0xa4, 0xa, 0x6f, 0x82, 0xf,
    0x80,

    /* U+0041 "A" */
    0xfe, 0x18, 0x61, 0x87, 0xf8, 0x61, 0x86, 0x18,
    0x40,

    /* U+0042 "B" */
    0xfa, 0x18, 0x61, 0x87, 0xf8, 0x61, 0x86, 0x1f,
    0x80,

    /* U+0043 "C" */
    0xfc, 0x21, 0x8, 0x2, 0x10, 0x84, 0x3e,

    /* U+0044 "D" */
    0xfa, 0x18, 0x61, 0x84, 0x8, 0x61, 0x86, 0x1f,
    0x80,

    /* U+0045 "E" */
    0xfc, 0x21, 0x8, 0x7e, 0x10, 0x84, 0x3e,

    /* U+0046 "F" */
    0xfc, 0x21, 0x8, 0x7e, 0x10, 0x84, 0x20,

    /* U+0047 "G" */
    0xfa, 0x8, 0x20, 0x80, 0x78, 0x61, 0x86, 0x1f,
    0xc0,

    /* U+0048 "H" */
    0x86, 0x18, 0x61, 0x87, 0xf8, 0x61, 0x86, 0x18,
    0x40,

    /* U+0049 "I" */
    0xfb, 0xe0,

    /* U+004A "J" */
    0x4, 0x10, 0x41, 0x4, 0x8, 0x61, 0x86, 0x1f,
    0xc0,

    /* U+004B "K" */
    0x8e, 0x29, 0x28, 0xc3, 0xf8, 0x61, 0x86, 0x18,
    0x40,

    /* U+004C "L" */
    0x82, 0x8, 0x20, 0x80, 0x8, 0x20, 0x82, 0xf,
    0xc0,

    /* U+004D "M" */
    0xff, 0x26, 0x4c, 0x99, 0x22, 0x20, 0xc1, 0x83,
    0x6, 0x8,

    /* U+004E "N" */
    0x87, 0x9b, 0x67, 0x86, 0x18, 0x61, 0x86, 0x10,

    /* U+004F "O" */
    0x7a, 0x18, 0x61, 0x84, 0x8, 0x61, 0x86, 0x17,
    0x80,

    /* U+0050 "P" */
    0xfa, 0x18, 0x61, 0x87, 0xf8, 0x20, 0x82, 0x8,
    0x0,

    /* U+0051 "Q" */
    0xfe, 0x18, 0x61, 0x84, 0x8, 0x61, 0x96, 0x2f,
    0x40,

    /* U+0052 "R" */
    0xfe, 0x18, 0x61, 0x87, 0xfc, 0x28, 0x92, 0x68,
    0xc0,

    /* U+0053 "S" */
    0xfe, 0x8, 0x20, 0x83, 0xf0, 0x41, 0x4, 0x1f,
    0xc0,

    /* U+0054 "T" */
    0xf9, 0x8, 0x42, 0x0, 0x84, 0x21, 0x8,

    /* U+0055 "U" */
    0x86, 0x18, 0x61, 0x84, 0x8, 0x61, 0x86, 0x1f,
    0xc0,

    /* U+0056 "V" */
    0x86, 0x18, 0x61, 0x84, 0x8, 0x73, 0x48, 0xc3,
    0x0,

    /* U+0057 "W" */
    0x83, 0x6, 0xc, 0x18, 0x22, 0x24, 0xc9, 0x93,
    0x27, 0xf8,

    /* U+0058 "X" */
    0xc5, 0x14, 0xda, 0x38, 0x3, 0x9a, 0x4d, 0x1c,
    0x40,

    /* U+0059 "Y" */
    0x6, 0x18, 0x61, 0x87, 0xf0, 0x41, 0x4, 0x1f,
    0xc0,

    /* U+005A "Z" */
    0xf4, 0x30, 0x86, 0x10, 0x2, 0x18, 0x43, 0xb,
    0x80,

    /* U+005B "[" */
    0xf2, 0x48, 0x24, 0x93, 0x80,

    /* U+005C "\\" */
    0xc1, 0x4, 0x18, 0x20, 0x1, 0x82, 0xc, 0x10,
    0x40,

    /* U+005D "]" */
    0xe4, 0x92, 0x9, 0x27, 0x80,

    /* U+005E "^" */
    0x31, 0xe4, 0xa1,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0xcc, 0x80,

    /* U+0061 "a" */
    0xfe, 0x18, 0x61, 0x87, 0xf8, 0x61, 0x86, 0x18,
    0x40,

    /* U+0062 "b" */
    0xfa, 0x18, 0x61, 0x87, 0xf8, 0x61, 0x86, 0x1f,
    0x80,

    /* U+0063 "c" */
    0xfc, 0x21, 0x8, 0x2, 0x10, 0x84, 0x3e,

    /* U+0064 "d" */
    0xfa, 0x18, 0x61, 0x84, 0x8, 0x61, 0x86, 0x1f,
    0x80,

    /* U+0065 "e" */
    0xfc, 0x21, 0x8, 0x7e, 0x10, 0x84, 0x3e,

    /* U+0066 "f" */
    0xfc, 0x21, 0x8, 0x7e, 0x10, 0x84, 0x20,

    /* U+0067 "g" */
    0xfa, 0x8, 0x20, 0x80, 0x78, 0x61, 0x86, 0x1f,
    0xc0,

    /* U+0068 "h" */
    0x86, 0x18, 0x61, 0x87, 0xf8, 0x61, 0x86, 0x18,
    0x40,

    /* U+0069 "i" */
    0xfb, 0xe0,

    /* U+006A "j" */
    0x4, 0x10, 0x41, 0x4, 0x8, 0x61, 0x86, 0x1f,
    0xc0,

    /* U+006B "k" */
    0x8e, 0x29, 0x28, 0xc3, 0xf8, 0x61, 0x86, 0x18,
    0x40,

    /* U+006C "l" */
    0x82, 0x8, 0x20, 0x80, 0x8, 0x20, 0x82, 0xf,
    0xc0,

    /* U+006D "m" */
    0xff, 0x26, 0x4c, 0x99, 0x22, 0x20, 0xc1, 0x83,
    0x6, 0x8,

    /* U+006E "n" */
    0x87, 0x9b, 0x67, 0x86, 0x18, 0x61, 0x86, 0x10,

    /* U+006F "o" */
    0x7a, 0x18, 0x61, 0x84, 0x8, 0x61, 0x86, 0x17,
    0x80,

    /* U+0070 "p" */
    0xfa, 0x18, 0x61, 0x87, 0xf8, 0x20, 0x82, 0x8,
    0x0,

    /* U+0071 "q" */
    0xfe, 0x18, 0x61, 0x84, 0x8, 0x61, 0x96, 0x2f,
    0x40,

    /* U+0072 "r" */
    0xfe, 0x18, 0x61, 0x87, 0xfc, 0x28, 0x92, 0x68,
    0xc0,

    /* U+0073 "s" */
    0xfe, 0x8, 0x20, 0x83, 0xf0, 0x41, 0x4, 0x1f,
    0xc0,

    /* U+0074 "t" */
    0xf9, 0x8, 0x42, 0x0, 0x84, 0x21, 0x8,

    /* U+0075 "u" */
    0x86, 0x18, 0x61, 0x84, 0x8, 0x61, 0x86, 0x1f,
    0xc0,

    /* U+0076 "v" */
    0x86, 0x18, 0x61, 0x84, 0x8, 0x73, 0x48, 0xc3,
    0x0,

    /* U+0077 "w" */
    0x83, 0x6, 0xc, 0x18, 0x22, 0x24, 0xc9, 0x93,
    0x27, 0xf8,

    /* U+0078 "x" */
    0xc5, 0x14, 0xda, 0x38, 0x3, 0x9a, 0x4d, 0x1c,
    0x40,

    /* U+0079 "y" */
    0x6, 0x18, 0x61, 0x87, 0xf0, 0x41, 0x4, 0x1f,
    0xc0,

    /* U+007A "z" */
    0xf4, 0x30, 0x86, 0x10, 0x2, 0x18, 0x43, 0xb,
    0x80,

    /* U+007B "{" */
    0x39, 0x8, 0x42, 0x60, 0x84, 0x21, 0xe,

    /* U+007C "|" */
    0xff, 0xe0,

    /* U+007D "}" */
    0xe1, 0x8, 0x42, 0xc, 0x84, 0x21, 0x38,

    /* U+007E "~" */
    0x20, 0xac, 0x30
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 116, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 116, .box_w = 1, .box_h = 11, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 116, .box_w = 3, .box_h = 3, .ofs_x = 2, .ofs_y = 8},
    {.bitmap_index = 5, .adv_w = 116, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 10, .adv_w = 116, .box_w = 6, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 22, .adv_w = 116, .box_w = 7, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 32, .adv_w = 116, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 43, .adv_w = 116, .box_w = 1, .box_h = 3, .ofs_x = 3, .ofs_y = 8},
    {.bitmap_index = 44, .adv_w = 116, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 49, .adv_w = 116, .box_w = 3, .box_h = 11, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 54, .adv_w = 116, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 58, .adv_w = 116, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 62, .adv_w = 116, .box_w = 1, .box_h = 3, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 63, .adv_w = 116, .box_w = 5, .box_h = 1, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 64, .adv_w = 116, .box_w = 1, .box_h = 1, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 65, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 116, .box_w = 1, .box_h = 11, .ofs_x = 6, .ofs_y = 0},
    {.bitmap_index = 85, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 94, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 112, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 121, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 116, .box_w = 1, .box_h = 6, .ofs_x = 3, .ofs_y = 3},
    {.bitmap_index = 158, .adv_w = 116, .box_w = 1, .box_h = 10, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 160, .adv_w = 116, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 163, .adv_w = 116, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 166, .adv_w = 116, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 169, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 205, .adv_w = 116, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 212, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 116, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 228, .adv_w = 116, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 235, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 244, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 253, .adv_w = 116, .box_w = 1, .box_h = 11, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 255, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 264, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 273, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 282, .adv_w = 116, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 116, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 300, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 318, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 327, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 336, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 345, .adv_w = 116, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 352, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 361, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 370, .adv_w = 116, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 380, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 389, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 398, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 407, .adv_w = 116, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 412, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 421, .adv_w = 116, .box_w = 3, .box_h = 11, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 426, .adv_w = 116, .box_w = 6, .box_h = 4, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 429, .adv_w = 116, .box_w = 6, .box_h = 1, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 430, .adv_w = 116, .box_w = 3, .box_h = 3, .ofs_x = 2, .ofs_y = 8},
    {.bitmap_index = 432, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 441, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 450, .adv_w = 116, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 457, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 466, .adv_w = 116, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 473, .adv_w = 116, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 480, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 489, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 498, .adv_w = 116, .box_w = 1, .box_h = 11, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 500, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 509, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 518, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 527, .adv_w = 116, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 537, .adv_w = 116, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 545, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 554, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 563, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 572, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 581, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 590, .adv_w = 116, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 597, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 606, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 615, .adv_w = 116, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 625, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 634, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 643, .adv_w = 116, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 652, .adv_w = 116, .box_w = 5, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 659, .adv_w = 116, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 661, .adv_w = 116, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 668, .adv_w = 116, .box_w = 7, .box_h = 3, .ofs_x = 0, .ofs_y = 5}
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
const lv_font_t digital7_R_16 = {
#else
lv_font_t digital7_R_16 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
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



#endif /*#if DIGITAL7_R_16*/

