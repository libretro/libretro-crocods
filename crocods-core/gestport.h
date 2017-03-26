/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
 * @author  Ludovic Delplanque <ludovic.deplanque@libertysurf.fr> for the original version pc-cpc v0.1x (22/01/2004)
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
 * Port emulation
 */

#include  "crocods.h"

#ifndef GESTPORT_H
#define GESTPORT_H

#ifdef __cplusplus
extern "C" {
#endif
    
    u8 ReadPort( core_crocods_t *core, u16 port );    
    void WritePort( core_crocods_t *core, u16 port, u8 val );
    
#ifdef __cplusplus
}
#endif

#endif
