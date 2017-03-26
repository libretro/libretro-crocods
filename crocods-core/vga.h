/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
 * @author  Ludovic Delplanque <ludovic.deplanque@libertysurf.fr> for the original version pc-cpc v0.1p (13/11/2002)
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
 * Gate Array emulation
 */

#include  "crocods.h"

#ifndef VGA_H
#define VGA_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Ecriture d'un registre du VGA
 *
 *  @param core The core.
 *  @param port
 *  @param val
 *  @return Void.
 */
void WriteVGA( core_crocods_t *core, u16 port, u8 val );
void WriteROM( core_crocods_t *core, int val );
void AddRom(core_crocods_t *core, const char *rom,int i);

BOOL InitMemCPC(core_crocods_t *core, const char *cpc6128_bin, const char *romdisc_bin);

void VGA_Interrupt(core_crocods_t *core);
void VGA_Update(core_crocods_t *core);

#ifdef __cplusplus
}
#endif


#endif
