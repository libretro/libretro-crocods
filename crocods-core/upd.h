/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
 * @author  Ludovic Delplanque <ludovic.deplanque@libertysurf.fr> for the original version pc-cpc v0.1w (18/02/2003)
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
 * UPD emulation
 */

#include  "crocods.h"

#ifndef UPD_H
#define UPD_H

#ifdef __cplusplus
extern "C" {
#endif
    
    int ReadUPD( core_crocods_t *core, int port );
    
    void WriteUPD( core_crocods_t *core, int Port, int val );
    
    void ResetUPD( core_crocods_t *core );
    
    void SetDiskUPD( core_crocods_t *core, char * );
    
    void EjectDiskUPD( core_crocods_t *core );
    
    int GetCurrTrack( core_crocods_t *core );
    
    void LireDiskMem( core_crocods_t *core, u8 *rom, u32 romsize, char *autofile);
    
#ifdef __cplusplus
}
#endif

#endif
