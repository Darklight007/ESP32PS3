/*******************************************************************************
 * Size: 32 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef DSEG_B_32
#define DSEG_B_32 1
#endif

#if DSEG_B_32

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0x0,

    /* U+002B "+" */
    0x1c, 0xe, 0x7, 0x3, 0x8f, 0xff, 0xfc, 0x70,
    0x38, 0x1c, 0x0,

    /* U+002D "-" */
    0xff, 0xff, 0xff,

    /* U+002E "." */
    0x6f, 0xf6,

    /* U+0030 "0" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xe0, 0x0, 0x7c, 0x0, 0x3,
    0xc0, 0x0, 0x3e, 0x0, 0x7, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xef, 0xff,
    0x7d, 0xff, 0xfb, 0x3f, 0xff, 0xc3, 0xff, 0xfc,

    /* U+0031 "1" */
    0x3, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xf7, 0x37,
    0x7f, 0xff, 0xff, 0xff, 0xff, 0xf7, 0x30,

    /* U+0032 "2" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0x1f, 0xff, 0xb0,
    0xff, 0xf7, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0,
    0xf0, 0x0, 0xf, 0xf, 0xff, 0x71, 0xff, 0xfb,
    0xdf, 0xff, 0x8e, 0xff, 0xf0, 0xf0, 0x0, 0xf,
    0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xef, 0xff,
    0xd, 0xff, 0xf8, 0x3f, 0xff, 0xc3, 0xff, 0xfc,

    /* U+0033 "3" */
    0xff, 0xff, 0x3f, 0xff, 0xc7, 0xff, 0xec, 0xff,
    0xf7, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0,
    0x3c, 0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf3,
    0xff, 0xdd, 0xff, 0xfb, 0x7f, 0xfe, 0xcf, 0xff,
    0x70, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3, 0xc0,
    0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3,
    0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x3f,
    0xfd, 0xdf, 0xff, 0xbf, 0xff, 0xf3, 0xff, 0xfc,

    /* U+0034 "4" */
    0x0, 0x0, 0xc, 0x0, 0x3, 0xe0, 0x0, 0x7f,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xfe, 0xff, 0xf7, 0xdf, 0xff, 0xb1, 0xff, 0xfb,
    0xf, 0xff, 0x70, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0x7, 0x0, 0x0,
    0x30, 0x0, 0x0,

    /* U+0035 "5" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0x8e,
    0xff, 0xf0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xef, 0xff, 0xd, 0xff, 0xf8,
    0x1f, 0xff, 0xb0, 0xff, 0xf7, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0xf, 0xff,
    0x71, 0xff, 0xfb, 0x3f, 0xff, 0xc3, 0xff, 0xfc,

    /* U+0036 "6" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0x8e,
    0xff, 0xf0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xef, 0xff, 0xd, 0xff, 0xf8,
    0xdf, 0xff, 0xbe, 0xff, 0xf7, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xef, 0xff,
    0x7d, 0xff, 0xfb, 0x3f, 0xff, 0xc3, 0xff, 0xfc,

    /* U+0037 "7" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xe0, 0x0, 0x7c, 0x0, 0x3,
    0x0, 0x0, 0x30, 0x0, 0x7, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0,
    0x70, 0x0, 0x3, 0x0, 0x0, 0x0,

    /* U+0038 "8" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xef, 0xff, 0x7d, 0xff, 0xfb,
    0xdf, 0xff, 0xbe, 0xff, 0xf7, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xef, 0xff,
    0x7d, 0xff, 0xfb, 0x3f, 0xff, 0xc3, 0xff, 0xfc,

    /* U+0039 "9" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xef, 0xff, 0x7d, 0xff, 0xfb,
    0x1f, 0xff, 0xb0, 0xff, 0xf7, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0xf, 0xff,
    0x71, 0xff, 0xfb, 0x3f, 0xff, 0xc3, 0xff, 0xfc,

    /* U+003A ":" */
    0x6f, 0xf6, 0x0, 0x0, 0x0, 0x0, 0x6, 0xff,
    0x60,

    /* U+0041 "A" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xef, 0xff, 0x7d, 0xff, 0xfb,
    0xdf, 0xff, 0xbe, 0xff, 0xf7, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xe0, 0x0,
    0x7c, 0x0, 0x3, 0x0, 0x0, 0x0,

    /* U+0042 "B" */
    0x0, 0x0, 0xc, 0x0, 0x0, 0xe0, 0x0, 0xf,
    0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xe, 0xff, 0xf0, 0xdf, 0xff, 0x8d, 0xff, 0xfb,
    0xef, 0xff, 0x7f, 0x0, 0xf, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xfe, 0xff, 0xf7, 0xdf, 0xff,
    0xb3, 0xff, 0xfc, 0x3f, 0xff, 0xc0,

    /* U+0043 "C" */
    0xf, 0xff, 0x7, 0xff, 0xed, 0xff, 0xfb, 0xbf,
    0xfc, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3,
    0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0,
    0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xe,
    0xff, 0xf3, 0x7f, 0xfe, 0x3f, 0xff, 0xcf, 0xff,
    0xf0,

    /* U+0044 "D" */
    0x0, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x70,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0,
    0xf0, 0xff, 0xf7, 0x1f, 0xff, 0xbd, 0xff, 0xfb,
    0xef, 0xff, 0x7f, 0x0, 0xf, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xfe, 0xff, 0xf7, 0xdf, 0xff,
    0xb3, 0xff, 0xfc, 0x3f, 0xff, 0xc0,

    /* U+0045 "E" */
    0x3f, 0xff, 0xcf, 0xff, 0xfd, 0xff, 0xfb, 0xbf,
    0xfc, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3,
    0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0,
    0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xe,
    0xff, 0xf3, 0x7f, 0xfe, 0xdf, 0xff, 0xbb, 0xff,
    0xcf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0,
    0x3c, 0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xef,
    0xff, 0x37, 0xff, 0xe3, 0xff, 0xfc, 0xff, 0xff,

    /* U+0046 "F" */
    0x3f, 0xff, 0xcf, 0xff, 0xfd, 0xff, 0xfb, 0xbf,
    0xfc, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3,
    0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0,
    0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xe,
    0xff, 0xf3, 0x7f, 0xfe, 0xdf, 0xff, 0xbb, 0xff,
    0xcf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0,
    0x3c, 0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xe0,
    0x0, 0x30, 0x0, 0x0, 0x0, 0x0,

    /* U+0047 "G" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0x8e,
    0xff, 0xf0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xe0, 0x0, 0xc, 0x0, 0x0,
    0xc0, 0x0, 0x3e, 0x0, 0x7, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xef, 0xff,
    0x7d, 0xff, 0xfb, 0x3f, 0xff, 0xc3, 0xff, 0xfc,

    /* U+0048 "H" */
    0x0, 0x0, 0xc, 0x0, 0x0, 0xe0, 0x0, 0xf,
    0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xe, 0xff, 0xf0, 0xdf, 0xff, 0x8d, 0xff, 0xfb,
    0xef, 0xff, 0x7f, 0x0, 0xf, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xfe, 0x0, 0x7, 0xc0, 0x0,
    0x30, 0x0, 0x0,

    /* U+0049 "I" */
    0x77, 0xff, 0xff, 0xff, 0xff, 0xff, 0x73,

    /* U+004A "J" */
    0x0, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x70,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0,
    0xf0, 0x0, 0x7, 0x0, 0x0, 0x3c, 0x0, 0x3,
    0xe0, 0x0, 0x7f, 0x0, 0xf, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xfe, 0xff, 0xf7, 0xdf, 0xff,
    0xb3, 0xff, 0xfc, 0x3f, 0xff, 0xc0,

    /* U+004B "K" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0x8e,
    0xff, 0xf0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xef, 0xff, 0xd, 0xff, 0xf8,
    0xdf, 0xff, 0xbe, 0xff, 0xf7, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xe0, 0x0,
    0x7c, 0x0, 0x3, 0x0, 0x0, 0x0,

    /* U+004C "L" */
    0x0, 0x0, 0x30, 0x0, 0xe, 0x0, 0x3, 0xc0,
    0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3,
    0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0,
    0x3, 0xc0, 0x0, 0xf0, 0x0, 0x38, 0x0, 0xc,
    0x0, 0x3, 0x0, 0x0, 0xe0, 0x0, 0x3c, 0x0,
    0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0,
    0x3c, 0x0, 0xf, 0x0, 0x3, 0xbf, 0xfc, 0xdf,
    0xff, 0x8f, 0xff, 0xf3, 0xff, 0xfc,

    /* U+004D "M" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xe0, 0x0, 0x7c, 0x0, 0x3,
    0xc0, 0x0, 0x3e, 0x0, 0x7, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xe0, 0x0,
    0x7c, 0x0, 0x3, 0x0, 0x0, 0x0,

    /* U+004E "N" */
    0xf, 0xff, 0x1, 0xff, 0xf8, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xe0, 0x0, 0x7c, 0x0, 0x3,
    0x0, 0x0, 0x0,

    /* U+004F "O" */
    0xf, 0xff, 0x1, 0xff, 0xf8, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xef, 0xff, 0x7d, 0xff, 0xfb,
    0x3f, 0xff, 0xc3, 0xff, 0xfc,

    /* U+0050 "P" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xef, 0xff, 0x7d, 0xff, 0xfb,
    0xdf, 0xff, 0x8e, 0xff, 0xf0, 0xf0, 0x0, 0xf,
    0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xe0, 0x0,
    0xc, 0x0, 0x0, 0x0, 0x0, 0x0,

    /* U+0051 "Q" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xef, 0xff, 0x7d, 0xff, 0xfb,
    0x1f, 0xff, 0xb0, 0xff, 0xf7, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0,
    0x70, 0x0, 0x3, 0x0, 0x0, 0x0,

    /* U+0052 "R" */
    0xf, 0xff, 0xf, 0xff, 0xf7, 0xff, 0xfd, 0xff,
    0xef, 0x0, 0x7, 0x80, 0x3, 0xc0, 0x1, 0xe0,
    0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0, 0x1e,
    0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0x80, 0x1,
    0x80, 0x0, 0x0, 0x0, 0x0,

    /* U+0053 "S" */
    0x0, 0x0, 0xc, 0x0, 0x0, 0xe0, 0x0, 0xf,
    0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xe, 0xff, 0xf0, 0xdf, 0xff, 0x81, 0xff, 0xfb,
    0xf, 0xff, 0x70, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0xff, 0xf7, 0x1f, 0xff,
    0xb3, 0xff, 0xfc, 0x3f, 0xff, 0xc0,

    /* U+0054 "T" */
    0x0, 0x0, 0x30, 0x0, 0xe, 0x0, 0x3, 0xc0,
    0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3,
    0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0,
    0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3b, 0xff, 0xcd,
    0xff, 0xfb, 0x7f, 0xfe, 0xef, 0xff, 0x3c, 0x0,
    0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0,
    0x3c, 0x0, 0xf, 0x0, 0x3, 0xbf, 0xfc, 0xdf,
    0xff, 0x8f, 0xff, 0xf3, 0xff, 0xfc,

    /* U+0055 "U" */
    0xc0, 0x0, 0x3e, 0x0, 0x7, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xef, 0xff,
    0x7d, 0xff, 0xfb, 0x3f, 0xff, 0xc3, 0xff, 0xfc,

    /* U+0056 "V" */
    0x0, 0x0, 0xc, 0x0, 0x3, 0xe0, 0x0, 0x7f,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xfe, 0x0, 0x7, 0xc0, 0x0, 0x3c, 0x0, 0x3,
    0xe0, 0x0, 0x7f, 0x0, 0xf, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xfe, 0xff, 0xf7, 0xdf, 0xff,
    0xb3, 0xff, 0xfc, 0x3f, 0xff, 0xc0,

    /* U+0057 "W" */
    0x0, 0x0, 0xc, 0x0, 0x3, 0xe0, 0x0, 0x7f,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xfe, 0xff, 0xf7, 0xdf, 0xff, 0xbd, 0xff, 0xfb,
    0xef, 0xff, 0x7f, 0x0, 0xf, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xfe, 0xff, 0xf7, 0xdf, 0xff,
    0xb3, 0xff, 0xfc, 0x3f, 0xff, 0xc0,

    /* U+0058 "X" */
    0x0, 0x0, 0xc, 0x0, 0x3, 0xe0, 0x0, 0x7f,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xfe, 0xff, 0xf7, 0xdf, 0xff, 0xbd, 0xff, 0xfb,
    0xef, 0xff, 0x7f, 0x0, 0xf, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xfe, 0x0, 0x7, 0xc0, 0x0,
    0x30, 0x0, 0x0,

    /* U+0059 "Y" */
    0x0, 0x0, 0xc, 0x0, 0x3, 0xe0, 0x0, 0x7f,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xfe, 0xff, 0xf7, 0xdf, 0xff, 0xb1, 0xff, 0xfb,
    0xf, 0xff, 0x70, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0xff, 0xf7, 0x1f, 0xff,
    0xb3, 0xff, 0xfc, 0x3f, 0xff, 0xc0,

    /* U+005A "Z" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0x1f, 0xff, 0xb0,
    0xff, 0xf7, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0,
    0xf0, 0x0, 0xf, 0x0, 0x0, 0x70, 0x0, 0x3,
    0xc0, 0x0, 0xe, 0x0, 0x0, 0xf0, 0x0, 0xf,
    0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xef, 0xff,
    0xd, 0xff, 0xf8, 0x3f, 0xff, 0xc3, 0xff, 0xfc,

    /* U+005F "_" */
    0x3f, 0xfc, 0x3f, 0xff, 0x3f, 0xff, 0xdf, 0xff,
    0xe0,

    /* U+0061 "a" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xef, 0xff, 0x7d, 0xff, 0xfb,
    0xdf, 0xff, 0xbe, 0xff, 0xf7, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xe0, 0x0,
    0x7c, 0x0, 0x3, 0x0, 0x0, 0x0,

    /* U+0062 "b" */
    0x0, 0x0, 0xc, 0x0, 0x0, 0xe0, 0x0, 0xf,
    0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xe, 0xff, 0xf0, 0xdf, 0xff, 0x8d, 0xff, 0xfb,
    0xef, 0xff, 0x7f, 0x0, 0xf, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xfe, 0xff, 0xf7, 0xdf, 0xff,
    0xb3, 0xff, 0xfc, 0x3f, 0xff, 0xc0,

    /* U+0063 "c" */
    0xf, 0xff, 0x7, 0xff, 0xed, 0xff, 0xfb, 0xbf,
    0xfc, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3,
    0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0,
    0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xe,
    0xff, 0xf3, 0x7f, 0xfe, 0x3f, 0xff, 0xcf, 0xff,
    0xf0,

    /* U+0064 "d" */
    0x0, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x70,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0,
    0xf0, 0xff, 0xf7, 0x1f, 0xff, 0xbd, 0xff, 0xfb,
    0xef, 0xff, 0x7f, 0x0, 0xf, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xfe, 0xff, 0xf7, 0xdf, 0xff,
    0xb3, 0xff, 0xfc, 0x3f, 0xff, 0xc0,

    /* U+0065 "e" */
    0x3f, 0xff, 0xcf, 0xff, 0xfd, 0xff, 0xfb, 0xbf,
    0xfc, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3,
    0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0,
    0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xe,
    0xff, 0xf3, 0x7f, 0xfe, 0xdf, 0xff, 0xbb, 0xff,
    0xcf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0,
    0x3c, 0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xef,
    0xff, 0x37, 0xff, 0xe3, 0xff, 0xfc, 0xff, 0xff,

    /* U+0066 "f" */
    0x3f, 0xff, 0xcf, 0xff, 0xfd, 0xff, 0xfb, 0xbf,
    0xfc, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3,
    0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0,
    0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xe,
    0xff, 0xf3, 0x7f, 0xfe, 0xdf, 0xff, 0xbb, 0xff,
    0xcf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0,
    0x3c, 0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xe0,
    0x0, 0x30, 0x0, 0x0, 0x0, 0x0,

    /* U+0067 "g" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0x8e,
    0xff, 0xf0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xe0, 0x0, 0xc, 0x0, 0x0,
    0xc0, 0x0, 0x3e, 0x0, 0x7, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xef, 0xff,
    0x7d, 0xff, 0xfb, 0x3f, 0xff, 0xc3, 0xff, 0xfc,

    /* U+0068 "h" */
    0x0, 0x0, 0xc, 0x0, 0x0, 0xe0, 0x0, 0xf,
    0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xe, 0xff, 0xf0, 0xdf, 0xff, 0x8d, 0xff, 0xfb,
    0xef, 0xff, 0x7f, 0x0, 0xf, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xfe, 0x0, 0x7, 0xc0, 0x0,
    0x30, 0x0, 0x0,

    /* U+0069 "i" */
    0x77, 0xff, 0xff, 0xff, 0xff, 0xff, 0x73,

    /* U+006A "j" */
    0x0, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x70,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0,
    0xf0, 0x0, 0x7, 0x0, 0x0, 0x3c, 0x0, 0x3,
    0xe0, 0x0, 0x7f, 0x0, 0xf, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xfe, 0xff, 0xf7, 0xdf, 0xff,
    0xb3, 0xff, 0xfc, 0x3f, 0xff, 0xc0,

    /* U+006B "k" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0x8e,
    0xff, 0xf0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xef, 0xff, 0xd, 0xff, 0xf8,
    0xdf, 0xff, 0xbe, 0xff, 0xf7, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xe0, 0x0,
    0x7c, 0x0, 0x3, 0x0, 0x0, 0x0,

    /* U+006C "l" */
    0x0, 0x0, 0x30, 0x0, 0xe, 0x0, 0x3, 0xc0,
    0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3,
    0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0,
    0x3, 0xc0, 0x0, 0xf0, 0x0, 0x38, 0x0, 0xc,
    0x0, 0x3, 0x0, 0x0, 0xe0, 0x0, 0x3c, 0x0,
    0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0,
    0x3c, 0x0, 0xf, 0x0, 0x3, 0xbf, 0xfc, 0xdf,
    0xff, 0x8f, 0xff, 0xf3, 0xff, 0xfc,

    /* U+006D "m" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xe0, 0x0, 0x7c, 0x0, 0x3,
    0xc0, 0x0, 0x3e, 0x0, 0x7, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xe0, 0x0,
    0x7c, 0x0, 0x3, 0x0, 0x0, 0x0,

    /* U+006E "n" */
    0xf, 0xff, 0x1, 0xff, 0xf8, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xe0, 0x0, 0x7c, 0x0, 0x3,
    0x0, 0x0, 0x0,

    /* U+006F "o" */
    0xf, 0xff, 0x1, 0xff, 0xf8, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xef, 0xff, 0x7d, 0xff, 0xfb,
    0x3f, 0xff, 0xc3, 0xff, 0xfc,

    /* U+0070 "p" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xef, 0xff, 0x7d, 0xff, 0xfb,
    0xdf, 0xff, 0x8e, 0xff, 0xf0, 0xf0, 0x0, 0xf,
    0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xe0, 0x0,
    0xc, 0x0, 0x0, 0x0, 0x0, 0x0,

    /* U+0071 "q" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0xdf, 0xff, 0xbe,
    0xff, 0xf7, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xff, 0x0, 0xf, 0xef, 0xff, 0x7d, 0xff, 0xfb,
    0x1f, 0xff, 0xb0, 0xff, 0xf7, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0,
    0x70, 0x0, 0x3, 0x0, 0x0, 0x0,

    /* U+0072 "r" */
    0xf, 0xff, 0xf, 0xff, 0xf7, 0xff, 0xfd, 0xff,
    0xef, 0x0, 0x7, 0x80, 0x3, 0xc0, 0x1, 0xe0,
    0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0, 0x1e,
    0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0x80, 0x1,
    0x80, 0x0, 0x0, 0x0, 0x0,

    /* U+0073 "s" */
    0x0, 0x0, 0xc, 0x0, 0x0, 0xe0, 0x0, 0xf,
    0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xe, 0xff, 0xf0, 0xdf, 0xff, 0x81, 0xff, 0xfb,
    0xf, 0xff, 0x70, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0xff, 0xf7, 0x1f, 0xff,
    0xb3, 0xff, 0xfc, 0x3f, 0xff, 0xc0,

    /* U+0074 "t" */
    0x0, 0x0, 0x30, 0x0, 0xe, 0x0, 0x3, 0xc0,
    0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3,
    0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0,
    0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3b, 0xff, 0xcd,
    0xff, 0xfb, 0x7f, 0xfe, 0xef, 0xff, 0x3c, 0x0,
    0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0,
    0x3c, 0x0, 0xf, 0x0, 0x3, 0xbf, 0xfc, 0xdf,
    0xff, 0x8f, 0xff, 0xf3, 0xff, 0xfc,

    /* U+0075 "u" */
    0xc0, 0x0, 0x3e, 0x0, 0x7, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xef, 0xff,
    0x7d, 0xff, 0xfb, 0x3f, 0xff, 0xc3, 0xff, 0xfc,

    /* U+0076 "v" */
    0x0, 0x0, 0xc, 0x0, 0x3, 0xe0, 0x0, 0x7f,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xfe, 0x0, 0x7, 0xc0, 0x0, 0x3c, 0x0, 0x3,
    0xe0, 0x0, 0x7f, 0x0, 0xf, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xfe, 0xff, 0xf7, 0xdf, 0xff,
    0xb3, 0xff, 0xfc, 0x3f, 0xff, 0xc0,

    /* U+0077 "w" */
    0x0, 0x0, 0xc, 0x0, 0x3, 0xe0, 0x0, 0x7f,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xfe, 0xff, 0xf7, 0xdf, 0xff, 0xbd, 0xff, 0xfb,
    0xef, 0xff, 0x7f, 0x0, 0xf, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xfe, 0xff, 0xf7, 0xdf, 0xff,
    0xb3, 0xff, 0xfc, 0x3f, 0xff, 0xc0,

    /* U+0078 "x" */
    0x0, 0x0, 0xc, 0x0, 0x3, 0xe0, 0x0, 0x7f,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xfe, 0xff, 0xf7, 0xdf, 0xff, 0xbd, 0xff, 0xfb,
    0xef, 0xff, 0x7f, 0x0, 0xf, 0xf0, 0x0, 0xff,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xfe, 0x0, 0x7, 0xc0, 0x0,
    0x30, 0x0, 0x0,

    /* U+0079 "y" */
    0x0, 0x0, 0xc, 0x0, 0x3, 0xe0, 0x0, 0x7f,
    0x0, 0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0,
    0x0, 0xff, 0x0, 0xf, 0xf0, 0x0, 0xff, 0x0,
    0xf, 0xf0, 0x0, 0xff, 0x0, 0xf, 0xf0, 0x0,
    0xfe, 0xff, 0xf7, 0xdf, 0xff, 0xb1, 0xff, 0xfb,
    0xf, 0xff, 0x70, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0xff, 0xf7, 0x1f, 0xff,
    0xb3, 0xff, 0xfc, 0x3f, 0xff, 0xc0,

    /* U+007A "z" */
    0x3f, 0xff, 0xc3, 0xff, 0xfc, 0x1f, 0xff, 0xb0,
    0xff, 0xf7, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0,
    0xf0, 0x0, 0xf, 0x0, 0x0, 0x70, 0x0, 0x3,
    0xc0, 0x0, 0xe, 0x0, 0x0, 0xf0, 0x0, 0xf,
    0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xef, 0xff,
    0xd, 0xff, 0xf8, 0x3f, 0xff, 0xc3, 0xff, 0xfc
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 102, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 418, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 129, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 12},
    {.bitmap_index = 13, .adv_w = 132, .box_w = 8, .box_h = 3, .ofs_x = 0, .ofs_y = 15},
    {.bitmap_index = 16, .adv_w = 0, .box_w = 4, .box_h = 4, .ofs_x = -2, .ofs_y = 0},
    {.bitmap_index = 18, .adv_w = 418, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 98, .adv_w = 418, .box_w = 4, .box_h = 29, .ofs_x = 19, .ofs_y = 2},
    {.bitmap_index = 113, .adv_w = 418, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 418, .box_w = 18, .box_h = 32, .ofs_x = 5, .ofs_y = 0},
    {.bitmap_index = 265, .adv_w = 418, .box_w = 20, .box_h = 30, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 340, .adv_w = 418, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 420, .adv_w = 418, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 500, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 578, .adv_w = 418, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 658, .adv_w = 418, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 738, .adv_w = 102, .box_w = 4, .box_h = 17, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 747, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 825, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 903, .adv_w = 418, .box_w = 18, .box_h = 18, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 944, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1022, .adv_w = 418, .box_w = 18, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1094, .adv_w = 418, .box_w = 18, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 1164, .adv_w = 418, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1244, .adv_w = 418, .box_w = 20, .box_h = 30, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 1319, .adv_w = 418, .box_w = 4, .box_h = 14, .ofs_x = 19, .ofs_y = 2},
    {.bitmap_index = 1326, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1404, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 1482, .adv_w = 418, .box_w = 18, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1552, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 1630, .adv_w = 418, .box_w = 20, .box_h = 17, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 1673, .adv_w = 418, .box_w = 20, .box_h = 18, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1718, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 1796, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 1874, .adv_w = 418, .box_w = 17, .box_h = 17, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 1911, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1989, .adv_w = 418, .box_w = 18, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2059, .adv_w = 418, .box_w = 20, .box_h = 16, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2099, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2177, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2255, .adv_w = 418, .box_w = 20, .box_h = 30, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 2330, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2408, .adv_w = 418, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2488, .adv_w = 418, .box_w = 17, .box_h = 4, .ofs_x = 5, .ofs_y = 0},
    {.bitmap_index = 2497, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 2575, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2653, .adv_w = 418, .box_w = 18, .box_h = 18, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2694, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2772, .adv_w = 418, .box_w = 18, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2844, .adv_w = 418, .box_w = 18, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 2914, .adv_w = 418, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2994, .adv_w = 418, .box_w = 20, .box_h = 30, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 3069, .adv_w = 418, .box_w = 4, .box_h = 14, .ofs_x = 19, .ofs_y = 2},
    {.bitmap_index = 3076, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 3154, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 3232, .adv_w = 418, .box_w = 18, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 3302, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 3380, .adv_w = 418, .box_w = 20, .box_h = 17, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 3423, .adv_w = 418, .box_w = 20, .box_h = 18, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 3468, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 3546, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 3624, .adv_w = 418, .box_w = 17, .box_h = 17, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 3661, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 3739, .adv_w = 418, .box_w = 18, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 3809, .adv_w = 418, .box_w = 20, .box_h = 16, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 3849, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 3927, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 4005, .adv_w = 418, .box_w = 20, .box_h = 30, .ofs_x = 3, .ofs_y = 1},
    {.bitmap_index = 4080, .adv_w = 418, .box_w = 20, .box_h = 31, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 4158, .adv_w = 418, .box_w = 20, .box_h = 32, .ofs_x = 3, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x1, 0xb, 0xd, 0xe
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 15, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 5, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    },
    {
        .range_start = 48, .range_length = 11, .glyph_id_start = 6,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 65, .range_length = 26, .glyph_id_start = 17,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 95, .range_length = 1, .glyph_id_start = 43,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 97, .range_length = 26, .glyph_id_start = 44,
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
    .cmap_num = 5,
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
const lv_font_t dseg_b_32 = {
#else
lv_font_t dseg_b_32 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 32,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -4,
    .underline_thickness = 2,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if DSEG_B_32*/

