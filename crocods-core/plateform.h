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

#include  "crocods.h"


#ifndef PLATEFORM_H
#define PLATEFORM_H



#ifdef __cplusplus
extern "C" {
#endif

#undef HACK_IRQ
#undef HACK_IR
#undef HACK_LDIR



#include "config.h"


#include "autotype.h"
#include "crtc.h"
#include "upd.h"
#include "vga.h"
#include "sound.h"
#include "snapshot.h"
#include "asic.h"

#ifndef min
#define min(a,b) (((a)<(b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a)>(b)) ? (a) : (b))
#endif

typedef enum 
{
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

typedef struct
{
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
}  AUTOTYPE;

typedef union
{
    USHORT Word;
    struct
    {
        UBYTE Low;
        UBYTE High;
    } Byte;
} Registre;

typedef struct
{
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
typedef struct
{
    char debut[ 0x30 ];  // "MV - CPCEMU Disk-File\r\nDisk-Info\r\n"
    UBYTE NbTracks;
    UBYTE NbHeads;
    SHORT DataSize; // 0x1300 = 256 + ( 512 * nbsecteurs )
    UBYTE Unused[ 0xCC ];
} CPCEMUEnt;

typedef struct
{
    UBYTE C;                // track
    UBYTE H;                // head
    UBYTE R;                // sect
    UBYTE N;                // size
    UBYTE ST1;              // Valeur ST1
    UBYTE ST2;              // Valeur ST2
    SHORT SectSize;         // Taille du secteur en octets
} CPCEMUSect;       // Description d'un secteur

typedef struct
{
    char ID[ 0x10 ];          // "Track-Info\r\n"
    UBYTE Track;
    UBYTE Head;
    SHORT Unused;
    UBYTE SectSize;       // 2
    UBYTE NbSect;         // 9
    UBYTE Gap3;           // 0x4E
    UBYTE OctRemp;        // 0xE5
    CPCEMUSect Sect[ 29 ];
} CPCEMUTrack;      //  Description d'une piste
                    //
#pragma pack()

typedef int ( * pfctUPD )( core_crocods_t *core, int );

typedef void (* pfctDraw)(core_crocods_t *, int, signed int, int );

typedef struct {
    int touchXpx;
    int touchYpx;
    int touchDown;
    u16 keys_pressed;
} IPC;

enum { MODE_OFF, MODE_WRITE, MODE_READ };

#define SIZE_BUF_TAPE   0x800   // Taille buffer tampon cassette  !!! Doit etre une puissance de 2 !!!

#define SOUND_CHANNELS  6           // On utilise 3 canaux pour les sons, et 3 autres canaux pour les bruits
#define SOUNDBUFCNT ((u16)(4096))   //367

#define MAX_ROM_EXT 256

typedef struct core_crocods_s {

    char debug_stopped;

    int keyEmul; // 2: Emul du clavier, 3: normal

    ASIC_DATA ASIC_Data;
    CPC_HW hw;

    /* Misc */
    int keyboardLayout;  // 0: default, 1:french, 2:spanish
    char turbo;
    char turbo_dont_skip;
    char disable_rendering;
    char vblank_just_occured; // For slow operations involving syscalls; these should only run once per vblank

    char inMenu;
    char isPaused;
    char openFilename[1024];

    int resize; // 1: auto, 2: 320, 3: no resize, 4: overscan

    char mustLeave;     // When main loop have to leave.

    char currentfile[256];
    int currentsnap; // 0,1 ou 2
    int snapsave;

    u16 *menubuffer;
    u16 *menubufferlow;

    int consolepos;
    char consolestring[1024];

    pfctDraw DrawFct;

    u16 TabPoints[ 4 ][ 256 ][ 4 ];
    u8 TabPointsDef[ 4 ][ 256 ][ 4 ];

//        int iconMenuX, iconMenuId;
    struct kmenu *selectedMenu;


    int Fmnbr;

    IPC ipc;
    int *borderX, *borderY;

    // sound

    int RegsPSG[ 16 ];  // Registres 0 à 15 du PSG
    int Freq[ SOUND_CHANNELS ]; // Tableau des fréquences
    int Vol[ SOUND_CHANNELS ];  // De 0 ˆ 255 - Tableau des volumes
    int EPeriod;  // Période d'enveloppe
    int ECount;  // Temps de la dernière modification de période d'enveloppe
    int Env[ 3 ];  // Enveloppes de volumes
    BOOL WritePSGReg13;  // Indique si une modification du registre 13 a été effectuée
    int EnvType;
    s32 NoiseGen;
    s16 Wave[6][SOUNDBUFCNT];
    int SoundBusy;


    // upd

    u8 ImgDsk[1024 * 1024]; // ImgDsk de stockage du fichier image disquette - 1024 au lieu de 512 pour la lecture des D7 3.5
    pfctUPD fct; // Pointeur de fonction du UPD
    int LongFic;

    int etat;    // Compteur phase fonction UPD
    CPCEMUTrack CurrTrackDatasDSK;   // Image d'une piste
    CPCEMUEnt Infos; // En-tête du fichier image
    int FlagWrite;   // Indique si écriture sur disquette effectuée
    int Image;   // Indique si fichier image chargé
    int PosData; // Position des données dans l'image
    int DriveBusy;
    int Status;  //  Status UPD

    int ST0; // Status interne 0
    int ST1;  // Status interne 1
    int ST2;  // Status interne 2
    int ST3;  // Status interne 3
    int C;   // Cylindre (nø piste)
    int H;   // ead     (nø tête)
    int R;  // Record   (nø secteur)
    int N;  // Number   (nbre d'octet poids forts)
    int Drive;   // Drive    ( 0=A, 1=B)
    int EOT; // Secteur final;
    int Busy;  // UPD occupé
    int Inter;   // Génération d'une interruption UPD
    int Moteur;  // Etat moteur UPD
    int IndexSecteur;   // Indique la position du secteur en cour sur la piste courante

    int updRsect, updRcntdata, updRnewPos;
    signed int updRTailleSect;

    int updWsect, updWcntdata, updWnewPos;
    signed int updWTailleSect;

    // mouse

    u8 kempstonMouseX, kempstonMouseY, kempstonMouseButton;

    // ppi

    u8 clav[ 16 ];  // Matrice du clavier (16 * 8 bits)
    int modePSG;  // Mode du PSG
    int RegPSGSel;  // Numéro de registre du PSG sélectionné
    u8 KeyboardScanned;

    int RegsPPI[ 4 ];  //  Registres inernes du PPI 8255
    int Output[ 3 ];  // Sorties du PPI 8255
    int Input[ 3 ];  // Entrées du PPI 8255
    int Mask[ 3 ];  // Masques calculés pour masquage entrées/sorties

    int ligneClav;  // Numéro de ligne du clavier sélectionnée


    FILE * fCas;   // Handle fichier lecture ou écriture cassette
    UBYTE BitTapeIn;  // Bit de lecture cassette pour le port F5xx
    int PosBit;   // Position de bit de calcul pour lecture/écriture cassette
    UBYTE OctetCalcul;    // Octet de calcul pour lecture/écriture cassette
    UBYTE BufTape[ SIZE_BUF_TAPE ];  // Buffer fichier cassette
    int PosBufTape;  // Index sur buffer fichier cassette
    int Mode;   // Mode d'accès cassette (lecture, écriture, ou off)

    // CRTC

    int RegsCRTC[ 32 ];     //  Registres du CRTC
    int OfsEcr;     // ??? A effacer ?
    int RegCRTCSel; //   Numéro du registre CRTC sélectionné
    int VSync;          // Signal VSync
    int CntHSync;   // Compteur position horizontale raster
    int XStart;   // Position début (en octets) de l'affichage vidéo sur une ligne
    int XEnd;       // Position fin (en octets) de l'affichage vidéo sur une ligne
    int DoResync;   //  Indique qu'il faut synchroniser l'affichage
    char changeFilter;
    USHORT TabAdrCrtc[ 0x10000 ];

    int TailleVBL, CptCharVert, NumLigneChar;
    int LigneCRTC, MaCRTC, SyncCount;

    // Audio Specific

    unsigned int buffer_size;
    unsigned int sample_rate;
    unsigned int audio_position;
    char audio_stream_started; // detects first copy request to minimize lag
    char audio_copy_in_progress;
    u16 *audio_buffer;

    // AutoType

    AUTOTYPE AutoType;

    // VGA
    UBYTE *MemCPC;              // CPC Memory
    u8 TabCoul[ 32 ];           // Couleurs R,V,B

    u8 *TabPOKE[4];
    u8 *TabPEEK[4];
    int RamSelect;  // Numéro de ram sélectionnée (128K)
    int Bloc;
    int lastMode;   // Mode écran sélectionné
    int DecodeurAdresse;    // ROMs activées/désactivées
    int NumRomExt; //  Numéro de rom supérieure sélectionnée
    int PenIndex;  // Numéro de stylot sélectionné
    int PenSelection;
    int ColourSelection;

    UBYTE ROMINF[ 0x4000 ];  // Rom inférieure
//        UBYTE ROMSUP[ 0x4000 ];  //  Rom supérieure
//        UBYTE ROMDISC[ 0x4000 ];  // Rom disque (Amsdos)
    UBYTE ROMEXT[MAX_ROM_EXT][ 0x4000 ];  // Rom disque (Amsdos)

    // Z80
    int IRQ;
    SRegs Z80;

#ifdef HACK_IRQ
    int halted;
#endif

    // inMenu

    u16 *icons;

    // Private

    int lastcolour;
    int usestylus, usestylusauto;
    int usemagnum;

    u16 BG_PALETTE[32];

    int hack_tabcoul;
    int UpdateInk;

    int x0,y0;
    int maxy;

    int screenBufferWidth;
    int screenBufferHeight;

    int Regs1, Regs2, Regs6, Regs7;       // Utilisé par le resize automatique
    u16 *MemBitmap;
    u16 MemBitmap_width;

    int dispframerate;

    char runStartApp;
    char runApp[258];
    char runParam[3][258];

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
    int beg,pos;
    int x,y;
};

struct kmenu *AddMenu(struct kmenu *parent, char *title, int id, int x, int y);


void Erreur( char * Msg );
void Info( char * Msg );
void InitPlateforme( core_crocods_t *core, u16 *screen, u16 screen_width );
void updateScreenBuffer( core_crocods_t *core, u16 *screen, u16 screen_width );

void LoadROMFile(char *filename);

typedef int (* pfctExecInstZ80)(core_crocods_t *core);
typedef void (* pfctResetZ80)(core_crocods_t *core);
typedef void (* pfctSetIRQZ80)(core_crocods_t *core, int);

extern pfctExecInstZ80 ExecInstZ80;
extern pfctResetZ80 ResetZ80;
extern pfctSetIRQZ80 SetIRQZ80;



void UpdateScreen( core_crocods_t *core );
int TraceLigneCrit( void );

void SetPalette(core_crocods_t *core, int color);
void RedefineKey(int key);
void UpdateTitlePalette(struct kmenu *current);
int ExecuteMenu(core_crocods_t *core, int n, struct kmenu *current);
void InitCalcPoints( core_crocods_t *core );

void videoinit(void);

int emulator_patch_ROM (core_crocods_t *core, u8 *pbROMlo);
void DispIcons(core_crocods_t *core);

int nds_ReadKey(core_crocods_t *core);

void nds_init(core_crocods_t *core);
void nds_initBorder(core_crocods_t *core,int *borderX, int *borderY);


int nds_video_unlock(void);
int nds_video_lock(void);
void nds_video_close(void);

void LoopMenu(core_crocods_t *core, struct kmenu *parent);


int nds_MapRGB(int r, int g, int b);

void LireRom(struct kmenu *FM, int autostart);



#define MOD_CPC_SHIFT   (0x01 << 8)
#define MOD_CPC_CTRL    (0x02 << 8)
#define MOD_EMU_KEY     (0x10 << 8)

#define NBCPCSCANCODE 81

typedef struct {
    char *value;
} keyname;

typedef enum {
    /* line 0, bit 0..bit 7 */
    CPC_CURSOR_UP, // = 0
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



extern CPC_SCANCODE keyown[11];


#define NBCPCKEY 74
typedef int CPC_KEY;

void CPC_SetScanCode(core_crocods_t *core, CPC_SCANCODE cpc_scancode);
void CPC_ClearScanCode(core_crocods_t *core, CPC_SCANCODE cpc_scancode);

void ResetCPC(core_crocods_t *core);

void CalcPoints(core_crocods_t *core);

void myprintf(const char *fmt, ...);
void myprintf0(core_crocods_t *core, const char *fmt, ...);
void drawconsole(void);

void cpcprint16(u16 *MemBitmap, u32 MemBitmap_width, int x, int y, char *pchStr, u16 bColor, int multi, char transparent);
void cpcprint(core_crocods_t *core, int x, int y, char *pchStr, u16 Colour);

u16 MyReadKey(void);


void ExecZ80Code(core_crocods_t *core, char *code, int len, SRegs *result);     // z80.h

typedef struct {
    int left,top,right,bottom;
} RECT;


void DispDisk(core_crocods_t *core, int reading);
void Autoexec(core_crocods_t *core);

void dispIcon(core_crocods_t *core, int i, int j, int dispiconX, int dispiconY, char select);


u8 *MyAlloc(int size, char *title);
//
// Tailles affichage ‚cran suivant la r‚solution voulue
//

#define     TAILLE_X_LOW    384
#define     TAILLE_Y_LOW    272

#define     TAILLE_X_HIGH   768
#define     TAILLE_Y_HIGH   544

#ifdef __cplusplus
}
#endif


#define ID_MENU 0
#define ID_ROM 10000000
#define ID_MONITOR_MENU 2
#define ID_COLOR_MONITOR 3
#define ID_GREEN_MONITOR 4
#define ID_SCREEN_MENU 5
#define ID_SCREEN_320 6
#define ID_SCREEN_NORESIZE 7
#define ID_SCREEN_OVERSCAN 8
#define ID_KEY_MENU 9
#define ID_KEY_KEYBOARD 10
#define ID_KEY_KEYPAD 11
#define ID_KEY_JOYSTICK 12
#define ID_DISPFRAMERATE 13
#define ID_NODISPFRAMERATE 14
#define ID_RESET 15
#define ID_SAVESNAP 16
#define ID_FILE 17
#define ID_DISK 18
#define ID_REDEFINE_UP 20
#define ID_REDEFINE_DOWN 21
#define ID_REDEFINE_LEFT 22
#define ID_REDEFINE_RIGHT 23
#define ID_REDEFINE_A 24
#define ID_REDEFINE_B 25
#define ID_REDEFINE_X 26
#define ID_REDEFINE_Y 27
#define ID_REDEFINE_L 28
#define ID_REDEFINE_R 29
#define ID_REDEFINE_START 30
#define ID_SCREEN_AUTO 31
#define ID_HACK_TABCOUL 32
#define ID_SWITCH_MONITOR 33
#define ID_ACTIVE_STYLUS 34
#define ID_DEACTIVE_STYLUS 35
#define ID_ACTIVE_MAGNUM 36
#define ID_MENU_ENTER 37
#define ID_MENU_EXIT 38
#define ID_PAUSE_ENTER 39
#define ID_PAUSE_EXIT 40
#define ID_NOHACK_TABCOUL 41
#define ID_DEBUG_ENTER 42
#define ID_DEBUG_EXIT 43

#endif
