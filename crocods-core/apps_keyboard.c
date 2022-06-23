#include "platform.h"

void Dispkey(core_crocods_t *core, CPC_KEY n, int status);
void DispScanCode(core_crocods_t *core, CPC_SCANCODE n, int status);
void PressKey(core_crocods_t *core, CPC_KEY n);

#define PG_LBMASK565 0xF7DE
#define PG_LBMASK555 0x7BDE

#define AlphaBlendFast(pixel, backpixel) (((((pixel) & PG_LBMASK565) >> 1) | (((backpixel) & PG_LBMASK565) >> 1)))

#define RGB565(R, G, B)                  ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

static u8 bit_values[8] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

int keymap[NBCPCKEY] = {
     CPC_FDOT,
     CPC_F1,
     CPC_F2,
     CPC_F3,
     CPC_F4,
     CPC_F5                   ,
     CPC_F6                   ,
     CPC_F7                   ,
     CPC_F8                   ,
     CPC_F9                   ,
     CPC_F0                   ,
     CPC_CURSOR_UP            ,
     CPC_CURSOR_LEFT          ,
     CPC_CURSOR_DOWN          ,
     CPC_CURSOR_RIGHT         ,

     CPC_ESC                  ,
     CPC_1                    ,
     CPC_2                    ,
     CPC_3                    ,
     CPC_4                    ,
     CPC_5                    ,
     CPC_6                    ,
     CPC_7                    ,
     CPC_8                    ,
     CPC_9                    ,
     CPC_ZERO                 ,
     CPC_MINUS                ,
     CPC_HAT                  ,
     CPC_CLR                  ,
     CPC_DEL                  ,

     CPC_TAB                  , //
     CPC_Q                    ,
     CPC_W                    ,
     CPC_E                    ,
     CPC_R                    ,
     CPC_T                    ,
     CPC_Y                    ,
     CPC_U                    ,
     CPC_I                    ,
     CPC_O                    ,
     CPC_P                    ,
     CPC_AT                   ,
     CPC_OPEN_SQUARE_BRACKET  ,
     CPC_RETURN               ,

     CPC_CAPS_LOCK            , //
     CPC_A                    ,
     CPC_S                    ,
     CPC_D                    ,
     CPC_F                    ,
     CPC_G                    ,
     CPC_H                    ,
     CPC_J                    ,
     CPC_K                    ,
     CPC_L                    ,
     CPC_COLON                ,
     CPC_SEMICOLON            ,
     CPC_CLOSE_SQUARE_BRACKET ,

     CPC_SHIFT                , //
     CPC_Z                    ,
     CPC_X                    ,
     CPC_C                    ,
     CPC_V                    ,
     CPC_B                    ,
     CPC_N                    ,
     CPC_M                    ,
     CPC_COMMA                ,
     CPC_DOT                  ,
     CPC_FORWARD_SLASH        ,
     CPC_BACKSLASH            ,
     CPC_SHIFT                ,

     CPC_CONTROL              ,
     CPC_COPY                 ,
     CPC_SPACE                ,
     CPC_SMALL_ENTER
};

