/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
 * @author  Ludovic Delplanque <ludovic.deplanque@libertysurf.fr> for the original version pc-cpc v0.1z (30/03/2004)
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
 * Z80 emulation
 */

#include  "crocods.h"

#ifndef Z80_H
#define Z80_H

#define     BIT0        0x01
#define     BIT1        0x02
#define     BIT2        0x04
#define     BIT3        0x08
#define     BIT4        0x10
#define     BIT5        0x20
#define     BIT6        0x40
#define     BIT7        0x80

// Flags Z80

#define     FLAG_0      0x00
#define     FLAG_C      0x01
#define     FLAG_N      0x02
#define     FLAG_V      0x04

#define     FLAG_H      0x10

#define     FLAG_Z      0x40
#define     FLAG_S      0x80


/********************************************************* !NAME! **************
* Nom : Registre
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Structures
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Structure d'un registre Z80
*
********************************************************** !0! ****************/

#include "plateform.h"

//struct SRegs_s;
//typedef struct  SRegs;

typedef int ( * pfct )( core_crocods_t *core );

//void ReadZ80(SRegs *z0);
//void WriteZ80(SRegs *z0);

#define     RegAF           core->Z80.AF.Word
#define     RegBC           core->Z80.BC.Word
#define     RegDE           core->Z80.DE.Word
#define     RegHL           core->Z80.HL.Word

#define     Reg_AF          core->Z80._AF.Word
#define     Reg_BC          core->Z80._BC.Word
#define     Reg_DE          core->Z80._DE.Word
#define     Reg_HL          core->Z80._HL.Word

#define     RegSP           core->Z80.SP.Word
#define     RegPC           core->Z80.PC.Word
#define     RegIX           core->Z80.IX.Word
#define     RegIY           core->Z80.IY.Word
#define     RegIR           core->Z80.IR.Word

#define     RegA            core->Z80.AF.Byte.High
#define     FLAGS           core->Z80.AF.Byte.Low
#define     RegB            core->Z80.BC.Byte.High
#define     RegC            core->Z80.BC.Byte.Low
#define     RegD            core->Z80.DE.Byte.High
#define     RegE            core->Z80.DE.Byte.Low
#define     RegH            core->Z80.HL.Byte.High
#define     RegL            core->Z80.HL.Byte.Low
#define     RegI            core->Z80.IR.Byte.High
#define     RegR            core->Z80.IR.Byte.Low
#define     RegIXH          core->Z80.IX.Byte.High
#define     RegIXL          core->Z80.IX.Byte.Low
#define     RegIYH          core->Z80.IY.Byte.High
#define     RegIYL          core->Z80.IY.Byte.Low




UBYTE Peek8Ext( core_crocods_t *core, USHORT adr );

void Poke8Ext( core_crocods_t *core, USHORT adr, UBYTE val );

int Z80_NMI( core_crocods_t *core );
int ___C9( core_crocods_t *core );

int ExecInstZ80_debug(core_crocods_t *core);

int ExecInstZ80_orig(core_crocods_t *core);
void ResetZ80_orig(core_crocods_t *core);
void SetIRQZ80_orig(core_crocods_t *core, int i);

void ExecZ80Code(core_crocods_t *core, char *code, int len, SRegs *result);

#endif
