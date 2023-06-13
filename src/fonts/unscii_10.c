/*******************************************************************************
 * Size: 10 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef UNSCII_10
#define UNSCII_10 1
#endif

#if UNSCII_10

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
    0x99,

    /* U+0023 "#" */
    0x57, 0xd5, 0xf5, 0x28,

    /* U+0024 "$" */
    0x5a, 0xa7, 0x52,

    /* U+0025 "%" */
    0x99, 0x24, 0x59,

    /* U+0026 "&" */
    0x73, 0x9f, 0x29, 0x3c,

    /* U+0027 "'" */
    0x5c,

    /* U+0028 "(" */
    0x6a, 0x49, 0x13,

    /* U+0029 ")" */
    0xc8, 0x92, 0x56,

    /* U+002A "*" */
    0x4b, 0xf7, 0x92,

    /* U+002B "+" */
    0x5d, 0x20,

    /* U+002C "," */
    0xd8,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0xc0,

    /* U+002F "/" */
    0x8, 0x84, 0x22, 0x21, 0x10, 0x80,

    /* U+0030 "0" */
    0xf9, 0xbd, 0x96,

    /* U+0031 "1" */
    0xc9, 0x25, 0xc0,

    /* U+0032 "2" */
    0xf1, 0x3c, 0x8f,

    /* U+0033 "3" */
    0xf1, 0x71, 0x9f,

    /* U+0034 "4" */
    0x36, 0xa5, 0xf1, 0x8,

    /* U+0035 "5" */
    0xf8, 0xf1, 0x9f,

    /* U+0036 "6" */
    0xf8, 0xf9, 0x9f,

    /* U+0037 "7" */
    0xe4, 0xa4, 0x80,

    /* U+0038 "8" */
    0xf9, 0xf9, 0x9f,

    /* U+0039 "9" */
    0xf9, 0xf1, 0x1e,

    /* U+003A ":" */
    0xcc,

    /* U+003B ";" */
    0x50, 0xd8,

    /* U+003C "<" */
    0x2a, 0x64, 0x40,

    /* U+003D "=" */
    0xe3, 0x80,

    /* U+003E ">" */
    0x88, 0xb5, 0x0,

    /* U+003F "?" */
    0xf5, 0xa4, 0x10,

    /* U+0040 "@" */
    0xf9, 0xbb, 0x8f,

    /* U+0041 "A" */
    0xf9, 0xf9, 0x99,

    /* U+0042 "B" */
    0xf9, 0xeb, 0x9f,

    /* U+0043 "C" */
    0xf8, 0x88, 0x9f,

    /* U+0044 "D" */
    0xf9, 0x99, 0x9e,

    /* U+0045 "E" */
    0xf8, 0xe8, 0x8f,

    /* U+0046 "F" */
    0xf8, 0xe8, 0x88,

    /* U+0047 "G" */
    0xf8, 0xb9, 0x9f,

    /* U+0048 "H" */
    0x99, 0xf9, 0x99,

    /* U+0049 "I" */
    0xe9, 0x25, 0xc0,

    /* U+004A "J" */
    0x11, 0x11, 0x9f,

    /* U+004B "K" */
    0x99, 0xea, 0x99,

    /* U+004C "L" */
    0x88, 0x88, 0x8f,

    /* U+004D "M" */
    0xdf, 0xb9, 0x99,

    /* U+004E "N" */
    0x9d, 0xdb, 0xb9,

    /* U+004F "O" */
    0xf9, 0x99, 0x9f,

    /* U+0050 "P" */
    0xf9, 0xf8, 0x88,

    /* U+0051 "Q" */
    0xf9, 0x99, 0x9f, 0x30,

    /* U+0052 "R" */
    0xf9, 0xf9, 0x99,

    /* U+0053 "S" */
    0xf8, 0x61, 0x9f,

    /* U+0054 "T" */
    0xe9, 0x24, 0x80,

    /* U+0055 "U" */
    0x99, 0x99, 0x9f,

    /* U+0056 "V" */
    0x99, 0x99, 0xe4,

    /* U+0057 "W" */
    0x99, 0xbf, 0xd9,

    /* U+0058 "X" */
    0x8f, 0xc8, 0x4d, 0xc4,

    /* U+0059 "Y" */
    0xda, 0x88, 0x42, 0x10,

    /* U+005A "Z" */
    0xf3, 0x64, 0x8f,

    /* U+005B "[" */
    0xea, 0xab,

    /* U+005C "\\" */
    0x84, 0x10, 0x82, 0x8, 0x42, 0x8,

    /* U+005D "]" */
    0xd5, 0x57,

    /* U+005E "^" */
    0x65, 0x90,

    /* U+005F "_" */
    0xf8,

    /* U+0060 "`" */
    0xa4,

    /* U+0061 "a" */
    0xff, 0x9f,

    /* U+0062 "b" */
    0x88, 0xf9, 0x9f,

    /* U+0063 "c" */
    0xf8, 0x8f,

    /* U+0064 "d" */
    0x11, 0xf9, 0x9f,

    /* U+0065 "e" */
    0xff, 0x8f,

    /* U+0066 "f" */
    0x74, 0xf4, 0x44,

    /* U+0067 "g" */
    0xf9, 0x9f, 0x1f,

    /* U+0068 "h" */
    0x88, 0xf9, 0x99,

    /* U+0069 "i" */
    0x43, 0x24, 0xc0,

    /* U+006A "j" */
    0x20, 0x92, 0x4f,

    /* U+006B "k" */
    0x88, 0x9f, 0xb9,

    /* U+006C "l" */
    0xc9, 0x24, 0xc0,

    /* U+006D "m" */
    0xfb, 0xb9,

    /* U+006E "n" */
    0xf9, 0x99,

    /* U+006F "o" */
    0xf9, 0x9f,

    /* U+0070 "p" */
    0xf9, 0x9f, 0x88,

    /* U+0071 "q" */
    0xf9, 0x9f, 0x11,

    /* U+0072 "r" */
    0xf8, 0x88,

    /* U+0073 "s" */
    0xff, 0x1f,

    /* U+0074 "t" */
    0x44, 0xf4, 0x47,

    /* U+0075 "u" */
    0x99, 0x9f,

    /* U+0076 "v" */
    0x99, 0x9f,

    /* U+0077 "w" */
    0x9b, 0xbd,

    /* U+0078 "x" */
    0x9f, 0xf9,

    /* U+0079 "y" */
    0x99, 0x9f, 0x1f,

    /* U+007A "z" */
    0xf2, 0x4f,

    /* U+007B "{" */
    0x32, 0x2e, 0x22, 0x23,

    /* U+007C "|" */
    0xff, 0x80,

    /* U+007D "}" */
    0xc4, 0x47, 0x44, 0x4c,

    /* U+007E "~" */
    0x6d, 0x80,

    /* U+007F "" */
    0x8c, 0xfd, 0xd9, 0x11
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 80, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 9},
    {.bitmap_index = 1, .adv_w = 80, .box_w = 1, .box_h = 7, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 2, .adv_w = 80, .box_w = 4, .box_h = 2, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 3, .adv_w = 80, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 7, .adv_w = 80, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 10, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 13, .adv_w = 80, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 17, .adv_w = 80, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 18, .adv_w = 80, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 21, .adv_w = 80, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 24, .adv_w = 80, .box_w = 6, .box_h = 4, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 27, .adv_w = 80, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 29, .adv_w = 80, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 30, .adv_w = 80, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 31, .adv_w = 80, .box_w = 1, .box_h = 2, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 32, .adv_w = 80, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 38, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 41, .adv_w = 80, .box_w = 3, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 44, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 47, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 50, .adv_w = 80, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 54, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 57, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 60, .adv_w = 80, .box_w = 3, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 63, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 66, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 69, .adv_w = 80, .box_w = 1, .box_h = 6, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 70, .adv_w = 80, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 72, .adv_w = 80, .box_w = 3, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 75, .adv_w = 80, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 77, .adv_w = 80, .box_w = 3, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 80, .adv_w = 80, .box_w = 3, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 83, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 86, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 89, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 92, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 95, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 98, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 101, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 104, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 107, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 110, .adv_w = 80, .box_w = 3, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 113, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 116, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 119, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 122, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 125, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 128, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 131, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 134, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 138, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 141, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 144, .adv_w = 80, .box_w = 3, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 147, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 150, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 153, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 156, .adv_w = 80, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 160, .adv_w = 80, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 164, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 167, .adv_w = 80, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 169, .adv_w = 80, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 175, .adv_w = 80, .box_w = 2, .box_h = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 177, .adv_w = 80, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 179, .adv_w = 80, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 80, .box_w = 2, .box_h = 3, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 181, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 183, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 186, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 188, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 191, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 193, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 196, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 199, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 202, .adv_w = 80, .box_w = 3, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 205, .adv_w = 80, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 211, .adv_w = 80, .box_w = 3, .box_h = 6, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 214, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 216, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 218, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 220, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 223, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 226, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 228, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 230, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 233, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 235, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 237, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 239, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 241, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 244, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 246, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 250, .adv_w = 80, .box_w = 1, .box_h = 9, .ofs_x = 2, .ofs_y = 1},
    {.bitmap_index = 252, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 256, .adv_w = 80, .box_w = 5, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 258, .adv_w = 80, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 1}
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
const lv_font_t unscii_10 = {
#else
lv_font_t unscii_10 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 10,          /*The maximum line height required by the font*/
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



#endif /*#if UNSCII_10*/

