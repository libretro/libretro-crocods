/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
 * @author  Ludovic Delplanque <ludovic.deplanque@libertysurf.fr> for the original version pc-cpc v0.1y (05/11/2002)
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
 * Plateform specific
 */

#include "crocods.h"

#include "apps_shared.h"

#ifndef PLATEFORM_H
#define PLATEFORM_H

#ifdef __cplusplus
extern "C"
{
#endif

#undef HACK_IRQ
#undef HACK_LDIR

#ifdef TARGET_OS_MAC
#define HAVE_FS
#endif

#ifdef GCW
#define HAVE_FS
#endif

#include "autotype.h"
#include "crtc.h"
#include "upd.h"
#include "vga.h"
#include "sound.h"
#include "snapshot.h"
#include "asic.h"

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MAX_PATH
#define MAX_PATH       2048
#endif

#define HBLANK_ACTIVE  (1 << 0)
#define VBLANK_ACTIVE  (1 << 1)
#define DISPTMG_ACTIVE (1 << 2)

#define HSYNC_INPUT    (1 << 0)
#define VSYNC_INPUT    (1 << 1)

typedef enum {
    /* CPC+ hardware design */
    /* ASIC combining 8255, CRTC and Gate Array */
    CPC_HW_CPCPLUS,
    /* CPC hardware design */
    /* seperate 8255, CRTC and Gate Array */
    CPC_HW_CPC,
    /* standard CPC464 hardware */
    CPC_HW_CPC464,
    /* standard CPC664 hardware */
    CPC_HW_CPC664,
    /* standard CPC6128 hardware */
    CPC_HW_CPC6128,
    /* cost down CPC464 */
    /* seperate 8255, combined CRTC and Gate Array */
    CPC_HW_CPC464_COST_DOWN,
    /* cost down CPC6128 */
    /* seperate 8255, combined CRTC, PAL and Gate Array */
    CPC_HW_CPC6128_COST_DOWN,
    /* KC Compact hardware design */
    /* Z8536 CIO */
    CPC_HW_KCCOMPACT
} CPC_HW;

typedef struct {
    char ch;
    /* the string; as ascii characters to type */
    char *sString;
    /* current position within the string */
    int nPos;
    /* number of characters remaining to type */
    int nCountRemaining;
    /* number of frames to waste before continuing */
    int nFrames;

    unsigned long nFlags;
} AUTOTYPE;

typedef union {
    USHORT Word;
#if RETRO_IS_LITTLE_ENDIAN
    struct {
        UBYTE Low;
        UBYTE High;
    } Byte;
#elif RETRO_IS_BIG_ENDIAN
    struct {
        UBYTE High;
        UBYTE Low;
    } Byte;
#else
#error Unknown endianness
#endif
} Registre;

typedef struct {
    Registre AF;
    Registre BC;
    Registre DE;
    Registre HL;
    Registre IR;
    UBYTE IFF1;
    UBYTE IFF2;
    Registre IX;
    Registre IY;
    Registre SP;
    Registre PC;
    UBYTE InterruptMode;
    Registre _AF;
    Registre _BC;
    Registre _DE;
    Registre _HL;
} SRegs;

// upd

#pragma pack(1)
typedef struct {
    char debut[0x30];     // "MV - CPCEMU Disk-File\r\nDisk-Info\r\n"
    UBYTE NbTracks;
    UBYTE NbHeads;
    SHORT DataSize;     // 0x1300 = 256 + ( 512 * nbsecteurs )
    UBYTE Unused[0xCC];
} CPCEMUEnt;

typedef struct {
    UBYTE C;            // track
    UBYTE H;            // head
    UBYTE R;            // sect
    UBYTE N;            // size
    UBYTE ST1;          // Valeur ST1
    UBYTE ST2;          // Valeur ST2
    SHORT SectSize;     // Taille du secteur en octets
} CPCEMUSect;           // Description d'un secteur

typedef struct {
    char ID[0x10];     // "Track-Info\r\n"
    UBYTE Track;
    UBYTE Head;
    SHORT Unused;
    UBYTE SectSize;     // 2
    UBYTE NbSect;       // 9
    UBYTE Gap3;    // 0x4E
    UBYTE OctRemp;      // 0xE5
    CPCEMUSect Sect[29];
} CPCEMUTrack;     //  Description d'une piste
                   //
#pragma pack()

typedef int (*pfctUPD)(core_crocods_t *core, int);

typedef void (*pfctDraw)(core_crocods_t *, int, signed int, int);

typedef struct {
    int touchXpx;
    int touchYpx;
    int touchDown;
    u16 keys_pressed;
} IPC;

enum {
    MODE_OFF,
    MODE_WRITE,
    MODE_READ
};

#define SIZE_BUF_TAPE  0x800 // Taille buffer tampon cassette  !!! Doit etre une puissance de 2 !!!

#define SOUND_CHANNELS 6          // On utilise 3 canaux pour les sons, et 3 autres canaux pour les bruits
#define SOUNDBUFCNT    ((u16)(4096)) //367

#define MAX_ROM_EXT    256

typedef void (*kFctVoidU16)(core_crocods_t *core, u16);

typedef struct core_crocods_s {
    char debug_stopped;

    char VDU_frame_completed;     // used by cap32
    int CPC_cycle_count;          // used by cap32

    int keyEmul;     // 2: Emul du clavier, 3: normal

    ASIC_DATA ASIC_Data;
    CPC_HW hw;

    /* Misc */
    int keyboardLayout;     // 0: default, 1:french, 2:spanish
    char turbo;
    u16 last_keys_pressed;     // key pressed used by menu, keyboard, ... (read by main.c)
    char wait_key_released;

    char inMenu;
    char inKeyboard;
    char isPaused;

    char *resources;
    int resources_len;

    char openFilename[MAX_PATH];
    char filename[MAX_PATH];     // Basename of filename

    char *home_dir;
    char *file_dir;

    int resize;     // 1: auto, 2: 320, 3: no resize, 4: overscan

    char currentfile[256];
    int currentsnap;     // 0,1 ou 2
    int snapsave;

    u16 *menubuffer;
    u16 *menubufferlow;

    int consolepos;
    char consolestring[1024];

    pfctDraw DrawFct;

    u16 TabPoints[4][256][4];
    u8 TabPointsDef[4][256][4];

    //        int iconMenuX, iconMenuId;
    struct kmenu *selectedMenu;

    int Fmnbr;

    IPC ipc;
    int *borderX, *borderY;

    // sound

    int RegsPSG[16];              // Registres 0 � 15 du PSG
    int Freq[SOUND_CHANNELS];     // Tableau des fr�quences
    int Vol[SOUND_CHANNELS];      // De 0 � 255 - Tableau des volumes
    int EPeriod;                  // P�riode d'enveloppe
    int ECount;                   // Temps de la derni�re modification de p�riode d'enveloppe
    int Env[3];                   // Enveloppes de volumes
    BOOL WritePSGReg13;           // Indique si une modification du registre 13 a �t� effectu�e
    int EnvType;
    s32 NoiseGen;
    s16 Wave[6][SOUNDBUFCNT];
    int SoundBusy;
    u8 soundEnabled;

    union {
#if RETRO_IS_LITTLE_ENDIAN
        struct {
            unsigned int low;
            unsigned int high;
        };
#elif RETRO_IS_BIG_ENDIAN
        struct {
            unsigned int high;
            unsigned int low;
        };
#else
#error Unknown endianness
#endif
        int64_t both;
    } snd_cycle_count_init;
    union {
#if RETRO_IS_LITTLE_ENDIAN
        struct {
            unsigned int low;
            unsigned int high;
        };
#elif RETRO_IS_BIG_ENDIAN
        struct {
            unsigned int high;
            unsigned int low;
        };
#else
#error Unknown endianness
#endif
        int64_t both;
    } psg_cycle_count;

    // icon
    int iconTimer;
    int iconToDislay;

    // upd

    u8 ImgDsk[1024 * 1024];     // ImgDsk de stockage du fichier image disquette - 1024 au lieu de 512 pour la lecture des D7 3.5
    pfctUPD fct;          // Pointeur de fonction du UPD
    int LongFic;

    int etat;             // Compteur phase fonction UPD
    CPCEMUTrack CurrTrackDatasDSK;     // Image d'une piste
    CPCEMUEnt Infos;      // En-t�te du fichier image
    int FlagWrite;        // Indique si �criture sur disquette effectu�e
    int Image;            // Indique si fichier image charg�
    int PosData;          // Position des donn�es dans l'image
    int DriveBusy;        // TODO: remove the field
    int Status;           //  Status UPD

    int ST0;              // Status interne 0
    int ST1;              // Status interne 1
    int ST2;              // Status interne 2
    int ST3;              // Status interne 3
    int C;                // Cylindre (n� piste)
    int H;                // ead     (n� t�te)
    int R;                // Record   (n� secteur)
    int N;                // Number   (nbre d'octet poids forts)
    int Drive;            // Drive    ( 0=A, 1=B)
    int EOT;              // Secteur final;
    int Busy;             // UPD occup�
    int Inter;            // G�n�ration d'une interruption UPD
    int Moteur;           // Etat moteur UPD
    int IndexSecteur;     // Indique la position du secteur en cour sur la piste courante

    int updRsect, updRcntdata, updRnewPos;
    signed int updRTailleSect;

    int updWsect, updWcntdata, updWnewPos;
    signed int updWTailleSect;

    // mouse

    u8 kempstonMouseX, kempstonMouseY, kempstonMouseButton;

    // ppi

    u8 clav[16];       // Matrice du clavier (16 * 8 bits)
    int modePSG;       // Mode du PSG
    int RegPSGSel;     // Num�ro de registre du PSG s�lectionn�
    u8 KeyboardScanned;

    int RegsPPI[4];     //  Registres inernes du PPI 8255
    int Output[3];      // Sorties du PPI 8255
    int Input[3];       // Entr�es du PPI 8255
    int Mask[3];        // Masques calcul�s pour masquage entr�es/sorties

    int ligneClav;     // Num�ro de ligne du clavier s�lectionn�e

    FILE *fCas;         // Handle fichier lecture ou �criture cassette
    UBYTE BitTapeIn;    // Bit de lecture cassette pour le port F5xx
    int PosBit;         // Position de bit de calcul pour lecture/�criture cassette
    UBYTE OctetCalcul;  // Octet de calcul pour lecture/�criture cassette
    UBYTE BufTape[SIZE_BUF_TAPE];     // Buffer fichier cassette
    int PosBufTape;     // Index sur buffer fichier cassette
    int Mode;           // Mode d'acc�s cassette (lecture, �criture, ou off)

    // CRTC (old)

    int TailleVBL, CptCharVert, NumLigneChar;
    int LigneCRTC, MaCRTC;

    // --- Monitor

    /* the sync inputs to the monitor; vertical and horizontal sync */
    unsigned char MonitorSyncInputs;

    /* active draw position */
    int MonitorScanLineCount;
    int MonitorHorizontalCount;

    int MonitorVCount;
    int MonitorHCount;

    /* if monitor is actively vtracing and for how many scanlines */
    /* lines or cycles?? */
    BOOL MonitorVTraceActive;
    int MonitorVTraceCount;

    /* if monitor is actively htracing and for how many scanlines */
    BOOL MonitorHTraceActive;
    int MonitorHTraceCount;

    /* number of scan-lines since end of VSYNC */
    int LinesAfterVTrace;     // a effacer (todo)
    //	int CharsAfterVSync;

    /* number of chars since end of hsync */
    //	int CharsAfterHSync;
    int CharsAfterHTrace;     // a effacer (todo)

    // --- New CRTC

    unsigned long CRTC_Flags;
    unsigned long CRTC_HalfHtotFlags;
    unsigned long CRTC_FlagsAtLastHsync;
    unsigned long CRTC_FlagsAtLastHtot;
    /* horizontal count */
    unsigned char HCount;
    /* start and end of line in char positions */
    unsigned char HStart, HEnd;
    /* Horizontal sync width */
    unsigned char HorizontalSyncWidth;
    /* horizontal sync width counter */
    unsigned char HorizontalSyncCount;

    /* raster counter (RA) */
    unsigned char RasterCounter;
    /* line counter */
    unsigned char LineCounter;
    /* Vertical sync width */
    unsigned char VerticalSyncWidth;
    unsigned char VerticalSyncWidthCount;
    /* vertical sync width counter */
    unsigned char VerticalSyncCount;

    /* INTERLACE STUFF */
    /* interlace and video mode number 0,1,2,3 */
    // unsigned char InterlaceAndVideoMode;
    /* frame - odd or even - used in interlace */
    unsigned char Frame;
    /* Vert Adjust Counter */
    unsigned char VertAdjustCount;
    /* delay for start and end of line defined by reg 8 */
    unsigned char HDelayReg8;

    /* type index of CRTC */
    unsigned char CRTC_Type;
    /* index of current register selected */
    unsigned char CRTC_Reg;
    /*unsigned short HDispAdd; */

    /* MA (memory address base) */
    int MA;     /* current value */
    /* MA of current line we are rendering (character line) */
    int MAStore;     /* this is the reload value */

    int CursorBlinkCount;      /* current flash count */
    int CursorBlinkOutput;     /* current output from flash */
    int CursorActiveLine;      /* cursor is active on this line */
    int CursorOutput;          /* final output */

    int CursorMA;
    /* line function */
    int LinesAfterFrameStart;
    int CharsAfterHsyncStart;
    int LinesAfterVsyncStart;

    // --- CRTC commun

    int XStart;     // Position d�but (en octets) de l'affichage vid�o sur une ligne
    int XEnd;       // Position fin   (en octets) de l'affichage vid�o sur une ligne
    char changeFilter;
    int crtc_updated;
    u16 RegsCRTC[32];     //  Registres du CRTC

    int CntHSync;     // Compteur position horizontale raster (GA ?)
    int SyncCount;

    // Audio Specific

    unsigned int buffer_size;
    unsigned int sample_rate;
    unsigned int audio_position;
    char audio_stream_started;     // detects first copy request to minimize lag
    char audio_copy_in_progress;
    u16 *audio_buffer;

    // AutoType

    AUTOTYPE AutoType;

    // VGA
    UBYTE *MemCPC;      // CPC Memory
    u8 TabCoul[32];     // Couleurs R,V,B

    u8 *TabPOKE[4];
    u8 *TabPEEK[4];
    int RamSelect;     // Num�ro de ram s�lectionn�e (128K)
    int Bloc;
    int lastMode;            // Mode �cran s�lectionn�
    int DecodeurAdresse;     // ROMs activ�es/d�sactiv�es
    int NumRomExt;           //  Num�ro de rom sup�rieure s�lectionn�e
    int PenIndex;            // Num�ro de stylot s�lectionn�
    int PenSelection;
    int ColourSelection;

    int DoResync;     // Indique qu'il faut synchroniser l'affichage  (a effacer)

    unsigned long BlankingOutput;     // Arnold
    unsigned long CRTCSyncInputs;     // Arnold

    int nHBlankCycle;
    int nVBlankCycle;

    UBYTE ROMINF[0x4000];                  // Rom inf�rieure
    UBYTE ROMEXT[MAX_ROM_EXT][0x4000];     // Rom disque (Amsdos)

    // Z80
    int IRQ;
    SRegs Z80;

#ifdef HACK_IRQ
    int halted;
#endif

    // inMenu

    u16 *icons;
    u16 *icons8;
    u16 *keyboard;
    u16 *tape;
    u16 *select;
    u16 *menu;

    // Private

    int lastcolour;
    int usestylus, usestylusauto;
    int usemagnum;

    u16 BG_PALETTE[32];     // From GA

    int hack_tabcoul;
    int UpdateInk;     //      GA: Update palette

    int x0, y0;
    int maxy;

    int screenBufferWidth;
    int screenBufferHeight;
    u8 screenIsOptimized;     // Don't always use the full 768x272 bitmap
    u8 scanline;

    int Regs1, Regs2, Regs6, Regs7;     // Used by the auto resize
    u16 MemBitmap[384 * 288 * 2];       // Max screen size

    u16 MemBitmap_width;
    
    u8 printer_port;

    u16 *overlayBitmap;
    u16 overlayBitmap_width, overlayBitmap_height;
    u16 overlayBitmap_posx, overlayBitmap_posy;
    char overlayBitmap_center;
    int dispframerate;
    int framecount;

    char runStartApp;
    char runApp[258];
    char runParam[3][258];

    int redefineKey;
    int redefineKeyKey;
    int redefineKeyRetour;

    kFctVoidU16 runApplication;

    char debug;
    char debugOneStep;
} core_crocods_t;

struct kmenu {
    struct kmenu *parent;
    char title[256];
    char *object;
    int nbr;
    int id;
    struct kmenu *firstchild, *lastchild;
    struct kmenu *nextSibling, *previousSibling;
    int beg, pos;
    int x, y;
};

void Erreur(char *Msg);
void Info(char *Msg);
void InitPlateforme(core_crocods_t *core, u16 screen_width);
void updateScreenBuffer(core_crocods_t *core, u16 *screen, u16 screen_width);

void LoadROMFile(char *filename);

typedef void (*kFctVoidVoid)(core_crocods_t *core);
typedef void (*kFctVoidU8)(core_crocods_t *core, u8);
typedef void (*kFctVoidU32)(core_crocods_t *core, u32);
typedef u8 (*kFctU8Void)(core_crocods_t *core);
typedef u16 (*kFctU16Void)(core_crocods_t *core);

extern kFctU16Void croco_cpu_doFrame;
extern kFctU16Void ExecInstZ80;
extern kFctVoidVoid ResetZ80;
extern kFctVoidU8 SetIRQZ80;

extern kFctU8Void ReadCRTC;
extern kFctVoidU8 WriteCRTC;
extern kFctVoidU8 RegisterSelectCRTC;
extern kFctVoidU32 CRTC_DoCycles;
extern kFctU8Void CRTC_DoLine;

extern kFctVoidVoid ResetCRTC;

extern kFctVoidVoid GateArray_Cycle;
extern kFctVoidVoid ResetVGA;

void UpdateScreen(core_crocods_t *core);
int TraceLigneCrit(void);

void SetPalette(core_crocods_t *core, int color);
void RedefineKey(core_crocods_t *core, int key);
void UpdateTitlePalette(struct kmenu *current);
int ExecuteMenu(core_crocods_t *core, int n, struct kmenu *current);
void InitCalcPoints(core_crocods_t *core);

int emulator_patch_ROM(core_crocods_t *core, u8 *pbROMlo);

int nds_ReadKey(core_crocods_t *core);

void nds_init(core_crocods_t *core);
void nds_initBorder(core_crocods_t *core, int *borderX, int *borderY);

void LoopMenu(core_crocods_t *core, struct kmenu *parent);

int nds_MapRGB(int r, int g, int b);

void LireRom(struct kmenu *FM, int autostart);

#define MOD_CPC_SHIFT (0x01 << 8)
#define MOD_CPC_CTRL  (0x02 << 8)
#define MOD_EMU_KEY   (0x10 << 8)

#define NBCPCSCANCODE 81

typedef struct {
    char *value;
} keyname;

typedef enum {
    /* line 0, bit 0..bit 7 */
    CPC_CURSOR_UP,     // = 0
    CPC_CURSOR_RIGHT,
    CPC_CURSOR_DOWN,
    CPC_F9,
    CPC_F6,
    CPC_F3,
    CPC_SMALL_ENTER,
    CPC_FDOT,
    /* line 1, bit 0..bit 7 */
    CPC_CURSOR_LEFT,
    CPC_COPY,
    CPC_F7,
    CPC_F8,
    CPC_F5,
    CPC_F1,
    CPC_F2,
    CPC_F0,
    /* line 2, bit 0..bit 7 */
    CPC_CLR,
    CPC_OPEN_SQUARE_BRACKET,
    CPC_RETURN,
    CPC_CLOSE_SQUARE_BRACKET,
    CPC_F4,
    CPC_SHIFT,
    CPC_FORWARD_SLASH,
    CPC_CONTROL,
    /* line 3, bit 0.. bit 7 */
    CPC_HAT,
    CPC_MINUS,
    CPC_AT,
    CPC_P,
    CPC_SEMICOLON,
    CPC_COLON,
    CPC_BACKSLASH,
    CPC_DOT,
    /* line 4, bit 0..bit 7 */
    CPC_ZERO,
    CPC_9,
    CPC_O,
    CPC_I,
    CPC_L,
    CPC_K,
    CPC_M,
    CPC_COMMA,
    /* line 5, bit 0..bit 7 */
    CPC_8,
    CPC_7,
    CPC_U,
    CPC_Y,
    CPC_H,
    CPC_J,
    CPC_N,
    CPC_SPACE,
    /* line 6, bit 0..bit 7 */
    CPC_6,
    CPC_5,
    CPC_R,
    CPC_T,
    CPC_G,
    CPC_F,
    CPC_B,
    CPC_V,
    /* line 7, bit 0.. bit 7 */
    CPC_4,
    CPC_3,
    CPC_E,
    CPC_W,
    CPC_S,
    CPC_D,
    CPC_C,
    CPC_X,
    /* line 8, bit 0.. bit 7 */
    CPC_1,
    CPC_2,
    CPC_ESC,
    CPC_Q,
    CPC_TAB,
    CPC_A,
    CPC_CAPS_LOCK,
    CPC_Z,
    /* line 9, bit 7..bit 0 */
    CPC_JOY_UP,
    CPC_JOY_DOWN,
    CPC_JOY_LEFT,
    CPC_JOY_RIGHT,
    CPC_JOY_FIRE1,
    CPC_JOY_FIRE2,
    CPC_SPARE,
    CPC_DEL,

    /* no key press */
    CPC_NIL
} CPC_SCANCODE;

extern CPC_SCANCODE keyown[13];

#define NBCPCKEY 74
typedef int CPC_KEY;

void CPC_SetScanCode(core_crocods_t *core, CPC_SCANCODE cpc_scancode);
void CPC_ClearScanCode(core_crocods_t *core, CPC_SCANCODE cpc_scancode);

void SoftResetCPC(core_crocods_t *core); // Reset CPC without modifiy ROM
void HardResetCPC(core_crocods_t *core); // Reset CPC AND modifiy ROM

void CalcPoints(core_crocods_t *core);

void drawconsole(void);

void cpcprint16_6w(core_crocods_t *core, u16 *MemBitmap, u32 MemBitmap_width, int x, int y, char *pchStr, u16 bColor, u16 backgroundColor, int multi, char transparent);
void cpcprint16_6w_limit(core_crocods_t *core, u16 *MemBitmap, u32 MemBitmap_width, int x, int y, char *pchStr, u16 bColor, u16 backgroundColor, int multi, char transparent, int miny, int maxy);
void cpcprint16(core_crocods_t *core, u16 *MemBitmap, u32 MemBitmap_width, int x, int y, char *pchStr, u16 bColor, u16 backgroundColor, int multi, char transparent);
void cpcprint(core_crocods_t *core, int x, int y, char *pchStr, u16 Colour);
void cpcprintOnScreen(core_crocods_t *core, char *pchStr);

u16 MyReadKey(void);

void ExecZ80Code(core_crocods_t *core, char *code, int len, SRegs *result);     // z80.h

typedef struct {
    int left, top, right, bottom;
} RECT;

void DispDisk(core_crocods_t *core, int reading);
void Autoexec(core_crocods_t *core);

void dispIcon(core_crocods_t *core, int i, int j, int dispiconX, int dispiconY, char select);
void dispIcon8(core_crocods_t *core, int i, int j, int icon);

u8 * MyAlloc(int size, char *title);
//
// Tailles affichage �cran suivant la r�solution voulue
//

#define TAILLE_X_LOW  384
#define TAILLE_Y_LOW  272

#define TAILLE_X_HIGH 768
#define TAILLE_Y_HIGH 544

#ifdef __cplusplus
}
#endif

