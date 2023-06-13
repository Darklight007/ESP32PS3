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

#ifndef CHIKAREGO2_10
#define CHIKAREGO2_10 1
#endif

#if CHIKAREGO2_10

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xe4,

    /* U+0022 "\"" */
    0xb4,

    /* U+0023 "#" */
    0x28, 0xa7, 0xd4, 0xf9, 0x45, 0x0,

    /* U+0024 "$" */
    0x5f, 0x37, 0xda,

    /* U+0025 "%" */
    0x7a, 0xa5, 0x8, 0x28, 0x54, 0x80,

    /* U+0026 "&" */
    0xf6, 0x65, 0x29, 0x78,

    /* U+0027 "'" */
    0xc0,

    /* U+0028 "(" */
    0x7f, 0xf5,

    /* U+0029 ")" */
    0x8d, 0xb6, 0xd4,

    /* U+002A "*" */
    0xfd, 0x0,

    /* U+002B "+" */
    0x5d, 0x0,

    /* U+002C "," */
    0xc0,

    /* U+002D "-" */
    0xe0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x24, 0x28, 0x20,

    /* U+0030 "0" */
    0xf9, 0x99, 0x9f,

    /* U+0031 "1" */
    0xd5, 0x50,

    /* U+0032 "2" */
    0xf1, 0x12, 0xcf,

    /* U+0033 "3" */
    0xf3, 0x71, 0x97,

    /* U+0034 "4" */
    0x32, 0xa5, 0xf1, 0x8,

    /* U+0035 "5" */
    0xf8, 0xf1, 0x9f,

    /* U+0036 "6" */
    0x7b, 0x99, 0x9f,

    /* U+0037 "7" */
    0xe5, 0xa4, 0x80,

    /* U+0038 "8" */
    0xf9, 0xf9, 0x9f,

    /* U+0039 "9" */
    0xf9, 0x9f, 0x3e,

    /* U+003A ":" */
    0xa0,

    /* U+003B ";" */
    0x98,

    /* U+003C "<" */
    0x2e, 0x32,

    /* U+003D "=" */
    0xff,

    /* U+003E ">" */
    0x8c, 0xa8,

    /* U+003F "?" */
    0xe5, 0xa0, 0x80,

    /* U+0040 "@" */
    0x78, 0x2b, 0x77, 0xa9, 0xe0,

    /* U+0041 "A" */
    0xf9, 0xf9, 0x99,

    /* U+0042 "B" */
    0xf9, 0xf9, 0x9f,

    /* U+0043 "C" */
    0xf8, 0x88, 0x9e,

    /* U+0044 "D" */
    0xf9, 0x99, 0x9f,

    /* U+0045 "E" */
    0xf8, 0xe8, 0x8f,

    /* U+0046 "F" */
    0xf8, 0xe8, 0x88,

    /* U+0047 "G" */
    0xf8, 0xb9, 0x9f,

    /* U+0048 "H" */
    0x99, 0xf9, 0x99,

    /* U+0049 "I" */
    0xfc,

    /* U+004A "J" */
    0x11, 0x19, 0x9f,

    /* U+004B "K" */
    0xbd, 0xcc, 0xb9,

    /* U+004C "L" */
    0x92, 0x49, 0xc0,

    /* U+004D "M" */
    0xcf, 0x3b, 0x6d, 0x86, 0x10,

    /* U+004E "N" */
    0xce, 0x6b, 0x79, 0xc4,

    /* U+004F "O" */
    0xf9, 0x99, 0x9f,

    /* U+0050 "P" */
    0xf9, 0xf8, 0x88,

    /* U+0051 "Q" */
    0xf9, 0x99, 0x9f, 0x10,

    /* U+0052 "R" */
    0xf9, 0xf9, 0x99,

    /* U+0053 "S" */
    0xf3, 0xbb, 0xc0,

    /* U+0054 "T" */
    0xe9, 0x24, 0x80,

    /* U+0055 "U" */
    0x99, 0x99, 0x9f,

    /* U+0056 "V" */
    0xbb, 0xbb, 0xbe,

    /* U+0057 "W" */
    0x93, 0x26, 0x4c, 0x99, 0x3f, 0x80,

    /* U+0058 "X" */
    0x99, 0xf9, 0x99,

    /* U+0059 "Y" */
    0xb6, 0xf4, 0x80,

    /* U+005A "Z" */
    0xf1, 0x3c, 0x8f,

    /* U+005B "[" */
    0xea, 0xab,

    /* U+005C "\\" */
    0x90, 0x22, 0x8,

    /* U+005D "]" */
    0xd5, 0x57,

    /* U+005E "^" */
    0x74,

    /* U+005F "_" */
    0xf8,

    /* U+0060 "`" */
    0x90,

    /* U+0061 "a" */
    0xff, 0x99, 0xf0,

    /* U+0062 "b" */
    0x8f, 0x99, 0x9f,

    /* U+0063 "c" */
    0xd6, 0x5c,

    /* U+0064 "d" */
    0x1f, 0x99, 0x9f,

    /* U+0065 "e" */
    0xf9, 0xf9, 0xf0,

    /* U+0066 "f" */
    0x7d, 0x24, 0x80,

    /* U+0067 "g" */
    0xf9, 0x99, 0xf9, 0xf0,

    /* U+0068 "h" */
    0x8f, 0x99, 0x99,

    /* U+0069 "i" */
    0xfc,

    /* U+006A "j" */
    0x24, 0x93, 0x58,

    /* U+006B "k" */
    0x8b, 0xe8, 0xeb,

    /* U+006C "l" */
    0xfc,

    /* U+006D "m" */
    0xff, 0x26, 0x4c, 0x99, 0x20,

    /* U+006E "n" */
    0xf9, 0x99, 0x90,

    /* U+006F "o" */
    0xf9, 0x99, 0xf0,

    /* U+0070 "p" */
    0xf9, 0x99, 0xf8,

    /* U+0071 "q" */
    0xf9, 0x99, 0xf1,

    /* U+0072 "r" */
    0xbe, 0x48,

    /* U+0073 "s" */
    0x5d, 0x54,

    /* U+0074 "t" */
    0x5d, 0x24, 0xc0,

    /* U+0075 "u" */
    0x99, 0x99, 0xf0,

    /* U+0076 "v" */
    0xbb, 0xba, 0xe0,

    /* U+0077 "w" */
    0x93, 0x26, 0x4c, 0x8f, 0xc0,

    /* U+0078 "x" */
    0x99, 0x69, 0x90,

    /* U+0079 "y" */
    0x99, 0x99, 0xf9, 0xf0,

    /* U+007A "z" */
    0xf2, 0x48, 0xf0,

    /* U+007B "{" */
    0x29, 0x64, 0x91,

    /* U+007C "|" */
    0xff,

    /* U+007D "}" */
    0x89, 0x34, 0x94,

    /* U+007E "~" */
    0x5a
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 50, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 40, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 60, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 3, .adv_w = 100, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 9, .adv_w = 70, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 12, .adv_w = 110, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 18, .adv_w = 100, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 22, .adv_w = 30, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 23, .adv_w = 50, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 25, .adv_w = 50, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 28, .adv_w = 70, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 30, .adv_w = 70, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 32, .adv_w = 40, .box_w = 1, .box_h = 2, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 33, .adv_w = 70, .box_w = 3, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 34, .adv_w = 40, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 35, .adv_w = 70, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 41, .adv_w = 50, .box_w = 2, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 43, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 46, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = -1, .ofs_y = 0},
    {.bitmap_index = 49, .adv_w = 90, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 53, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 56, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 59, .adv_w = 80, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 62, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 65, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 68, .adv_w = 40, .box_w = 1, .box_h = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 40, .box_w = 1, .box_h = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 70, .adv_w = 70, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 72, .adv_w = 80, .box_w = 4, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 73, .adv_w = 70, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 75, .adv_w = 80, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 78, .adv_w = 110, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 89, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 95, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 98, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 101, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 104, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 107, .adv_w = 40, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 111, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 70, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 120, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 100, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 126, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 136, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 70, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 80, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 145, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 151, .adv_w = 120, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 80, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 166, .adv_w = 50, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 168, .adv_w = 70, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 171, .adv_w = 50, .box_w = 2, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 173, .adv_w = 70, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 174, .adv_w = 100, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 175, .adv_w = 50, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 176, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 179, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 182, .adv_w = 70, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 184, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 190, .adv_w = 70, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 197, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 200, .adv_w = 40, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 201, .adv_w = 70, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 204, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 40, .box_w = 1, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 120, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 213, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 216, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 219, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 222, .adv_w = 80, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 225, .adv_w = 70, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 70, .box_w = 3, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 229, .adv_w = 60, .box_w = 3, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 232, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 235, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 238, .adv_w = 120, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 246, .adv_w = 80, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 250, .adv_w = 80, .box_w = 4, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 253, .adv_w = 50, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 256, .adv_w = 30, .box_w = 1, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 257, .adv_w = 50, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 260, .adv_w = 80, .box_w = 4, .box_h = 2, .ofs_x = 0, .ofs_y = 3}
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
const lv_font_t ChiKareGo2_10 = {
#else
lv_font_t ChiKareGo2_10 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 10,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if CHIKAREGO2_10*/

