#include "plateform.h"
#include "z80.h"
#include "crtc.h"
#include "vga.h"

void ResetCRTC( core_crocods_t *core )
{
    memset( core->RegsCRTC, -1, sizeof( core->RegsCRTC ) );

    core->TailleVBL = 16;
    core->CptCharVert = 0;
    core->NumLigneChar = 0;
    core->LigneCRTC = 0;
    core->MaCRTC = 0;
    core->SyncCount = 0;

    core->RegsCRTC[0]=0;
}


/********************************************************* !NAME! **************
* Nom : WriteCRTC
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Ecriture d'un registre du CRTC
*
* Résultat    : /
*
* Variables globales modifiées : RegsCRTC, RegCRTCSel, XStart, XEnd
*
********************************************************** !0! ****************/
void WriteCRTC( core_crocods_t *core, u16 adr, u8 val )
{
    adr &= 0xBF00;
    if (adr == 0xBC00) {
        core->RegCRTCSel = val & 0x1F;
    } else {
        if (adr == 0xBD00) {
            switch(core->RegCRTCSel) { // only registers 0 - 15 can be written to
            case 1: // horizontal displayed
                core->RegsCRTC[core->RegCRTCSel] = val;
                break;
            case 2: // horizontal sync position
                core->RegsCRTC[core->RegCRTCSel] = val;
                break;
            case 8: // interlace and skew
                val &= 0xF3; // sur PreAsic, Asic, HD6845S
                // val &= 0x03;  // 03 sur HD6845R, UM6845R, MC6845
                core->RegsCRTC[core->RegCRTCSel] = val;
                break;
            case 0: // horizontal total
                core->RegsCRTC[0] = val;
                break;
            case 3: // sync width
            case 13: // start address low byte
            case 15: // cursor address low byte
                core->RegsCRTC[core->RegCRTCSel] = val;
                break;
            case 4: // vertical total
                val &= 0x7F; // sur PreAsic, Asic, HD6845S
                // val &= 0x0F; // sur HD6845R, UM6845R, MC6845
                core->RegsCRTC[core->RegCRTCSel] = val;
                break;
            case 6: // vertical displayed
            case 7: // vertical sync position
            case 10: // cursor start raster
                val &= 0x7F;
                core->RegsCRTC[core->RegCRTCSel] = val;
                break;
            case 5: // vertical total adjust
            case 9: // maximum raster count
            case 11: // cursor end raster
                val &= 0x1F;
                core->RegsCRTC[core->RegCRTCSel] = val;
                break;
            case 12: // start address high byte
            case 14: // cursor address high byte
                val &= 0x3F;
                core->RegsCRTC[core->RegCRTCSel] = val;
                break;
            }
        }
    }
    core->XStart = max( ( 50 - core->RegsCRTC[ 2 ] ) << 1, 0 );
    core->XEnd = min( core->XStart + ( core->RegsCRTC[ 1 ] << 1 ), 96 );
}


/********************************************************* !NAME! **************
* Nom : CalcCRTCLine
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Calcul et affiche une ligne CRTC
*
* Résultat    : La ligne suivante du CRTC à afficher
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
int CalcCRTCLine( core_crocods_t *core )
{
    if ( !--core->TailleVBL ) {
        core->VSync = 0;
    }

    // if ( ++LigneCRTC >= RegsCRTC[ 5 ] )
    if ( ++core->LigneCRTC ) {
        if ( core->NumLigneChar == core->RegsCRTC[ 9 ] ) {
            core->NumLigneChar = 0;
            core->CptCharVert = ( core->CptCharVert + 1 ) & 0x7F;
            core->MaCRTC += core->RegsCRTC[ 1 ];
        }
        else {
            core->NumLigneChar = ( core->NumLigneChar + 1 ) & 0x1F;
        }

        if ( core->CptCharVert == core->RegsCRTC[ 4 ] + 1 ) {
            core->NumLigneChar = 0; // ### Pas sur...
            core->CptCharVert = 0;
            core->MaCRTC = core->RegsCRTC[ 13 ] | ( core->RegsCRTC[ 12 ] << 8 );
        }

        if ( core->CptCharVert == core->RegsCRTC[ 7 ]  && !core->NumLigneChar ) {
            core->LigneCRTC = 0;
            core->TailleVBL = 16;
            core->SyncCount = 2;
            core->VSync = 1;
            core->DoResync = 1;
        } else {
            int y = core->LigneCRTC - 32;
            if ( y >= 0 && y < TAILLE_Y_LOW ) {
                core->DrawFct( core, y
                               , core->CptCharVert < core->RegsCRTC[ 6 ] ? core->MaCRTC << 1 : -1
                               , ( ( core->NumLigneChar ) << 11 ) | ( ( core->MaCRTC & 0x3000 ) << 2 )
                               );
            }
            else
            if ( core->LigneCRTC > 312 ) {
                core->LigneCRTC = 0;
                core->DoResync = 0;
            }
        }
    }

// ASIC HSync

    if (core->hw==CPC_HW_CPCPLUS) {
        /* yes. See if ASIC interrupts are required */
        // ASIC_HSync(CRTC_InternalState.LineCounter, CRTC_InternalState.RasterCounter);
    }

// VGA Update

    VGA_Update(core);

    return( core->LigneCRTC );
}
