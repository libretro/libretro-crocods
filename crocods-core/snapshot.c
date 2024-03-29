#include  "snapshot.h"

#include  "platform.h"
#include  "sound.h"
#include  "crtc.h"
#include  "ppi.h"
#include  "vga.h"
#include  "z80.h"
#include "z80_cap32.h"

extern t_z80regs z80; // in cap32

#ifdef USE_SNAPSHOT

#pragma pack(1)
typedef struct {
    u16 AF;
    u16 BC;
    u16 DE;
    u16 HL;
    u16 IR;
    UBYTE IFF1;
    UBYTE IFF2;
    u16 IX;
    u16 IY;
    u16 SP;
    u16 PC;
    UBYTE InterruptMode;
    u16 _AF;
    u16 _BC;
    u16 _DE;
    u16 _HL;
} _SRegs;

typedef struct {
    char Id[ 0x10 ];  // "MV - SNA'
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

    // v2

    u8 ram_size[2];         // 6B-6C
    u8 CPCType; // 0 = CPC464, 1 = CPC664, 2 = CPC6128, 3 = unknown, 4 = 6128 Plus, 5 = 464 Plus, 6 = GX4000
    u8 intNumber;
    u8 multimode[6];

    // v3

    u8 drvA_DOSfilename[13];
    u8 drvB_DOSfilename[13];
    u8 cart_DOSfilename[13];
    u8 fdc_motor;
    u8 drvA_current_track;
    u8 drvB_current_track;
    u8 drvC_current_track;
    u8 drvD_current_track;
    u8 printer_data;
    u8 psg_env_step;
    u8 psg_env_direction;
    u8 crtc_type;
    u8 crtc_addr[2];
    u8 crtc_scanline[2];
    u8 crtc_char_count[2];
    u8 crtc_line_count;
    u8 crtc_raster_count;
    u8 crtc_vt_adjust_count;
    u8 crtc_hsw_count;
    u8 crtc_vsw_count;
    u8 crtc_flags[2];
    u8 ga_int_delay;
    u8 ga_sl_count;
    u8 z80_int_pending;

    u8 Unused[ 75];
} StSnapShot;

#pragma pack()

int getSnapshotSize(core_crocods_t *core)
{
    return sizeof(StSnapShot) + 0x20000;
}

char * getSnapshot(core_crocods_t *core, int *len)
{
    char *buffer;

    int i;
    StSnapShot SnapShot;

    strcpy(SnapShot.Id, "MV - SNA");
    SnapShot.Version = 1;

    if (croco_cpu_doFrame == cap32_cpu_doFrame) {
        SnapShot.Z80.AF = z80.AF.w.l;
        SnapShot.Z80.BC = z80.BC.w.l;
        SnapShot.Z80.DE = z80.DE.w.l;
        SnapShot.Z80.HL = z80.HL.w.l;

        reg_pair IR;
        IR.b.h = z80.I;
        IR.b.l = z80.R;
        SnapShot.Z80.IR = IR.w.l;

        SnapShot.Z80.IFF1 = z80.IFF1;
        SnapShot.Z80.IFF2 = z80.IFF2;
        SnapShot.Z80.IX = z80.IX.w.l;
        SnapShot.Z80.IY = z80.IY.w.l;
        SnapShot.Z80.SP = z80.SP.w.l;
        SnapShot.Z80.PC = z80.PC.w.l;
        SnapShot.Z80.InterruptMode = z80.IM;
        SnapShot.Z80._AF = z80.AFx.w.l;
        SnapShot.Z80._BC = z80.BCx.w.l;
        SnapShot.Z80._DE = z80.DEx.w.l;
        SnapShot.Z80._HL = z80.HLx.w.l;
    } else {
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
    }

    SnapShot.InkReg = (UBYTE)core->PenIndex;
    for (i = 0; i < 16; i++) {
        SnapShot.InkData[ i ] = (UBYTE)core->TabCoul[ i ];
    }

    SnapShot.VGARom = (UBYTE)core->DecodeurAdresse;
    SnapShot.VGARam = (UBYTE)core->RamSelect;
    SnapShot.CRTCIndex = (UBYTE)core->CRTC_Reg;
    for (i = 0; i < 18; i++) {
        SnapShot.CRTCReg[ i ] = (UBYTE)core->RegsCRTC[ i ];
    }

    SnapShot.NumRom = (UBYTE)core->NumRomExt;
    for (i = 0; i < 4; i++) {
        SnapShot.PPI[ i ] = (UBYTE)core->RegsPPI[ i ];
    }

    SnapShot.PsgIndex = (UBYTE)core->RegPSGSel;
    for (i = 0; i < 16; i++) {
        SnapShot.PsgData[ i ] = 0; // ( UBYTE )RegsPSG[ i ]; // On ne sauve pas les registres PSG
    }
    SnapShot.ram_size[0] = 128;
    SnapShot.ram_size[1] = 0;

    *len = sizeof(StSnapShot) + 0x20000;

    buffer = (char *)malloc(*len);
    memcpy(buffer, &SnapShot, sizeof(StSnapShot));
    memcpy(buffer + sizeof(StSnapShot), core->MemCPC, 0x20000);

    return buffer;
}

