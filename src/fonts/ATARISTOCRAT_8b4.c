/*******************************************************************************
 * Size: 8 px
 * Bpp: 4
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef ATARISTOCRAT_8B4
#define ATARISTOCRAT_8B4 1
#endif

#if ATARISTOCRAT_8B4

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */

    /* U+0021 "!" */
    0xff, 0xf9, 0x90,

    /* U+0022 "\"" */
    0xf0, 0xfc, 0xc,

    /* U+0023 "#" */
    0x44, 0x90, 0xcc, 0xf4, 0x88, 0xf0, 0xcc, 0xf4,

    /* U+0024 "$" */
    0x6c, 0x9b, 0x96, 0x0, 0xc9, 0xc6,

    /* U+0025 "%" */
    0xf0, 0xc3, 0x81, 0x98, 0x39, 0xf,

    /* U+0026 "&" */
    0x4c, 0xc0, 0x4f, 0x60, 0xf6, 0xf6, 0x99, 0x94,

    /* U+0027 "'" */
    0x88, 0x66,

    /* U+0028 "(" */
    0xf, 0x48, 0x90, 0x69, 0x0, 0x84,

    /* U+0029 ")" */
    0x4f, 0x0, 0x98, 0x9, 0x64, 0x80,

    /* U+002A "*" */
    0x46, 0xb0, 0x9f, 0xc4, 0xe, 0x80, 0x46, 0xb0,

    /* U+002B "+" */
    0xf, 0x9, 0xf9, 0xf, 0x0,

    /* U+002C "," */
    0x11, 0x88, 0xc0,

    /* U+002D "-" */
    0x99, 0x40,

    /* U+002E "." */
    0x11, 0x88,

    /* U+002F "/" */
    0x0, 0xf0, 0x83, 0x98, 0x9, 0x0,

    /* U+0030 "0" */
    0xf9, 0xff, 0x9f, 0xf0, 0xf8, 0x98,

    /* U+0031 "1" */
    0x4f, 0x0, 0xf0, 0xf, 0x9, 0xf9,

    /* U+0032 "2" */
    0x99, 0xf0, 0x9, 0x1c, 0x1c, 0xc9,

    /* U+0033 "3" */
    0x9c, 0xc0, 0xc4, 0x4, 0xbb, 0x9e,

    /* U+0034 "4" */
    0xf, 0x8b, 0x98, 0xcc, 0xc0, 0x88,

    /* U+0035 "5" */
    0xf9, 0x99, 0x98, 0x0, 0xfb, 0x9e,

    /* U+0036 "6" */
    0xe9, 0x4f, 0x96, 0xf0, 0xf9, 0x99,

    /* U+0037 "7" */
    0x99, 0xf0, 0xb8, 0x69, 0x8, 0x80,

    /* U+0038 "8" */
    0xf9, 0xf9, 0x99, 0xf0, 0xf9, 0x99,

    /* U+0039 "9" */
    0xb9, 0xb8, 0x9f, 0x1, 0xe4, 0xc3,

    /* U+003A ":" */
    0x88, 0x11, 0x88, 0x11,

    /* U+003B ";" */
    0x88, 0x11, 0x88, 0xc1,

    /* U+003C "<" */
    0xf, 0x4c, 0x30, 0x69, 0x0, 0x94,

    /* U+003D "=" */
    0x99, 0x90, 0x0, 0x99, 0x90,

    /* U+003E ">" */
    0xc8, 0x1, 0x94, 0x1e, 0xc, 0x10,

    /* U+003F "?" */
    0x99, 0xf0, 0x39, 0xb, 0x10, 0x90,

    /* U+0040 "@" */
    0xf9, 0xff, 0x8e, 0xf6, 0xb8, 0x99,

    /* U+0041 "A" */
    0x4f, 0x4f, 0x3f, 0xf9, 0xff, 0xf,

    /* U+0042 "B" */
    0xf9, 0xff, 0x99, 0xf0, 0xff, 0x99,

    /* U+0043 "C" */
    0xe9, 0xbf, 0x0, 0xf0, 0xe, 0x9b,

    /* U+0044 "D" */
    0xfc, 0x8f, 0x1f, 0xf1, 0xef, 0xc0,

    /* U+0045 "E" */
    0xf9, 0x9f, 0x94, 0xf0, 0xf, 0x99,

    /* U+0046 "F" */
    0xf9, 0x9f, 0x94, 0xf0, 0xf, 0x0,

    /* U+0047 "G" */
    0xf9, 0x9f, 0x4c, 0xf0, 0xf8, 0x9f,

    /* U+0048 "H" */
    0xf0, 0xff, 0x9f, 0xf0, 0xff, 0xf,

    /* U+0049 "I" */
    0x4f, 0x40, 0xf0, 0xf, 0x4, 0xf4,

    /* U+004A "J" */
    0x0, 0xf0, 0xf, 0x0, 0xfc, 0x9c,

    /* U+004B "K" */
    0xf8, 0xcf, 0x91, 0xfe, 0x1f, 0x1c,

    /* U+004C "L" */
    0xf0, 0xf, 0x0, 0xf0, 0xf, 0x99,

    /* U+004D "M" */
    0xf8, 0xf8, 0xf9, 0x98, 0xf1, 0x88, 0xf0, 0x88,

    /* U+004E "N" */
    0xf6, 0xff, 0xff, 0xf6, 0xff, 0xf,

    /* U+004F "O" */
    0xf9, 0xff, 0xf, 0xf0, 0xf8, 0x98,

    /* U+0050 "P" */
    0xf9, 0xff, 0x97, 0xf0, 0xf, 0x0,

    /* U+0051 "Q" */
    0xc9, 0xcf, 0xf, 0xf4, 0xee, 0x9c,

    /* U+0052 "R" */
    0xf9, 0xff, 0xb9, 0xf3, 0x9f, 0xf,

    /* U+0053 "S" */
    0xf9, 0x97, 0x96, 0x0, 0xbb, 0x9e,

    /* U+0054 "T" */
    0x9f, 0x90, 0xf0, 0xf, 0x0, 0xf0,

    /* U+0055 "U" */
    0xf0, 0xff, 0xf, 0xf0, 0xfc, 0x9f,

    /* U+0056 "V" */
    0xf0, 0xff, 0xf, 0xf3, 0xf4, 0xf4,

    /* U+0057 "W" */
    0xf0, 0x88, 0xf0, 0x88, 0xff, 0xf8, 0xf3, 0x98,

    /* U+0058 "X" */
    0xf0, 0xf8, 0xf8, 0x8c, 0x8f, 0xf,

    /* U+0059 "Y" */
    0xf0, 0xfe, 0x6e, 0x3f, 0x30, 0xf0,

    /* U+005A "Z" */
    0x99, 0xf0, 0x88, 0x88, 0xf, 0x99,

    /* U+005B "[" */
    0xf9, 0xf0, 0xf0, 0xf9,

    /* U+005C "\\" */
    0xf0, 0x3, 0x80, 0x8, 0x90, 0x9,

    /* U+005D "]" */
    0x9c, 0xf, 0xf, 0xf, 0x9f,

    /* U+005E "^" */
    0x8, 0x0, 0x39, 0xb0, 0xb1, 0x74,

    /* U+005F "_" */
    0x99, 0x90,

    /* U+0060 "`" */
    0xb3, 0x3b,

    /* U+0061 "a" */
    0x49, 0xb6, 0x9f, 0xc9, 0xf0,

    /* U+0062 "b" */
    0xf0, 0xf, 0x98, 0xf0, 0xff, 0x9b,

    /* U+0063 "c" */
    0x89, 0x4f, 0x0, 0xb9, 0x40,

    /* U+0064 "d" */
    0x0, 0xf8, 0x9f, 0xf0, 0xfb, 0x9f,

    /* U+0065 "e" */
    0x89, 0x8f, 0x9c, 0xb9, 0x40,

    /* U+0066 "f" */
    0x4c, 0x4c, 0xc4, 0x88, 0x8, 0x80,

    /* U+0067 "g" */
    0x89, 0xff, 0xf, 0x79, 0xf9, 0x98,

    /* U+0068 "h" */
    0xf0, 0xf, 0x9b, 0xf0, 0xff, 0xf,

    /* U+0069 "i" */
    0x44, 0xc8, 0x88, 0xcc,

    /* U+006A "j" */
    0x9, 0xf, 0xf, 0xf, 0x98,

    /* U+006B "k" */
    0xf0, 0xf, 0xc, 0xfc, 0x3f, 0x3c,

    /* U+006C "l" */
    0xc8, 0x88, 0x88, 0xcc,

    /* U+006D "m" */
    0xfb, 0xf3, 0xf8, 0x98, 0xf0, 0x88,

    /* U+006E "n" */
    0xf9, 0xbf, 0xf, 0xf0, 0xf0,

    /* U+006F "o" */
    0x89, 0x8f, 0xf, 0xb9, 0xb0,

    /* U+0070 "p" */
    0xf9, 0x8f, 0xf, 0xf9, 0xbf, 0x0,

    /* U+0071 "q" */
    0x89, 0xff, 0xf, 0xb9, 0xf0, 0xf,

    /* U+0072 "r" */
    0xf9, 0xbf, 0x0, 0xf0, 0x0,

    /* U+0073 "s" */
    0xc9, 0x96, 0x96, 0x99, 0xc0,

    /* U+0074 "t" */
    0xf, 0x9, 0xf9, 0xf, 0x0, 0x89,

    /* U+0075 "u" */
    0xf0, 0xff, 0xf, 0xb9, 0xf0,

    /* U+0076 "v" */
    0xf0, 0xfe, 0x3e, 0x3f, 0x30,

    /* U+0077 "w" */
    0xf0, 0x88, 0xe9, 0x96, 0x89, 0xf0,

    /* U+0078 "x" */
    0xc0, 0xc3, 0xf3, 0xc6, 0xc0,

    /* U+0079 "y" */
    0xf0, 0xfe, 0xf, 0x69, 0xf9, 0x98,

    /* U+007A "z" */
    0x9c, 0xc1, 0xc1, 0xcc, 0x90,

    /* U+007B "{" */
    0x8c, 0x4c, 0x60, 0x86, 0x0, 0xc4,

    /* U+007C "|" */
    0x88, 0x88, 0x88, 0x88, 0x88,

    /* U+007D "}" */
    0x9f, 0x0, 0xe4, 0xe, 0x9, 0x80,

    /* U+007E "~" */
    0xff, 0x8, 0x11, 0xf1, 0x0, 0x30
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 40, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 40, .box_w = 1, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3, .adv_w = 56, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 6, .adv_w = 64, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 14, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 20, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 26, .adv_w = 64, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 34, .adv_w = 32, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 36, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 42, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 48, .adv_w = 64, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 56, .adv_w = 56, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 61, .adv_w = 32, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 64, .adv_w = 48, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 66, .adv_w = 32, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 80, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 98, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 104, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 110, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 116, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 134, .adv_w = 32, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 32, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 56, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 153, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 159, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 165, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 171, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 177, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 183, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 189, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 195, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 213, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 219, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 231, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 237, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 64, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 251, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 257, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 263, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 269, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 275, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 281, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 287, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 299, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 305, .adv_w = 64, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 313, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 319, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 325, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 331, .adv_w = 40, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 335, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 341, .adv_w = 40, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 346, .adv_w = 64, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 352, .adv_w = 56, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 354, .adv_w = 40, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 356, .adv_w = 56, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 361, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 367, .adv_w = 48, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 372, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 378, .adv_w = 56, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 383, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 389, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 395, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 401, .adv_w = 40, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 405, .adv_w = 40, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 410, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 416, .adv_w = 40, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 420, .adv_w = 64, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 426, .adv_w = 56, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 431, .adv_w = 56, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 442, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 448, .adv_w = 56, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 453, .adv_w = 56, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 458, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 464, .adv_w = 56, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 469, .adv_w = 56, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 474, .adv_w = 64, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 480, .adv_w = 56, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 485, .adv_w = 56, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 491, .adv_w = 56, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 496, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 502, .adv_w = 32, .box_w = 2, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 507, .adv_w = 48, .box_w = 3, .box_h = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 513, .adv_w = 64, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 0}
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
    .bpp = 4,
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
const lv_font_t ATARISTOCRAT_8b4 = {
#else
lv_font_t ATARISTOCRAT_8b4 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 6,          /*The maximum line height required by the font*/
    .base_line = 1,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if ATARISTOCRAT_8B4*/

