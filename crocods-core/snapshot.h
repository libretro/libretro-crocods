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
 * Snapshot management
 * Use description from http://www.cpcwiki.eu/index.php/Format:SNA_snapshot_file_format
 */

#include "crocods.h"

#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#ifdef __cplusplus
extern "C"
{
#endif

    int getSnapshotSize(core_crocods_t *core);
    char *getSnapshot(core_crocods_t *core, int *len);

    void LireSnapshotMem(core_crocods_t *core, u8 *snap);

#ifdef __cplusplus
}
#endif

#endif
