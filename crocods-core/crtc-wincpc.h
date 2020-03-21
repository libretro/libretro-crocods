/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
 * @author  Ludovic Delplanque <ludovic.deplanque@libertysurf.fr> for the original version pc-cpc v0.1r (05/11/2002)
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
 * CRTC management
 */

#include  "crocods.h"

#ifndef CRTC_WINCPC_H
#define CRTC_WINCPC_H

#ifdef __cplusplus
extern "C" {
#endif



u8 wincpc_ReadCRTC(core_crocods_t *core);
void wincpc_WriteCRTC(core_crocods_t *core, u8 val);
void wincpc_RegisterSelectCRTC(core_crocods_t *core, u8 val);

u8 wincpc_CRTC_DoLine(core_crocods_t *core);
void UpdateSTateCRTC(core_crocods_t *core, int RegIndex);

void wincpc_ResetCRTC(core_crocods_t *core);

void wincpc_CRTC_DoCycles(core_crocods_t *core, u32 Cycles);

u16 wincpc_cpu_doFrame(core_crocods_t *core);

#ifdef __cplusplus
}
#endif

#endif