typedef enum {
    ID_NULL,
    ID_ROM,
    ID_MONITOR_MENU,
    ID_COLOR_MONITOR,
    ID_GREEN_MONITOR,
    ID_SCREEN_MENU,
    ID_SCREEN_320,
    ID_SCREEN_NORESIZE,
    ID_SCREEN_OVERSCAN,
    ID_KEY_MENU,
    ID_KEY_KEYBOARD,
    ID_KEY_KEYPAD,
    ID_KEY_JOYSTICK,
    ID_DISPFRAMERATE,
    ID_NODISPFRAMERATE,
    ID_RESET,
    ID_SAVESNAP,
    ID_AUTODISK,
    ID_DISK,
    ID_REDEFINE_UP,
    ID_REDEFINE_DOWN,
    ID_REDEFINE_LEFT,
    ID_REDEFINE_RIGHT,
    ID_REDEFINE_A,
    ID_REDEFINE_B,
    ID_REDEFINE_X,
    ID_REDEFINE_Y,
    ID_REDEFINE_L,
    ID_REDEFINE_R,
    ID_REDEFINE_START,
    ID_SCREEN_AUTO,
    ID_HACK_TABCOUL,
    ID_SWITCH_MONITOR,
    ID_ACTIVE_STYLUS,
    ID_DEACTIVE_STYLUS,
    ID_ACTIVE_MAGNUM,
    ID_MENU_ENTER,
    ID_MENU_EXIT,
    ID_PAUSE_ENTER,
    ID_PAUSE_EXIT,
    ID_NOHACK_TABCOUL,
    ID_DEBUG_ENTER,
    ID_DEBUG_EXIT,
    ID_USE_CRTC_WINCPC,
    ID_USE_CRTC_ARNOLD,

    ID_EXIT,
    ID_DEBUG_MENU,
    ID_SNAP_MENU,
    ID_LOADSNAP,
    ID_DISK_MENU,
    ID_SAVE_SETTINGS,
    ID_SAVE_LOCALSETTINGS,

    ID_PLAY_TAPE,
    ID_AUTORUN,
    ID_INSERTDISK,
    ID_USE_CRTC_CAP32,

    ID_SOUND_ENABLE,
    ID_SOUND_DISABLE,

    ID_SHOW_VIRTUALKEYBOARD,
    ID_SHOW_DEBUGGER,
    ID_SHOW_INFOPANEL,

    ID_NO_SCANLINE,
    ID_SCANLINE_5,
    ID_SCANLINE_10,
    ID_SCANLINE_15,
    ID_SCANLINE_20,
    ID_SCANLINEMENU,

    ID_SHOW_GUESTINFO,

    ID_REDEFINE_L2,
    ID_REDEFINE_R2,

    ID_ENABLE_TURBO,
    ID_DISABLE_TURBO,
    ID_TURBOMENU,

    ID_SOUNDMENU,

    ID_SHOW_BROWSER,
    ID_FRAMERATEMENU,

    ID_MENU,
    ID_SCREEN_NEXT,
    ID_COLORMONITOR_MENU,
    ID_REDEFINE_MENU,
    ID_ADVANCED_MENU,
    ID_HACKMENU,
    ID_ACTIONMENU,
    ID_DEVMENU
} PLATEFORM_MENU;

