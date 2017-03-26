/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
 * @author  Ludovic Delplanque <ludovic.deplanque@libertysurf.fr> for the original version pc-cpc v0.1x (02/02/2004)
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
 * PPI 8255 emulation
 */

#include  "crocods.h"

#ifndef PPI_H
#define PPI_H

#ifdef __cplusplus
extern "C" {
#endif
    
    void WritePPI( core_crocods_t *core, int adr, int val );
    
    int ReadPPI( core_crocods_t *core, int adr );
    void WriteCas( core_crocods_t *core );
    
    void ReadCas( core_crocods_t *core );
    
    void OpenTapWrite( core_crocods_t *core, char * Nom );
    
    void OpenTapRead( core_crocods_t *core, char * Nom );
    
    void CloseTap( core_crocods_t *core );
    
    void ResetPPI( core_crocods_t *core);

    
    BOOL Keyboard_HasBeenScanned(core_crocods_t *core);
    void Keyboard_Reset(core_crocods_t *core);
    
#ifdef __cplusplus
}
#endif


#endif


