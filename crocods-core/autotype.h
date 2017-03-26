/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
 * @author  Kevin Thacker - Arnold emulator(c) Copyright 1995-2003
 * @author  Troels K <troels@trak.dk> for the ASCII_TO_CPCKEY_MAP - http://cpcloader.trak.dk
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
 * Autotype
 */

#include  "crocods.h"

#ifndef __AUTOTYPE_HEADER_INCLUDED__
#define __AUTOTYPE_HEADER_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif
    
    void ASCII_to_CPC(core_crocods_t *core, int nASCII, BOOL bKeyDown);
    
    /* auto-type is active and is "typing" */
#define AUTOTYPE_ACTIVE 0x01
    /* auto-type is performing key release action */
    /* if clear, auto-type is performing key press action */
#define AUTOTYPE_RELEASE 0x02
    /* if set, auto-type is waiting for first keyboard scan to be done */
#define AUTOTYPE_WAITING 0x04
    
    
    void AutoType_Init(core_crocods_t *core);
    BOOL AutoType_Active(core_crocods_t *core);
    void AutoType_SetString(core_crocods_t *core, const char *sString, BOOL bWaitInput);
    void AutoType_Update(core_crocods_t *core);
    
    
#ifdef __cplusplus
}
#endif

#endif
