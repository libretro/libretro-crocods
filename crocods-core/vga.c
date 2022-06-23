#include "vga.h"

#include "z80.h"

#include "plateform.h"
#include "monitor.h"

#define   ROMINF_OFF 0x04
#define   ROMSUP_OFF 0x08

/** @brief Mapping de la m�moire du CPC en fonction de la s�lection des roms et rams
 *
 *  @param core The core.
 *  @return Void.
 */
static void SetMemCPC(core_crocods_t *core)
{
    int AdjRam[ 8 ][ 4 ][ 8 ] =
    {
        // C0       C1       C2       C3       C4       C5       C6       C7
        { { 0x00000, 0x00000, 0x10000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000 },
            { 0x04000, 0x04000, 0x14000, 0x0C000, 0x10000, 0x14000, 0x18000, 0x1C000 },
            { 0x08000, 0x08000, 0x18000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000 },
            { 0x0C000, 0x1C000, 0x1C000, 0x1C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000 } },

        // C8       C9       CA       CB       CC       CD       CE       CF
        { { 0x00000, 0x00000, 0x20000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000 },
            { 0x04000, 0x04000, 0x24000, 0x0C000, 0x20000, 0x24000, 0x28000, 0x2C000 },
            { 0x08000, 0x08000, 0x28000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000 },
            { 0x0C000, 0x2C000, 0x2C000, 0x2C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000 } },

        // D0       D1       D2       D3       D4       D5       D6       D7
        { { 0x00000, 0x00000, 0x30000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000 },
            { 0x04000, 0x04000, 0x34000, 0x0C000, 0x30000, 0x34000, 0x38000, 0x3C000 },
            { 0x08000, 0x08000, 0x38000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000 },
            { 0x0C000, 0x3C000, 0x3C000, 0x3C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000 } },

        // D8       D9       DA       DB       DC       DD       DE       DF
        { { 0x00000, 0x00000, 0x40000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000 },
            { 0x04000, 0x04000, 0x44000, 0x0C000, 0x40000, 0x44000, 0x48000, 0x4C000 },
            { 0x08000, 0x08000, 0x48000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000 },
            { 0x0C000, 0x4C000, 0x4C000, 0x4C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000 } },

        // E0       E1       E2       E3       E4       E5       E6       E7
        { { 0x00000, 0x00000, 0x50000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000 },
            { 0x04000, 0x04000, 0x54000, 0x0C000, 0x50000, 0x54000, 0x58000, 0x5C000 },
            { 0x08000, 0x08000, 0x58000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000 },
            { 0x0C000, 0x5C000, 0x5C000, 0x5C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000 } },

        // E8       E9       EA       EB       EC       ED       EE       EF
        { { 0x00000, 0x00000, 0x60000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000 },
            { 0x04000, 0x04000, 0x64000, 0x0C000, 0x60000, 0x64000, 0x68000, 0x6C000 },
            { 0x08000, 0x08000, 0x68000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000 },
            { 0x0C000, 0x6C000, 0x6C000, 0x6C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000 } },

        // F0       F1       F2       F3       F4       F5       F6       F7
        { { 0x00000, 0x00000, 0x70000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000 },
            { 0x04000, 0x04000, 0x74000, 0x0C000, 0x70000, 0x74000, 0x78000, 0x7C000 },
            { 0x08000, 0x08000, 0x78000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000 },
            { 0x0C000, 0x7C000, 0x7C000, 0x7C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000 } },

        // F8       F9       FA       FB       FC       FD       FE       FF
        { { 0x00000, 0x00000, 0x80000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000 },
            { 0x04000, 0x04000, 0x84000, 0x0C000, 0x80000, 0x84000, 0x88000, 0x8C000 },
            { 0x08000, 0x08000, 0x88000, 0x08000, 0x08000, 0x08000, 0x08000, 0x08000 },
            { 0x0C000, 0x8C000, 0x8C000, 0x8C000, 0x0C000, 0x0C000, 0x0C000, 0x0C000 } }

        // 0123     0127     4567     0327     0423     0523     0623     0723
    };

    core->TabPOKE[ 0 ] = &core->MemCPC[ AdjRam[ core->Bloc ][ 0 ][ core->RamSelect ] ];
    if (core->DecodeurAdresse & ROMINF_OFF) {
        core->TabPEEK[ 0 ] = &core->MemCPC[ AdjRam[ core->Bloc ][ 0 ][ core->RamSelect ] ];
    } else {
        core->TabPEEK[ 0 ] = core->ROMINF;
    }

    core->TabPOKE[ 1 ] =
        core->TabPEEK[ 1 ] = &core->MemCPC[ AdjRam[ core->Bloc ][ 1 ][ core->RamSelect ] ];

    core->TabPOKE[ 2 ] =
        core->TabPEEK[ 2 ] = &core->MemCPC[ AdjRam[ core->Bloc ][ 2 ][ core->RamSelect ] ];

    core->TabPOKE[ 3 ] = &core->MemCPC[ AdjRam[ core->Bloc ][ 3 ][ core->RamSelect ] ];

    if (core->DecodeurAdresse & ROMSUP_OFF) {
        core->TabPEEK[ 3 ] = &core->MemCPC[ AdjRam[ core->Bloc ][ 3 ][ core->RamSelect ] ];
    } else {
        core->TabPEEK[ 3 ] = core->ROMEXT[ core->NumRomExt ];
    }
} // SetMemCPC

