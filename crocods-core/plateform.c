#include "plateform.h"


#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "autotype.h"
#include "snapshot.h"
#include "config.h"
#include "sound.h"

#include "z80.h"




#include "ppi.h"
#include "vga.h"
#include "upd.h"
#include "crtc.h"

#include "gif.h"


#include "cpcfont.h"

extern const unsigned char icons_gif[];
extern unsigned int icons_gif_len;


#define timers2ms(tlow,thigh)(tlow | (thigh<<16)) >> 5

#define AlphaBlendFast(pixel,backpixel) (((((pixel) & 0x7bde) >> 1) | (((backpixel) & 0x7bde) >> 1)) | 0x8000)

//static u16 AlphaBlend(u16 pixel, u16 backpixel, u16 opacity)
//{
//  // Blend image with background, based on opacity
//  // Code optimization from http://www.gamedev.net/reference/articles/article817.asp
//  // result = destPixel + ((srcPixel - destPixel) * ALPHA) / 256
//
//  u16 dwAlphaRBtemp = (backpixel & 0x7c1f);
//  u16 dwAlphaGtemp = (backpixel & 0x03e0);
//  u16 dw5bitOpacity = (opacity >> 3);
//
//  return (
//      ((dwAlphaRBtemp + ((((pixel & 0x7c1f) - dwAlphaRBtemp) * dw5bitOpacity) >> 5)) & 0x7c1f) |
//      ((dwAlphaGtemp + ((((pixel & 0x03e0) - dwAlphaGtemp) * dw5bitOpacity) >> 5)) & 0x03e0) | 0x8000
//  );
//}


#define MAX_ROM_MODS 2
#include "rom_mods.h"

/*
   inline char toupper(const char toLower)
   {
   if ((toLower >= 'a') && (toLower <= 'z'))
   return char(toLower - 0x20);
   return toLower;
   }
 */

int emulator_patch_ROM (core_crocods_t *core, u8 *pbROMlo)
{
    u8 *pbPtr;

    int CPCkeyboard=core->keyboardLayout;

    if (CPCkeyboard<1) {
        return 0;
    }

    // pbPtr = pbROMlo + 0x1d69; // location of the keyboard translation table on 664 or 6128
    pbPtr = pbROMlo + 0x1eef; // location of the keyboard translation table on 6128
    memcpy(pbPtr, cpc_keytrans[CPCkeyboard-1], 240); // patch the CPC OS ROM with the chosen keyboard layout

    pbPtr = pbROMlo + 0x3800;
    memcpy(pbPtr, cpc_charset[CPCkeyboard-1], 2048); // add the corresponding character set

    return 0;
}

#define MAXFILE 1024


void myconsoleClear(core_crocods_t *core);

void SetRect(RECT *R, int left, int top, int right, int bottom);
void FillRect(RECT *R, u16 color);
void DrawRect(RECT *R, u16 color);

void UpdateKeyMenu(void);






// 384


pfctExecInstZ80 ExecInstZ80;
pfctResetZ80 ResetZ80;
pfctSetIRQZ80 SetIRQZ80;

static u8 bit_values[8] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};



static int frame=0, msgframe=0;
static char msgbuf[33] = {0};


u16 *kbdBuffer;


#ifdef USE_DEBUG
int DebugMode = 0;
int Cont = 1;
#endif

char *keyname0[] = {
    "CURSOR_UP", // = 0
    "CURSOR_RIGHT",
    "CURSOR_DOWN",
    "F9",
    "F6",
    "F3",
    "SMALL_ENTER",
    "FDOT",
    /* line 1", bit 0..bit 7 */
    "CURSOR_LEFT",
    "COPY",
    "F7",
    "F8",
    "F5",
    "F1",
    "F2",
    "F0",
    /* line 2", bit 0..bit 7 */
    "CLR",
    "OPEN_SQUARE_BRACKET",
    "RETURN",
    "CLOSE_SQUARE_BRACKET",
    "F4",
    "SHIFT",
    "FORWARD_SLASH",
    "CONTROL",
    /* line 3", bit 0.. bit 7 */
    "HAT",
    "MINUS",
    "AT",
    "P",
    "SEMICOLON",
    "COLON",
    "BACKSLASH",
    "DOT",
    /* line 4", bit 0..bit 7 */
    "ZERO",
    "9",
    "O",
    "I",
    "L",
    "K",
    "M",
    "COMMA",
    /* line 5", bit 0..bit 7 */
    "8",
    "7",
    "U",
    "Y",
    "H",
    "J",
    "N",
    "SPACE",
    /* line 6", bit 0..bit 7 */
    "6",
    "5",
    "R",
    "T",
    "G",
    "F",
    "B",
    "V",
    /* line 7", bit 0.. bit 7 */
    "4",
    "3",
    "E",
    "W",
    "S",
    "D",
    "C",
    "X",
    /* line 8", bit 0.. bit 7 */
    "1",
    "2",
    "ESC",
    "Q",
    "TAB",
    "A",
    "CAPS_LOCK",
    "Z",
    /* line 9", bit 7..bit 0 */
    "JOY_UP",
    "JOY_DOWN",
    "JOY_LEFT",
    "JOY_RIGHT",
    "JOY_FIRE1",
    "JOY_FIRE2",
    "SPARE",
    "DEL",

    /* no key press */
    "NIL"
};

int RgbCPCdef[ 32 ] =  {
    0x7F7F7F, // Blanc            (13)
    0x7F7F7F, // Blanc            (13)
    0x00FF7F, // Vert Marin       (19)
    0xFFFF7F, // Jaune Pastel     (25)
    0x00007F, // Bleu              (1)
    0xFF007F, // Pourpre           (7)
    0x007F7F, // Turquoise        (10)
    0xFF7F7F, // Rose             (16)
    0xFF007F, // Pourpre           (7)
    0xFFFF00, // Jaune vif        (24)
    0xFFFF00, // Jaune vif        (24)
    0xFFFFFF, // Blanc Brillant   (26)
    0xFF0000, // Rouge vif         (6)
    0xFF00FF, // Magenta vif       (8)
    0xFF7F00, // Orange           (15)
    0xFF7FFF, // Magenta pastel   (17)
    0x00007F, // Bleu              (1)
    0x00FF7F, // Vert Marin       (19)
    0x00FF00, // Vert vif         (18)
    0x00FFFF, // Turquoise vif    (20)
    0x000000, // Noir              (0)
    0x0000FF, // Bleu vif          (2)
    0x007F00, // Vert              (9)
    0x007FFF, // Bleu ciel        (11)
    0x7F007F, // Magenta           (4)
    0x7FFF7F, // Vert pastel      (22)
    0x7FFF00, // Vert citron      (21)
    0x7FFFFF, // Turquoise pastel (23)
    0x7F0000, // Rouge             (3)
    0x7F00FF, // Mauve             (5)
    0x7F7F00, // Jaune            (12)
    0x7F7FFF // Bleu pastel      (14)
};

void TraceLigne8B512( core_crocods_t *core, int y, signed int AdrLo, int AdrHi );


typedef struct {
    int normal;
} CPC_MAP;

int cpckeypressed[NBCPCKEY];

