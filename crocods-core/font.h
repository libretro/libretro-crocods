/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
 * @version 2.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * https://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * Font
 */

#include  "crocods.h"

#ifndef _font_h_
#define _font_h_

//byte order: big endian
//tiling grid: 8x8

//dimension data
#define font_WIDTH	8
#define font_HEIGHT	768

//byte array representing the picture
extern const u16 fontData[];
extern const u16 fontPalette[];
extern const u32 fontDataSize;
extern const u32 fontPaletteSize;

#endif // _font_h_