RECT keypos[NBCPCKEY] = {
    { 0,   51,    14,     72                             }, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
    { 15,  51,    33,     72                             }, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
    { 34,  51,    52,     72                             }, // 17    0x40 | MOD_CPC_SHIFT,   // CPC_0
    { 53,  51,    70,     72                             }, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
    { 71,  51,    87,     72                             }, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
    { 88,  51,    104,    72                             }, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
    { 105, 51,    121,    72                             }, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
    { 122, 51,    138,    72                             }, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
    { 139, 51,    155,    72                             }, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
    { 156, 51,    172,    72                             }, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
    { 173, 51,    189,    72                             }, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
    { 190, 51,    206,    72                             }, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
    { 207, 51,    223,    72                             }, //    0x50 | MOD_CPC_SHIFT,   // CPC_LEFT
    { 224, 51,    240,    72                             }, //    0x41 | MOD_CPC_SHIFT,   // CPC_UP
    { 241, 51,    255,    72                             }, // CP_RIGHT

    { 0,   73,    14,     94                             }, // (0)
    { 15,  73,    33,     94                             }, //    0x80 | MOD_CPC_SHIFT,   // CPC_1
    { 34,  73,    52,     94                             }, //    0x81 | MOD_CPC_SHIFT,   // CPC_2
    { 53,  73,    70,     94                             }, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
    { 71,  73,    87,     94                             }, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
    { 88,  73,    104,    94                             }, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
    { 105, 73,    121,    94                             }, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
    { 122, 73,    138,    94                             }, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
    { 139, 73,    155,    94                             }, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
    { 156, 73,    172,    94                             }, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
    { 173, 73,    189,    94                             }, // (10)  0x40 | MOD_CPC_SHIFT,   // CPC_0
    { 190, 73,    206,    94                             }, //    0x70 | MOD_CPC_SHIFT,   // CPC_=
    { 207, 73,    223,    94                             }, //    0x61 | MOD_CPC_SHIFT,   // CPC_LAMBDA
    { 224, 73,    240,    94                             }, //    0x60 | MOD_CPC_SHIFT,   // CPC_CLR
    { 241, 73,    256,    94                             }, //    0x51 | MOD_CPC_SHIFT,   // CPC_DEL

    { 0,   95,    19,     116                            },
    { 20,  95,    38,     116                            }, //    0x83,                   // CPC_a
    { 39,  95,    57,     116                            }, //    0x73,                   // CPC_z
    { 58,  95,    76,     116                            }, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
    { 77,  95,    95,     116                            }, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
    { 96,  95,    114,    116                            }, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
    { 115, 95,    133,    116                            }, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
    { 134, 95,    152,    116                            }, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
    { 153, 95,    171,    116                            }, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
    { 172, 95,    190,    116                            }, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
    { 191, 95,    207,    116                            }, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
    { 208, 95,    224,    116                            }, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
    { 225, 95,    241,    116                            }, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
    { 242, 95,    256,    138                            }, //    0x22,                   // CPC_RETURN

    { 0,   117,   21,     138                            },
    { 22,  117,   40,     138                            }, //    0x83,                   // CPC_A
    { 41,  117,   59,     138                            }, //    0x73,                   // CPC_S
    { 60,  117,   78,     138                            }, //    0x71 | MOD_CPC_SHIFT,   // CPC_D
    { 79,  117,   97,     138                            }, //    0x70 | MOD_CPC_SHIFT,   // CPC_F
    { 98,  117,   116,    138                            }, //    0x61 | MOD_CPC_SHIFT,   // CPC_G
    { 117, 117,   135,    138                            }, //    0x60 | MOD_CPC_SHIFT,   // CPC_H
    { 136, 117,   154,    138                            }, //    0x51 | MOD_CPC_SHIFT,   // CPC_J
    { 155, 117,   173,    138                            }, //    0x50 | MOD_CPC_SHIFT,   // CPC_K
    { 174, 117,   190,    138                            }, //    0x41 | MOD_CPC_SHIFT,   // CPC_L
    { 191, 117,   207,    138                            }, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
    { 208, 117,   224,    138                            }, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
    { 225, 117,   241,    138                            }, //    0x61 | MOD_CPC_SHIFT,   // CPC_5

    { 0,   139,   28,     160                            }, // SHIFT
    { 29,  139,   47,     160                            }, //    0x81 | MOD_CPC_SHIFT,   // CPC_2
    { 48,  139,   66,     160                            }, //    0x71 | MOD_CPC_SHIFT,   // CPC_3
    { 67,  139,   85,     160                            }, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
    { 86,  139,   104,    160                            }, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
    { 105, 139,   123,    160                            }, //    0x60 | MOD_CPC_SHIFT,   // CPC_6
    { 124, 139,   142,    160                            }, //    0x51 | MOD_CPC_SHIFT,   // CPC_7
    { 143, 139,   161,    160                            }, //    0x50 | MOD_CPC_SHIFT,   // CPC_8
    { 162, 139,   178,    160                            }, //    0x41 | MOD_CPC_SHIFT,   // CPC_9
    { 179, 139,   195,    160                            }, //    0x40 | MOD_CPC_SHIFT,   // CPC_0
    { 196, 139,   212,    160                            }, //    0x70 | MOD_CPC_SHIFT,   // CPC_4
    { 213, 139,   229,    160                            }, //    0x61 | MOD_CPC_SHIFT,   // CPC_5
    { 230, 139,   256,    160                            }, //    0x60 | MOD_CPC_SHIFT,   // CPC_6

    { 0,   161,   57,     182                            }, //    0x55,                   // CPC_j
    { 58,  161,   95,     182                            }, //    0x55,                   // CPC_j
    { 96,  161,   207,    182                            }, //    0x55,                   // CPC_j
    { 208, 161,   256,    182                            } //    0x55,                   // CPC_j
};

