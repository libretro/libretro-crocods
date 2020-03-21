/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
 * @author  Kevin Thacker for the original version on Arnold (1995-2001)
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
 * Monitor Emulation
 */

#include  "crocods.h"

#ifndef MONITOR_H
#define MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

void Graphics_Update(core_crocods_t *core);

void Monitor_Reset(core_crocods_t *core);

void Monitor_DoVsyncStart(core_crocods_t *core);
void Monitor_DoVsyncEnd(core_crocods_t *core);
void Monitor_DoHsyncStart(core_crocods_t *core);
void Monitor_DoHsyncEnd(core_crocods_t *core);

#ifdef __cplusplus
}
#endif


#endif // ifndef MONITOR_H
