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
 * Read Gif
 */

#include  "crocods.h"

void ReadBackgroundGifInfo(u32 *w, u32 *h, unsigned char *from, int size);
int ReadBackgroundGif(u16 *dest, char *filename);

int ReadBackgroundGif16(u16 *dest, unsigned char *from, int size);

typedef struct {
	u32           dwTop;
	u32           dwWidth;
	u32           dwHeight;
	s32            lTracking;
	int*           pOffset;
	int*           pWidth;
	int*           pKerningLeft;
	int*           pKerningRight;
	u32           dwFlags;
	u16 *pSurface;
    
} KKFONT;

