#include "plateform.h"
#include "z80.h"
#include "crtc.h"
#include "vga.h"

void VGA_Update(core_crocods_t *core);

void wincpc_ResetCRTC(core_crocods_t *core)
{
    memset(core->RegsCRTC, -1, sizeof(core->RegsCRTC) );

    core->TailleVBL = 16;
    core->CptCharVert = 0;
    core->NumLigneChar = 0;
    core->LigneCRTC = 0;
    core->MaCRTC = 0;
    core->SyncCount = 0;
    
            core->screenIsOptimized = 1;


    core->RegsCRTC[0] = 0;
}

const unsigned char wincpc_HD6845S_ReadMaskTable[32] =
{
    0x000,  /* Horizontal Total */
    0x000,  /* Horizontal Displayed */
    0x000,  /* Horizontal Sync Position */
    0x000,  /* Sync Widths */
    0x000,  /* Vertical Total */
    0x000,  /* Vertical Adjust */
    0x000,  /* Vertical Displayed */
    0x000,  /* Vertical Sync Position */
    0x000,  /* Interlace and Skew */
    0x000,  /* Maximum Raster Address */
    0x000,  /* Cursor Start */
    0x000,  /* Cursor End */
    0x0ff,  /* Screen Addr (H) */
    0x0ff,  /* Screen Addr (L) */
    0x0ff,  /* Cursor (H) */
    0x0ff,  /* Cursor (L) */

    0x0ff,  /* Light Pen (H) */
    0x0ff,  /* Light Pen (L) */
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
};

void wincpc_CRTC_DoCycles(core_crocods_t *core, u32 Cycles)
{
}

u8 wincpc_ReadCRTC(core_crocods_t *core)
{
    int CRTC_RegIndex = (core->CRTC_Reg & 0x01f);

    /* unreadable registers return 0 */
    return (core->RegsCRTC[CRTC_RegIndex] & wincpc_HD6845S_ReadMaskTable[CRTC_RegIndex]);
}

