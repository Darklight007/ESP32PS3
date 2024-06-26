/*******************************************************************************
 * Size: 22 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef TAURI_R_22
#define TAURI_R_22 1
#endif

#if TAURI_R_22

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x0, 0x6,
    0xff, 0x60,

    /* U+0022 "\"" */
    0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7,

    /* U+0023 "#" */
    0x3, 0xc, 0x1, 0x86, 0x0, 0xc3, 0x0, 0xe3,
    0x80, 0x71, 0xc0, 0x30, 0xc1, 0xff, 0xfe, 0xff,
    0xff, 0xe, 0x38, 0x7, 0x1c, 0x3, 0xc, 0x1,
    0x86, 0x0, 0xc3, 0x7, 0xff, 0xfb, 0xff, 0xfc,
    0x30, 0xc0, 0x18, 0x60, 0xc, 0x30, 0x6, 0x18,
    0x7, 0x1c, 0x0,

    /* U+0024 "$" */
    0x1c, 0xe, 0x7, 0x3, 0x83, 0xf3, 0xfb, 0xc5,
    0xc0, 0xe0, 0x70, 0x1c, 0xf, 0x83, 0xe0, 0xf8,
    0x1c, 0xf, 0x3, 0x81, 0xe1, 0xff, 0xef, 0xe0,
    0xe0, 0x70, 0x38, 0x1c, 0x0,

    /* U+0025 "%" */
    0x3e, 0x6, 0x3, 0xf8, 0x70, 0x3d, 0xe3, 0x81,
    0xc7, 0x38, 0xe, 0x39, 0xc0, 0x71, 0xcc, 0x3,
    0x8e, 0xe0, 0x1e, 0xf7, 0x0, 0x7f, 0x30, 0x1,
    0xf3, 0x9f, 0x0, 0x1d, 0xfc, 0x1, 0xde, 0xf0,
    0xe, 0xe3, 0x80, 0x77, 0x1c, 0x7, 0x38, 0xe0,
    0x39, 0xc7, 0x1, 0x8f, 0x78, 0x1c, 0x3f, 0x80,
    0xe0, 0xf8,

    /* U+0026 "&" */
    0x1f, 0x0, 0xfe, 0x7, 0x88, 0x1c, 0x0, 0x70,
    0x1, 0xc1, 0xc3, 0x87, 0xf, 0x1c, 0x1f, 0xfc,
    0xff, 0xf7, 0x7, 0x38, 0x1c, 0xe0, 0x73, 0x81,
    0xce, 0x7, 0x1c, 0x3c, 0x7f, 0xfc, 0x7e, 0xf0,

    /* U+0027 "'" */
    0xff, 0xff, 0xff,

    /* U+0028 "(" */
    0x2, 0x1c, 0x78, 0xe3, 0x8e, 0x1c, 0x38, 0xe1,
    0xc3, 0x87, 0xe, 0x1c, 0x38, 0x70, 0x70, 0xe1,
    0xe1, 0xc1, 0xc1, 0xc1, 0x80,

    /* U+0029 ")" */
    0x81, 0xc3, 0xc3, 0x83, 0x83, 0x87, 0xe, 0xe,
    0x1c, 0x38, 0x70, 0xe1, 0xc3, 0x87, 0x1c, 0x38,
    0xe1, 0xc7, 0x1c, 0x30, 0x0,

    /* U+002A "*" */
    0xc, 0x3, 0xc, 0xcf, 0xb7, 0x7f, 0x87, 0x87,
    0xfb, 0xb7, 0xcc, 0xc3, 0x0, 0xc0,

    /* U+002B "+" */
    0x1c, 0x7, 0x1, 0xc0, 0x70, 0x1c, 0x3f, 0xff,
    0xfc, 0x70, 0x1c, 0x7, 0x1, 0xc0,

    /* U+002C "," */
    0x6f, 0xf7, 0x37, 0xec,

    /* U+002D "-" */
    0xff, 0xff,

    /* U+002E "." */
    0x6f, 0xf6,

    /* U+002F "/" */
    0x0, 0x60, 0x1c, 0x3, 0x0, 0xe0, 0x18, 0x3,
    0x0, 0xe0, 0x18, 0x7, 0x0, 0xc0, 0x18, 0x7,
    0x0, 0xc0, 0x38, 0x6, 0x0, 0xc0, 0x38, 0x6,
    0x1, 0xc0, 0x30, 0x6, 0x1, 0xc0, 0x30, 0x0,

    /* U+0030 "0" */
    0xf, 0x3, 0xfc, 0x39, 0xe7, 0xe, 0x60, 0x6e,
    0x7, 0xe0, 0x7e, 0x7, 0xe0, 0x7e, 0x7, 0xe0,
    0x7e, 0x7, 0xe0, 0x76, 0x6, 0x70, 0xe7, 0x9c,
    0x3f, 0xc0, 0xf0,

    /* U+0031 "1" */
    0x1d, 0xff, 0xf7, 0x1c, 0x71, 0xc7, 0x1c, 0x71,
    0xc7, 0x1c, 0x71, 0xc7, 0x1c, 0x70,

    /* U+0032 "2" */
    0x3e, 0x3f, 0xec, 0x3a, 0x7, 0x1, 0xc0, 0x70,
    0x1c, 0x7, 0x3, 0x81, 0xe0, 0x70, 0x3c, 0x1e,
    0x7, 0x3, 0x81, 0xc0, 0xff, 0xff, 0xf0,

    /* U+0033 "3" */
    0x3f, 0xf, 0xf1, 0x8f, 0x0, 0xe0, 0x1c, 0x3,
    0x80, 0xe0, 0x78, 0x1f, 0x3, 0xf0, 0xf, 0x0,
    0x70, 0xe, 0x1, 0xe0, 0x3e, 0xe, 0xff, 0x87,
    0xe0,

    /* U+0034 "4" */
    0x1, 0xc0, 0x1e, 0x0, 0xf0, 0xf, 0x80, 0x7c,
    0x6, 0xe0, 0x77, 0x3, 0x38, 0x31, 0xc1, 0x8e,
    0x18, 0x71, 0xc3, 0x8f, 0xff, 0xff, 0xfc, 0x7,
    0x0, 0x38, 0x1, 0xc0, 0xe, 0x0,

    /* U+0035 "5" */
    0x7f, 0xcf, 0xf9, 0xc0, 0x38, 0x7, 0x0, 0xe0,
    0x1c, 0x3, 0xf8, 0x7f, 0xc8, 0x38, 0x3, 0x80,
    0x70, 0xe, 0x1, 0xc0, 0x7e, 0x1e, 0xff, 0x8f,
    0xe0,

    /* U+0036 "6" */
    0xf, 0xc1, 0xfc, 0x38, 0x47, 0x0, 0x70, 0xe,
    0x0, 0xe7, 0x8f, 0xfc, 0xf8, 0xee, 0xf, 0xe0,
    0x7e, 0x7, 0xe0, 0x76, 0x7, 0x70, 0xf7, 0x8e,
    0x3f, 0xc0, 0xf8,

    /* U+0037 "7" */
    0xff, 0xff, 0xf8, 0x7, 0x0, 0xe0, 0x38, 0x7,
    0x0, 0xe0, 0x38, 0x7, 0x0, 0xe0, 0x38, 0x7,
    0x0, 0xe0, 0x38, 0x7, 0x0, 0xe0, 0x38, 0x7,
    0x0,

    /* U+0038 "8" */
    0x1f, 0x87, 0xf9, 0xe7, 0xb8, 0x77, 0xe, 0xe1,
    0xce, 0x70, 0xfc, 0x3f, 0x8e, 0x3b, 0x83, 0xf0,
    0x7e, 0xf, 0xc1, 0xf8, 0x3b, 0x8e, 0x7f, 0xc3,
    0xe0,

    /* U+0039 "9" */
    0x1f, 0x3, 0xfc, 0x71, 0xee, 0xe, 0xe0, 0x6e,
    0x7, 0xe0, 0x7e, 0x7, 0x70, 0xf7, 0xf7, 0x1e,
    0x70, 0x7, 0x0, 0x70, 0xe, 0x0, 0xe2, 0x3c,
    0x3f, 0x83, 0xf0,

    /* U+003A ":" */
    0x6f, 0xf6, 0x0, 0x0, 0x6, 0xff, 0x60,

    /* U+003B ";" */
    0x6f, 0xf6, 0x0, 0x0, 0x6, 0xff, 0x73, 0x3e,
    0xc0,

    /* U+003C "<" */
    0x3, 0x81, 0xc0, 0xe0, 0x70, 0x38, 0x1c, 0xe,
    0x1, 0xc0, 0x38, 0x7, 0x0, 0xe0, 0x1c, 0x3,
    0x80,

    /* U+003D "=" */
    0xff, 0xff, 0xf0, 0x0, 0x0, 0x0, 0x3f, 0xff,
    0xfc,

    /* U+003E ">" */
    0x70, 0x7, 0x0, 0x70, 0x7, 0x0, 0x70, 0x7,
    0x0, 0x78, 0x1c, 0x7, 0x1, 0xc0, 0x70, 0x1c,
    0x7, 0x0,

    /* U+003F "?" */
    0x7e, 0x7f, 0xa1, 0xe0, 0x70, 0x38, 0x1c, 0x1c,
    0x1e, 0x3e, 0x1c, 0xe, 0x7, 0x0, 0x0, 0x0,
    0x0, 0x30, 0x3c, 0x1e, 0x6, 0x0,

    /* U+0040 "@" */
    0x0, 0xfe, 0x0, 0x3f, 0xf8, 0xf, 0x3, 0xc1,
    0xc0, 0xe, 0x38, 0x0, 0x63, 0x0, 0x7, 0x60,
    0x7e, 0x36, 0x1f, 0xe3, 0xe3, 0xc6, 0x3c, 0x38,
    0xe3, 0xc7, 0xe, 0x3c, 0x70, 0xc3, 0xc7, 0xc,
    0x6c, 0x70, 0xc6, 0xc7, 0x1c, 0xee, 0x3e, 0xfc,
    0x61, 0xc7, 0x87, 0x0, 0x0, 0x38, 0x1, 0x1,
    0xe0, 0x70, 0xf, 0xfe, 0x0, 0x3f, 0x80,

    /* U+0041 "A" */
    0x7, 0x80, 0x3c, 0x3, 0xe0, 0x1b, 0x0, 0xdc,
    0xe, 0xe0, 0x73, 0x3, 0x9c, 0x18, 0xe1, 0xc7,
    0xe, 0x18, 0x70, 0xe3, 0xff, 0x3f, 0xf9, 0xc0,
    0xce, 0x7, 0x60, 0x3f, 0x1, 0xc0,

    /* U+0042 "B" */
    0xfe, 0x1f, 0xf3, 0x8f, 0x70, 0xee, 0x1d, 0xc3,
    0xb8, 0x77, 0x1c, 0xff, 0x1f, 0xf3, 0x87, 0x70,
    0x7e, 0xf, 0xc1, 0xf8, 0x3f, 0xe, 0xff, 0xdf,
    0xe0,

    /* U+0043 "C" */
    0xf, 0xcf, 0xf3, 0x85, 0xc0, 0x70, 0x38, 0xe,
    0x3, 0x80, 0xe0, 0x38, 0xe, 0x3, 0x80, 0xe0,
    0x1c, 0x7, 0x0, 0xe1, 0x1f, 0xc3, 0xf0,

    /* U+0044 "D" */
    0xfe, 0xf, 0xf8, 0xe3, 0xce, 0xe, 0xe0, 0xee,
    0x7, 0xe0, 0x7e, 0x7, 0xe0, 0x7e, 0x7, 0xe0,
    0x7e, 0x7, 0xe0, 0x7e, 0xe, 0xe0, 0xee, 0x3c,
    0xff, 0x8f, 0xe0,

    /* U+0045 "E" */
    0xff, 0xbf, 0xee, 0x3, 0x80, 0xe0, 0x38, 0xe,
    0x3, 0x80, 0xff, 0x3f, 0xce, 0x3, 0x80, 0xe0,
    0x38, 0xe, 0x3, 0x80, 0xff, 0xff, 0xf0,

    /* U+0046 "F" */
    0xff, 0xff, 0xf8, 0x1c, 0xe, 0x7, 0x3, 0x81,
    0xc0, 0xff, 0x7f, 0xb8, 0x1c, 0xe, 0x7, 0x3,
    0x81, 0xc0, 0xe0, 0x70, 0x0,

    /* U+0047 "G" */
    0xf, 0xc1, 0xfe, 0x38, 0x67, 0x0, 0x70, 0xe,
    0x0, 0xe0, 0xe, 0x0, 0xe3, 0xfe, 0x3f, 0xe0,
    0x7e, 0x7, 0xe0, 0x77, 0x7, 0x70, 0x73, 0x87,
    0x1f, 0xf0, 0xfc,

    /* U+0048 "H" */
    0xe0, 0xfc, 0x1f, 0x83, 0xf0, 0x7e, 0xf, 0xc1,
    0xf8, 0x3f, 0xff, 0xff, 0xfc, 0x1f, 0x83, 0xf0,
    0x7e, 0xf, 0xc1, 0xf8, 0x3f, 0x7, 0xe0, 0xfc,
    0x1c,

    /* U+0049 "I" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,

    /* U+004A "J" */
    0x1c, 0x71, 0xc7, 0x1c, 0x71, 0xc7, 0x1c, 0x71,
    0xc7, 0x1c, 0x71, 0xcf, 0xfb, 0xc0,

    /* U+004B "K" */
    0xe0, 0xee, 0x1c, 0xe1, 0xce, 0x38, 0xe3, 0xe,
    0x70, 0xe6, 0xe, 0xe0, 0xfe, 0xf, 0xe0, 0xff,
    0xf, 0x30, 0xe3, 0x8e, 0x18, 0xe1, 0xce, 0x1c,
    0xe0, 0xee, 0xe,

    /* U+004C "L" */
    0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
    0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
    0xff, 0xff,

    /* U+004D "M" */
    0xf0, 0x1f, 0xf0, 0x7f, 0xe0, 0xff, 0xc1, 0xff,
    0xc7, 0xfd, 0x8d, 0xfb, 0x1b, 0xf3, 0x67, 0xe6,
    0xcf, 0xcf, 0x9f, 0x8e, 0x3f, 0x1c, 0x7e, 0x18,
    0xfc, 0x21, 0xf8, 0x3, 0xf0, 0x7, 0xe0, 0xf,
    0xc0, 0x1c,

    /* U+004E "N" */
    0xe0, 0x7e, 0x7, 0xf0, 0x7f, 0x7, 0xf8, 0x7f,
    0x87, 0xec, 0x7e, 0xc7, 0xe6, 0x7e, 0x67, 0xe3,
    0x7e, 0x37, 0xe1, 0xfe, 0x1f, 0xe0, 0xfe, 0xf,
    0xe0, 0x7e, 0x7,

    /* U+004F "O" */
    0xf, 0x3, 0xfc, 0x79, 0xe7, 0xe, 0x70, 0xee,
    0x7, 0xe0, 0x7e, 0x7, 0xe0, 0x7e, 0x7, 0xe0,
    0x7e, 0x7, 0xe0, 0x77, 0xe, 0x70, 0xe7, 0x9e,
    0x3f, 0xc0, 0xf0,

    /* U+0050 "P" */
    0xff, 0x1f, 0xf3, 0x8f, 0x70, 0x7e, 0xf, 0xc1,
    0xf8, 0x3f, 0x7, 0xe1, 0xdf, 0xf3, 0xfc, 0x70,
    0xe, 0x1, 0xc0, 0x38, 0x7, 0x0, 0xe0, 0x1c,
    0x0,

    /* U+0051 "Q" */
    0xf, 0x3, 0xfc, 0x79, 0xe7, 0xe, 0x70, 0xee,
    0x7, 0xe0, 0x7e, 0x7, 0xe0, 0x7e, 0x7, 0xe0,
    0x7e, 0x7, 0xe0, 0xf7, 0xe, 0x79, 0xe7, 0xfe,
    0x3f, 0xc0, 0xf0, 0x6, 0x0, 0x60, 0x7, 0x80,
    0x38,

    /* U+0052 "R" */
    0xff, 0x1f, 0xf3, 0x8f, 0x70, 0x7e, 0xf, 0xc1,
    0xf8, 0x3f, 0x7, 0xe1, 0xdf, 0xf3, 0xf8, 0x73,
    0x8e, 0x31, 0xc7, 0x38, 0xe7, 0xe, 0xe1, 0xfc,
    0x1c,

    /* U+0053 "S" */
    0x3f, 0x3f, 0xbc, 0x5c, 0xe, 0x7, 0x3, 0xc0,
    0xf0, 0x7c, 0x1f, 0x83, 0xc0, 0xf0, 0x38, 0x1c,
    0xf, 0xe, 0xff, 0x3e, 0x0,

    /* U+0054 "T" */
    0xff, 0xff, 0xfc, 0x38, 0x7, 0x0, 0xe0, 0x1c,
    0x3, 0x80, 0x70, 0xe, 0x1, 0xc0, 0x38, 0x7,
    0x0, 0xe0, 0x1c, 0x3, 0x80, 0x70, 0xe, 0x1,
    0xc0,

    /* U+0055 "U" */
    0xe0, 0xfc, 0x1f, 0x83, 0xf0, 0x7e, 0xf, 0xc1,
    0xf8, 0x3f, 0x7, 0xe0, 0xfc, 0x1f, 0x83, 0xf0,
    0x7e, 0xf, 0xc1, 0xf8, 0x3b, 0x8e, 0x3f, 0x83,
    0xe0,

    /* U+0056 "V" */
    0xe0, 0x3b, 0x1, 0x9c, 0x1c, 0xe0, 0xe7, 0x7,
    0x18, 0x30, 0xe1, 0x87, 0x1c, 0x38, 0xe0, 0xc7,
    0x7, 0x30, 0x39, 0x80, 0xdc, 0x6, 0xe0, 0x36,
    0x1, 0xf0, 0x7, 0x80, 0x3c, 0x0,

    /* U+0057 "W" */
    0x60, 0x70, 0x77, 0x7, 0x7, 0x70, 0x70, 0x77,
    0xf, 0x6, 0x70, 0xf0, 0x67, 0xf, 0x8e, 0x30,
    0xd8, 0xe3, 0xd, 0x8e, 0x39, 0xd8, 0xc3, 0x99,
    0x8c, 0x39, 0x9c, 0xc1, 0x99, 0xcc, 0x19, 0x8d,
    0xc1, 0xb8, 0xd8, 0x1b, 0xd, 0x81, 0xf0, 0xd8,
    0xf, 0xf, 0x80, 0xf0, 0x78,

    /* U+0058 "X" */
    0x70, 0x73, 0x83, 0x8e, 0x38, 0x71, 0xc1, 0xdc,
    0xe, 0xe0, 0x3e, 0x1, 0xf0, 0x7, 0x0, 0x38,
    0x3, 0xe0, 0x1b, 0x81, 0xdc, 0xc, 0x70, 0xe3,
    0x86, 0xe, 0x70, 0x73, 0x1, 0xc0,

    /* U+0059 "Y" */
    0xe0, 0x33, 0x3, 0x9c, 0x1c, 0x61, 0xc3, 0x8e,
    0x1c, 0x60, 0x77, 0x3, 0xb0, 0xf, 0x80, 0x7c,
    0x1, 0xc0, 0xe, 0x0, 0x70, 0x3, 0x80, 0x1c,
    0x0, 0xe0, 0x7, 0x0, 0x38, 0x0,

    /* U+005A "Z" */
    0x7f, 0xef, 0xfc, 0x7, 0x0, 0xe0, 0x38, 0x7,
    0x1, 0xc0, 0x38, 0xe, 0x1, 0xc0, 0x38, 0xe,
    0x1, 0xc0, 0x70, 0xe, 0x3, 0x80, 0x7f, 0xff,
    0xfc,

    /* U+005B "[" */
    0xff, 0xfe, 0x38, 0xe3, 0x8e, 0x38, 0xe3, 0x8e,
    0x38, 0xe3, 0x8e, 0x38, 0xe3, 0x8e, 0x38, 0xe3,
    0x8f, 0xff,

    /* U+005C "\\" */
    0xc0, 0x1c, 0x1, 0x80, 0x30, 0x7, 0x0, 0x60,
    0xe, 0x0, 0xc0, 0x18, 0x3, 0x80, 0x30, 0x7,
    0x0, 0x60, 0xc, 0x1, 0xc0, 0x18, 0x3, 0x80,
    0x30, 0x6, 0x0, 0xe0, 0xc, 0x1, 0xc0, 0x18,

    /* U+005D "]" */
    0xff, 0xfc, 0x38, 0x70, 0xe1, 0xc3, 0x87, 0xe,
    0x1c, 0x38, 0x70, 0xe1, 0xc3, 0x87, 0xe, 0x1c,
    0x38, 0x70, 0xe1, 0xff, 0xff,

    /* U+005E "^" */
    0xe, 0x1, 0xc0, 0x7c, 0xd, 0x81, 0xb0, 0x37,
    0xe, 0x61, 0x8c, 0x31, 0xce, 0x39, 0x83, 0x30,
    0x6e, 0xe,

    /* U+005F "_" */
    0xff, 0xff, 0xff, 0xff,

    /* U+0060 "`" */
    0xe1, 0xc7, 0xe, 0x18,

    /* U+0061 "a" */
    0x3f, 0x1f, 0xe4, 0x3c, 0x7, 0x1, 0xcf, 0xf7,
    0xff, 0xc7, 0xe1, 0xf8, 0x7f, 0x3d, 0xff, 0x3d,
    0xc0,

    /* U+0062 "b" */
    0xe0, 0x38, 0xe, 0x3, 0x80, 0xe0, 0x38, 0xe,
    0xf3, 0xfe, 0xe3, 0xb8, 0x7e, 0x1f, 0x87, 0xe1,
    0xf8, 0x7e, 0x1f, 0x87, 0xe3, 0xbf, 0xcf, 0xe0,

    /* U+0063 "c" */
    0x1f, 0x1f, 0xdc, 0x3e, 0xe, 0x7, 0x3, 0x81,
    0xc0, 0xe0, 0x78, 0x1c, 0x27, 0xf1, 0xf0,

    /* U+0064 "d" */
    0x0, 0xe0, 0x1c, 0x3, 0x80, 0x70, 0xe, 0x1,
    0xc7, 0xf9, 0xff, 0x70, 0xfe, 0x1f, 0x83, 0xf0,
    0x7e, 0xf, 0xc1, 0xf8, 0x3f, 0x87, 0x70, 0xe7,
    0xfc, 0x7b, 0x80,

    /* U+0065 "e" */
    0x1f, 0xf, 0xe7, 0x1f, 0x87, 0xe1, 0xff, 0xff,
    0xff, 0x80, 0xe0, 0x3c, 0x7, 0x8, 0xfe, 0x1f,
    0x0,

    /* U+0066 "f" */
    0x1c, 0xf7, 0x1c, 0x71, 0xcf, 0xff, 0x71, 0xc7,
    0x1c, 0x71, 0xc7, 0x1c, 0x71, 0xc7, 0x0,

    /* U+0067 "g" */
    0x3e, 0xf7, 0xff, 0xf3, 0x8e, 0x1c, 0xe1, 0xce,
    0x1c, 0x73, 0xc7, 0xf8, 0x1f, 0x7, 0x0, 0x70,
    0x7, 0xf8, 0x3f, 0xce, 0x1e, 0xe0, 0xee, 0xe,
    0xf1, 0xe7, 0xfc, 0x3f, 0x0,

    /* U+0068 "h" */
    0xe0, 0x38, 0xe, 0x3, 0x80, 0xe0, 0x38, 0xe,
    0xf3, 0xfe, 0xf3, 0xf8, 0x7e, 0x1f, 0x87, 0xe1,
    0xf8, 0x7e, 0x1f, 0x87, 0xe1, 0xf8, 0x7e, 0x1c,

    /* U+0069 "i" */
    0xff, 0x80, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x80,

    /* U+006A "j" */
    0x77, 0x70, 0x0, 0x77, 0x77, 0x77, 0x77, 0x77,
    0x77, 0x77, 0x77, 0x7e, 0xc0,

    /* U+006B "k" */
    0xe0, 0x1c, 0x3, 0x80, 0x70, 0xe, 0x1, 0xc0,
    0x38, 0x77, 0x1c, 0xe7, 0x1d, 0xc3, 0xb0, 0x7e,
    0xf, 0xe1, 0xfc, 0x39, 0xc7, 0x1c, 0xe3, 0x9c,
    0x3b, 0x83, 0x80,

    /* U+006C "l" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80,

    /* U+006D "m" */
    0xef, 0x1e, 0x7f, 0xdf, 0xbc, 0xf9, 0xfc, 0x38,
    0x7e, 0x1c, 0x3f, 0xe, 0x1f, 0x87, 0xf, 0xc3,
    0x87, 0xe1, 0xc3, 0xf0, 0xe1, 0xf8, 0x70, 0xfc,
    0x38, 0x7e, 0x1c, 0x38,

    /* U+006E "n" */
    0xef, 0x3f, 0xef, 0x3f, 0x87, 0xe1, 0xf8, 0x7e,
    0x1f, 0x87, 0xe1, 0xf8, 0x7e, 0x1f, 0x87, 0xe1,
    0xc0,

    /* U+006F "o" */
    0x1f, 0x7, 0xf1, 0xc7, 0x70, 0x7e, 0xf, 0xc1,
    0xf8, 0x3f, 0x7, 0xe0, 0xfc, 0x39, 0xc7, 0x1f,
    0xc1, 0xf0,

    /* U+0070 "p" */
    0xef, 0x3f, 0xee, 0x3b, 0x8f, 0xe1, 0xf8, 0x7e,
    0x1f, 0x87, 0xe1, 0xf8, 0x7e, 0x3b, 0xfc, 0xfe,
    0x38, 0xe, 0x3, 0x80, 0xe0, 0x38, 0xe, 0x0,

    /* U+0071 "q" */
    0x1f, 0xcf, 0xf7, 0x1f, 0x87, 0xe1, 0xf8, 0x7e,
    0x1f, 0x87, 0xe1, 0xf8, 0x77, 0x1d, 0xff, 0x3d,
    0xc0, 0x70, 0x1c, 0x7, 0x1, 0xc0, 0x70, 0x1c,

    /* U+0072 "r" */
    0xef, 0xff, 0xef, 0xe, 0x1c, 0x38, 0x70, 0xe1,
    0xc3, 0x87, 0xe, 0x0,

    /* U+0073 "s" */
    0x3e, 0x7e, 0xe0, 0xe0, 0xf0, 0x78, 0x7c, 0x1e,
    0xf, 0x7, 0x87, 0xfe, 0xfc,

    /* U+0074 "t" */
    0x70, 0xe1, 0xc3, 0x8f, 0xdf, 0x9c, 0x38, 0x70,
    0xe1, 0xc3, 0x87, 0xe, 0x1c, 0x3f, 0x3e,

    /* U+0075 "u" */
    0xe1, 0xf8, 0x7e, 0x1f, 0x87, 0xe1, 0xf8, 0x7e,
    0x1f, 0x87, 0xe1, 0xf8, 0x7f, 0x3d, 0xff, 0x3d,
    0xc0,

    /* U+0076 "v" */
    0xe0, 0xec, 0x1d, 0x83, 0x38, 0xe7, 0x1c, 0x63,
    0xc, 0x61, 0xdc, 0x1b, 0x3, 0x60, 0x7c, 0x7,
    0x0, 0xe0,

    /* U+0077 "w" */
    0xe0, 0xe1, 0xf8, 0x78, 0x66, 0x1e, 0x19, 0xc7,
    0x86, 0x71, 0xf3, 0x8c, 0xcc, 0xc3, 0x33, 0x30,
    0xcc, 0xcc, 0x33, 0x33, 0x6, 0xc7, 0xc1, 0xe1,
    0xe0, 0x78, 0x78, 0x1e, 0x1e, 0x0,

    /* U+0078 "x" */
    0x70, 0xce, 0x38, 0xe6, 0xd, 0xc1, 0xf0, 0x1e,
    0x3, 0x80, 0xf8, 0x1b, 0x7, 0x70, 0xc7, 0x38,
    0xe6, 0xe,

    /* U+0079 "y" */
    0xe0, 0xfc, 0x1f, 0x83, 0x30, 0xe7, 0x1c, 0xe3,
    0xc, 0x61, 0xdc, 0x3b, 0x83, 0x60, 0x7c, 0xf,
    0x80, 0xe0, 0xc, 0x3, 0x80, 0x70, 0x1c, 0x7,
    0x0, 0xc0, 0x0,

    /* U+007A "z" */
    0x7f, 0xdf, 0xe0, 0x38, 0x1c, 0x7, 0x3, 0x80,
    0xe0, 0x70, 0x1c, 0xe, 0x3, 0x81, 0xff, 0x7f,
    0xc0,

    /* U+007B "{" */
    0x1e, 0x7c, 0xe1, 0xc3, 0x87, 0xe, 0x1c, 0x38,
    0x71, 0xc7, 0xe, 0xe, 0xe, 0x1c, 0x38, 0x70,
    0xe1, 0xc3, 0x87, 0xf, 0x8f,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,

    /* U+007D "}" */
    0xf8, 0xfc, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c,
    0x1c, 0x1c, 0xe, 0x7, 0x7, 0xe, 0x1c, 0x1c,
    0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0xfc, 0xf8,

    /* U+007E "~" */
    0x7c, 0x5f, 0xfb, 0x1e, 0x0,

    /* U+007F "" */
    0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 105, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 126, .box_w = 4, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 11, .adv_w = 172, .box_w = 8, .box_h = 8, .ofs_x = 2, .ofs_y = 11},
    {.bitmap_index = 19, .adv_w = 328, .box_w = 17, .box_h = 20, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 62, .adv_w = 184, .box_w = 9, .box_h = 25, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 91, .adv_w = 383, .box_w = 21, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 141, .adv_w = 275, .box_w = 14, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 99, .box_w = 3, .box_h = 8, .ofs_x = 2, .ofs_y = 11},
    {.bitmap_index = 176, .adv_w = 194, .box_w = 7, .box_h = 24, .ofs_x = 3, .ofs_y = -3},
    {.bitmap_index = 197, .adv_w = 194, .box_w = 7, .box_h = 24, .ofs_x = 3, .ofs_y = -3},
    {.bitmap_index = 218, .adv_w = 227, .box_w = 10, .box_h = 11, .ofs_x = 2, .ofs_y = 9},
    {.bitmap_index = 232, .adv_w = 231, .box_w = 10, .box_h = 11, .ofs_x = 2, .ofs_y = 3},
    {.bitmap_index = 246, .adv_w = 128, .box_w = 4, .box_h = 8, .ofs_x = 2, .ofs_y = -4},
    {.bitmap_index = 250, .adv_w = 192, .box_w = 8, .box_h = 2, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 252, .adv_w = 124, .box_w = 4, .box_h = 4, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 254, .adv_w = 269, .box_w = 11, .box_h = 23, .ofs_x = 3, .ofs_y = -3},
    {.bitmap_index = 286, .adv_w = 230, .box_w = 12, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 313, .adv_w = 154, .box_w = 6, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 327, .adv_w = 200, .box_w = 10, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 350, .adv_w = 204, .box_w = 11, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 375, .adv_w = 231, .box_w = 13, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 405, .adv_w = 213, .box_w = 11, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 430, .adv_w = 222, .box_w = 12, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 457, .adv_w = 194, .box_w = 11, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 482, .adv_w = 221, .box_w = 11, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 507, .adv_w = 222, .box_w = 12, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 534, .adv_w = 126, .box_w = 4, .box_h = 13, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 541, .adv_w = 132, .box_w = 4, .box_h = 17, .ofs_x = 2, .ofs_y = -4},
    {.bitmap_index = 550, .adv_w = 209, .box_w = 10, .box_h = 13, .ofs_x = 2, .ofs_y = 1},
    {.bitmap_index = 567, .adv_w = 231, .box_w = 10, .box_h = 7, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 576, .adv_w = 209, .box_w = 11, .box_h = 13, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 594, .adv_w = 173, .box_w = 9, .box_h = 19, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 616, .adv_w = 393, .box_w = 20, .box_h = 22, .ofs_x = 2, .ofs_y = -5},
    {.bitmap_index = 671, .adv_w = 218, .box_w = 13, .box_h = 18, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 701, .adv_w = 231, .box_w = 11, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 726, .adv_w = 194, .box_w = 10, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 749, .adv_w = 234, .box_w = 12, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 776, .adv_w = 198, .box_w = 10, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 799, .adv_w = 187, .box_w = 9, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 820, .adv_w = 229, .box_w = 12, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 847, .adv_w = 249, .box_w = 11, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 872, .adv_w = 110, .box_w = 3, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 879, .adv_w = 126, .box_w = 6, .box_h = 18, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 893, .adv_w = 221, .box_w = 12, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 920, .adv_w = 171, .box_w = 8, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 938, .adv_w = 305, .box_w = 15, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 972, .adv_w = 256, .box_w = 12, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 999, .adv_w = 232, .box_w = 12, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1026, .adv_w = 213, .box_w = 11, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1051, .adv_w = 232, .box_w = 12, .box_h = 22, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 1084, .adv_w = 220, .box_w = 11, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1109, .adv_w = 182, .box_w = 9, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1130, .adv_w = 193, .box_w = 11, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1155, .adv_w = 249, .box_w = 11, .box_h = 18, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1180, .adv_w = 207, .box_w = 13, .box_h = 18, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1210, .adv_w = 328, .box_w = 20, .box_h = 18, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1255, .adv_w = 214, .box_w = 13, .box_h = 18, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1285, .adv_w = 207, .box_w = 13, .box_h = 18, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1315, .adv_w = 200, .box_w = 11, .box_h = 18, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1340, .adv_w = 162, .box_w = 6, .box_h = 24, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 1358, .adv_w = 269, .box_w = 11, .box_h = 23, .ofs_x = 3, .ofs_y = -3},
    {.bitmap_index = 1390, .adv_w = 162, .box_w = 7, .box_h = 24, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1411, .adv_w = 177, .box_w = 11, .box_h = 13, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 1429, .adv_w = 231, .box_w = 16, .box_h = 2, .ofs_x = -1, .ofs_y = -4},
    {.bitmap_index = 1433, .adv_w = 17, .box_w = 6, .box_h = 5, .ofs_x = -4, .ofs_y = 15},
    {.bitmap_index = 1437, .adv_w = 195, .box_w = 10, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1454, .adv_w = 205, .box_w = 10, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1478, .adv_w = 165, .box_w = 9, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1493, .adv_w = 210, .box_w = 11, .box_h = 19, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1520, .adv_w = 191, .box_w = 10, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1537, .adv_w = 119, .box_w = 6, .box_h = 19, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1552, .adv_w = 208, .box_w = 12, .box_h = 19, .ofs_x = 1, .ofs_y = -6},
    {.bitmap_index = 1581, .adv_w = 215, .box_w = 10, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1605, .adv_w = 104, .box_w = 3, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1613, .adv_w = 105, .box_w = 4, .box_h = 25, .ofs_x = 0, .ofs_y = -6},
    {.bitmap_index = 1626, .adv_w = 195, .box_w = 11, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1653, .adv_w = 100, .box_w = 3, .box_h = 19, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1661, .adv_w = 336, .box_w = 17, .box_h = 13, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1689, .adv_w = 218, .box_w = 10, .box_h = 13, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1706, .adv_w = 205, .box_w = 11, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1724, .adv_w = 203, .box_w = 10, .box_h = 19, .ofs_x = 2, .ofs_y = -6},
    {.bitmap_index = 1748, .adv_w = 203, .box_w = 10, .box_h = 19, .ofs_x = 1, .ofs_y = -6},
    {.bitmap_index = 1772, .adv_w = 139, .box_w = 7, .box_h = 13, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1784, .adv_w = 163, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1797, .adv_w = 132, .box_w = 7, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1812, .adv_w = 212, .box_w = 10, .box_h = 13, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1829, .adv_w = 177, .box_w = 11, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1847, .adv_w = 287, .box_w = 18, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1877, .adv_w = 183, .box_w = 11, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1895, .adv_w = 179, .box_w = 11, .box_h = 19, .ofs_x = 0, .ofs_y = -6},
    {.bitmap_index = 1922, .adv_w = 175, .box_w = 10, .box_h = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1939, .adv_w = 179, .box_w = 7, .box_h = 24, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 1960, .adv_w = 113, .box_w = 2, .box_h = 27, .ofs_x = 2, .ofs_y = -5},
    {.bitmap_index = 1967, .adv_w = 179, .box_w = 8, .box_h = 24, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1991, .adv_w = 226, .box_w = 11, .box_h = 3, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 1996, .adv_w = 0, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0}
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
const lv_font_t Tauri_R_22 = {
#else
lv_font_t Tauri_R_22 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 28,          /*The maximum line height required by the font*/
    .base_line = 6,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 2,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if TAURI_R_22*/

