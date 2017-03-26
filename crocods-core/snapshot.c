#include  "snapshot.h"

#include  "plateform.h"
#include  "config.h"
#include  "sound.h"
#include  "crtc.h"
#include  "ppi.h"
#include  "vga.h"
#include  "z80.h"

#ifdef USE_SNAPSHOT


/********************************************************* !NAME! **************
 * Nom : StSnapShot
 ********************************************************** !PATHS! *************
 * !./V1!\!./V2!\!./V3!\!./V4!\Structures
 ********************************************************** !1! *****************
 *
 * Fichier     : !./FPTH\/FLE!, ligne : !./LN!
 *
 * Description : Structure en-tête fichier snapshot
 *
 ********************************************************** !0! ****************/
#pragma pack(1)
typedef struct
{
    u16    AF;
    u16    BC;
    u16    DE;
    u16    HL;
    u16    IR;
    UBYTE  IFF1;
    UBYTE  IFF2;
    u16    IX;
    u16    IY;
    u16    SP;
    u16    PC;
    UBYTE  InterruptMode;
    u16    _AF;
    u16    _BC;
    u16    _DE;
    u16    _HL;
} _SRegs;

typedef struct
{
    char  Id[ 0x10 ]; // "MV - SNA'
    UBYTE Version;
    _SRegs Z80;
    UBYTE InkReg;
    UBYTE InkData[ 17 ];
    UBYTE VGARom;
    UBYTE VGARam;
    UBYTE CRTCIndex;
    UBYTE CRTCReg[ 18 ];
    UBYTE NumRom;
    UBYTE PPI[ 4 ];
    UBYTE PsgIndex;
    UBYTE PsgData[ 16 ];
    u8 ram_size[2];
    UBYTE Unused[ 0x93 ];
} StSnapShot;
#pragma pack()

int getSnapshotSize(core_crocods_t *core) {
    return sizeof(StSnapShot)+0x20000;
}

char *getSnapshot(core_crocods_t *core, int *len) {
    char *buffer;

    int i;
    StSnapShot SnapShot;

    strcpy( SnapShot.Id, "MV - SNA" );
    SnapShot.Version = 1;

    SnapShot.Z80.AF = core->Z80.AF.Word;
    SnapShot.Z80.BC = core->Z80.BC.Word;
    SnapShot.Z80.DE = core->Z80.DE.Word;
    SnapShot.Z80.HL = core->Z80.HL.Word;
    SnapShot.Z80.IR = core->Z80.IR.Word;
    SnapShot.Z80.IFF1 = core->Z80.IFF1;
    SnapShot.Z80.IFF2 = core->Z80.IFF2;
    SnapShot.Z80.IX = core->Z80.IX.Word;
    SnapShot.Z80.IY = core->Z80.IY.Word;
    SnapShot.Z80.SP = core->Z80.SP.Word;
    SnapShot.Z80.PC = core->Z80.PC.Word;
    SnapShot.Z80.InterruptMode = core->Z80.InterruptMode;
    SnapShot.Z80._AF = core->Z80._AF.Word;
    SnapShot.Z80._BC = core->Z80._BC.Word;
    SnapShot.Z80._DE = core->Z80._DE.Word;
    SnapShot.Z80._HL = core->Z80._HL.Word;

    SnapShot.InkReg = ( UBYTE )core->PenIndex;
    for ( i = 0; i < 16; i++ )
        SnapShot.InkData[ i ] = ( UBYTE )core->TabCoul[ i ];

    SnapShot.VGARom = ( UBYTE )core->DecodeurAdresse;
    SnapShot.VGARam = ( UBYTE )core->RamSelect;
    SnapShot.CRTCIndex = ( UBYTE )core->RegCRTCSel;
    for ( i = 0; i < 18; i++ )
        SnapShot.CRTCReg[ i ] = ( UBYTE )core->RegsCRTC[ i ];

    SnapShot.NumRom = ( UBYTE )core->NumRomExt;
    for ( i = 0; i < 4; i++ )
        SnapShot.PPI[ i ] = ( UBYTE )core->RegsPPI[ i ];

    SnapShot.PsgIndex = ( UBYTE )core->RegPSGSel;
    for ( i = 0; i < 16; i++ )
        SnapShot.PsgData[ i ] = 0; // ( UBYTE )RegsPSG[ i ]; // On ne sauve pas les registres PSG

    SnapShot.ram_size[0] = 128;
    SnapShot.ram_size[1] = 0;

    *len = sizeof(StSnapShot)+0x20000;

    buffer = (char*)malloc(*len);
    memcpy(buffer, &SnapShot, sizeof(StSnapShot));
    memcpy(buffer + sizeof(StSnapShot), core->MemCPC, 0x20000);

    return buffer;
}