void DispKeyboard(core_crocods_t *core, u16 keys_pressed0)
{
    static int key = 0;

    int y;

    u16 keys_pressed = appli_begin(core, keys_pressed0);

    core->overlayBitmap_width = 256;
    core->overlayBitmap_height = 192;
    core->overlayBitmap_posx = (320 - 256) / 2;
    core->overlayBitmap_posy = (240 - 192) / 2;
    core->overlayBitmap_center = 1;

    u16 *pdwAddr = core->overlayBitmap; // + ((j * 32) * core->MemBitmap_width) + (i * 32);

    for (y = 0; y < 192; y++) {
        memcpy(pdwAddr, core->keyboard + y * 256, 256 * 2);
        pdwAddr += 320;
    }

    if (((keys_pressed & KEY_B) == KEY_B) || ((keys_pressed & KEY_R) == KEY_R)) {
        if (core->redefineKey) {
            core->redefineKey = 0;
            core->redefineKeyRetour = -1;
            return;
        }
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;
        // ExecuteMenu(core, ID_MENU_EXIT, NULL);
    }

    Dispkey(core, key, 2); // ou 2

    if ((keys_pressed & KEY_LEFT) == KEY_LEFT) {
        key--;
        if (key < 0) {
            key = NBCPCKEY - 1;
        }
    }
    if ((keys_pressed & KEY_RIGHT) == KEY_RIGHT) {
        key++;
        if (key >= NBCPCKEY) {
            key = 0;
        }
    }
    if ((keys_pressed & KEY_DOWN) == KEY_DOWN) {
        int nkey = key + 1;
        while (nkey != NBCPCKEY) {
            if ((keypos[nkey].left >= keypos[key].left) && (keypos[nkey].top > keypos[key].top)) {
                break;
            }
            nkey++;
        }
        if (nkey != NBCPCKEY) {
            key = nkey;
        }
    }
    if ((keys_pressed & KEY_UP) == KEY_UP) {
        int nkey = key;
        while (nkey != -1) {
            if ((keypos[nkey].left <= keypos[key].left) && (keypos[nkey].top < keypos[key].top)) {
                break;
            }
            nkey--;
        }
        if (nkey != -1) {
            key = nkey;
        }
    }

    if (core->redefineKey) {
        if ((keys_pressed & KEY_A) == KEY_A) {      // Don't use repeat value
            core->redefineKey = 0;
            core->redefineKeyRetour = key;
        }
    } else {
        if ((keys_pressed0 & KEY_A) == KEY_A) {
            PressKey(core, key);
        }
    }
} /* DispKeyboard */

void DispScanCode(core_crocods_t *core, CPC_SCANCODE scancode, int status)
{
    int n;

    for (n = 0; n < NBCPCKEY; n++) {
        if (keymap[n] == scancode) {
            Dispkey(core, n, status);
        }
    }
}

// 1: active
// 2: on
// 0: off