//#define ID_NULL                 0
//#define ID_ROM                  1
//#define ID_MONITOR_MENU         2
//#define ID_COLOR_MONITOR        3
//#define ID_GREEN_MONITOR        4
//#define ID_SCREEN_MENU          5
//#define ID_SCREEN_320           6
//#define ID_SCREEN_NORESIZE      7
//#define ID_SCREEN_OVERSCAN      8
//#define ID_KEY_MENU             9
//#define ID_KEY_KEYBOARD         10
//#define ID_KEY_KEYPAD           11
//#define ID_KEY_JOYSTICK         12
//#define ID_DISPFRAMERATE        13
//#define ID_NODISPFRAMERATE      14
//#define ID_RESET                15
//#define ID_SAVESNAP             16
//#define ID_AUTODISK             17
//#define ID_DISK                 18
//#define ID_REDEFINE_UP          20
//#define ID_REDEFINE_DOWN        21
//#define ID_REDEFINE_LEFT        22
//#define ID_REDEFINE_RIGHT       23
//#define ID_REDEFINE_A           24
//#define ID_REDEFINE_B           25
//#define ID_REDEFINE_X           26
//#define ID_REDEFINE_Y           27
//#define ID_REDEFINE_L           28
//#define ID_REDEFINE_R           29
//#define ID_REDEFINE_START       30
//#define ID_SCREEN_AUTO          31
//#define ID_HACK_TABCOUL         32
//#define ID_SWITCH_MONITOR       33
//#define ID_ACTIVE_STYLUS        34
//#define ID_DEACTIVE_STYLUS      35
//#define ID_ACTIVE_MAGNUM        36
//#define ID_MENU_ENTER           37
//#define ID_MENU_EXIT            38
//#define ID_PAUSE_ENTER          39
//#define ID_PAUSE_EXIT           40
//#define ID_NOHACK_TABCOUL       41
//#define ID_DEBUG_ENTER          42
//#define ID_DEBUG_EXIT           43
//#define ID_USE_CRTC_WINCPC      44
//#define ID_USE_CRTC_ARNOLD      45
//#define ID_EXIT                 46
//#define ID_DEBUG_MENU           47
//#define ID_SNAP_MENU            48
//#define ID_LOADSNAP             49
//#define ID_DISK_MENU            50
//#define ID_SAVE_SETTINGS        51
//#define ID_SAVE_LOCALSETTINGS   52
//#define ID_PLAY_TAPE            53
//#define ID_AUTORUN              54
//#define ID_INSERTDISK           55
//#define ID_USE_CRTC_CAP32       56
//#define ID_SOUND_ENABLE         57
//#define ID_SOUND_DISABLE        58
//#define ID_SHOW_VIRTUALKEYBOARD 59
//#define ID_SHOW_DEBUGGER        60
//#define ID_SHOW_INFOPANEL       61
//#define ID_NO_SCANLINE          62
//#define ID_SCANLINE_5           63
//#define ID_SCANLINE_10          64
//#define ID_SCANLINE_15          65
//#define ID_SCANLINE_20          66
//#define ID_SCANLINEMENU         67
//#define ID_SHOW_GUESTINFO       68
//#define ID_REDEFINE_L2          69
//#define ID_REDEFINE_R2          70
//#define ID_ENABLE_TURBO         71
//#define ID_DISABLE_TURBO        72
//#define ID_TURBOMENU            73
//#define ID_SOUNDMENU            74
//#define ID_SHOW_BROWSER         75
//#define ID_FRAMERATEMENU        76
//#define ID_MENU                 77
//#define ID_SCREEN_NEXT          78
//#define ID_COLORMONITOR_MENU 79
//#define ID_REDEFINE_MENU  80
//#define ID_ADVANCED_MENU  81
//#define ID_HACKMENU 82
//#define ID_ACTIONMENU 83
//#define ID_DEVMENU 84

u8 * FS_Readfile(char *filename, u32 *romsize);

void appendIcon(core_crocods_t *core, int x, int y, int timer);

void loadIni(core_crocods_t *core, int local);
void saveIni(core_crocods_t *core, int local);

u32 getTicks(void);

#endif // ifndef PLATEFORM_H
