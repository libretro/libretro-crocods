/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
 * @author  Ludovic Delplanque <ludovic.deplanque@libertysurf.fr> for the original version pc-cpc v0.1p (05/11/2002)
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
 * AY3-8912 emulation
 */

#include "crocods.h"

#ifndef AY8912_H
#define AY8912_H

#define SOUNDV2 OK

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        unsigned short left;
        unsigned short right;
    } GB_sample_t;

    void Reset8912(core_crocods_t *core);

    void Write8912(core_crocods_t *core, int r, int v);
    int Read8912(core_crocods_t *core, int r);

    void Loop8912(core_crocods_t *core, int uS);

    BOOL IsWritePSGReg13(core_crocods_t *core);

    void SetSound(core_crocods_t *core, int Channel, int Freq, int Volume);

    void initSound(core_crocods_t *gb, int r);
    void procSound(core_crocods_t *gb);

    void crocods_copy_sound_buffer(core_crocods_t *gb, GB_sample_t *dest, unsigned int count);

#ifdef __cplusplus
}
#endif

#endif