void Dispkey(core_crocods_t *core, CPC_KEY n, int status)
{
    int x, y;
    u16 color;



    u16 *pdwAddr = core->overlayBitmap; // + ((j * 32) * core->MemBitmap_width) + (i * 32);

    switch (status) {
        case 0:
        case 16:
            for (y = keypos[n].top; y < keypos[n].bottom; y++) {
                for (x = keypos[n].left; x < keypos[n].right; x++) {
                    pdwAddr[x + y * 320] = core->keyboard[x + y * 256];
                }
            }
            break;
        case 17:
        case 1:
            color = RGB565(15, 0, 0);
            for (y = keypos[n].top; y < keypos[n].bottom; y++) {
                for (x = keypos[n].left; x < keypos[n].right; x++) {
                    pdwAddr[x + y * 320] = AlphaBlendFast(core->keyboard[x + y * 256], color);
                }
            }
            break;
        case 2:
        case 18:
            color = RGB565(0, 15, 0);
            for (y = keypos[n].top; y < keypos[n].bottom; y++) {
                for (x = keypos[n].left; x < keypos[n].right; x++) {
                    pdwAddr[x + y * 320] = ~core->keyboard[x + y * 256] | 0x8000;
                    // backBuffer[x+y*256]=AlphaBlendFast(kbdBuffer[x+y*256], color);
                }
            }
            break;
    } /* switch */
} /* Dispkey */

int shifted = 0;
int ctrled = 0;
int copyed = 0;

void PressKey(core_crocods_t *core, CPC_KEY n)
{
    CPC_SCANCODE cpc_scancode;

    cpc_scancode = keymap[n];

    Dispkey(core, n, 1);

    if (shifted) {
        DispScanCode(core, CPC_SHIFT, 0 | 16);
        shifted = 0;
        core->clav[0x25 >> 4] &= ~bit_values[0x25 & 7]; // key needs to be SHIFTed
    }
    if (ctrled) {
        DispScanCode(core, CPC_CONTROL, 0);
        ctrled = 0;
        core->clav[0x27 >> 4] &= ~bit_values[0x27 & 7]; // CONTROL key is held down
    }
    if (copyed) {
        DispScanCode(core, CPC_COPY, 0);
        copyed = 0;
    }

    // mydebug("Send %d (%d,%d,%d,%d)\n", n, shifted, ctrled, copyed, cpc_scancode);

    core->clav[(u8)cpc_scancode >> 3] &= ~bit_values[(u8)cpc_scancode & 7];

    switch (cpc_scancode) {
        case CPC_SHIFT:
            if (shifted) {
                DispScanCode(core, cpc_scancode, 0 | 16);
                shifted = 0;
            } else {
                DispScanCode(core, cpc_scancode, 1 | 16);
                shifted = 1;
            }
            break;
        case CPC_CONTROL:
            if (ctrled) {
                DispScanCode(core, cpc_scancode, 0 | 16);
                ctrled = 0;
            } else {
                DispScanCode(core, cpc_scancode, 1 | 16);
                ctrled = 1;
            }
            break;
        case CPC_COPY:
            if (copyed) {
                DispScanCode(core, cpc_scancode, 0 | 16);
                copyed = 0;
            } else {
                DispScanCode(core, cpc_scancode, 1 | 16);
                copyed = 1;
            }
            break;
        default:
            break;
    } /* switch */
} /* PressKey */

char CPC_isScanCode(core_crocods_t *core, CPC_SCANCODE cpc_scancode)
{
    if (core->clav[(u8)cpc_scancode >> 3] & bit_values[(u8)cpc_scancode & 7])
        return 0;
    return 1;
}

void CPC_SetScanCode(core_crocods_t *core, CPC_SCANCODE cpc_scancode)
{
    core->clav[(u8)cpc_scancode >> 3] &= ~bit_values[(u8)cpc_scancode & 7];
}

void CPC_ClearScanCode(core_crocods_t *core, CPC_SCANCODE cpc_scancode)
{
    core->clav[(u8)cpc_scancode >> 3] |= bit_values[(u8)cpc_scancode & 7];
    // DispScanCode(core, cpc_scancode, 0);
}
