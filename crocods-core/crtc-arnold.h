/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
 * @author  Kevin Thacker for the original version on Arnold (1995-2015)
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

#ifndef CRTC_ARNOLD_H
#define CRTC_ARNOLD_H

#ifdef __cplusplus
extern "C" {
#endif

#define CRTC_VS_FLAG          0x001 /* Vsync active */
#define CRTC_HS_FLAG          0x002 /* Hsync active */
#define CRTC_HDISP_FLAG       0x004 /* Horizontal Display Timing */
#define CRTC_VDISP_FLAG       0x008 /* Vertical Display Timing */
#define CRTC_HTOT_FLAG        0x010 /* HTot reached */
#define CRTC_VTOT_FLAG        0x020 /* VTot reached */
#define CRTC_MR_FLAG          0x040 /* Max Raster reached */
#define CRTC_VADJ_FLAG        0x080
#define CRTC_R8DT_FLAG        0x100
#define CRTC_VSCNT_FLAG       0x200
#define CRTC_HSCNT_FLAG       0x400
#define CRTC_VSALLOWED_FLAG   0x800
#define CRTC_VADJWANTED_FLAG  0x01000
#define CRTC_INTERLACE_ACTIVE 0x02000

#define CRTC_ClearFlag(x) core->CRTC_Flags &= ~x
#define CRTC_SetFlag(x)   core->CRTC_Flags |= x

void TraceWord8B512(core_crocods_t *core, int x0, int y, int Adr);

u8 arn_ReadCRTC(core_crocods_t *core);
void arn_WriteCRTC(core_crocods_t *core, u8 val);
void arn_RegisterSelectCRTC(core_crocods_t *core, u8 val);

void UpdateSTateCRTC(core_crocods_t *core, int RegIndex);

void arn_ResetCRTC(core_crocods_t *core);

void arn_CRTC_DoCycles(core_crocods_t *core, u32 Cycles);

void arn_GateArray_Cycle(core_crocods_t *core);
void arn_ResetVGA(core_crocods_t *core);

u16 arn_cpu_doFrame(core_crocods_t *core);

#ifdef __cplusplus
}
#endif

#endif
