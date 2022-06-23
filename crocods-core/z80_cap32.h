/**
* @file
* @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
* @author  Ludovic Delplanque <ludovic.deplanque@libertysurf.fr> for the original version pc-cpc v0.1z (30/03/2004)
* @author  Ulrich Doewich  (Caprice32 up to version 2.00b2)
* @author  Juergen Buchmueller (MAME Z80 core v3.3)
* @author  Marat Fayzullin
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
* Zilog Z80A Microprocessor emulation (T-States)
*/

#ifndef Z80_CAP32_H
#define Z80_CAP32_H

#include  "crocods.h"
#include  "platform.h"
#include  "retro_endianness.h"


typedef unsigned short word;
typedef unsigned char byte;
typedef uint32_t        dword;

typedef union {
#if RETRO_IS_LITTLE_ENDIAN
   struct { byte l, h, h2, h3; } b;
   struct { word l, h; } w;
#elif RETRO_IS_BIG_ENDIAN
   struct { byte h3, h2, h, l; } b;
   struct { word h, l; } w;
#else
#error Unknown endianness
#endif
   dword d;
}  reg_pair;

#define Sflag  0x80 // sign flag
#define Zflag  0x40 // zero flag
#define Hflag  0x10 // halfcarry flag
#define Pflag  0x04 // parity flag
#define Vflag  0x04 // overflow flag
#define Nflag  0x02 // negative flag
#define Cflag  0x01 // carry flag
#define Xflags 0x28 // bit 5 & 3 flags

typedef struct {
   reg_pair AF, BC, DE, HL, PC, SP, AFx, BCx, DEx, HLx, IX, IY;
   byte I, R, Rb7, IFF1, IFF2, IM, HALT, EI_issued, int_pending;
   dword break_point, trace;
} t_z80regs;



#define EC_BREAKPOINT      10
#define EC_FRAME_COMPLETE  30
#define EC_CYCLE_COUNT     40
#define EC_SOUND_BUFFER    50



byte read_mem(word addr);
void write_mem(word addr, byte val);

void ResetZ80_cap32(core_crocods_t *core0);
void SetIRQZ80_cap32(core_crocods_t *core, u8 i);

int z80_execute(void);
int z80_execute_debug(void);

// Handle prefixed bits instructions.
void z80_execute_pfx_cb_instruction(void);

// Handle prefixed IX instructions.
void z80_execute_pfx_dd_instruction(void);

// Handle prefixed IX bit instructions.
void z80_execute_pfx_ddcb_instruction(void);

// Handle prefixed extended instructions.
void z80_execute_pfx_ed_instruction(void);

// Handle prefixed IY instructions.
void z80_execute_pfx_fd_instruction(void);

// Handle prefixed IY bit instructions.
void z80_execute_pfx_fdcb_instruction(void);

#endif