/********************************************************* !NAME! **************
 * Nom : SauveSnap
 ********************************************************** !PATHS! *************
 * !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
 ********************************************************** !1! *****************
 *
 * Fichier     : !./FPTH\/FLE!, ligne : !./LN!
 *
 * Description : Sauvegarde d'un fichier snapshot
 *
 * Résultat    : /
 *
 * Variables globales modifiées : /
 *
 ********************************************************** !0! ****************/
void SauveSnap(core_crocods_t *core, char * Nom)
{
    FILE * fp = fopen( Nom, "wb" );

    if (fp)  {
        int len;
        char *buffer = getSnapshot(core, &len);
        if (buffer != NULL) {
            fwrite( buffer, 1, len, fp );
            free(buffer);
        }
        fclose(fp);
    }
}


/********************************************************* !NAME! **************
 * Nom : LireSnap
 ********************************************************** !PATHS! *************
 * !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
 ********************************************************** !1! *****************
 *
 * Fichier     : !./FPTH\/FLE!, ligne : !./LN!
 *
 * Description : Lecture d'un fichier snapshot
 *
 * Résultat    : /
 *
 * Variables globales modifiées : Z80, PenSenect, RegPSGSel, RomExt, RegCRTCSel
 *
 ********************************************************** !0! ****************/


/********************************************************* !NAME! **************
 * Nom : LireSnap
 ********************************************************** !PATHS! *************
 * !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
 ********************************************************** !1! *****************
 *
 * Fichier     : !./FPTH\/FLE!, ligne : !./LN!
 *
 * Description : Lecture d'un fichier snapshot
 *
 * Résultat    : /
 *
 * Variables globales modifiées : Z80, PenSenect, RegPSGSel, RomExt, RegCRTCSel
 *
 ********************************************************** !0! ****************/
void LireSnapshotMem(core_crocods_t *core, u8 *snap)
{
    int i;
    StSnapShot SnapShot;

    memcpy(&SnapShot, snap, sizeof( SnapShot ));
    if ( ! strncmp( SnapShot.Id, "MV - SNA", 8 ) ) {
        int dwSnapSize;
        dwSnapSize = SnapShot.ram_size[0] + (SnapShot.ram_size[1] * 256); // memory dump size
        dwSnapSize &= ~0x3f; // limit to multiples of 64

        memcpy(core->MemCPC, snap+sizeof(SnapShot), dwSnapSize * 1024);

        core->Z80.AF.Word = SnapShot.Z80.AF;
        core->Z80.BC.Word = SnapShot.Z80.BC;
        core->Z80.DE.Word = SnapShot.Z80.DE;
        core->Z80.HL.Word = SnapShot.Z80.HL;
        core->Z80.IR.Word = SnapShot.Z80.IR;
        core->Z80.IFF1 = SnapShot.Z80.IFF1;
        core->Z80.IFF2 = SnapShot.Z80.IFF2;
        core->Z80.IX.Word = SnapShot.Z80.IX;
        core->Z80.IY.Word = SnapShot.Z80.IY;
        core->Z80.SP.Word = SnapShot.Z80.SP;
        core->Z80.PC.Word = SnapShot.Z80.PC;
        core->Z80.InterruptMode = SnapShot.Z80.InterruptMode;
        core->Z80._AF.Word = SnapShot.Z80._AF;
        core->Z80._BC.Word = SnapShot.Z80._BC;
        core->Z80._DE.Word = SnapShot.Z80._DE;
        core->Z80._HL.Word = SnapShot.Z80._HL;

        // PPI
        WritePPI( core, 0xF400, SnapShot.PPI[ 0 ] );
        //         WritePPI( 0xF500, SnapShot.PPI[ 1 ] );
        WritePPI( core, 0xF600, SnapShot.PPI[ 2 ] );
        WritePPI( core, 0xF700, SnapShot.PPI[ 3 ] );

        Reset8912(core);
        for ( i = 0; i < 16; i++ ) {
            Write8912( core, i, SnapShot.PsgData[ i ] );
        }

        core->RegPSGSel = SnapShot.PsgIndex;

        // GA
        for (i=0;i<17;i++) {
            WriteVGA( core, 0, i );
            WriteVGA( core, 0, 0x40 | SnapShot.InkData[ i ] );
        }
        core->PenSelection = SnapShot.InkReg;
        WriteVGA( core, 0, core->PenSelection & 0x1F);

        core->NumRomExt = SnapShot.NumRom;
        WriteVGA( core, 0, 0x80 | SnapShot.VGARom );
        WriteVGA( core, 0, 0xC0 | SnapShot.VGARam );

        // CRTC
        for ( i = 0; i < 18; i++ ) {
            WriteCRTC( core, 0xBC00, i );
            WriteCRTC( core, 0xBD00, SnapShot.CRTCReg[i] );
        }
        core->RegCRTCSel = SnapShot.CRTCIndex;

        core->UpdateInk=1;
    }
}

#endif