void wincpc_RegisterSelectCRTC(core_crocods_t *core, u8 val)
{
    core->CRTC_Reg = val & 0x1F;
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
* Variables globales modifiées : RegsCRTC, CRTC_Reg, XStart, XEnd
*
********************************************************** !0! ****************/
void wincpc_WriteCRTC(core_crocods_t *core, u8 val)
{
    switch (core->CRTC_Reg) {        // only registers 0 - 15 can be written to
        case 1:     // horizontal displayed
            core->RegsCRTC[core->CRTC_Reg] = val;
            break;
        case 2:     // horizontal sync position
            core->RegsCRTC[core->CRTC_Reg] = val;
            break;
        case 8:     // interlace and skew
            val &= 0xF3;     // sur PreAsic, Asic, HD6845S
            // val &= 0x03;  // 03 sur HD6845R, UM6845R, MC6845
            core->RegsCRTC[core->CRTC_Reg] = val;
            break;
        case 0:     // horizontal total
            core->RegsCRTC[0] = val;
            break;
        case 3:     // sync width
        case 13:     // start address low byte
        case 15:     // cursor address low byte
            core->RegsCRTC[core->CRTC_Reg] = val;
            break;
        case 4:     // vertical total
            val &= 0x7F;     // sur PreAsic, Asic, HD6845S
            // val &= 0x0F; // sur HD6845R, UM6845R, MC6845
            core->RegsCRTC[core->CRTC_Reg] = val;
            break;
        case 6:     // vertical displayed
        case 7:     // vertical sync position
        case 10:     // cursor start raster
            val &= 0x7F;
            core->RegsCRTC[core->CRTC_Reg] = val;
            break;
        case 5:     // vertical total adjust
        case 9:     // maximum raster count
        case 11:     // cursor end raster
            val &= 0x1F;
            core->RegsCRTC[core->CRTC_Reg] = val;
            break;
        case 12:     // start address high byte
        case 14:     // cursor address high byte
            val &= 0x3F;
            core->RegsCRTC[core->CRTC_Reg] = val;
            break;
    }

    core->XStart = max( (50 - core->RegsCRTC[ 2 ]) << 1, 0);
    core->XEnd = min(core->XStart + (core->RegsCRTC[ 1 ] << 1), 96);
}

/********************************************************* !NAME! **************
* Nom : wincpc_CRTC_DoLine
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
u8 wincpc_CRTC_DoLine(core_crocods_t *core)
{
    if (!--core->TailleVBL) {
        CRTC_ClearFlag(CRTC_VS_FLAG);
    }

    // if ( ++LigneCRTC >= RegsCRTC[ 5 ] )
    if (++core->LigneCRTC) {
        if (core->NumLigneChar == core->RegsCRTC[ 9 ]) {
            core->NumLigneChar = 0;
            core->CptCharVert = (core->CptCharVert + 1) & 0x7F;
            core->MaCRTC += core->RegsCRTC[ 1 ];
        } else {
            core->NumLigneChar = (core->NumLigneChar + 1) & 0x1F;
        }

        if (core->CptCharVert == core->RegsCRTC[ 4 ] + 1) {
            core->NumLigneChar = 0; // ### Pas sur...
            core->CptCharVert = 0;
            core->MaCRTC = core->RegsCRTC[ 13 ] | (core->RegsCRTC[ 12 ] << 8);
        }

        if (core->CptCharVert == core->RegsCRTC[ 7 ]  && !core->NumLigneChar) {
            core->LigneCRTC = 0;
            core->TailleVBL = 16;
            core->SyncCount = 2;
            // core->DoResync = 1;
            CRTC_SetFlag(CRTC_VS_FLAG);
        } else {
            int y = core->LigneCRTC - 32;
            if (y >= 0 && y < TAILLE_Y_LOW) {
                core->DrawFct(core, y
                              , core->CptCharVert < core->RegsCRTC[ 6 ] ? core->MaCRTC << 1 : -1
                              , ( (core->NumLigneChar) << 11) | ( (core->MaCRTC & 0x3000) << 2)
                              );
            } else
            if (core->LigneCRTC > 312) {
                core->LigneCRTC = 0;
                //    core->DoResync = 0;
            }
        }
    }

// ASIC HSync

    if (core->hw == CPC_HW_CPCPLUS) {
        /* yes. See if ASIC interrupts are required */
        // ASIC_HSync(CRTC_InternalState.LineCounter, CRTC_InternalState.RasterCounter);
    }

// VGA Update

    VGA_Update(core);

    return(core->LigneCRTC);

//    return( core->DoResync == 0); // core->LigneCRTC );
}

void VGA_Update(core_crocods_t *core)
{
    core->CntHSync++;
    if (!core->SyncCount) {
        //
        // Si 52 lignes comptée -> Génération d'une interruption
        //
        if (core->CntHSync == 52) {
            core->CntHSync = 0;

            SetIRQZ80(core, 1);
        }
    } else {
        core->SyncCount--;
        if (core->SyncCount == 0) {
            if (core->CntHSync & 32) {   // Interrupt line counter overflowed
                SetIRQZ80(core, 1);
            }

            core->CntHSync = 0;
        }
    }
} // VGA_Update

void wincpc_ResetVGA(core_crocods_t *core)
{
}

// --- doFrame

u16 wincpc_cpu_doFrame(core_crocods_t *core)
{
    long TimeOut = 0;
    int byCycle = 0;
    int i;

    // Do CRTC by lines
    do {
        int cycle = ExecInstZ80(core);                 // Run one CRTC Line of Z80
        byCycle += cycle;

        for (i = 0; i < cycle / 6; i++) {        // Why 6 ???
            procSound(core);
        }

        TimeOut += core->RegsCRTC[ 0 ] + 1;
    }  while (CRTC_DoLine(core) != 0);

    return TimeOut;
}
