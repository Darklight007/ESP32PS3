/*******************************************************************************
 * Size: 18 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef TAURI_R_18
#define TAURI_R_18 1
#endif

#if TAURI_R_18

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0x6d, 0xb6, 0xdb, 0x60, 0xf, 0xf8,

    /* U+0022 "\"" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x30,

    /* U+0023 "#" */
    0x6, 0x30, 0x18, 0x80, 0x46, 0x3, 0x18, 0xc,
    0x61, 0xff, 0xf0, 0xc4, 0x2, 0x30, 0x18, 0xc0,
    0x63, 0xf, 0xff, 0x86, 0x60, 0x11, 0x80, 0xc6,
    0x3, 0x18, 0x0,

    /* U+0024 "$" */
    0x30, 0x60, 0xc3, 0xec, 0x18, 0x30, 0x70, 0x70,
    0x70, 0x70, 0x70, 0x60, 0xe1, 0xfe, 0x70, 0x60,
    0xc0,

    /* U+0025 "%" */
    0x38, 0x30, 0x36, 0x18, 0x31, 0x8c, 0x18, 0xcc,
    0xc, 0x66, 0x6, 0x36, 0x1, 0xb3, 0x0, 0x71,
    0x9c, 0x1, 0x9b, 0x0, 0xd8, 0xc0, 0xcc, 0x60,
    0x66, 0x30, 0x33, 0x18, 0x30, 0xd8, 0x18, 0x38,

    /* U+0026 "&" */
    0x1f, 0x3, 0x10, 0x60, 0x6, 0x0, 0x60, 0xc3,
    0xc, 0x3f, 0xe6, 0xc, 0xc0, 0xcc, 0xc, 0xc0,
    0xcc, 0xc, 0x61, 0xc3, 0xff,

    /* U+0027 "'" */
    0xff, 0xf0,

    /* U+0028 "(" */
    0xc, 0x73, 0x9c, 0x63, 0x8c, 0x30, 0xc3, 0xc,
    0x30, 0x61, 0x87, 0xe, 0x1c, 0x30,

    /* U+0029 ")" */
    0xc3, 0x87, 0xe, 0x18, 0x70, 0xc3, 0xc, 0x30,
    0xc3, 0x18, 0x63, 0x9c, 0xe3, 0x0,

    /* U+002A "*" */
    0xc, 0x26, 0x5b, 0x67, 0xe3, 0xf3, 0x6d, 0x32,
    0x18,

    /* U+002B "+" */
    0x18, 0x18, 0x18, 0xff, 0x18, 0x18, 0x18, 0x18,

    /* U+002C "," */
    0xef, 0xf3, 0x6c,

    /* U+002D "-" */
    0xfc,

    /* U+002E "." */
    0xff, 0x80,

    /* U+002F "/" */
    0x0, 0xc0, 0x60, 0x18, 0xc, 0x3, 0x0, 0x80,
    0x60, 0x18, 0xc, 0x3, 0x1, 0x80, 0x60, 0x10,
    0xc, 0x3, 0x1, 0x80, 0x60, 0x30, 0x0,

    /* U+0030 "0" */
    0x1e, 0xc, 0xe6, 0x19, 0x3, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0xc0, 0xf0, 0x3c, 0x9, 0x86, 0x73,
    0x7, 0x80,

    /* U+0031 "1" */
    0x3f, 0xff, 0x31, 0x8c, 0x63, 0x18, 0xc6, 0x31,
    0x8c,

    /* U+0032 "2" */
    0x7c, 0xc6, 0x83, 0x3, 0x3, 0x3, 0x7, 0xe,
    0xe, 0x1c, 0x38, 0x70, 0x70, 0xff,

    /* U+0033 "3" */
    0x7c, 0x23, 0x0, 0xc0, 0x60, 0x70, 0x70, 0x70,
    0x3e, 0x3, 0x80, 0xc0, 0x60, 0x3c, 0x33, 0xf0,

    /* U+0034 "4" */
    0x3, 0x1, 0xc0, 0x70, 0x3c, 0x1b, 0x6, 0xc3,
    0x30, 0xcc, 0x63, 0x38, 0xcf, 0xfc, 0xc, 0x3,
    0x0, 0xc0,

    /* U+0035 "5" */
    0x7f, 0x30, 0x18, 0xc, 0x6, 0x3, 0x1, 0xf8,
    0x86, 0x1, 0x80, 0xc0, 0x60, 0x3c, 0x37, 0xe0,

    /* U+0036 "6" */
    0x1f, 0x18, 0x98, 0x8, 0xc, 0x6, 0xf3, 0x8d,
    0x83, 0xc1, 0xe0, 0xf0, 0x6c, 0x36, 0x30, 0xf0,

    /* U+0037 "7" */
    0xff, 0x81, 0x81, 0xc0, 0xe0, 0x60, 0x70, 0x38,
    0x18, 0x1c, 0xe, 0x6, 0x7, 0x3, 0x1, 0x80,

    /* U+0038 "8" */
    0x3c, 0x3b, 0x98, 0xcc, 0x66, 0x31, 0xb0, 0xf0,
    0xc6, 0xc1, 0xe0, 0xf0, 0x78, 0x36, 0x31, 0xf0,

    /* U+0039 "9" */
    0x3c, 0x31, 0xb0, 0xd8, 0x3c, 0x1e, 0xf, 0x6,
    0xc7, 0x3d, 0x80, 0xc0, 0x40, 0x64, 0x63, 0xe0,

    /* U+003A ":" */
    0xff, 0x80, 0x7, 0xfc,

    /* U+003B ";" */
    0xee, 0xe0, 0x0, 0xe, 0xff, 0x76, 0xc0,

    /* U+003C "<" */
    0x3, 0x7, 0x7, 0x7, 0x6, 0x3, 0x0, 0xe0,
    0x38, 0xe, 0x1, 0x80,

    /* U+003D "=" */
    0xff, 0x0, 0x0, 0x0, 0xff,

    /* U+003E ">" */
    0x60, 0x30, 0x1c, 0xe, 0x7, 0x7, 0xe, 0x1c,
    0x30, 0x60,

    /* U+003F "?" */
    0xf9, 0x18, 0x18, 0x30, 0x63, 0x8e, 0x18, 0x30,
    0x0, 0x0, 0x7, 0xe, 0x1c, 0x0,

    /* U+0040 "@" */
    0x3, 0xf8, 0x7, 0x7, 0x6, 0x0, 0xc6, 0x0,
    0x66, 0x0, 0x1b, 0xf, 0x8f, 0x1c, 0xc7, 0x8c,
    0x63, 0xcc, 0x31, 0xe6, 0x18, 0xf3, 0x1c, 0xd9,
    0x8e, 0xc6, 0x7b, 0xc3, 0x0, 0x0, 0xc0, 0x0,
    0x38, 0x30, 0x7, 0xf0, 0x0,

    /* U+0041 "A" */
    0xe, 0x1, 0x40, 0x6c, 0xd, 0x81, 0xb0, 0x23,
    0xc, 0x61, 0x8c, 0x31, 0x8f, 0xf9, 0x83, 0x30,
    0x66, 0xf, 0xc0, 0xc0,

    /* U+0042 "B" */
    0xfc, 0x63, 0xb0, 0xd8, 0x6c, 0x36, 0x33, 0xf9,
    0x86, 0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x37, 0xf0,

    /* U+0043 "C" */
    0x1f, 0x98, 0x58, 0x8, 0xc, 0x6, 0x3, 0x1,
    0x80, 0xc0, 0x60, 0x18, 0xc, 0x3, 0x8, 0xfc,

    /* U+0044 "D" */
    0xfc, 0x63, 0x30, 0xd8, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0xc1, 0xe0, 0xf0, 0x58, 0x6c, 0x67, 0xe0,

    /* U+0045 "E" */
    0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0,
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xff,

    /* U+0046 "F" */
    0xff, 0x83, 0x6, 0xc, 0x18, 0x3f, 0x60, 0xc1,
    0x83, 0x6, 0xc, 0x18, 0x0,

    /* U+0047 "G" */
    0x1f, 0x18, 0x98, 0x18, 0xc, 0x6, 0x3, 0x1,
    0x8f, 0xc1, 0xe0, 0xd8, 0x6c, 0x33, 0x18, 0xfc,

    /* U+0048 "H" */
    0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1e, 0xf, 0xff,
    0x83, 0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1e, 0xc,

    /* U+0049 "I" */
    0xff, 0xff, 0xff, 0xf0,

    /* U+004A "J" */
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x6c,

    /* U+004B "K" */
    0xc3, 0x63, 0xb1, 0x99, 0x8c, 0xc6, 0xc3, 0xe1,
    0xf8, 0xec, 0x67, 0x31, 0x98, 0xec, 0x36, 0x1c,

    /* U+004C "L" */
    0xc1, 0x83, 0x6, 0xc, 0x18, 0x30, 0x60, 0xc1,
    0x83, 0x6, 0xc, 0x1f, 0xc0,

    /* U+004D "M" */
    0xe0, 0x7f, 0x7, 0xf0, 0xff, 0xf, 0xd9, 0xbd,
    0x9b, 0xcf, 0x3c, 0xf3, 0xc6, 0x3c, 0x63, 0xc6,
    0x3c, 0x3, 0xc0, 0x3c, 0x3,

    /* U+004E "N" */
    0xc1, 0xf0, 0xf8, 0x7e, 0x3f, 0x1e, 0xcf, 0x67,
    0x9b, 0xcd, 0xe3, 0xf1, 0xf8, 0x7c, 0x3e, 0xc,

    /* U+004F "O" */
    0x1e, 0xc, 0xc6, 0x1b, 0x2, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0xc0, 0xf0, 0x3c, 0xd, 0x86, 0x33,
    0x7, 0x80,

    /* U+0050 "P" */
    0xfc, 0x63, 0xb0, 0x78, 0x3c, 0x1e, 0xf, 0x7,
    0x86, 0xfc, 0x60, 0x30, 0x18, 0xc, 0x6, 0x0,

    /* U+0051 "Q" */
    0x1e, 0xc, 0xc6, 0x1b, 0x2, 0xc0, 0xf0, 0x3c,
    0xf, 0x3, 0xc0, 0xf0, 0x3e, 0x1d, 0x86, 0x3f,
    0x7, 0x80, 0xc0, 0x30, 0x7, 0x0,

    /* U+0052 "R" */
    0xfe, 0x61, 0xb0, 0x78, 0x3c, 0x1e, 0xf, 0xd,
    0xfc, 0xce, 0x63, 0x31, 0xd8, 0x6c, 0x3e, 0xc,

    /* U+0053 "S" */
    0x3c, 0xc3, 0x6, 0xe, 0x1e, 0x1e, 0x1e, 0x1e,
    0x1c, 0x18, 0x38, 0xdf, 0x0,

    /* U+0054 "T" */
    0xff, 0x8c, 0x6, 0x3, 0x1, 0x80, 0xc0, 0x60,
    0x30, 0x18, 0xc, 0x6, 0x3, 0x1, 0x80, 0xc0,

    /* U+0055 "U" */
    0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0xc1, 0xe0, 0xf0, 0x78, 0x36, 0x31, 0xf0,

    /* U+0056 "V" */
    0xe0, 0xd8, 0x36, 0xd, 0x87, 0x71, 0x8c, 0x63,
    0x18, 0xc6, 0x11, 0x6, 0xc1, 0xb0, 0x6c, 0xa,
    0x3, 0x80,

    /* U+0057 "W" */
    0xe1, 0xc3, 0x61, 0xc3, 0x61, 0xc3, 0x61, 0xc3,
    0x63, 0xc6, 0x73, 0x66, 0x33, 0x66, 0x33, 0x66,
    0x32, 0x66, 0x32, 0x64, 0x36, 0x24, 0x16, 0x3c,
    0x1e, 0x3c, 0x1c, 0x3c,

    /* U+0058 "X" */
    0x60, 0xce, 0x30, 0xc6, 0x1d, 0x81, 0xb0, 0x3c,
    0x3, 0x80, 0x70, 0x1f, 0x3, 0x60, 0xce, 0x18,
    0xc6, 0x1d, 0xc1, 0x80,

    /* U+0059 "Y" */
    0xc1, 0xd8, 0x66, 0x19, 0xcc, 0x33, 0xc, 0x81,
    0xe0, 0x78, 0xc, 0x3, 0x0, 0xc0, 0x30, 0xc,
    0x3, 0x0,

    /* U+005A "Z" */
    0x7f, 0x81, 0xc1, 0xc0, 0xe0, 0x60, 0x70, 0x30,
    0x38, 0x18, 0x1c, 0xe, 0xe, 0x7, 0x3, 0xfc,

    /* U+005B "[" */
    0xfe, 0x31, 0x8c, 0x63, 0x18, 0xc6, 0x31, 0x8c,
    0x63, 0x18, 0xc7, 0xc0,

    /* U+005C "\\" */
    0xc0, 0x10, 0x6, 0x1, 0x80, 0x30, 0xc, 0x1,
    0x80, 0x60, 0xc, 0x3, 0x0, 0xc0, 0x18, 0x6,
    0x0, 0xc0, 0x30, 0x6, 0x1, 0x80, 0x20,

    /* U+005D "]" */
    0xf8, 0xc6, 0x31, 0x8c, 0x63, 0x18, 0xc6, 0x31,
    0x8c, 0x63, 0x1f, 0xc0,

    /* U+005E "^" */
    0x1c, 0xe, 0x5, 0x86, 0xc3, 0x61, 0x91, 0x8c,
    0xc6, 0x63, 0x60, 0xc0,

    /* U+005F "_" */
    0xff, 0xf8,

    /* U+0060 "`" */
    0xce, 0x63,

    /* U+0061 "a" */
    0x7e, 0x46, 0x3, 0x3, 0x3f, 0xe3, 0xc3, 0xc3,
    0xe7, 0x7f,

    /* U+0062 "b" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc6, 0xc3,
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc6, 0xfc,

    /* U+0063 "c" */
    0x3e, 0xc7, 0x6, 0xc, 0x18, 0x30, 0x60, 0x62,
    0x7c,

    /* U+0064 "d" */
    0x3, 0x3, 0x3, 0x3, 0x3, 0x3f, 0x63, 0xc3,
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x63, 0x3f,

    /* U+0065 "e" */
    0x3c, 0x66, 0xc3, 0xc3, 0xff, 0xc0, 0xc0, 0xe0,
    0x61, 0x3f,

    /* U+0066 "f" */
    0x3b, 0x18, 0xc6, 0x7d, 0x8c, 0x63, 0x18, 0xc6,
    0x31, 0x80,

    /* U+0067 "g" */
    0x3f, 0xf3, 0xb0, 0xd8, 0x6c, 0x71, 0xf1, 0x80,
    0xc0, 0x60, 0x3f, 0xb8, 0x78, 0x3e, 0x3b, 0xf0,

    /* U+0068 "h" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0xe7, 0xc3,
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,

    /* U+0069 "i" */
    0xe0, 0x1, 0xb6, 0xdb, 0x6d, 0xb0,

    /* U+006A "j" */
    0x70, 0x0, 0x6, 0x66, 0x66, 0x66, 0x66, 0x66,
    0x6e, 0xc0,

    /* U+006B "k" */
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc6, 0xcc, 0xdc,
    0xf8, 0xf8, 0xf8, 0xcc, 0xce, 0xc6, 0xc3,

    /* U+006C "l" */
    0xff, 0xff, 0xff, 0xfc,

    /* U+006D "m" */
    0xfe, 0xfb, 0x9e, 0x7c, 0x30, 0xf0, 0xc3, 0xc3,
    0xf, 0xc, 0x3c, 0x30, 0xf0, 0xc3, 0xc3, 0xf,
    0xc, 0x30,

    /* U+006E "n" */
    0xfe, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0xc3, 0xc3,

    /* U+006F "o" */
    0x3e, 0x31, 0xb0, 0x78, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0x63, 0x1f, 0x0,

    /* U+0070 "p" */
    0xfc, 0xc6, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0xc6, 0xfc, 0xc0, 0xc0, 0xc0, 0xc0,

    /* U+0071 "q" */
    0x3f, 0x63, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0x63, 0x3f, 0x3, 0x3, 0x3, 0x3,

    /* U+0072 "r" */
    0xdf, 0xf1, 0x8c, 0x63, 0x18, 0xc6, 0x0,

    /* U+0073 "s" */
    0x7b, 0xc, 0x38, 0xf1, 0xf1, 0xc3, 0x8f, 0xe0,

    /* U+0074 "t" */
    0x61, 0x86, 0x3e, 0x61, 0x86, 0x18, 0x61, 0x86,
    0x18, 0x3c,

    /* U+0075 "u" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0xe7, 0x7f,

    /* U+0076 "v" */
    0xc1, 0xb1, 0xd8, 0xcc, 0x63, 0x31, 0xb0, 0xd8,
    0x2c, 0x1c, 0xe, 0x0,

    /* U+0077 "w" */
    0xc3, 0x8c, 0xc7, 0x19, 0x8e, 0x33, 0x34, 0x66,
    0x6c, 0x84, 0xdb, 0xd, 0x36, 0x1a, 0x2c, 0x3c,
    0x70, 0x38, 0xe0,

    /* U+0078 "x" */
    0x63, 0x39, 0x8d, 0x83, 0xc1, 0xc0, 0xe0, 0xf8,
    0x6c, 0x63, 0x31, 0xc0,

    /* U+0079 "y" */
    0xe1, 0xb0, 0xd8, 0xec, 0x63, 0x31, 0x98, 0xd8,
    0x3c, 0x1e, 0xe, 0x3, 0x1, 0x81, 0x81, 0x80,

    /* U+007A "z" */
    0x7f, 0x7, 0xe, 0xe, 0x1c, 0x1c, 0x38, 0x38,
    0x70, 0x7f,

    /* U+007B "{" */
    0x1e, 0x60, 0xc1, 0x83, 0x6, 0xc, 0x18, 0xe1,
    0xc0, 0xc1, 0x83, 0x6, 0xc, 0x18, 0x30, 0x3c,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xff, 0xff,

    /* U+007D "}" */
    0xe0, 0xc3, 0xc, 0x30, 0xc3, 0xe, 0x1c, 0x73,
    0xc, 0x30, 0xc3, 0xc, 0x33, 0x80,

    /* U+007E "~" */
    0x71, 0x47, 0x80,

    /* U+007F "" */
    0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 86, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 103, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 7, .adv_w = 141, .box_w = 6, .box_h = 6, .ofs_x = 2, .ofs_y = 9},
    {.bitmap_index = 12, .adv_w = 268, .box_w = 14, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 39, .adv_w = 150, .box_w = 7, .box_h = 19, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 56, .adv_w = 313, .box_w = 17, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 88, .adv_w = 225, .box_w = 12, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 81, .box_w = 2, .box_h = 6, .ofs_x = 2, .ofs_y = 9},
    {.bitmap_index = 111, .adv_w = 159, .box_w = 6, .box_h = 18, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 125, .adv_w = 159, .box_w = 6, .box_h = 18, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 139, .adv_w = 185, .box_w = 9, .box_h = 8, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 148, .adv_w = 189, .box_w = 8, .box_h = 8, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 156, .adv_w = 105, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 159, .adv_w = 157, .box_w = 6, .box_h = 1, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 160, .adv_w = 102, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 162, .adv_w = 220, .box_w = 10, .box_h = 18, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 185, .adv_w = 189, .box_w = 10, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 203, .adv_w = 126, .box_w = 5, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 212, .adv_w = 163, .box_w = 8, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 226, .adv_w = 167, .box_w = 9, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 242, .adv_w = 189, .box_w = 10, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 260, .adv_w = 174, .box_w = 9, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 276, .adv_w = 182, .box_w = 9, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 159, .box_w = 9, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 308, .adv_w = 181, .box_w = 9, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 324, .adv_w = 182, .box_w = 9, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 103, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 344, .adv_w = 108, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 351, .adv_w = 171, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 363, .adv_w = 189, .box_w = 8, .box_h = 5, .ofs_x = 2, .ofs_y = 4},
    {.bitmap_index = 368, .adv_w = 171, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 378, .adv_w = 142, .box_w = 7, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 392, .adv_w = 322, .box_w = 17, .box_h = 17, .ofs_x = 2, .ofs_y = -4},
    {.bitmap_index = 429, .adv_w = 179, .box_w = 11, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 449, .adv_w = 189, .box_w = 9, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 465, .adv_w = 159, .box_w = 9, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 481, .adv_w = 191, .box_w = 9, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 497, .adv_w = 162, .box_w = 8, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 511, .adv_w = 153, .box_w = 7, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 524, .adv_w = 188, .box_w = 9, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 540, .adv_w = 204, .box_w = 9, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 556, .adv_w = 90, .box_w = 2, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 560, .adv_w = 103, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 567, .adv_w = 181, .box_w = 9, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 583, .adv_w = 140, .box_w = 7, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 596, .adv_w = 250, .box_w = 12, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 617, .adv_w = 210, .box_w = 9, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 633, .adv_w = 189, .box_w = 10, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 651, .adv_w = 174, .box_w = 9, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 667, .adv_w = 189, .box_w = 10, .box_h = 17, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 689, .adv_w = 180, .box_w = 9, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 705, .adv_w = 149, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 718, .adv_w = 158, .box_w = 9, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 734, .adv_w = 203, .box_w = 9, .box_h = 14, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 750, .adv_w = 169, .box_w = 10, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 768, .adv_w = 268, .box_w = 16, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 796, .adv_w = 175, .box_w = 11, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 816, .adv_w = 169, .box_w = 10, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 834, .adv_w = 164, .box_w = 9, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 850, .adv_w = 133, .box_w = 5, .box_h = 18, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 862, .adv_w = 220, .box_w = 10, .box_h = 18, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 885, .adv_w = 133, .box_w = 5, .box_h = 18, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 897, .adv_w = 144, .box_w = 9, .box_h = 10, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 909, .adv_w = 189, .box_w = 13, .box_h = 1, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 911, .adv_w = 14, .box_w = 4, .box_h = 4, .ofs_x = -3, .ofs_y = 11},
    {.bitmap_index = 913, .adv_w = 160, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 923, .adv_w = 167, .box_w = 8, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 938, .adv_w = 135, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 947, .adv_w = 172, .box_w = 8, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 962, .adv_w = 157, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 972, .adv_w = 98, .box_w = 5, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 982, .adv_w = 170, .box_w = 9, .box_h = 14, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 998, .adv_w = 176, .box_w = 8, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1013, .adv_w = 85, .box_w = 3, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1019, .adv_w = 86, .box_w = 4, .box_h = 19, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 1029, .adv_w = 159, .box_w = 8, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1044, .adv_w = 81, .box_w = 2, .box_h = 15, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1048, .adv_w = 275, .box_w = 14, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1066, .adv_w = 178, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1076, .adv_w = 167, .box_w = 9, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1088, .adv_w = 166, .box_w = 8, .box_h = 14, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 1102, .adv_w = 166, .box_w = 8, .box_h = 14, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 1116, .adv_w = 113, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1123, .adv_w = 133, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1131, .adv_w = 108, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1141, .adv_w = 173, .box_w = 8, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1151, .adv_w = 144, .box_w = 9, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1163, .adv_w = 235, .box_w = 15, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1182, .adv_w = 149, .box_w = 9, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1194, .adv_w = 146, .box_w = 9, .box_h = 14, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 1210, .adv_w = 144, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1220, .adv_w = 147, .box_w = 7, .box_h = 18, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1236, .adv_w = 93, .box_w = 2, .box_h = 20, .ofs_x = 2, .ofs_y = -4},
    {.bitmap_index = 1241, .adv_w = 147, .box_w = 6, .box_h = 18, .ofs_x = 2, .ofs_y = -1},
    {.bitmap_index = 1255, .adv_w = 185, .box_w = 9, .box_h = 2, .ofs_x = 2, .ofs_y = 6},
    {.bitmap_index = 1258, .adv_w = 0, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0}
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
const lv_font_t Tauri_R_18 = {
#else
lv_font_t Tauri_R_18 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 21,          /*The maximum line height required by the font*/
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



#endif /*#if TAURI_R_18*/

