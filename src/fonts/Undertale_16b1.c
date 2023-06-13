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

#ifndef UNDERTALE_16B1
#define UNDERTALE_16B1 1
#endif

#if UNDERTALE_16B1

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0x6f, 0xff, 0x66, 0x60, 0x66,

    /* U+0022 "\"" */
    0xde, 0xd3, 0x20,

    /* U+0023 "#" */
    0x6c, 0xdb, 0xfb, 0x66, 0xcd, 0xbf, 0xb6, 0x6c,

    /* U+0024 "$" */
    0x30, 0xc7, 0xb3, 0xcf, 0x7, 0x83, 0xcf, 0x37,
    0x8c, 0x30,

    /* U+0025 "%" */
    0x61, 0x26, 0x53, 0x41, 0x5, 0x94, 0xc9, 0xc,

    /* U+0026 "&" */
    0x38, 0xd9, 0xb1, 0xc7, 0x1b, 0xf3, 0x66, 0x76,

    /* U+0027 "'" */
    0xf6,

    /* U+0028 "(" */
    0x36, 0xcc, 0xcc, 0xc6, 0x30,

    /* U+0029 ")" */
    0xc6, 0x33, 0x33, 0x36, 0xc0,

    /* U+002A "*" */
    0x66, 0x3c, 0xff, 0x3c, 0x66,

    /* U+002B "+" */
    0x30, 0xcf, 0xcc, 0x30,

    /* U+002C "," */
    0xf6,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0xc, 0x31, 0x86, 0x30, 0xc6, 0x18, 0xc3, 0x0,

    /* U+0030 "0" */
    0x7b, 0x3d, 0xf7, 0xff, 0xbe, 0xf3, 0x78,

    /* U+0031 "1" */
    0x33, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0xfc,

    /* U+0032 "2" */
    0x7b, 0x3c, 0xc3, 0x18, 0xc6, 0x30, 0xfc,

    /* U+0033 "3" */
    0x7b, 0x3c, 0xc3, 0x38, 0x3c, 0xf3, 0x78,

    /* U+0034 "4" */
    0xc, 0x73, 0xdb, 0xcf, 0x3f, 0xc3, 0xc,

    /* U+0035 "5" */
    0xff, 0xc, 0x30, 0xf8, 0x3c, 0xf3, 0x78,

    /* U+0036 "6" */
    0x3d, 0x8c, 0x30, 0xfb, 0x3c, 0xf3, 0x78,

    /* U+0037 "7" */
    0xfc, 0x30, 0xc6, 0x18, 0xc3, 0xc, 0x30,

    /* U+0038 "8" */
    0x7b, 0x3c, 0xf3, 0x7b, 0x3c, 0xf3, 0x78,

    /* U+0039 "9" */
    0x7b, 0x3c, 0xf3, 0x7c, 0x30, 0xc6, 0x70,

    /* U+003A ":" */
    0xf0, 0x3c,

    /* U+003B ";" */
    0xf0, 0x3d, 0x80,

    /* U+003C "<" */
    0x8, 0xcc, 0xcc, 0x30, 0xc3, 0x8,

    /* U+003D "=" */
    0xfc, 0xf, 0xc0,

    /* U+003E ">" */
    0x86, 0x18, 0x61, 0x99, 0x98, 0x80,

    /* U+003F "?" */
    0x7b, 0x3c, 0xc3, 0x18, 0xc0, 0xc, 0x30,

    /* U+0040 "@" */
    0x7b, 0x3c, 0xf7, 0xdf, 0x7c, 0x30, 0x7c,

    /* U+0041 "A" */
    0x7b, 0x3c, 0xf3, 0xcf, 0xfc, 0xf3, 0xcc,

    /* U+0042 "B" */
    0xfb, 0x3c, 0xf3, 0xfb, 0x3c, 0xf3, 0xf8,

    /* U+0043 "C" */
    0x7b, 0x3c, 0xf0, 0xc3, 0xc, 0xf3, 0x78,

    /* U+0044 "D" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0xf8,

    /* U+0045 "E" */
    0xff, 0xc, 0x30, 0xfb, 0xc, 0x30, 0xfc,

    /* U+0046 "F" */
    0xff, 0xc, 0x30, 0xfb, 0xc, 0x30, 0xc0,

    /* U+0047 "G" */
    0x7b, 0x3c, 0xf0, 0xdf, 0x3c, 0xf3, 0x78,

    /* U+0048 "H" */
    0xcf, 0x3c, 0xf3, 0xff, 0x3c, 0xf3, 0xcc,

    /* U+0049 "I" */
    0xfc, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0xfc,

    /* U+004A "J" */
    0x3c, 0x30, 0xc3, 0xc, 0x3c, 0xf3, 0x78,

    /* U+004B "K" */
    0xcf, 0x3d, 0xb6, 0xf3, 0x6d, 0xb3, 0xcc,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xfc,

    /* U+004D "M" */
    0x83, 0x8f, 0xbf, 0xfd, 0x78, 0xf1, 0xe3, 0xc6,

    /* U+004E "N" */
    0xcf, 0x3e, 0xfb, 0xdf, 0x7c, 0xf3, 0xcc,

    /* U+004F "O" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+0050 "P" */
    0xfb, 0x3c, 0xf3, 0xfb, 0xc, 0x30, 0xc0,

    /* U+0051 "Q" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0x78, 0xc1,
    0xc0,

    /* U+0052 "R" */
    0xfb, 0x3c, 0xf3, 0xfb, 0x3c, 0xf3, 0xcc,

    /* U+0053 "S" */
    0x7b, 0x3c, 0xf0, 0x78, 0x3c, 0xf3, 0x78,

    /* U+0054 "T" */
    0xfc, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30,

    /* U+0055 "U" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0x78,

    /* U+0056 "V" */
    0xcf, 0x3c, 0xf3, 0xcd, 0xe7, 0x8c, 0x30,

    /* U+0057 "W" */
    0xc7, 0x8f, 0x5e, 0xbd, 0x7a, 0xf5, 0xb6, 0x6c,

    /* U+0058 "X" */
    0xcf, 0x37, 0x9e, 0x31, 0xe7, 0xb3, 0xcc,

    /* U+0059 "Y" */
    0xcf, 0x3c, 0xf3, 0x78, 0xc3, 0xc, 0x30,

    /* U+005A "Z" */
    0xfc, 0x30, 0xc6, 0x31, 0x8c, 0x30, 0xfc,

    /* U+005B "[" */
    0xfc, 0xcc, 0xcc, 0xcc, 0xf0,

    /* U+005C "\\" */
    0xcc, 0xc6, 0x66, 0x33, 0x30,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0x33, 0xf0,

    /* U+005E "^" */
    0x31, 0xec, 0xe1,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0xcc,

    /* U+0061 "a" */
    0x7b, 0x30, 0xdf, 0xcf, 0x37, 0xc0,

    /* U+0062 "b" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xf3, 0xf8,

    /* U+0063 "c" */
    0x7b, 0x3c, 0xf0, 0xcf, 0x37, 0x80,

    /* U+0064 "d" */
    0xc, 0x37, 0xf3, 0xcf, 0x3c, 0xf3, 0x7c,

    /* U+0065 "e" */
    0x7b, 0x3c, 0xff, 0xc3, 0x37, 0x80,

    /* U+0066 "f" */
    0x1c, 0xc3, 0x3f, 0x30, 0xc3, 0xc, 0x30,

    /* U+0067 "g" */
    0x7f, 0x3c, 0xf3, 0xcf, 0x37, 0xc3, 0xcd, 0xe0,

    /* U+0068 "h" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xf3, 0xcc,

    /* U+0069 "i" */
    0x30, 0xc0, 0x3c, 0x30, 0xc3, 0xc, 0x33, 0xf0,

    /* U+006A "j" */
    0xc, 0x30, 0xf, 0xc, 0x30, 0xc3, 0xc, 0x30,
    0xf3, 0x78,

    /* U+006B "k" */
    0xc3, 0xc, 0xf3, 0xdb, 0xcd, 0xb3, 0xcc,

    /* U+006C "l" */
    0xf0, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0xfc,

    /* U+006D "m" */
    0xed, 0xff, 0x5e, 0xbd, 0x7a, 0xf1, 0x80,

    /* U+006E "n" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3c, 0xc0,

    /* U+006F "o" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x37, 0x80,

    /* U+0070 "p" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3f, 0xb0, 0xc3, 0x0,

    /* U+0071 "q" */
    0x7f, 0x3c, 0xf3, 0xcf, 0x37, 0xc3, 0xc, 0x30,

    /* U+0072 "r" */
    0xfb, 0x3c, 0xf0, 0xc3, 0xc, 0x0,

    /* U+0073 "s" */
    0x7b, 0x3c, 0x1e, 0xf, 0x37, 0x80,

    /* U+0074 "t" */
    0x30, 0xcf, 0xcc, 0x30, 0xc3, 0xc, 0x1c,

    /* U+0075 "u" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x37, 0xc0,

    /* U+0076 "v" */
    0xcf, 0x3c, 0xde, 0x78, 0xc3, 0x0,

    /* U+0077 "w" */
    0xc7, 0xaf, 0x5e, 0xbd, 0x6d, 0x9b, 0x0,

    /* U+0078 "x" */
    0xcf, 0x37, 0x8c, 0x7b, 0x3c, 0xc0,

    /* U+0079 "y" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x37, 0xc3, 0xcd, 0xe0,

    /* U+007A "z" */
    0xfc, 0x31, 0x8c, 0x63, 0xf, 0xc0,

    /* U+007B "{" */
    0x3b, 0x18, 0xcc, 0x31, 0x8c, 0x38,

    /* U+007C "|" */
    0xff, 0xff, 0xc0,

    /* U+007D "}" */
    0xe1, 0x8c, 0x61, 0x98, 0xc6, 0xe0,

    /* U+007E "~" */
    0x77, 0xb8,

    /* U+00A0 " " */
    0x0,

    /* U+00A1 "¡" */
    0x66, 0x6, 0x66, 0xff, 0xf6,

    /* U+00A2 "¢" */
    0x30, 0xc7, 0xb3, 0xc3, 0x37, 0x8c, 0x30,

    /* U+00A3 "£" */
    0x3c, 0xcd, 0x9b, 0xf, 0x8c, 0x19, 0xb3, 0xfc,

    /* U+00A4 "¤" */
    0x80, 0x42, 0x6, 0x0,

    /* U+00A5 "¥" */
    0xcf, 0x3c, 0xf3, 0x7b, 0xf3, 0x3f, 0x30,

    /* U+00A6 "¦" */
    0xff, 0x3f, 0xc0,

    /* U+00A7 "§" */
    0x7b, 0x3c, 0x1e, 0xcf, 0x3c, 0xf3, 0x78, 0x3c,
    0xde,

    /* U+00A8 "¨" */
    0xcf, 0x30,

    /* U+00A9 "©" */
    0x3c, 0x42, 0x99, 0xa5, 0xa1, 0xa5, 0x99, 0x42,
    0x3c,

    /* U+00AA "ª" */
    0x7b, 0x30, 0xdf, 0xcf, 0x37, 0xc0,

    /* U+00AB "«" */
    0x33, 0x66, 0xcc, 0x66, 0x33,

    /* U+00AC "¬" */
    0xfc, 0x30, 0xc0,

    /* U+00AE "®" */
    0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0xa5, 0x42,
    0x3c,

    /* U+00B0 "°" */
    0x69, 0x96,

    /* U+00B1 "±" */
    0x30, 0xcf, 0xcc, 0x30, 0xf, 0xc0,

    /* U+00B2 "²" */
    0x80,

    /* U+00B4 "´" */
    0x78,

    /* U+00B5 "µ" */
    0xcf, 0x3c, 0xf3, 0xcf, 0x3f, 0xb0, 0xc3, 0x0,

    /* U+00B6 "¶" */
    0x7f, 0xef, 0xdf, 0xb7, 0x66, 0xcd, 0x9b, 0x36,

    /* U+00B7 "·" */
    0xf0,

    /* U+00B8 "¸" */
    0x30, 0xfc,

    /* U+00BA "º" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x37, 0x80,

    /* U+00BB "»" */
    0xcc, 0x66, 0x33, 0x66, 0xcc,

    /* U+00BD "½" */
    0x80,

    /* U+00BE "¾" */
    0x80,

    /* U+00BF "¿" */
    0x30, 0xc0, 0xc, 0x63, 0xc, 0xf3, 0x78,

    /* U+00C0 "À" */
    0x60, 0xc0, 0x1e, 0xcf, 0x3c, 0xf3, 0xff, 0x3c,
    0xf3,

    /* U+00C1 "Á" */
    0x18, 0xc0, 0x1e, 0xcf, 0x3c, 0xf3, 0xff, 0x3c,
    0xf3,

    /* U+00C2 "Â" */
    0x7b, 0x30, 0x1e, 0xcf, 0x3c, 0xf3, 0xff, 0x3c,
    0xf3,

    /* U+00C3 "Ã" */
    0x77, 0xb8, 0x3, 0xcc, 0xd9, 0xb3, 0x66, 0xfd,
    0x9b, 0x36, 0x60,

    /* U+00C4 "Ä" */
    0xcf, 0x30, 0x1e, 0xcf, 0x3c, 0xf3, 0xff, 0x3c,
    0xf3,

    /* U+00C5 "Å" */
    0xcd, 0xe0, 0x1e, 0xcf, 0x3c, 0xf3, 0xff, 0x3c,
    0xf3,

    /* U+00C6 "Æ" */
    0x7f, 0xb3, 0x66, 0xcd, 0xff, 0x36, 0x6c, 0xde,

    /* U+00C7 "Ç" */
    0x7b, 0x3c, 0xf0, 0xc3, 0xc, 0xf3, 0x78, 0xc1,
    0xbc,

    /* U+00C8 "È" */
    0x60, 0xc0, 0x3f, 0xc3, 0xc, 0x3e, 0xc3, 0xc,
    0x3f,

    /* U+00C9 "É" */
    0x18, 0xc0, 0x3f, 0xc3, 0xc, 0x3e, 0xc3, 0xc,
    0x3f,

    /* U+00CA "Ê" */
    0x7b, 0x30, 0x3f, 0xc3, 0xc, 0x3e, 0xc3, 0xc,
    0x3f,

    /* U+00CB "Ë" */
    0xcf, 0x30, 0x3f, 0xc3, 0xc, 0x3e, 0xc3, 0xc,
    0x3f,

    /* U+00CC "Ì" */
    0x60, 0xc0, 0x3f, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0x3f,

    /* U+00CD "Í" */
    0x18, 0xc0, 0x3f, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0x3f,

    /* U+00CE "Î" */
    0x7b, 0x30, 0x3f, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0x3f,

    /* U+00CF "Ï" */
    0xcf, 0x30, 0x3f, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0x3f,

    /* U+00D0 "Ð" */
    0x7c, 0xcd, 0x9b, 0x3f, 0x6c, 0xd9, 0xb3, 0x7c,

    /* U+00D1 "Ñ" */
    0x77, 0xb8, 0x6, 0x6c, 0xdd, 0xbb, 0x6e, 0xdd,
    0x9b, 0x36, 0x60,

    /* U+00D2 "Ò" */
    0x60, 0xc0, 0x1e, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xde,

    /* U+00D3 "Ó" */
    0x18, 0xc0, 0x1e, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xde,

    /* U+00D4 "Ô" */
    0x7b, 0x30, 0x1e, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xde,

    /* U+00D5 "Õ" */
    0x77, 0xb8, 0x3, 0xcc, 0xd9, 0xb3, 0x66, 0xcd,
    0x9b, 0x33, 0xc0,

    /* U+00D6 "Ö" */
    0xcf, 0x30, 0x1e, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xde,

    /* U+00D7 "×" */
    0xcd, 0xe3, 0x1e, 0xcc,

    /* U+00D8 "Ø" */
    0x3d, 0x67, 0x66, 0x6e, 0x7e, 0x76, 0x66, 0xe6,
    0xbc,

    /* U+00D9 "Ù" */
    0x60, 0xc0, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xde,

    /* U+00DA "Ú" */
    0x18, 0xc0, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xde,

    /* U+00DB "Û" */
    0x7b, 0x30, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xde,

    /* U+00DC "Ü" */
    0xcf, 0x30, 0x33, 0xcf, 0x3c, 0xf3, 0xcf, 0x3c,
    0xde,

    /* U+00DD "Ý" */
    0x18, 0xc0, 0x33, 0xcf, 0x3c, 0xde, 0x30, 0xc3,
    0xc,

    /* U+00DE "Þ" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3f, 0xb0, 0xc0,

    /* U+00DF "ß" */
    0x7b, 0x3c, 0xf3, 0xdb, 0x3c, 0xf3, 0xd8,

    /* U+00E0 "à" */
    0x60, 0xc0, 0x1e, 0xcc, 0x37, 0xf3, 0xcd, 0xf0,

    /* U+00E1 "á" */
    0x18, 0xc0, 0x1e, 0xcc, 0x37, 0xf3, 0xcd, 0xf0,

    /* U+00E2 "â" */
    0x31, 0xec, 0xc0, 0x7b, 0x30, 0xdf, 0xcf, 0x37,
    0xc0,

    /* U+00E3 "ã" */
    0x77, 0xb8, 0x3, 0xcc, 0xc1, 0x9f, 0x66, 0xcc,
    0xf8,

    /* U+00E4 "ä" */
    0xcf, 0x30, 0x1e, 0xcc, 0x37, 0xf3, 0xcd, 0xf0,

    /* U+00E5 "å" */
    0x7b, 0x37, 0x80, 0x7b, 0x30, 0xdf, 0xcf, 0x37,
    0xc0,

    /* U+00E6 "æ" */
    0x76, 0xdb, 0x1b, 0x7f, 0xd8, 0xdb, 0x6e,

    /* U+00E7 "ç" */
    0x7b, 0x3c, 0xf0, 0xcf, 0x37, 0x8c, 0x1b, 0xc0,

    /* U+00E8 "è" */
    0x60, 0xc0, 0x1e, 0xcf, 0x3f, 0xf0, 0xcd, 0xe0,

    /* U+00E9 "é" */
    0x18, 0xc0, 0x1e, 0xcf, 0x3f, 0xf0, 0xcd, 0xe0,

    /* U+00EA "ê" */
    0x31, 0xec, 0xc0, 0x7b, 0x3c, 0xff, 0xc3, 0x37,
    0x80,

    /* U+00EB "ë" */
    0xcf, 0x30, 0x1e, 0xcf, 0x3f, 0xf0, 0xcd, 0xe0,

    /* U+00EC "ì" */
    0x60, 0xc0, 0x3c, 0x30, 0xc3, 0xc, 0x33, 0xf0,

    /* U+00ED "í" */
    0x18, 0xc0, 0x3c, 0x30, 0xc3, 0xc, 0x33, 0xf0,

    /* U+00EE "î" */
    0x31, 0xec, 0xc0, 0xf0, 0xc3, 0xc, 0x30, 0xcf,
    0xc0,

    /* U+00EF "ï" */
    0xcf, 0x30, 0x3c, 0x30, 0xc3, 0xc, 0x33, 0xf0,

    /* U+00F0 "ð" */
    0x18, 0x7c, 0x33, 0xec, 0xd9, 0xb3, 0x66, 0x78,

    /* U+00F1 "ñ" */
    0x77, 0xb8, 0x7, 0xcc, 0xd9, 0xb3, 0x66, 0xcd,
    0x98,

    /* U+00F2 "ò" */
    0x60, 0xc0, 0x1e, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00F3 "ó" */
    0x18, 0xc0, 0x1e, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00F4 "ô" */
    0x31, 0xec, 0xc0, 0x7b, 0x3c, 0xf3, 0xcf, 0x37,
    0x80,

    /* U+00F5 "õ" */
    0x77, 0xb8, 0x3, 0xcc, 0xd9, 0xb3, 0x66, 0xcc,
    0xf0,

    /* U+00F6 "ö" */
    0xcf, 0x30, 0x1e, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+00F7 "÷" */
    0x30, 0xc0, 0x3f, 0x0, 0xc3, 0x0,

    /* U+00F8 "ø" */
    0x3d, 0x67, 0x6e, 0x7e, 0x76, 0xe6, 0xbc,

    /* U+00F9 "ù" */
    0x60, 0xc0, 0x33, 0xcf, 0x3c, 0xf3, 0xcd, 0xf0,

    /* U+00FA "ú" */
    0x18, 0xc0, 0x33, 0xcf, 0x3c, 0xf3, 0xcd, 0xf0,

    /* U+00FB "û" */
    0x31, 0xec, 0xc0, 0xcf, 0x3c, 0xf3, 0xcf, 0x37,
    0xc0,

    /* U+00FC "ü" */
    0xcf, 0x30, 0x33, 0xcf, 0x3c, 0xf3, 0xcd, 0xf0,

    /* U+00FD "ý" */
    0x18, 0xc0, 0x33, 0xcf, 0x3c, 0xf3, 0xcd, 0xf0,
    0xf3, 0x78,

    /* U+00FE "þ" */
    0xc3, 0xf, 0xb3, 0xcf, 0x3c, 0xf3, 0xfb, 0xc,
    0x0,

    /* U+00FF "ÿ" */
    0xcf, 0x30, 0x33, 0xcf, 0x3c, 0xf3, 0xcd, 0xf0,
    0xf3, 0x78
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 80, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 80, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 6, .adv_w = 96, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 9, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 17, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 27, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 35, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 43, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 44, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 49, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 54, .adv_w = 144, .box_w = 8, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 59, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 63, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 64, .adv_w = 80, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 65, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 66, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 74, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 81, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 88, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 95, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 109, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 116, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 137, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 48, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 149, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 155, .adv_w = 112, .box_w = 6, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 158, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 164, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 171, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 185, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 192, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 199, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 213, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 220, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 241, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 248, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 255, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 262, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 270, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 291, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 300, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 307, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 314, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 321, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 328, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 335, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 343, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 350, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 357, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 364, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 369, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 374, .adv_w = 80, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 379, .adv_w = 112, .box_w = 6, .box_h = 4, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 382, .adv_w = 112, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 383, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 384, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 390, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 397, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 403, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 410, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 416, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 423, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 431, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 438, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 446, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 456, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 463, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 470, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 477, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 483, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 489, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 497, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 505, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 511, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 517, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 524, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 530, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 536, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 543, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 549, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 557, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 563, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 569, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 572, .adv_w = 96, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 578, .adv_w = 128, .box_w = 7, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 580, .adv_w = 80, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 581, .adv_w = 80, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 586, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 593, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 601, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 605, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 612, .adv_w = 48, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 615, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 624, .adv_w = 112, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 626, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 635, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 641, .adv_w = 144, .box_w = 8, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 646, .adv_w = 112, .box_w = 6, .box_h = 3, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 649, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 658, .adv_w = 80, .box_w = 4, .box_h = 4, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 660, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 666, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 667, .adv_w = 64, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 668, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 676, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 684, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 685, .adv_w = 96, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 687, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 693, .adv_w = 144, .box_w = 8, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 698, .adv_w = 176, .box_w = 1, .box_h = 1, .ofs_x = 9, .ofs_y = 4},
    {.bitmap_index = 699, .adv_w = 80, .box_w = 1, .box_h = 1, .ofs_x = 3, .ofs_y = 8},
    {.bitmap_index = 700, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 707, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 716, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 725, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 734, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 745, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 754, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 763, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 771, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 780, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 789, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 798, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 807, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 816, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 825, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 834, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 843, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 852, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 860, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 871, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 880, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 889, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 898, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 909, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 918, .adv_w = 112, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 922, .adv_w = 144, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 931, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 940, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 949, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 958, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 967, .adv_w = 112, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 976, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 983, .adv_w = 112, .box_w = 6, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 990, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 998, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1006, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1015, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1024, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1032, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1041, .adv_w = 144, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1048, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1056, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1064, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1072, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1081, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1089, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1097, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1105, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1114, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1122, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1130, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1139, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1147, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1155, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1164, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1173, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1181, .adv_w = 112, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 1187, .adv_w = 144, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1194, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1202, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1210, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1219, .adv_w = 112, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1227, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 1237, .adv_w = 112, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1246, .adv_w = 112, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint8_t glyph_id_ofs_list_1[] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 12, 0, 13, 0,
    14, 15, 16, 0, 17, 18, 19, 20,
    21, 0, 22, 23
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 160, .range_length = 28, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = glyph_id_ofs_list_1, .list_length = 28, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL
    },
    {
        .range_start = 189, .range_length = 67, .glyph_id_start = 120,
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
    .cmap_num = 3,
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
const lv_font_t Undertale_16b1 = {
#else
lv_font_t Undertale_16b1 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if UNDERTALE_16B1*/