RECT keypos[NBCPCKEY] = {
    {0,51,14,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
    {15,51,33,72}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
    {34,51,52,72}, // 17    0x40 | MOD_CPC_SHIFT,   // CPC_0
    {53,51,70,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
    {71,51,87,72}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
    {88,51,104,72}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
    {105,51,121,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
    {122,51,138,72}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
    {139,51,155,72}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
    {156,51,172,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
    {173,51,189,72}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
    {190,51,206,72}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
    {207,51,223,72}, //    0x50 | MOD_CPC_SHIFT,   // CPC_LEFT
    {224,51,240,72}, //    0x41 | MOD_CPC_SHIFT,   // CPC_UP
    {241,51,255,72}, // CP_RIGHT

    {0,73,14,94}, // (0)
    {15,73,33,94}, //    0x80 | MOD_CPC_SHIFT,   // CPC_1
    {34,73,52,94}, //    0x81 | MOD_CPC_SHIFT,   // CPC_2
    {53,73,70,94}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
    {71,73,87,94}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
    {88,73,104,94}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
    {105,73,121,94}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
    {122,73,138,94}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
    {139,73,155,94}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
    {156,73,172,94}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
    {173,73,189,94}, // (10)  0x40 | MOD_CPC_SHIFT,   // CPC_0
    {190,73,206,94}, //    0x70 | MOD_CPC_SHIFT,   // CPC_=
    {207,73,223,94}, //    0x61 | MOD_CPC_SHIFT,   // CPC_LAMBDA
    {224,73,240,94}, //    0x60 | MOD_CPC_SHIFT,   // CPC_CLR
    {241,73,256,94}, //    0x51 | MOD_CPC_SHIFT,   // CPC_DEL

    {0,95,19,116},
    {20,95,38,116}, //    0x83,                   // CPC_a
    {39,95,57,116}, //    0x73,                   // CPC_z
    {58,95,76,116}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
    {77,95,95,116}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
    {96,95,114,116}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
    {115,95,133,116}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
    {134,95,152,116}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
    {153,95,171,116}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
    {172,95,190,116}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
    {191,95,207,116}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
    {208,95,224,116}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
    {225,95,241,116}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
    {242,95,256,138},//    0x22,                   // CPC_RETURN

    {0,117,21,138},
    {22,117,40,138}, //    0x83,                   // CPC_A
    {41,117,59,138}, //    0x73,                   // CPC_S
    {60,117,78,138}, //    0x71 | MOD_CPC_SHIFT,   // CPC_D
    {79,117,97,138}, //    0x70 | MOD_CPC_SHIFT,   // CPC_F
    {98,117,116,138}, //    0x61 | MOD_CPC_SHIFT,   // CPC_G
    {117,117,135,138}, //    0x60 | MOD_CPC_SHIFT,   // CPC_H
    {136,117,154,138}, //    0x51 | MOD_CPC_SHIFT,   // CPC_J
    {155,117,173,138}, //    0x50 | MOD_CPC_SHIFT,   // CPC_K
    {174,117,190,138}, //    0x41 | MOD_CPC_SHIFT,   // CPC_L
    {191,117,207,138}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
    {208,117,224,138}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
    {225,117,241,138}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5

    {0,139,28,160}, // SHIFT
    {29,139,47,160}, //    0x81 | MOD_CPC_SHIFT,   // CPC_2
    {48,139,66,160}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
    {67,139,85,160}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
    {86,139,104,160}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
    {105,139,123,160}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
    {124,139,142,160}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
    {143,139,161,160}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
    {162,139,178,160}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
    {179,139,195,160}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
    {196,139,212,160}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
    {213,139,229,160}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
    {230,139,256,160}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6

    {0,161,57,182}, //    0x55,                   // CPC_j
    {58,161,95,182}, //    0x55,                   // CPC_j
    {96,161,207,182}, //    0x55,                   // CPC_j
    {208,161,256,182} //    0x55,                   // CPC_j
};

/*
   RECT keypos[NBCPCKEY] = {
        {2,116,15,130},    // (0)
        {16,116,29,130},   //    0x80 | MOD_CPC_SHIFT,   // CPC_1
        {30,116,43,130},   //    0x81 | MOD_CPC_SHIFT,   // CPC_2
        {44,116,57,130}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
        {58,116,71,130}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
        {72,116,85,130}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
        {86,116,99,130}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
        {100,116,113,130}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
        {114,116,127,130}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
        {128,116,141,130}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
        {142,116,155,130}, // (10)  0x40 | MOD_CPC_SHIFT,   // CPC_0
        {156,116,169,130}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
        {170,116,183,130}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
        {184,116,197,130}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
        {198,116,211,130}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
        {213,116,226,130}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
        {227,116,240,130}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
        {241,116,254,130}, // 17    0x40 | MOD_CPC_SHIFT,   // CPC_0

        {2,131,19,145},
        {20,131,33,145}, //    0x83,                   // CPC_a
        {34,131,47,145}, //    0x73,                   // CPC_z
        {48,131,61,145}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
        {62,131,75,145}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
        {76,131,89,145}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
        {90,131,103,145}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
        {104,131,117,145}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
        {118,131,131,145}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
        {132,131,145,145}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
        {146,131,159,145}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
        {160,131,173,145}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
        {174,131,187,145}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
        {191,131,211,160},//    0x22,                   // CPC_RETURN
        {213,131,226,145}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
        {227,131,240,145}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
        {241,131,254,145}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0

        {2,146,22,160},
        {23,146,36,160}, //    0x83,                   // CPC_a
        {37,146,50,160}, //    0x73,                   // CPC_z
        {51,146,64,160}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
        {65,146,78,160}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
        {79,146,92,160}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
        {93,146,106,160}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
        {107,146,120,160}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
        {121,146,134,160}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
        {135,146,148,160}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
        {149,146,162,160}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
        {163,146,176,160}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
        {177,146,190,160}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
        {213,146,226,160}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
        {227,146,240,160}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
        {241,146,254,160}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0

        {2,161,29,175},
        {30,161,43,175}, //    0x81 | MOD_CPC_SHIFT,   // CPC_2
        {44,161,57,175}, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
        {58,161,71,175}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
        {72,161,85,175}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
        {86,161,99,175}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
        {100,161,113,175}, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
        {114,161,127,175}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
        {128,161,141,175}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
        {142,161,155,175}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
        {156,161,169,175}, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
        {170,161,183,175}, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
        {184,161,211,175}, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
        {213,161,227,175}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
        {228,161,240,175}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
        {241,161,254,175}, //    0x40 | MOD_CPC_SHIFT,   // CPC_0

        {1,176,34,190}, //    0x55,                   // CPC_j
        {35,176,55,190}, //    0x55,                   // CPC_j
        {56,176,167,190}, //    0x55,                   // CPC_j
        {168,176,211,190}, //    0x55,                   // CPC_j
        {213,176,227,190}, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
        {228,176,240,190}, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
        {241,176,254,190}
   };
 */



CPC_SCANCODE keyown[11];

CPC_MAP keymap[NBCPCKEY] = {
    { CPC_FDOT },
    { CPC_F1 },
    { CPC_F2 },
    { CPC_F3 },
    { CPC_F4 },
    { CPC_F5 },
    { CPC_F6 },
    { CPC_F7 },
    { CPC_F8 },
    { CPC_F9 },
    { CPC_F0 },
    { CPC_CURSOR_UP },
    { CPC_CURSOR_LEFT },
    { CPC_CURSOR_DOWN },
    { CPC_CURSOR_RIGHT },

    { CPC_ESC },
    { CPC_1 },
    { CPC_2 },
    { CPC_3 },
    { CPC_4 },
    { CPC_5 },
    { CPC_6 },
    { CPC_7 },
    { CPC_8 },
    { CPC_9 },
    { CPC_ZERO },
    { CPC_MINUS },
    { CPC_HAT },
    { CPC_CLR },
    { CPC_DEL },

    { CPC_TAB }, //
    { CPC_Q },
    { CPC_W },
    { CPC_E },
    { CPC_R },
    { CPC_T },
    { CPC_Y },
    { CPC_U },
    { CPC_I },
    { CPC_O },
    { CPC_P },
    { CPC_AT },
    { CPC_OPEN_SQUARE_BRACKET },
    { CPC_RETURN },

    { CPC_CAPS_LOCK }, //
    { CPC_A },
    { CPC_S },
    { CPC_D },
    { CPC_F },
    { CPC_G },
    { CPC_H },
    { CPC_J },
    { CPC_K },
    { CPC_L },
    { CPC_COLON },
    { CPC_SEMICOLON },
    { CPC_CLOSE_SQUARE_BRACKET },

    { CPC_SHIFT }, //
    { CPC_Z },
    { CPC_X },
    { CPC_C },
    { CPC_V },
    { CPC_B },
    { CPC_N },
    { CPC_M },
    { CPC_COMMA },
    { CPC_DOT },
    { CPC_FORWARD_SLASH },
    { CPC_BACKSLASH },
    { CPC_SHIFT },


    { CPC_CONTROL },
    { CPC_COPY },
    { CPC_SPACE },
    { CPC_SMALL_ENTER }
};



struct kmenu root;
struct kmenu *menuRomId;
struct kmenu *menuDiskId;
struct kmenu *FirstROM;
struct kmenu *keyMenu;

struct kmenu *AddMenu(struct kmenu *parent, char *title, int id, int x, int y)
{
    struct kmenu *kcur;

    kcur=(struct kmenu *)calloc(sizeof(struct kmenu),1);
    kcur->parent=parent;
    kcur->firstchild=NULL;
    kcur->lastchild=NULL;
    kcur->nextSibling=NULL;
    kcur->previousSibling=NULL;
    kcur->nbr=0;
    strcpy(kcur->title, title);
    kcur->id = id;
    kcur->x = x;
    kcur->y = y;

    if (kcur->parent->nbr==0) {
        kcur->parent->firstchild=kcur;
        kcur->parent->lastchild=kcur;
        kcur->parent->nbr=1;
    } else {
        struct kmenu *i, *i0;

        i0=NULL;
        i=kcur->parent->firstchild;
        do {
            if (strcmp(kcur->title,i->title)<0) {
                break; // placer kcur juste avant i
            }
            i0=i;
            i=i->nextSibling;
        } while(i!=NULL);

        i=NULL;

        if (i==NULL) {
            kcur->previousSibling=kcur->parent->lastchild;
            kcur->parent->lastchild->nextSibling=kcur;
            kcur->parent->lastchild=kcur;
        } else {
            kcur->nextSibling=i;
            if (i0==NULL) {
                kcur->parent->firstchild=kcur;
            } else {
                i0->nextSibling=kcur;
            }
        }
        kcur->parent->nbr++;
    }

    return kcur;
}





void SelectSNAP(void);

/*
   void LireRom(struct kmenu *FM, int autostart)
   {
   u8 *rom=NULL;
   u32 romsize=0;
   char autofile[256];

   myprintf("Loading %s", FM->object); // FM->title
   rom = FS_Readfile(FM->object, &romsize);

   if (rom==NULL) {
   myprintf("Rom not found");
   return;
   }

 #ifdef USE_SNAPSHOT
   if (!memcmp(rom, "MV - SNA", 8 )) {
   LireSnapshotMem(rom);
   currentsnap=0;
   strcpy(currentfile, FM->object);
   }
 #endif
   if ( (!memcmp(rom, "MV - CPCEMU", 11)) || (!memcmp(rom,"EXTENDED", 8)) ) {
   autofile[0]=0;
   myprintf("Load Disk");
   LireDiskMem(rom, romsize, autofile);
   myprintf("Disk loaded");
   if ((autofile[0]!=0) && (autostart)) {
   char buffer[256];
   sprintf(buffer,"run\"%s\n", autofile);
   AutoType_SetString(buffer, TRUE);
   }
   currentsnap=0;
   strcpy(currentfile, FM->object);
   }
   free(rom);



   }
 */


void SelectSNAP(void)
{
#ifdef USE_FAT
#ifndef USE_CONSOLE
    u16 keys_pressed;

    dmaCopy(menubufferlow, backBuffer, 256*192*2);

    currentsnap=0;

    // Wait key off
    do {
        keys_pressed = ~(REG_KEYINPUT);
    } while ((keys_pressed & (KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))!=0);

    DrawTextLeft(backBuffer, filefont, RGB15(31,31,0) | 0x8000, 15,  1, "Select Save Slot");

    int x;
    RECT r;

    for(x=0; x<3; x++) {
        char id[2];
        char *buf;
        char snap[256];
        int haveimg, havesnap;


        sprintf(snap, "/%s.%d", currentfile, currentsnap+1);
        buf=strchr(snap,'.');
        *buf='_';
        havesnap = FileExists(snap);

        sprintf(snap, "/%s_i.%d", currentfile, currentsnap+1);
        buf=strchr(snap,'.');
        *buf='_';
        haveimg = FileExists(snap);

        sprintf(id,"%d", x+1);
        SetRect(&r, 5+x*83, 130, 5+x*83+80, 50+130);
        FillRect(&r, RGB15((156>>3), (178>>3), (165>>3))|0x8000);
        DrawText(backBuffer, font, 7+x*83, 132, id);
        if (!havesnap) {
            DrawText(backBuffer, font, 7+x*83, 152, "Empty");
        }
    }

    while(1) {
        for(x=0; x<3; x++) {
            SetRect(&r, 4+x*83, 129, 6+x*83+80, 51+130);

            if (x==currentsnap) {
                DrawRect(&r,  RGB15(0,31,31) | 0x8000);
            } else {
                DrawRect(&r,  RGB15(16,0,0) | 0x8000);
            }
        }

        keys_pressed = MyReadKey();

        if ((keys_pressed & KEY_LEFT)==KEY_LEFT) {
            if (currentsnap==0) {
                currentsnap=2;
            } else {
                currentsnap--;
            }
        }
        if ((keys_pressed & KEY_RIGHT)==KEY_RIGHT) {
            if (currentsnap==2) {
                currentsnap=0;
            } else {
                currentsnap++;
            }
        }

        if ((keys_pressed & KEY_A)==KEY_A) {
            break;
        }

    }
#endif
#endif
    return;
}




void SetRect(RECT *R, int left, int top, int right, int bottom)
{
    R->left=left;
    R->top=top;
    R->right=right;
    R->bottom=bottom;
}

#ifndef USE_CONSOLE
void FillRect(RECT *R, u16 color)
{
    int x,y;

    for(y=R->top; y<R->bottom; y++) {
        for(x=R->left; x<R->right; x++) {
            backBuffer[x+y*256]=color;
        }
    }
}

void DrawRect(RECT *R, u16 color)
{
    int x,y;

    for(y=R->top; y<R->bottom; y++) {
        backBuffer[R->left+y*256]=color;
        backBuffer[(R->right-1)+y*256]=color;
    }
    for(x=R->left; x<R->right; x++) {
        backBuffer[x+R->top*256]=color;
        backBuffer[x+(R->bottom-1)*256]=color;
    }
}

void DrawLift(RECT *max, RECT *dest, u16 coloron, u16 coloroff)
{
    int sizelift=8;
    RECT *r;
    RECT r0;
    /*
       if ( (max->right-max->left) > (dest->right-dest->left) ) {
       int x1,x2;

       int dr0,dr,dl,mr,ml;

       dr=dest->right;
       if ( (max->bottom-max->top) > (dest->bottom-dest->top) ) {
       dr0=dest->right-sizelift;
       } else {
       dr0=dr;
       }
       dl=dest->left;
       mr=max->right;
       ml=max->left;

       x1=dl+((dl-ml)*(dr0-dl))/(mr-ml);
       x2=dr0-((mr-dr)*(dr0-dl))/(mr-ml);

       r=&r0;

       SetRect(r, x1, dest->top, x2, dest->top+sizelift);
       FillRect(r, coloroff);

       SetRect(r, x1, dest->top, x2, dest->top+sizelift);
       FillRect(r, coloron);

                   SetRect(r, x1, dest->top, x2, dest->top+sizelift);
                   FillRect(r, coloroff);
                   }
     */

    if ( (max->bottom-max->top) > (dest->bottom-dest->top) ) {
        int y1,y2;

        int dt0,dt,db,mt,mb;

        db=dest->bottom;
        dt=dest->top;

        if ( (max->right-max->left) > (dest->right-dest->left) ) {
            dt0=dest->top+sizelift;
        } else {
            dt0=dt;
        }
        mb=max->bottom;
        mt=max->top;

        y1=dt0+((dt-mt)*(db-dt0))/(mb-mt);
        y2=db-((mb-db)*(db-dt))/(mb-mt);

        //  y2=y1+((db-dt)*(db-dt))/(mb-mt);

        if (y2>mb) {
            y2=mb; // pas normal... mais ca arrive :(
        }

        r=&r0;

        if (dest->top<y1) {
            SetRect(r, dest->right - sizelift, dest->top, dest->right, y1);
            FillRect(r, coloroff);
        }

        SetRect(r, dest->right - sizelift, y1, dest->right, y2);
        FillRect(r, coloron);

        if (dest->bottom>y2) {
            SetRect(r, dest->right - sizelift, y2, dest->right, dest->bottom);
            FillRect(r, coloroff);
        }
    }

    return;
}
#endif

// -1: dernier couleur
// 0: vert
// 1: couleur
// 3: inactif

#define RGB15(R,G,B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

void SetPalette(core_crocods_t *core, int color)
{

    int i;

    //    if ( (color==0) || (color==1) ) {
    //        core->lastcolour=color;
    //        return;
    //    }

    if (color==-1) {
        color=core->lastcolour;
    }

    if (color==1) {
        for ( i = 0; i < 32; i++ ) {
            int r = ( RgbCPCdef[ i ] >> 16 ) & 0xFF;
            int g = ( RgbCPCdef[ i ] >> 8 ) & 0xFF;
            int b = ( RgbCPCdef[ i ] >> 0 ) & 0xFF;

            core->BG_PALETTE[i]=RGB15(r,g,b);
        }
        core->lastcolour=color;
    }

    if (color==0) {
        for ( i = 0; i < 32; i++ ) {
            int r = ( RgbCPCdef[ i ] >> 16 ) & 0xFF;
            int g = ( RgbCPCdef[ i ] >> 8 ) & 0xFF;
            int b = ( RgbCPCdef[ i ] >> 0 ) & 0xFF;


            g=(r+g+b)/3;
            b=0;
            r=0;

            core->BG_PALETTE[i]=RGB15(r,g,b);
        }
        core->lastcolour=color;
    }
    if (color==3) {

        for ( i = 0; i < 32; i++ ) {
            int z;
            int r = ( RgbCPCdef[ i ] >> 16 ) & 0xFF;
            int g = ( RgbCPCdef[ i ] >> 8 ) & 0xFF;
            int b = ( RgbCPCdef[ i ] >> 0 ) & 0xFF;

            z=(r+g+b)/3;

            core->BG_PALETTE[i]=RGB15(z,z,z);
        }
    }

    core->UpdateInk=1;

}

void RedefineKey(int key)
{
#ifndef USE_CONSOLE
    int x,y,n;
    dmaCopy(kbdBuffer, backBuffer, SCREEN_WIDTH * SCREEN_HEIGHT * 2);

    keyEmul=3;

    while(((~IPC->buttons) & (1 << 6))==0) ;

    x=IPC->touchXpx;
    y=IPC->touchYpx;

    for (n=0; n<NBCPCKEY; n++) {
        if ( (x>=keypos[n].left) && (x<=keypos[n].right) && (y>=keypos[n].top) && (y<=keypos[n].bottom) ) {
            keyown[key]=keymap[n].normal;
            break;
        }
    }
    UpdateKeyMenu();
#endif
}

void UpdateTitlePalette(struct kmenu *current)
{
    /*
       if (lastcolour==1) {
       sprintf(current->title,"Monitor: [COLOR] - Green");
       } else {
       sprintf(current->title,"Monitor: Color - [GREEN]");
       }
     */
}

// Retour: 1 -> return emulator
//         0 -> return to parent
//         2 -> return to item (switch)


int ExecuteMenu(core_crocods_t *core, int n, struct kmenu *current)
{
    switch(n) {
    case ID_SWITCH_MONITOR:
        return 0;
        break;
    case ID_COLOR_MONITOR:
        SetPalette(core, 1);
        return 0;
        break;
    case ID_GREEN_MONITOR:
        SetPalette(core, 0);
        return 0;
        break;
    case ID_SCREEN_AUTO:
        core->screenBufferWidth = 320;
        core->screenBufferHeight = 200;

        core->MemBitmap_width = 320;

        core->resize=1;
        core->DrawFct = TraceLigne8B512;
        core->Regs1=0;
        core->Regs2=0;
        core->Regs6=0;
        core->Regs7=0;
        core->changeFilter=1; // Flag to ask to the display to change the resolution

        return 1;
        break;
    case ID_SCREEN_320:
        core->screenBufferWidth = 320;
        core->screenBufferHeight = 200;

        core->MemBitmap_width = 320;

        core->resize=2;
        core->DrawFct = TraceLigne8B512;
        //     BG3_XDX = 320; // 256; // 360 - 54;  // 360 = DISPLAY_X // Taille d'affichage
        //     BG3_CX = (XStart*4) << 8;
        core->x0=(core->XStart*4);
        core->y0=40;
        core->maxy=64;
        core->changeFilter=1; // Flag to ask to the display to change the resolution

        return 0;
        break;
    case ID_SCREEN_NORESIZE:
        core->resize=3;
        core->DrawFct = TraceLigne8B512;
        //     BG3_XDX = 256; // 256; // 360 - 54;  // 360 = DISPLAY_X // Taille d'affichage
        //     BG3_CX = (XStart*4) << 8;
        core->x0=(core->XStart*4);
        core->y0=40;
        core->maxy=80;
        core->changeFilter=1; // Flag to ask to the display to change the resolution

        return 0;
        break;
    case ID_SCREEN_OVERSCAN:
        core->screenBufferWidth = 384;
        core->screenBufferHeight = 272;

        core->resize=4;
        core->DrawFct = TraceLigne8B512;
        //   BG3_XDX = 384; // 256; // 360 - 54;  // 360 = DISPLAY_X // Taille d'affichage
        //   BG3_CX = 0;
        core->x0=(core->XStart*4);
        core->y0=0;
        core->changeFilter=1; // Flag to ask to the display to change the resolution

        return 0;
        break;
    case ID_KEY_KEYBOARD:
        core->keyEmul=2; //  Emul du clavier
        return 0;
        break;

    case ID_KEY_KEYPAD:
        keyown[0]=CPC_CURSOR_UP;
        keyown[1]=CPC_CURSOR_DOWN;
        keyown[2]=CPC_CURSOR_LEFT;
        keyown[3]=CPC_CURSOR_RIGHT;
        keyown[4]=CPC_RETURN;
        keyown[5]=CPC_SPACE;
        keyown[6]=CPC_SPACE;

        core->keyEmul=3; //  Emul du clavier fleche
        return 0;
        break;
    case ID_KEY_JOYSTICK:
        keyown[0]=CPC_JOY_UP;
        keyown[1]=CPC_JOY_DOWN;
        keyown[2]=CPC_JOY_LEFT;
        keyown[3]=CPC_JOY_RIGHT;
        keyown[4]=CPC_RETURN;
        keyown[5]=CPC_JOY_FIRE1;
        keyown[6]=CPC_JOY_FIRE2;

        core->keyEmul=3; //  Emul du joystick
        return 0;
        break;
    case ID_DISPFRAMERATE:
        core->dispframerate=1;
        return 0;
        break;
    case ID_NODISPFRAMERATE:
        core->dispframerate=0;
        return 0;
        break;
    case ID_RESET:
        myprintf("Reset CPC");
        ExecuteMenu(core, ID_MENU_EXIT, NULL);
        ResetCPC(core);
        return 0;
        break;
    case ID_SAVESNAP:
    {
        char *buf;
        char snap[1024];

        sprintf(snap, "/%s.%d.sna", core->openFilename, core->currentsnap+1);
        buf=strchr(snap,'.');
        *buf='_';
        SauveSnap(core, snap);

        return 1;
        break;
    }
    case ID_FILE:
        // LireRom(current,1);
        return 1;
        break;
    case ID_DISK:
        //LireRom(current,0);
        return 1;
        break;
    case ID_REDEFINE_UP:
        RedefineKey(0);
        return 2;
        break;
    case ID_REDEFINE_DOWN:
        RedefineKey(1);
        return 2;
        break;
    case ID_REDEFINE_LEFT:
        RedefineKey(2);
        return 2;
        break;
    case ID_REDEFINE_RIGHT:
        RedefineKey(3);
        return 2;
        break;
    case ID_REDEFINE_START:
        RedefineKey(4);
        return 2;
        break;
    case ID_REDEFINE_A:
        RedefineKey(5);
        return 2;
        break;
    case ID_REDEFINE_B:
        RedefineKey(6);
        return 2;
        break;
    case ID_REDEFINE_X:
        RedefineKey(7);
        return 2;
        break;
    case ID_REDEFINE_Y:
        RedefineKey(8);
        return 2;
        break;
    case ID_REDEFINE_L:
        RedefineKey(9);
        return 2;
        break;
    case ID_REDEFINE_R:
        RedefineKey(0);
        return 2;
        break;
    case ID_HACK_TABCOUL:
        core->hack_tabcoul = 1;
        return 2;
        break;
    case ID_NOHACK_TABCOUL:
        core->hack_tabcoul =  0;
        return 2;
        break;
    case ID_ACTIVE_MAGNUM:
        core->usemagnum=1;
        break;
    case ID_MENU_ENTER:
        ExecuteMenu(core,ID_PAUSE_ENTER,NULL);
        core->inMenu = 1;
        break;
    case ID_MENU_EXIT:
        ExecuteMenu(core,ID_PAUSE_EXIT,NULL);
        core->inMenu = 0;
        break;
    case ID_PAUSE_ENTER:
        core->isPaused = 1;
        SetPalette(core, 3);
        break;
    case ID_PAUSE_EXIT:
        core->isPaused = 0;
        core->inMenu = 0;
        SetPalette(core, -1);
        break;
    case ID_DEBUG_ENTER:
        ExecInstZ80 = ExecInstZ80_debug;
        core->debug = 1;
        break;
    case ID_DEBUG_EXIT:
        ExecInstZ80 = ExecInstZ80_orig;
        core->debug = 0;
        break;
    default:
        break;
    }
    return 1;
}




u16 MyReadKey(void)
{
    /*
       u16 keys_pressed, my_keys_pressed;

       do {
       keys_pressed = ~(REG_KEYINPUT);
       } while ((keys_pressed & (KEY_LEFT | KEY_RIGHT | KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))==0);

       my_keys_pressed = keys_pressed;

       do {
       keys_pressed = ~(REG_KEYINPUT);
       } while ((keys_pressed & (KEY_LEFT | KEY_RIGHT | KEY_DOWN | KEY_UP | KEY_A | KEY_B | KEY_L | KEY_R))!=0);
     */
    return 0; // my_keys_pressed;
}

void InitCalcPoints( core_crocods_t *core )
{
    int a, b, c, d, i;

    // Pour le mode 0
    for ( i = 0; i < 256; i++ )
    {
        a = ( i >> 7 )
            + ( ( i & 0x20 ) >> 3 )
            + ( ( i & 0x08 ) >> 2 )
            + ( ( i & 0x02 ) << 2 );
        b = ( ( i & 0x40 ) >> 6 )
            + ( ( i & 0x10 ) >> 2 )
            + ( ( i & 0x04 ) >> 1 )
            + ( ( i & 0x01 ) << 3 );
        core->TabPointsDef[ 0 ][ i ][ 0 ] = (u8)a;
        core->TabPointsDef[ 0 ][ i ][ 1 ] = (u8)a;
        core->TabPointsDef[ 0 ][ i ][ 2 ] = (u8)b;
        core->TabPointsDef[ 0 ][ i ][ 3 ] = (u8)b;
    }

    // Pour le mode 1
    for ( i = 0; i < 256; i++ )
    {
        a = ( i >> 7 ) + ( ( i & 0x08 ) >> 2 );
        b = ( ( i & 0x40 ) >> 6 ) + ( ( i & 0x04 ) >> 1 );
        c = ( ( i & 0x20 ) >> 5 ) + ( i & 0x02 );
        d = ( ( i & 0x10 ) >> 4 ) + ( ( i & 0x01 ) << 1 );
        core->TabPointsDef[ 1 ][ i ][ 0 ] = (u8)a;
        core->TabPointsDef[ 1 ][ i ][ 1 ] = (u8)b;
        core->TabPointsDef[ 1 ][ i ][ 2 ] = (u8)c;
        core->TabPointsDef[ 1 ][ i ][ 3 ] = (u8)d;
    }

    // Pour le mode 2
    for ( i = 0; i < 256; i++ )
    {
        core->TabPointsDef[ 2 ][ i ][ 0 ] = i >> 7;
        core->TabPointsDef[ 2 ][ i ][ 1 ] = ( i & 0x20 ) >> 5;
        core->TabPointsDef[ 2 ][ i ][ 2 ] = ( i & 0x08 ) >> 3;
        core->TabPointsDef[ 2 ][ i ][ 3 ] = ( i & 0x02 ) >> 1;
    }

    // Mode 3 = Mode 0 ???
    for ( i = 0; i < 256; i++ )
        for ( a = 0; a < 4; a++ )
            core->TabPointsDef[ 3 ][ i ][ a ] = core->TabPointsDef[ 0 ][ i ][ a ];
}

void CalcPoints( core_crocods_t *core )
{
    int i,j;

    if ((core->lastMode>=0) && (core->lastMode<=3)) {
        for (i=0; i<256; i++) {
            for(j=0; j<4; j++) {
                core->TabPoints[core->lastMode][i][j] = core->BG_PALETTE[core->TabCoul[ core->TabPointsDef[core->lastMode][i][j]]];
            }
            /* *(u32*)(&core->TabPoints[lastMode][i][0]) = (TabCoul[ core->TabPointsDef[lastMode][i][0] ] << 0) + (TabCoul[ core->TabPointsDef[lastMode][i][1] ] << 8) + (TabCoul[ core->TabPointsDef[lastMode][i][2] ] << 16) + (TabCoul[ core->TabPointsDef[lastMode][i][3] ] << 24);
             */
        }
    }
    core->UpdateInk=0;
}




/********************************************************* !NAME! **************
* Nom : InitPlateforme
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Initialisation diverses
*
* Résultat    : /
*
* Variables globales modifiées : JoyKey, clav
*
********************************************************** !0! ****************/
void InitPlateforme( core_crocods_t *core, unsigned short *screen, u16 screen_width )
{
    core->MemBitmap=screen;
    core->MemBitmap_width=screen_width;

    InitCalcPoints(core);
    CalcPoints(core);
    memset( core->clav, 0xFF, sizeof( core->clav ) );
    memset( cpckeypressed, 0, sizeof(cpckeypressed));

    core->ipc.keys_pressed=0;

    core->inMenu=0;
    core->isPaused=0;
}

void updateScreenBuffer( core_crocods_t *core, unsigned short *screen, u16 screen_width )
{
    core->MemBitmap=screen;
    core->MemBitmap_width=screen_width;
}

/********************************************************* !NAME! **************
* Nom : Erreur
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Affichage d'un message d'erreur
*
* Résultat    : /
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
void Erreur( char * Msg )
{
    myprintf("Error: %s", Msg);
}


/********************************************************* !NAME! **************
* Nom : Info
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Affichage d'un message d'information
*
* Résultat    : /
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
void Info( char * Msg )
{
    myprintf("Info: %s", Msg);
}

//Bordure: 32 a gauche, 32 a droite
//Border: 36 en haut, 36 en bas

static int updated=0;


void TraceLigne8B512( core_crocods_t *core, int y, signed int AdrLo, int AdrHi )
{
    // if (y>yMax) yMax=y;
    // if (y<yMin) yMin=y;

    y-=core->y0;

    if ((y<0) || (y>=272)) { // A verifier (200?)
        return;
    }

    if ((!core->hack_tabcoul) && (core->UpdateInk==1)) { // It's would be beter to put before each lines
        CalcPoints(core);
    }

    updated=1;

    u16 *p;

    p = (u16*)core->MemBitmap;
    p += (y*core->MemBitmap_width);

    //    for (int i=0;i<384;i++) {
    //        p[i]=rand()&0xFFFF;
    //    }
    //    return;

    if (core->lastMode!=2) {
        if ( AdrLo < 0 ) {
            if ((core->resize!=1) && (core->resize!=2)) {
                int x;
                for (x=0; x<384; x++) {
                    p[x]=core->BG_PALETTE[core->TabCoul[ 16 ]];
                }
            }
        } else {
            int x;

            if (core->resize==4) {
                for(x=0; x<core->XStart*4; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            } if (core->resize==2) {
                for(x=0; x<(core->XStart-8)*4; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            } else {
                // p+=XStart*4;
            }

            for (x = core->XStart; x < core->XEnd; x++ ) {
                u16 *tab = &(core->TabPoints[ core->lastMode ][ core->MemCPC[ ( AdrLo & 0x7FF ) | AdrHi ] ][0]);
                memcpy(p,tab,4*sizeof(u16));
                p+=4;
                AdrLo++;
            }

            if (core->resize==4) {
                for(x=0; x<(96-core->XEnd)*4; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            } else if (core->resize==2) {
                for(x=0; x<(88-core->XEnd)*4; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            }
        }

    } else { // If mode 2
        //        core->BG_PALETTE[0]=0;

        p += (y*core->MemBitmap_width);

        if ( AdrLo < 0 ) {
            if ((core->resize!=1) && (core->resize!=2)) {
                int x;
                for (x=0; x<384*2; x++) {
                    p[x]=core->BG_PALETTE[core->TabCoul[ 16 ]];
                }
            }
        } else {
            int x;

            if (core->resize==4) {
                for(x=0; x<core->XStart*8; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            } if (core->resize==2) {
                for(x=0; x<(core->XStart-8)*8; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            } else {
                // p+=XStart*4;
            }

            for (x = core->XStart; x < core->XEnd; x++ ) {
                u8 car = core->MemCPC[ ( AdrLo & 0x7FF ) | AdrHi ];
                int i;

                for (i=0; i<8; i++) {
                    *(p+7-i) = core->BG_PALETTE[core->TabCoul[car&1]];
                    car = (car>>1);
                }
                p+=8;
                AdrLo++;
            }

            if (core->resize==4) {
                for(x=0; x<(96-core->XEnd)*8; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            } else if (core->resize==2) {
                for(x=0; x<(88-core->XEnd)*8; x++) {
                    *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                    p++;
                }
            }
        }


    }

}

/********************************************************* !NAME! **************
* Nom : UpdateScreen
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Affiche l'écran du CPC
*
* Résultat    : /
*
* Variables globales modifiées : /
*
********************************************************** !0! ****************/
void UpdateScreen( core_crocods_t *core )
{

    frame++;

    if (core->resize==1) { // Auto resize ?
        if ((core->RegsCRTC[2]!=core->Regs2) || (core->RegsCRTC[6]!=core->Regs6) || (core->RegsCRTC[1]!=core->Regs1) || (core->RegsCRTC[7]!=core->Regs7)) {
            int x1,x2, y1,y2;
            int height;

            x1 = max( ( 50 - core->RegsCRTC[2] ) << 3, 0 );
            x2 = min( x1 + ( core->RegsCRTC[1] << 3 ), 384 );

            // y1 = max( (272-(RegsCRTC[6]<<3))>>1, 0);
            y1 = max( ( 35 - core->RegsCRTC[7] ) << 3, 0);
            y2 = min( y1 + (core->RegsCRTC[6] << 3), 272);

            core->DrawFct = TraceLigne8B512;
            //       BG3_XDX = (x2-x1);
            //     BG3_CX = x1 << 8;

            height=(y2-y1);
            if (height<192) height=192;


            core->x0=x1;
            core->y0=y1; // Redbug
            core->maxy=0;

            (*core->borderX)=(384-(x2-x1))/2;
            (*core->borderY)=(272-(y2-y1))/2;

            //            printf("*** Resize to %dx%d\n", x2-x1, y2-y1); // FM->title
            //            printf("*** Border to %dx%d\n",  (*core->borderX),  (*core->borderY)); // FM->title

            core->Regs1=core->RegsCRTC[1];
            core->Regs2=core->RegsCRTC[2];
            core->Regs6=core->RegsCRTC[6];
            core->Regs7=core->RegsCRTC[7];

            core->screenBufferWidth = x2-x1;
            core->screenBufferHeight = y2-y1;

            core->MemBitmap_width = x2-x1;

            core->changeFilter = 1;
        }
    }


    if (msgframe>frame-50*3) {
        //        int alpha;
        //        alpha=(msgframe-(frame-50*3))*4;
        //        if (alpha>255) alpha=255;

        cpcprint(core, 0,40, msgbuf, 1);
    }

    if (updated) {
        // dmaCopy(MemBitmap, frontBuffer, CPC_VISIBLE_SCR_WIDTH * CPC_VISIBLE_SCR_HEIGHT);
        //
        updated=0;

        if (core->UpdateInk==1) { // It's would be beter to put before each lines
            CalcPoints(core);
        }
    }
}



int shifted=0;
int ctrled=0;
int copyed=0;


void Dispkey(CPC_KEY n, int status);
void DispScanCode(CPC_SCANCODE n, int status);
void PressKey(core_crocods_t *core, CPC_KEY n);

void PressKey(core_crocods_t *core, CPC_KEY n)
{
    CPC_SCANCODE cpc_scancode;
    cpc_scancode=keymap[n].normal;

    Dispkey(n, 1);

    if (shifted) {
        DispScanCode(CPC_SHIFT, 0 | 16);
        shifted=0;
        core->clav[0x25 >> 4] &= ~bit_values[0x25 & 7]; // key needs to be SHIFTed
    }
    if (ctrled) {
        DispScanCode(CPC_CONTROL, 0);
        ctrled=0;
        core->clav[0x27 >> 4] &= ~bit_values[0x27 & 7]; // CONTROL key is held down
    }
    if (copyed) {
        DispScanCode(CPC_COPY, 0);
        copyed=0;
    }

    core->clav[(u8)cpc_scancode >> 3] &= ~bit_values[(u8)cpc_scancode & 7];

    switch(cpc_scancode) {
    case CPC_SHIFT:
        if (shifted) {
            DispScanCode(cpc_scancode, 0 | 16);
            shifted=0;
        } else {
            DispScanCode(cpc_scancode, 1 | 16);
            shifted=1;
        }
        break;
    case CPC_CONTROL:
        if (ctrled) {
            DispScanCode(cpc_scancode, 0 | 16);
            ctrled=0;
        } else {
            DispScanCode(cpc_scancode, 1 | 16);
            ctrled=1;
        }
        break;
    case CPC_COPY:
        if (copyed) {
            DispScanCode(cpc_scancode, 0 | 16);
            copyed=0;
        } else {
            DispScanCode(cpc_scancode, 1 | 16);
            copyed=1;
        }
        break;
    default:
        break;
    }
}

char CPC_isScanCode(core_crocods_t *core, CPC_SCANCODE cpc_scancode)
{
    //    printf("%d %d\n", core->clav[(u8)cpc_scancode >> 3] , bit_values[(u8)cpc_scancode & 7]);

    if (core->clav[(u8)cpc_scancode >> 3] & bit_values[(u8)cpc_scancode & 7]) {
        return 0;
    }

    printf("Scancode: %d\n", cpc_scancode);
    return 1;
}


void CPC_SetScanCode(core_crocods_t *core, CPC_SCANCODE cpc_scancode)
{
    core->clav[(u8)cpc_scancode >> 3] &= ~bit_values[(u8)cpc_scancode & 7];
    DispScanCode(cpc_scancode, 1);
}

void CPC_ClearScanCode(core_crocods_t *core, CPC_SCANCODE cpc_scancode)
{
    core->clav[(u8)cpc_scancode >> 3] |= bit_values[(u8)cpc_scancode & 7];
    DispScanCode(cpc_scancode, 0);
}


void DispScanCode(CPC_SCANCODE scancode, int status)
{
    int n;

    for(n=0; n<NBCPCKEY; n++) {
        if (keymap[n].normal == scancode)  {
            Dispkey(n, status);
        }
    }
}

// 1: active
// 2: on
// 0: off

void Dispkey(CPC_KEY n, int status)
{
#ifndef USE_CONSOLE
    int x,y;
    u16 color;


    if ((status&16)!=16) {
        if ((keymap[n].normal==CPC_SHIFT) || (keymap[n].normal==CPC_CONTROL) || (keymap[n].normal==CPC_COPY)) {
            return;
        }
    }

    switch(status) {
    case 0:
    case 16:
        for(y=keypos[n].top; y<keypos[n].bottom; y++) {
            for(x=keypos[n].left; x<keypos[n].right; x++) {
                backBuffer[x+y*256]=kbdBuffer[x+y*256];
            }
        }
        break;
    case 17:
    case 1:
        color=RGB15(15,0,0);
        for(y=keypos[n].top; y<keypos[n].bottom; y++) {
            for(x=keypos[n].left; x<keypos[n].right; x++) {
                backBuffer[x+y*256]=AlphaBlendFast(kbdBuffer[x+y*256], color);
            }
        }
        cpckeypressed[n]=2;
        break;
    case 2:
    case 18:
        color=RGB15(0,15,0);
        for(y=keypos[n].top; y<keypos[n].bottom; y++) {
            for(x=keypos[n].left; x<keypos[n].right; x++) {
                backBuffer[x+y*256]=~kbdBuffer[x+y*256]|0x8000;
                // backBuffer[x+y*256]=AlphaBlendFast(kbdBuffer[x+y*256], color);
            }
        }
        break;
    }
#endif
}

void DispDisk(core_crocods_t *core, int reading)
{

    dispIcon(core, 0, 0, 0, 0, 0);



#ifndef USE_CONSOLE
    int x,y;
    RECT r;
    u16 color;

    // SetRect(&r, 222,88,254,113);
    SetRect(&r, 230,1,254,33);

    switch(reading) {
    case 0:
        for(y=r.top; y<r.bottom; y++) {
            for(x=r.left; x<r.right; x++) {
                backBuffer[x+y*256]=kbdBuffer[x+y*256];
            }
        }
        break;
    case 1:
        color=RGB15(15,0,0);
        for(y=r.top; y<r.bottom; y++) {
            for(x=r.left; x<r.right; x++) {
                // backBuffer[x+y*256]=AlphaBlendFast(kbdBuffer[x+y*256], color);
                backBuffer[x+y*256]=~kbdBuffer[x+y*256]|0x8000;
            }
        }
        break;
    }
#endif
}


int nds_ReadKey(core_crocods_t *core)
{
    if (AutoType_Active(core)) {
        AutoType_Update(core);
    } else {
        u16 keys_pressed;
        static u16 oldkey;
        int n;

        //       scanKeys();
        //     keys_pressed = keysHeld();

        keys_pressed = core->ipc.keys_pressed;

        // memset(core->clav,0xFF,16);

        if (core->ipc.touchDown==1) {
            int x,y,n;

            x=core->ipc.touchXpx;
            y=core->ipc.touchYpx;

            /* if ((x>0) & (x<32) & (y>=25) & (y<=36)) {
               ExecuteMenu(core, ID_RESET, NULL);
               ipc.touchDown=0;
               }
             */

            if ( (x>=230) && (x<=254) && (y>=1) && (y<=33) ) { // 52
                core->inMenu=1;
            }

            for (n=0; n<NBCPCKEY; n++) {
                if ( (x>=keypos[n].left) && (x<=keypos[n].right) && (y>=keypos[n].top) && (y<=keypos[n].bottom) ) {
                    PressKey(core, n);
                    break;
                }
            }
        }

        if (core->keyEmul==3) {
            if ((keys_pressed & KEY_UP)==KEY_UP) {
                CPC_SetScanCode(core, keyown[0]);
            } else {
                CPC_ClearScanCode(core, keyown[0]);
            }

            if ((keys_pressed & KEY_DOWN)==KEY_DOWN) {
                CPC_SetScanCode(core, keyown[1]);
            } else {
                CPC_ClearScanCode(core, keyown[1]);
            }

            if ((keys_pressed & KEY_LEFT)==KEY_LEFT) {
                CPC_SetScanCode(core, keyown[2]);
            } else {
                CPC_ClearScanCode(core, keyown[2]);
            }

            if ((keys_pressed & KEY_RIGHT)==KEY_RIGHT) {
                CPC_SetScanCode(core, keyown[3]);
            } else {
                CPC_ClearScanCode(core, keyown[3]);
            }

            if ((keys_pressed & KEY_START)==KEY_START) {
                CPC_SetScanCode(core, keyown[4]);
            } else {
                CPC_ClearScanCode(core, keyown[4]);
            }

            if ((keys_pressed & KEY_A)==KEY_A) {
                CPC_SetScanCode(core, keyown[5]);
            } else {
                CPC_ClearScanCode(core, keyown[5]);
            }

            if ((keys_pressed & KEY_B)==KEY_B) {
                CPC_SetScanCode(core, keyown[6]);
            } else {
                CPC_ClearScanCode(core, keyown[6]);
            }

            if ((keys_pressed & KEY_X)==KEY_X) {
                CPC_SetScanCode(core, keyown[7]);
            } else {
                CPC_ClearScanCode(core, keyown[7]);
            }

            if ((keys_pressed & KEY_Y)==KEY_Y) {
                CPC_SetScanCode(core, keyown[8]);
            } else {
                CPC_ClearScanCode(core, keyown[8]);
            }
        }

        for(n=0; n<NBCPCKEY; n++) {
            if (cpckeypressed[n]!=0) {
                cpckeypressed[n]--;
                if(cpckeypressed[n]==0) {
                    Dispkey(n, 0);
                }
            }
        }

        oldkey = keys_pressed;
    }

    return 0;
}

void videoinit(void)
{
}

void nds_initBorder( core_crocods_t *core, int *_borderX, int *_borderY)
{
    core->borderX=_borderX;
    core->borderY=_borderY;
}

void nds_init(core_crocods_t *core)
{
    core->mustLeave = 0;

    core->icons=(u16*)malloc(448*320*2);

    ReadBackgroundGif16(core->icons, (unsigned char*)&icons_gif, icons_gif_len);


    core->Fmnbr=0;

    struct kmenu *id;

    core->usestylus=0;
    core->usestylusauto=1;
    core->usemagnum=0;
    core->hack_tabcoul=0;
    core->UpdateInk=1;

    core->Regs1=0;
    core->Regs2=0;
    core->Regs6=0;
    core->Regs7=0; // Utilisé par le resize automatique

    ExecInstZ80 = ExecInstZ80_orig;
    ResetZ80 = ResetZ80_orig;
    SetIRQZ80 = SetIRQZ80_orig;


    root.nbr=0;
    //    menuRomId=AddMenu(&root, "Autostart ROM", ID_MENU);
    menuDiskId=AddMenu(&root, "Load Disk", ID_MENU, 0, 0);
    AddMenu(menuDiskId, "Disk", ID_GREEN_MONITOR, 0, 0); // Tofix
    AddMenu(menuDiskId, "Autostart", ID_COLOR_MONITOR, 0,1); // Tofix
    AddMenu(menuDiskId, "Insert", ID_GREEN_MONITOR, 0, 2); // Tofix
    // FS_getFileList(addFile);

    id=AddMenu(&root, "Switch monitor", ID_MONITOR_MENU, 0, 0);
    AddMenu(id, "Color Monitor", ID_COLOR_MONITOR, 1, 1);
    AddMenu(id, "Green Monitor", ID_GREEN_MONITOR, 1, 0);

    id=AddMenu(&root, "Resize", ID_SCREEN_MENU, 0, 0);
    AddMenu(id, "Auto", ID_SCREEN_AUTO, 2, 2);
    AddMenu(id, "320x200", ID_SCREEN_320, 2, 3);
    AddMenu(id, "Overscan", ID_SCREEN_OVERSCAN, 2, 0);

    id=AddMenu(&root, "Pad emulation", ID_KEY_MENU, 0, 0);
    AddMenu(id, "Keyboard Emulation", ID_KEY_KEYBOARD, 3, 0);
    AddMenu(id, "Set to Joystick", ID_KEY_JOYSTICK, 3, 1);
    AddMenu(id, "Set to Keypad", ID_KEY_KEYPAD, 3, 2);

    //    keyMenu=AddMenu(id, "Redefine Keys", ID_MENU);
    //    AddMenu(keyMenu, "Up: XXXXXXXXXX", ID_REDEFINE_UP);
    //    AddMenu(keyMenu, "Down: XXXXXXXXXX", ID_REDEFINE_DOWN);
    //    AddMenu(keyMenu, "Left: XXXXXXXXXX", ID_REDEFINE_LEFT);
    //    AddMenu(keyMenu, "Right: XXXXXXXXXX", ID_REDEFINE_RIGHT);
    //    AddMenu(keyMenu, "A: XXXXXXXXXX", ID_REDEFINE_A);
    //    AddMenu(keyMenu, "B: XXXXXXXXXX", ID_REDEFINE_B);
    //    AddMenu(keyMenu, "X: XXXXXXXXXX", ID_REDEFINE_X);
    //    AddMenu(keyMenu, "Y: XXXXXXXXXX", ID_REDEFINE_Y);
    //    AddMenu(keyMenu, "START: XXXXXXXXXX", ID_REDEFINE_START);
    // AddMenu(keyMenu, "L: XXXXXXXXXX", ID_REDEFINE_L);
    // AddMenu(keyMenu, "R: XXXXXXXXXX", ID_REDEFINE_R);

    id=AddMenu(&root, "Debug", ID_MENU, 0, 0);
    AddMenu(id, "Display framerate", ID_DISPFRAMERATE, 4, 0);
    AddMenu(id, "Don't display framerate", ID_NODISPFRAMERATE, 4, 1);

    //    id=AddMenu(&root,"Hack", ID_MENU, 0, 0);
    //    AddMenu(id, "Only one ink refresh per frame: N", ID_HACK_TABCOUL, 5, 0);
    //    AddMenu(id, "Normal ink refresh per frame: N", ID_NOHACK_TABCOUL, 5, 0);

    //    AddMenu(&root, "Reset CPC", ID_RESET);

    id=AddMenu(&root, "State", ID_MENU, 6, 0);
    //    AddMenu(id, "Load State", ID_SAVESNAP, 6, 0);
    AddMenu(id, "Save State", ID_SAVESNAP, 6, 1);


    id = AddMenu(&root, "Action", ID_MENU, 7, 0);
    AddMenu(id, "Reset", ID_RESET, 7, 1);

    ExecuteMenu(core, ID_COLOR_MONITOR, NULL);
    //   ExecuteMenu(ID_SCREEN_AUTO, NULL);

    //   ExecuteMenu(core, ID_SCREEN_320, NULL);
    ExecuteMenu(core, ID_SCREEN_AUTO, NULL);
    //    ExecuteMenu(core, ID_KEY_JOYSTICK, NULL);
    ExecuteMenu(core, ID_KEY_KEYBOARD, NULL);
    ExecuteMenu(core, ID_NODISPFRAMERATE, NULL);
    //   ExecuteMenu(core, ID_HACK_TABCOUL, NULL);

    //    ExecuteMenu(core, ID_DEBUG_ENTER, NULL);

    strcpy(core->currentfile,"nofile");
}

void Autoexec(core_crocods_t *core)
{
    if (core->Fmnbr==0) {
        SetPalette(core, -1);
        return;
    }
    if (core->Fmnbr==1) {
        //   LireRom(FirstROM,1);
    }
}

int nds_video_unlock(void)
{
    return 1; // OK
}

int nds_video_lock(void)
{
    return 1; // OK
}

void nds_video_close(void)
{
}

void myconsoleClear(core_crocods_t *core)
{
    memset(core->consolestring,0,1024);
    core->consolepos=0;
}


void myprintf0(core_crocods_t *core, const char *fmt, ...)
{
    char tmp[512];

    va_list args;

    va_start(args, fmt);
    vsprintf(tmp,fmt,args);
    va_end(args);

    if (tmp[0]=='\n') {
        core->consolepos++;
        if (core->consolepos==8) {
            memcpy(core->consolestring,core->consolestring+128,1024-128);
            core->consolepos=7;
        }
    }

    memcpy(core->consolestring+core->consolepos*128, tmp, 128);
    core->consolestring[core->consolepos*128-1]=0;
}

void myprintf(const char *fmt, ...)
{
    char tmp[512];
    int n;

    va_list args;

    va_start(args, fmt);
    vsprintf(tmp,fmt,args);
    va_end(args);


    strncpy(msgbuf, tmp, 32);
    msgbuf[32]=0;
    msgframe=frame;
    for(n=(int)strlen(msgbuf); n<32; n++) {
        msgbuf[n]=' ';
    }


#ifdef USE_CONSOLE
    printf("%s\n", tmp);
#else

    if (backBuffer==NULL) {
        return;
    } else {
#ifdef USE_ALTERSCREEN
        int n;
        int width;
        int x,y;

        memcpy(consolestring+consolepos*128, tmp, 128);
        consolestring[consolepos*128-1]=0;

        if (!inMenu) {
            for(n=0; n<8; n++) {
                if (n<=consolepos) {
                    if (consolestring[n*128]==1) {
                        width = DrawText(backBuffer, fontred, 110, 5+n*10, consolestring+n*128+1);
                    } else {
                        width = DrawText(backBuffer, font, 110, n*10+5, consolestring+n*128);
                    }
                } else {
                    width = 0;
                }
                for(y=5+n*10; y<5+(n+1)*10; y++) {
                    for(x=110+width; x<253; x++) {
                        backBuffer[x+y*256]=RGB15((156>>3), (178>>3), (165>>3))|0x8000; // 156, 178, 165
                    }
                }
            }
        }
#endif

        consolepos++;
        if (consolepos==8) {
            memcpy(consolestring,consolestring+128,1024-128);
            consolepos=7;
        }
        // for(n=0;n<20;n++) swiWaitForVBlank();
    }
#endif
}

void ResetCPC(core_crocods_t *core)
{
    Keyboard_Reset(core);
    WriteVGA(core, 0, 0x89 );
    ResetZ80(core);
    ResetCRTC(core);
    ResetPPI(core);

    Reset8912(core);
}

u16 computeColor(int x, int y, int frame) {

    u8 Sinus[256]={131,134,137,141,144,147,150,153,156,159,162,165,168,171,174,177,180,183,186,188,191,194,196,199,202,204,207,209,212,214,216,219,221,223,225,227,229,231,233,234,236,238,239,241,242,244,245,246,247,249,250,250,251,252,253,254,254,255,255,255,255,255,255,255,255,255,255,255,255,255,254,254,253,252,251,250,250,249,247,246,245,244,242,241,239,238,236,234,233,231,229,227,225,223,221,219,216,214,212,209,207,204,202,199,196,194,191,188,186,183,180,177,174,171,168,165,162,159,156,153,150,147,144,141,137,134,131,128,125,122,119,115,112,109,106,103,100,97,94,91,88,85,82,79,76,73,70,68,65,62,60,57,54,52,49,47,44,42,40,37,35,33,31,29,27,25,23,22,20,18,17,15,14,12,11,10,9,7,6,6,5,4,3,2,2,1,1,1,0,0,0,0,0,0,0,1,1,1,2,2,3,4,5,6,6,7,9,10,11,12,14,15,17,18,20,22,23,25,27,29,31,33,35,37,40,42,44,47,49,52,54,57,60,62,65,68,70,73,76,79,82,85,88,91,94,97,100,103,106,109,112,115,119,122,125,128};


    u8 r,g,b;

    x=0;
    y=y*4;

    y=y/2;
    frame=frame/2;

    u8 pal = (Sinus[(x+y)%256] + Sinus[Sinus[(frame+x)%256]] + Sinus[Sinus[(frame+y)%256]])%256;

    r = Sinus[(pal+142)%256];
    g = Sinus[(pal+112)%256];
    b = Sinus[(pal+74)%256];

    return RGB15(r,g,b);
}

void cpcprint16(u16 *MemBitmap, u32 MemBitmap_width, int x, int y, char *pchStr, u16 bColor, int multi, char transparent)
{
    static int frame=0;

    int iLen, iIdx, iRow, iCol;
    u8 bRow;
    u16 *pdwAddr;
    int n;
    int mx, my, mz;

    frame++;

    u16 backgroundColor = RGB15(0,0,0x7F);

    if (bColor==1) {
        bColor = RGB15(0xFF,0xFF,0);
    }

    pdwAddr = (u16*)MemBitmap + (y*MemBitmap_width) + x;

    iLen = (int)strlen(pchStr); // number of characters to process
    for (n = 0; n < iLen; n++) {
        u16 *pdwLine;
        iIdx = (int)pchStr[n]; // get the ASCII value
        if ((iIdx < FNT_MIN_CHAR) || (iIdx > FNT_MAX_CHAR)) { // limit it to the range of chars in the font
            iIdx = FNT_BAD_CHAR;
        }
        iIdx -= FNT_MIN_CHAR; // zero base the index
        pdwLine = pdwAddr; // keep a reference to the current screen position
        for (iRow = 0; iRow < FNT_CHAR_HEIGHT; iRow++) { // loop for all rows in the font character
            for(my = 0; my < multi; my++) {
                u16 *pdPixel;
                char first=1;

                pdPixel = pdwLine;
                bRow = bFont[iIdx]; // get the bitmap information for one row
                for (iCol = 0; iCol < 8; iCol++) { // loop for all columns in the font character


                    for (mx=0; mx<multi; mx++) {


                    // bColor = computeColor(iCol + n*8, iRow, frame);
                    bColor = computeColor((n)*8*multi+iCol*multi+mx, multi+iRow*multi+my, frame);

// transparent=0;      backgroundColor = bColor;

                        if (bRow & 0x80) {

                            // if (multi>1) {

                            //     for (mz=1; mz<=2; mz++) {

                            //         if (*(pdPixel-MemBitmap_width*mz)!=bColor) {
                            //             *(pdPixel-MemBitmap_width*mz)=backgroundColor;
                            //         }
                            //         if (*(pdPixel-mz)!=bColor) {
                            //             *(pdPixel-mz)=backgroundColor;
                            //         }

                            //         *(pdPixel+MemBitmap_width*mz)=backgroundColor;
                            //         *(pdPixel+mz)=backgroundColor;
                            //     }
                            // }

                            *pdPixel = bColor;
                            first=0;

                        } else if (!transparent) {
                            *pdPixel =  backgroundColor;
                        }

                        pdPixel++;
                    }

                    bRow <<= 1; // advance to the next bit
                }
                pdwLine += MemBitmap_width;
            }
            iIdx += FNT_CHARS; // advance to next row in font data
        }
        pdwAddr += FNT_CHAR_WIDTH * multi; // set screen address to next character position
    }
}


void cpcprint(core_crocods_t *core, int x, int y, char *pchStr, u16 bColor)
{
    cpcprint16(core->MemBitmap, core->MemBitmap_width, x, y, pchStr, bColor, 1, 0);
}

void dispIcon(core_crocods_t *core, int i, int j, int dispiconX, int dispiconY, char select) {

    int x,y;

    //    u16 *pdwAddr = (u16*)core->MemBitmap + ((j*32)*core->MemBitmap_width) + (224+x) - (i*32);

    //    printf("%d %d\n", i, j);

    u16 *pdwAddr = (u16*)core->MemBitmap + ((j*32)*core->MemBitmap_width) + (i*32);

    if (core->lastMode==2) {
        pdwAddr += ((j*32)*core->MemBitmap_width);
    }


    for(y=0; y<32; y++) {
        u16 *pdPixel;

        pdPixel = pdwAddr;

        for(x=0; x<32; x++) {
            u16 car;
            car=core->icons[(x+dispiconX*32)+(y+dispiconY*32)*448];
            if (car!=33840) {
                if (select!=0) {
                    car=car/2;
                }
                // if (car!=transpcolor) {
                *pdPixel = car;
            }
            pdPixel++;
            // }
        }

        pdwAddr += core->MemBitmap_width;

        if (core->lastMode==2) {
            pdwAddr += core->MemBitmap_width;
        }
    }
}


void selectedMenu(core_crocods_t *core, struct kmenu *currentMenu) {
    if (currentMenu!=NULL) {
        core->selectedMenu = currentMenu;
    }
}

void selectMenuUp(core_crocods_t *core) {
    if (core->selectedMenu->parent->parent==NULL) {
        selectedMenu(core, core->selectedMenu->lastchild);
    } else {
        if (core->selectedMenu->previousSibling==NULL) {
            core->selectedMenu = core->selectedMenu->parent;
        } else {
            selectedMenu(core, core->selectedMenu->previousSibling);
        }
    }
}

void selectMenuDown(core_crocods_t *core) {
    if (core->selectedMenu->parent->parent==NULL) {
        selectedMenu(core, core->selectedMenu->firstchild);
    } else {
        if (core->selectedMenu->nextSibling==NULL) {
            core->selectedMenu = core->selectedMenu->parent;
        } else {
            selectedMenu(core, core->selectedMenu->nextSibling);
        }
    }
}

void DispIcons(core_crocods_t *core)
{
    int dirY=0;

    if (core->selectedMenu==NULL) {
        core->selectedMenu = root.firstchild;
    }

    //    if (core->iconMenuX!=-1) {
    //        cpcprint(core, 0,40, "     ", 255);
    //    }

    if (CPC_isScanCode(core, CPC_CURSOR_LEFT)) {
        CPC_ClearScanCode(core, CPC_CURSOR_LEFT);
        if (core->selectedMenu->parent->parent==NULL) {
            selectedMenu(core, core->selectedMenu->previousSibling);
        } else {
            selectedMenu(core, core->selectedMenu->parent->previousSibling);
        }
    }
    if (CPC_isScanCode(core, CPC_CURSOR_RIGHT)) {
        CPC_ClearScanCode(core, CPC_CURSOR_RIGHT);
        if (core->selectedMenu->parent->parent==NULL) {
            selectedMenu(core, core->selectedMenu->nextSibling);
        } else {
            selectedMenu(core, core->selectedMenu->parent->nextSibling);
        }
    }
    if (CPC_isScanCode(core, CPC_CURSOR_UP)) {
        dirY = -1;
        CPC_ClearScanCode(core, CPC_CURSOR_UP);
        selectMenuUp(core);
    }
    if (CPC_isScanCode(core, CPC_CURSOR_DOWN)) {
        dirY = 1;
        CPC_ClearScanCode(core, CPC_CURSOR_DOWN);
        selectMenuDown(core);
    }
    if (CPC_isScanCode(core, CPC_RETURN)) {
        CPC_ClearScanCode(core, CPC_RETURN);
        if (core->selectedMenu->id != ID_MENU) {
            ExecuteMenu(core, core->selectedMenu->id, NULL);
            selectedMenu(core, core->selectedMenu->parent);
        }
    }

    struct kmenu *currentMenu;
    int i;
    currentMenu = root.firstchild;

    for(i=0; i<root.nbr; i++) {
        int z;
        u16 transpcolor;

        transpcolor = 64543; // RGB15(31,0,31);
        z=0;
        switch(i) {
        case 0:
            z=ID_FILE;
            break;
        case 1:
            z= (core->lastcolour==0) ? ID_GREEN_MONITOR : ID_COLOR_MONITOR;
            break;
        case 2:
            if (core->resize==1) { // AUTO
                z=ID_SCREEN_AUTO;
            } else if (core->resize==2) { // 320
                z=ID_SCREEN_320;
            } else if (core->resize==3) { // NO-RESIZE
                z=ID_SCREEN_NORESIZE; // Not used
            } else if (core->resize==4) { // OVERSCAN
                z=ID_SCREEN_OVERSCAN;
            }
            break;
        case 3:
            if (core->keyEmul==2) {
                z=ID_KEY_KEYBOARD;
            } else if (core->keyEmul==3) {
                if (keyown[0]==CPC_JOY_UP) {
                    z=ID_KEY_JOYSTICK;
                } else if (keyown[0]==CPC_CURSOR_UP) {
                    z=ID_KEY_KEYPAD;
                }
            }
            break;
        case 4:
            z= core->dispframerate ? ID_DISPFRAMERATE : ID_NODISPFRAMERATE;
            break;
        case 5:
            //                z= (core->multikeypressed==1) ? 1 : 0;
            break;
        case 6:
            z=0;
            //                if (HaveSlotSnap(currentfile, currentsnap)) {
            //                    nbr=4;
            //                } else {
            //                }
            break;
        }

        //        int dispicon;
        //
        //        if (i==0) {
        //            //            dispicon = (Image==0) ? 4 : 0;
        //            dispicon = 0;
        //        } else {
        //            dispicon = z;
        //        }

        //        dispIcon(core, i, 0, i, dispicon, 0);

        struct kmenu *currentSubMenu = currentMenu->firstchild;
        char displayIcon = 0;

        do {
            if (currentSubMenu->id == z) {
                dispIcon(core, i, 0, currentSubMenu->x, currentSubMenu->y,  (core->selectedMenu==currentMenu));
                displayIcon = 1;
                break;
            }

            currentSubMenu = currentSubMenu->nextSibling;
        } while (currentSubMenu!=NULL);

        if (!displayIcon) {
            dispIcon(core, i, 0, currentMenu->x, currentMenu->y,  (core->selectedMenu==currentMenu));

        }

        if ((core->selectedMenu->parent==currentMenu) || (core->selectedMenu==currentMenu)) {
            int j, cur, zicon;

            zicon=z;
            //            nbricon=nbr;


            cur=0;
            j=1;

            currentSubMenu = currentMenu->firstchild;

            do {

                if (currentSubMenu->id == z) {
                    cur++;

                    if (currentSubMenu == core->selectedMenu) {
                        if (dirY==1) {
                            selectMenuDown(core);
                        } else if (dirY==-1) {
                            selectMenuUp(core);
                        }
                    }

                    currentSubMenu = currentSubMenu->nextSibling;


                    if (currentSubMenu==NULL) {
                        break;
                    }
                }

                dispIcon(core, i, j, currentSubMenu->x, currentSubMenu->y, (currentSubMenu == core->selectedMenu));


                cur++;

                j++;
                currentSubMenu = currentSubMenu->nextSibling;
            } while (currentSubMenu!=NULL);

        }

        currentMenu = currentMenu->nextSibling;
    }
}

/*
   void LoopMenu(core_crocods_t *core, struct kmenu *parent)
   {
   PauseSound();

   menubuffer=(u16*)malloc(256*192*2);



   if (parent==NULL) {
   iconAutoInsert=-1;
   LoadMenu(&root);
   } else {
   LoadMenu(parent);
   }
   SetPalette(-1);


 #ifndef USE_CONSOLE
   dmaCopy(menubuffer, backBuffer, 256*192*2);

 #ifdef USE_ALTERSCREEN
   int x,y,width,n;

   for(n=0;n<8;n++) {
   if (n<=consolepos) {
   if (consolestring[n*128]==1) {
   width = DrawText(backBuffer, fontred, 110, 5+n*10, consolestring+n*128+1);
   } else {
   width = DrawText(backBuffer, font, 110, n*10+5, consolestring+n*128);
   }
   } else {
   width = 0;
   }
   for(y=5+n*10;y<5+(n+1)*10;y++) {
   for(x=110+width;x<253;x++) {
   backBuffer[x+y*256]=RGB15((156>>3), (178>>3), (165>>3))|0x8000;  // 156, 178, 165
   }
   }
   }
 #endif
 #endif

   free(menubuffer);

   DispIcons();

   inMenu=0;

   if (dispframerate==0) {
   cpcprint16i(0,192-8, "                                  ", 255);
   }

   PlaySound();
   }

 */


// retour 1 si on doit revenir a l'emulator
// retour 0 si on doit revenir au parent

/*
   int LoadMenu(core_crocods_t *core, struct kmenu *parent)
   {
   // RECT r, ralbum;
   struct kmenu *first;

   if (parent->nbr==0) {
   return 1;
   }

   // SetRect(&ralbum,248,4,253,85);   // 4, 110


   while(1) {
   int i,n;
   struct kmenu *selected=NULL;
   char *bufpos[7];

   swiWaitForVBlank();
   myconsoleClear();

   first=parent->firstchild;
   i=0;
   n=0;

   memset(bufpos, 0, sizeof(u8*)*7);

   while(1) {
   if ( ((i-parent->pos+3)>=0) & ((i-parent->pos+3)<7) ) {
   bufpos[i-parent->pos+3]=first->title;
   }
   if (i==parent->pos) {
   selected=first;
   }
   i++;
   first=first->next;
   if (first==NULL) {
   break;
   }
   }
 #ifndef USE_CONSOLE
   dmaCopy(menubufferlow, backBuffer, 256*192*2);

   {
   u32 n;
   char buffer[256];



   for(n=0;n<7;n++) {
   if (bufpos[n]!=NULL) {
   u16 color;
   color = RGB15((31 - abs(3-n)*6), (31 - abs(3-n)*6), (31 - abs(3-n)*6)) | 0x8000;
   DrawTextLeft(backBuffer, filefont, color, 2, (n*20)+26, bufpos[n]);
   }
   }
   sprintf(buffer,"\x03 Select \x04 Back");
   DrawTextCenter(backBuffer, filefont, RGB15(0,0,31) | 0x8000,  176, buffer);
   if (parent!=&root) {
   DrawTextLeft(backBuffer, filefont, RGB15(31,31,0) | 0x8000, 15, 1, parent->title);
   } else {
   DrawTextLeft(backBuffer, filefont, RGB15(31,31,0) | 0x8000, 15, 1, "Root");
   }
   }
 #endif



   keys_pressed = MyReadKey();

   if ((keys_pressed & KEY_UP)==KEY_UP) {
   parent->pos--;
   if(parent->pos<0) {
   parent->pos=parent->nbr-1;
   parent->beg=parent->nbr-7;
   }
   while(parent->pos-parent->beg<0) {
   parent->beg--;
   }
   }
   if ((keys_pressed & KEY_DOWN)==KEY_DOWN)  {
   parent->pos++;
   if (parent->pos>=parent->nbr) {
   parent->pos=0;
   parent->beg=0;
   }
   while(parent->pos-parent->beg>=7) {
   parent->beg++;
   }
   }
   if ((keys_pressed & KEY_A)==KEY_A) {
   int retour;
   if (selected->firstchild!=NULL) {
   if (LoadMenu(selected)==1) {
   return 1;
   }
   }
   retour=ExecuteMenu(selected->id, selected);
   if (retour!=2) {
   return retour;
   }
   }
   if ((keys_pressed & KEY_L)==KEY_L) {
   parent->pos-=8;
   if(parent->pos<0) {
   parent->pos=parent->nbr-1;
   parent->beg=parent->nbr-7;
   }
   while(parent->pos-parent->beg<0) {
   parent->beg--;
   }
   }
   if ((keys_pressed & KEY_R)==KEY_R)  {
   parent->pos+=7;
   if (parent->pos>=parent->nbr) {
   parent->pos=0;
   parent->beg=0;
   }
   while(parent->pos-parent->beg>=7) {
   parent->beg++;
   }
   }

   if ((keys_pressed & KEY_B)==KEY_B) {
   return 0;
   }
   }
   }
 */

u8 *MyAlloc(int size, char *title)
{
    u8 *mem;
    mem=(u8*)malloc(size);
    if (mem==NULL) {
        myprintf("Allocate %s (%d): FAILED", title, size);
        // while(1) swiWaitForVBlank();
    } else {
        //    myprintf("Allocate %s (%d): OK", title, size);
    }
    return mem;
}

