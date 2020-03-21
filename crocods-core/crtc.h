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
 * CRTC management
 */

#include  "crocods.h"

#ifndef CRTC_H
#define CRTC_H

#include "crtc-arnold.h"
#include "crtc-wincpc.h"
#include "crtc-cap32.h"

#ifdef __cplusplus
extern "C" {
#endif





void UpdateSTateCRTC(core_crocods_t *core, int RegIndex);

#ifdef __cplusplus
}
#endif

#endif