void WriteVGA(core_crocods_t *core, u16 port, u8 val)
{
    u8 newVal = val & 0x1F;
    u8 Use512Ko = 1;

    //    myprintf("VGA: %d %d", val&0x1F, val >> 6);

    switch (val >> 6) {
        case 0: // function 00xxxxxx
            core->PenSelection = val;

            if ((newVal & 0x10) == 0) {
                core->PenIndex = val & 0x0f;
            } else {
                core->PenIndex = 0x10;
            }
            break;

        case 1: // function 01xxxxxx
            core->ColourSelection = val;

            if (core->TabCoul[ core->PenIndex ] != newVal) {
                core->TabCoul[ core->PenIndex ] = newVal;
                core->UpdateInk = 1; // Update ink if necessary
            }
            break;

        case 2: // function 10xxxxxx

            if (core->DecodeurAdresse != val) {
//                printf("Change decodeur: %d\n", core->DecodeurAdresse);
            }

            core->DecodeurAdresse = val;
            core->lastMode = val & 3;  // requested_scr_mode
            core->changeFilter = 1;

            // printf("WriteVGA 2\n");

            SetMemCPC(core);
            if (val & 0x10) {
                core->CntHSync = 0;
                SetIRQZ80(core, 0);
            }
            core->UpdateInk = 1;

            break;

        case 3: // function 11xxxxxx        // PAL_WriteConfig

            core->RamSelect = val & 7;
            core->Bloc = Use512Ko * ( (val >> 3) & 7);

//            printf("Change ram: %d\n", core->RamSelect);

            SetMemCPC(core);
            core->UpdateInk = 1;    

            break;
    } // switch
} // WriteVGA

/********************************************************* !NAME! **************
* Nom : WriteROM
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : S�lection du num�ro de rom sup�rieure
*
* R�sultat    : /
*
* Variables globales modifi�es : RomExt
*
********************************************************** !0! ****************/
void WriteROM(core_crocods_t *core, int val)
{
    core->NumRomExt = val;
    SetMemCPC(core);

//    printf("Change rom (writeRom): %d\n", core->NumRomExt);
}

void AddRom(core_crocods_t *core, const char *rom, int i)
{
    memcpy(core->ROMEXT[ i ], rom, sizeof(core->ROMEXT[ i ]) );

//    WriteVGA(core, 0, 0x89);
//    WriteVGA(core, 0, 0xC0);
}

/********************************************************* !NAME! **************
* Nom : InitMemCPC
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Initialisation : lecture des roms du cpc
*
* R�sultat    : TRUE si ok, FALSE sinon
*
* Variables globales modifi�es : ROMINF, ROMSUP, ROMDISC
*
********************************************************** !0! ****************/
BOOL InitMemCPC(core_crocods_t *core, const char *cpc6128_bin, const char *romdisc_bin)
{
    core->MemCPC = MyAlloc(0x20000, "Memory CPC"); // 128Ko

    memcpy(core->ROMINF, cpc6128_bin, sizeof(core->ROMINF));

    memcpy(core->ROMEXT[0], cpc6128_bin + 0x4000, sizeof(core->ROMEXT[0]));
    memcpy(core->ROMEXT[7], romdisc_bin, sizeof(core->ROMEXT[7]));

    emulator_patch_ROM(core, core->ROMINF); // Patch de la langue (entre autres)

    WriteVGA(core, 0, 0x89);
    WriteVGA(core, 0, 0xC0);

    return(TRUE);
}
