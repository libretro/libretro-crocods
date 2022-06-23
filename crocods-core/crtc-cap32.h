/* Caprice32 - Amstrad CPC Emulator
   (c) Copyright 1997-2004 Ulrich Doewich

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "crocods.h"

#ifndef CRTC_CAP32_H
#define CRTC_CAP32_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned char   cap32_bool;
typedef unsigned char   byte;
typedef uint32_t        dword;
typedef uint16_t        word;

#pragma pack(1)

// The next 4 bytes must remain together
#if RETRO_IS_LITTLE_ENDIAN
typedef union {
    dword combined;
    struct {
        byte monVSYNC;
        byte inHSYNC;
        union {
            word combined;
            struct {
                byte DISPTIMG;
                byte HDSPTIMG;
            };
        } dt;
    };
} t_flags1;
// The next two bytes must remain together
typedef union {
    word combined;
    struct {
        byte NewDISPTIMG;
        byte NewHDSPTIMG;
    };
} t_new_dt;
#elif RETRO_IS_BIG_ENDIAN
typedef union {
    dword combined;
    struct {
        union {
            word combined;
            struct {
	        byte HDSPTIMG;
                byte DISPTIMG;
            };
        } dt;
        byte inHSYNC;
        byte monVSYNC;
    };
} t_flags1;
// The next two bytes must remain together
typedef union {
    word combined;
    struct {
        byte NewHDSPTIMG;
        byte NewDISPTIMG;
    };
} t_new_dt;
#else
#error Unknown endianness
#endif

#pragma pack()

void cap32_crtc_cycle(core_crocods_t *core, u32 repeat_count);

void cap32_update_skew(core_crocods_t *core);
void cap32_CharMR1(core_crocods_t *core);
void cap32_CharMR2(core_crocods_t *core);
void cap32_prerender_border(core_crocods_t *core);
void cap32_prerender_border_half(core_crocods_t *core);
void cap32_prerender_sync(core_crocods_t *core);
void cap32_prerender_sync_half(core_crocods_t *core);
void cap32_prerender_normal(core_crocods_t *core);
void cap32_prerender_normal_half(core_crocods_t *core);
void cap32_prerender_normal_plus(core_crocods_t *core);
void cap32_prerender_normal_half_plus(core_crocods_t *core);
void cap32_crtc_init(core_crocods_t *core);
void cap32_crtc_reset(core_crocods_t *core);
dword cap32_shiftLittleEndianDwordTriplet(core_crocods_t *core, dword val1, dword val2, dword val3, int byteShift);

void cap32_render16bpp(core_crocods_t *core);

void cap32_WriteCRTC(core_crocods_t *core, u8 val);
u8 cap32_ReadCRTC(core_crocods_t *core);
void cap32_RegisterSelectCRTC(core_crocods_t *core, u8 val);

void cap32_ResetVGA(core_crocods_t *core);
void cap32_endofline(core_crocods_t *core);

u16 cap32_cpu_doFrame(core_crocods_t *core);

#ifdef __cplusplus
}
#endif

#endif