void LireSnapshotMem(core_crocods_t *core, u8 *snap)
{
    int i;
    StSnapShot SnapShot;


    memcpy(&SnapShot, snap, sizeof(SnapShot));
    if (!strncmp(SnapShot.Id, "MV - SNA", 8)) {
        if (SnapShot.Version > 1) {
            appendIcon(core, 0, 4, 60); // TODO: send "Could not read Snapshot"
            return;
        }

        int dwSnapSize;
        dwSnapSize = SnapShot.ram_size[0] + (SnapShot.ram_size[1] * 256); // memory dump size
        dwSnapSize &= ~0x3f; // limit to multiples of 64

        if (dwSnapSize != 0)
            memcpy(core->MemCPC, snap + sizeof(SnapShot), dwSnapSize * 1024);

        if (SnapShot.Version >= 1) {        // Load v1 specific
            if (croco_cpu_doFrame == cap32_cpu_doFrame) {
                z80.AF.w.l = SnapShot.Z80.AF;
                z80.BC.w.l = SnapShot.Z80.BC;
                z80.DE.w.l = SnapShot.Z80.DE;
                z80.HL.w.l = SnapShot.Z80.HL;

                reg_pair IR;
                IR.w.l = SnapShot.Z80.IR;

                z80.I = IR.b.h;
                z80.R = IR.b.l;

                z80.IFF1 = SnapShot.Z80.IFF1;
                z80.IFF2 = SnapShot.Z80.IFF2;
                z80.IX.w.l = SnapShot.Z80.IX;
                z80.IY.w.l = SnapShot.Z80.IY;
                z80.SP.w.l = SnapShot.Z80.SP;
                z80.PC.w.l = SnapShot.Z80.PC;
                z80.IM = SnapShot.Z80.InterruptMode;
                z80.AFx.w.l = SnapShot.Z80._AF;
                z80.BCx.w.l = SnapShot.Z80._BC;
                z80.DEx.w.l = SnapShot.Z80._DE;
                z80.HLx.w.l = SnapShot.Z80._HL;
            } else {
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
            }

            // PPI
            WritePPI(core, 0xF400, SnapShot.PPI[ 0 ]);
            //         WritePPI( 0xF500, SnapShot.PPI[ 1 ] );
            WritePPI(core, 0xF600, SnapShot.PPI[ 2 ]);
            WritePPI(core, 0xF700, SnapShot.PPI[ 3 ]);

            Reset8912(core);
            for (i = 0; i < 16; i++) {
                Write8912(core, i, SnapShot.PsgData[ i ]);
            }

            core->RegPSGSel = SnapShot.PsgIndex;

            // GA
            for (i = 0; i < 17; i++) {
                WriteVGA(core, 0, i);
                WriteVGA(core, 0, 0x40 | SnapShot.InkData[ i ]);
            }
            core->PenSelection = SnapShot.InkReg;
            WriteVGA(core, 0, core->PenSelection & 0x1F);

            core->NumRomExt = SnapShot.NumRom;
            WriteVGA(core, 0, 0x80 | SnapShot.VGARom);
            WriteVGA(core, 0, 0xC0 | SnapShot.VGARam);

            // CRTC
            for (i = 0; i < 18; i++) {
                RegisterSelectCRTC(core, i);
                WriteCRTC(core, SnapShot.CRTCReg[i]);
            }
            core->CRTC_Reg = SnapShot.CRTCIndex;

            core->UpdateInk = 1;
        }
    }
}     /* LireSnapshotMem */

#endif /* ifdef USE_SNAPSHOT */
