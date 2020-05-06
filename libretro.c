// TEst
#include "libretro.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/time.h>

#include "deps/ziptool.h"

#include "crocods-core/plateform.h"
#include "crocods-core/gif.h"

extern const unsigned char icons_gif[];
extern const unsigned char cpc6128_bin[];
extern const unsigned char romdisc_bin[];

#define maxByCycle 400 // 50 fois par frame

bool loadGame(void);
void loadDisk(BOOL autoStart);
void loadSnapshot(void);

void updateFromEnvironnement();

void guestScreenDraw(core_crocods_t *core);
void guestInit(void);

char Core_Key_Sate[512];
char Core_old_Key_Sate[512];


static int KeySymToCPCKey[RETROK_LAST];

retro_log_printf_t log_cb;
retro_video_refresh_t video_cb;

static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;

retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

struct CrocoKeyMap {
    unsigned port;
    unsigned index;

    CPC_SCANCODE scanCode;
} crocokeymap[] = {
    {0, RETRO_DEVICE_ID_JOYPAD_A,      CPC_JOY_FIRE1     },
    {0, RETRO_DEVICE_ID_JOYPAD_B,      CPC_JOY_FIRE2     },
    {0, RETRO_DEVICE_ID_JOYPAD_UP,     CPC_JOY_UP        },
    {0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  CPC_JOY_RIGHT     },
    {0, RETRO_DEVICE_ID_JOYPAD_LEFT,   CPC_JOY_LEFT      },
    {0, RETRO_DEVICE_ID_JOYPAD_DOWN,   CPC_JOY_DOWN      },
    {0, RETRO_DEVICE_ID_JOYPAD_X,      CPC_NIL           },
    {0, RETRO_DEVICE_ID_JOYPAD_Y,      CPC_NIL           }, // 7
    {0, RETRO_DEVICE_ID_JOYPAD_L,      CPC_NIL           },
    {0, RETRO_DEVICE_ID_JOYPAD_R,      CPC_NIL           },
    {0, RETRO_DEVICE_ID_JOYPAD_SELECT, CPC_SPACE         },
    {0, RETRO_DEVICE_ID_JOYPAD_START,  CPC_RETURN        }, // 11

    {1, RETRO_DEVICE_ID_JOYPAD_A,      CPC_SPACE         },
    {1, RETRO_DEVICE_ID_JOYPAD_B,      CPC_SPACE         },
    {1, RETRO_DEVICE_ID_JOYPAD_UP,     CPC_CURSOR_UP     },
    {1, RETRO_DEVICE_ID_JOYPAD_RIGHT,  CPC_CURSOR_RIGHT  },
    {1, RETRO_DEVICE_ID_JOYPAD_LEFT,   CPC_CURSOR_LEFT   },
    {1, RETRO_DEVICE_ID_JOYPAD_DOWN,   CPC_CURSOR_DOWN   },
    {1, RETRO_DEVICE_ID_JOYPAD_X,      CPC_NIL           },
    {1, RETRO_DEVICE_ID_JOYPAD_Y,      CPC_NIL           },
    {1, RETRO_DEVICE_ID_JOYPAD_L,      CPC_NIL           },
    {1, RETRO_DEVICE_ID_JOYPAD_R,      CPC_NIL           },
    {1, RETRO_DEVICE_ID_JOYPAD_SELECT, CPC_SPACE         },
    {1, RETRO_DEVICE_ID_JOYPAD_START,  CPC_RETURN        }

};

// Crocods variable

core_crocods_t gb;
int bx, by;

u8 *disk = NULL;
u32 diskLength;

u8 *snapshot = NULL;
u32 snapshotLength;

char autoString[256];

// end of crocods variable

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
    va_list va;

    (void)level;

    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}

void retro_init(void)
{
    char *savedir = NULL;
    int i;

    guestInit();


    environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &savedir);


    char oldOpenFilename[MAX_PATH+1];
    strcpy(oldOpenFilename, gb.openFilename);

    memset(&gb, 0, sizeof(gb));

    strcpy(gb.openFilename, oldOpenFilename);

    // Get map layout

    gb.keyboardLayout = 1; // French

    for (i = 0; i < RETROK_LAST; i++) {
        KeySymToCPCKey[i] = CPC_NIL;
    }

    /* International key mappings */
    KeySymToCPCKey[RETROK_0] = CPC_ZERO;
    KeySymToCPCKey[RETROK_1] = CPC_1;
    KeySymToCPCKey[RETROK_2] = CPC_2;
    KeySymToCPCKey[RETROK_3] = CPC_3;
    KeySymToCPCKey[RETROK_4] = CPC_4;
    KeySymToCPCKey[RETROK_5] = CPC_5;
    KeySymToCPCKey[RETROK_6] = CPC_6;
    KeySymToCPCKey[RETROK_7] = CPC_7;
    KeySymToCPCKey[RETROK_8] = CPC_8;
    KeySymToCPCKey[RETROK_9] = CPC_9;
    KeySymToCPCKey[RETROK_a] = CPC_A;
    KeySymToCPCKey[RETROK_b] = CPC_B;
    KeySymToCPCKey[RETROK_c] = CPC_C;
    KeySymToCPCKey[RETROK_d] = CPC_D;
    KeySymToCPCKey[RETROK_e] = CPC_E;
    KeySymToCPCKey[RETROK_f] = CPC_F;
    KeySymToCPCKey[RETROK_g] = CPC_G;
    KeySymToCPCKey[RETROK_h] = CPC_H;
    KeySymToCPCKey[RETROK_i] = CPC_I;
    KeySymToCPCKey[RETROK_j] = CPC_J;
    KeySymToCPCKey[RETROK_k] = CPC_K;
    KeySymToCPCKey[RETROK_l] = CPC_L;
    KeySymToCPCKey[RETROK_m] = CPC_M;
    KeySymToCPCKey[RETROK_n] = CPC_N;
    KeySymToCPCKey[RETROK_o] = CPC_O;
    KeySymToCPCKey[RETROK_p] = CPC_P;
    KeySymToCPCKey[RETROK_q] = CPC_Q;
    KeySymToCPCKey[RETROK_r] = CPC_R;
    KeySymToCPCKey[RETROK_s] = CPC_S;
    KeySymToCPCKey[RETROK_t] = CPC_T;
    KeySymToCPCKey[RETROK_u] = CPC_U;
    KeySymToCPCKey[RETROK_v] = CPC_V;
    KeySymToCPCKey[RETROK_w] = CPC_W;
    KeySymToCPCKey[RETROK_x] = CPC_X;
    KeySymToCPCKey[RETROK_y] = CPC_Y;
    KeySymToCPCKey[RETROK_z] = CPC_Z;
    KeySymToCPCKey[RETROK_SPACE] = CPC_SPACE;
    KeySymToCPCKey[RETROK_COMMA] = CPC_COMMA;
    KeySymToCPCKey[RETROK_PERIOD] = CPC_DOT;
    KeySymToCPCKey[RETROK_SEMICOLON] = CPC_COLON;
    KeySymToCPCKey[RETROK_MINUS] = CPC_MINUS;
    KeySymToCPCKey[RETROK_EQUALS] = CPC_HAT;
    KeySymToCPCKey[RETROK_LEFTBRACKET] = CPC_AT;
    KeySymToCPCKey[RETROK_RIGHTBRACKET] = CPC_OPEN_SQUARE_BRACKET;

    KeySymToCPCKey[RETROK_TAB] = CPC_TAB;
    KeySymToCPCKey[RETROK_RETURN] = CPC_RETURN;
    KeySymToCPCKey[RETROK_BACKSPACE] = CPC_DEL;
    KeySymToCPCKey[RETROK_ESCAPE] = CPC_ESC;

    // KeySymToCPCKey[RETROK_Equals & 0x0ff)] = CPC_CLR;

    KeySymToCPCKey[RETROK_UP] = CPC_CURSOR_UP;
    KeySymToCPCKey[RETROK_DOWN] = CPC_CURSOR_DOWN;
    KeySymToCPCKey[RETROK_LEFT] = CPC_CURSOR_LEFT;
    KeySymToCPCKey[RETROK_RIGHT] = CPC_CURSOR_RIGHT;

    KeySymToCPCKey[RETROK_KP0] = CPC_F0;
    KeySymToCPCKey[RETROK_KP1] = CPC_F1;
    KeySymToCPCKey[RETROK_KP2] = CPC_F2;
    KeySymToCPCKey[RETROK_KP3] = CPC_F3;
    KeySymToCPCKey[RETROK_KP4] = CPC_F4;
    KeySymToCPCKey[RETROK_KP5] = CPC_F5;
    KeySymToCPCKey[RETROK_KP6] = CPC_F6;
    KeySymToCPCKey[RETROK_KP7] = CPC_F7;
    KeySymToCPCKey[RETROK_KP8] = CPC_F8;
    KeySymToCPCKey[RETROK_KP9] = CPC_F9;

    KeySymToCPCKey[RETROK_KP_PERIOD] = CPC_FDOT;

    KeySymToCPCKey[RETROK_LSHIFT] = CPC_SHIFT;
    KeySymToCPCKey[RETROK_RSHIFT] = CPC_SHIFT;
    KeySymToCPCKey[RETROK_LCTRL] = CPC_CONTROL;
    KeySymToCPCKey[RETROK_RCTRL] = CPC_CONTROL;
    KeySymToCPCKey[RETROK_CAPSLOCK] = CPC_CAPS_LOCK;

    KeySymToCPCKey[RETROK_KP_ENTER] = CPC_SMALL_ENTER;

    KeySymToCPCKey[RETROK_DELETE] = CPC_JOY_LEFT;
    KeySymToCPCKey[RETROK_END] = CPC_JOY_DOWN;
    KeySymToCPCKey[RETROK_PAGEDOWN] = CPC_JOY_RIGHT;
    KeySymToCPCKey[RETROK_INSERT] = CPC_JOY_FIRE1;
    KeySymToCPCKey[RETROK_HOME] = CPC_JOY_UP;
    KeySymToCPCKey[RETROK_PAGEUP] = CPC_JOY_FIRE2;

    KeySymToCPCKey[0x0134] = CPC_COPY; /* Alt */
    KeySymToCPCKey[0x0137] = CPC_COPY; /* Compose */

    // Init nds

    nds_initBorder(&gb, &bx, &by);
    nds_init(&gb);

    updateFromEnvironnement();


    AutoType_Init(&gb);

    if (ReadConfig()) {
        HardResetCPC(&gb);

    } else {
        //   NSLog(@"Fichier de configuration du CPC non trouvÈ.");
    }

    initSound(&gb, 44100);

//     printf("End of retro_init\n");


} /* retro_init */

void retro_deinit(void)
{

}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
    (void)port;
    (void)device;


}

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name = "crocods";
    info->need_fullpath = false;
    info->valid_extensions = "sna|dsk|kcr";

#ifdef GIT_VERSION
    info->library_version = "git" GIT_VERSION;
#else
    info->library_version = "svn";
#endif

}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    info->timing.fps = 50.0;
    info->timing.sample_rate = 44100.0;

    info->geometry.base_width = 384 * 2;
    info->geometry.base_height = 288;

    info->geometry.max_width = 384 * 2;
    info->geometry.max_height = 288;
    info->geometry.aspect_ratio = 1;
}

void retro_set_environment(retro_environment_t cb)
{
    struct retro_log_callback logging;

    environ_cb = cb;

    if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging)) {
        log_cb = logging.log;
    } else {
        log_cb = fallback_log;
    }

    log_cb = fallback_log;


    static const struct retro_variable vars[] = {
        // { "crocods_computertype", "Machine Type (Restart); CPC 464|CPC 6128" },
        // { "crocods_vdpsync", "VDP Sync Type (Restart); Auto|50Hz|60Hz" },
        // {"crocods_greenmonitor", "Color Monitor; color|green"         },
        // {"crocods_resize",       "Resize; Auto|320x200|Overscan"      },
        // {"crocods_hack",         "Speed hack; no|yes"                 },
        {NULL, NULL},
    };

    cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void *)vars);


    static const struct retro_controller_description port[] = {
        {"RetroPad",      RETRO_DEVICE_JOYPAD     },
        {"RetroKeyboard", RETRO_DEVICE_KEYBOARD   },
    };

    static const struct retro_controller_info ports[] = {
        {port, 2},
        {port, 2},
        {NULL, 0},
    };

    cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void *)ports);

} /* retro_set_environment */

void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}

void retro_reset(void)
{
    // printf("retro_reset\n");

    ResetZ80(&gb);
    ResetUPD(&gb);
    ResetCRTC(&gb);

    loadGame();
}

// static void frame_time_cb(retro_usec_t usec)
// {
//     frame_time = usec / 1000000.0;

// }


void readIni(void)
{
    /*
     * CURSOR_UP, CURSOR_RIGHT, CURSOR_DOWN, F9, F6, F3, SMALL_ENTER, FDOT, CURSOR_LEFT, COPY, F7, F8, F5, F1, F2, F0, CLR, OPEN_SQUARE_BRACKET, RETURN, CLOSE_SQUARE_BRACKET, F4, SHIFT, FORWARD_SLASH, CONTROL, HAT, MINUS, AT, P, SEMICOLON, COLON, BACKSLASH, DOT, ZERO, 9, O, I, L, K, M, COMMA, 8, 7, U, Y, H, J, N, SPACE, 6, 5, R, T, G, F, B, V, 4, 3, E, W, S, D, C, X, 1, 2, ESC, Q, TAB, A, CAPS_LOCK, Z, JOY_UP, JOY_DOWN, JOY_LEFT, JOY_RIGHT, JOY_FIRE1, JOY_FIRE2, SPARE, DEL */

}

void updateFromEnvironnement()
{
    // struct retro_variable pk1var = {"crocods_greenmonitor"};

    // if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &pk1var) && pk1var.value) {
    //     if (!strcmp(pk1var.value, "green")) {
    //         ExecuteMenu(&gb, ID_GREEN_MONITOR, NULL);
    //     } else if (!strcmp(pk1var.value, "color")) {
    //         ExecuteMenu(&gb, ID_COLOR_MONITOR, NULL);
    //     }
    // }

    // struct retro_variable pk2var = {"crocods_resize"};
    // if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &pk2var) && pk2var.value) {
    //     if (!strcmp(pk2var.value, "Auto")) {
    //         ExecuteMenu(&gb, ID_SCREEN_AUTO, NULL);
    //     } else if (!strcmp(pk2var.value, "320x200")) {
    //         ExecuteMenu(&gb, ID_SCREEN_320, NULL);
    //     } else if (!strcmp(pk2var.value, "Overscan")) {
    //         ExecuteMenu(&gb, ID_SCREEN_OVERSCAN, NULL);
    //     }
    // }

    // struct retro_variable pk3var = {"crocods_hack"};
    // if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &pk3var) && pk3var.value) {
    //     if (!strcmp(pk3var.value, "no")) {
    //         ExecuteMenu(&gb, ID_NOHACK_TABCOUL, NULL);
    //     } else if (!strcmp(pk3var.value, "yes")) {
    //         ExecuteMenu(&gb, ID_HACK_TABCOUL, NULL);
    //     }
    // }

} /* updateFromEnvironnement */

void retro_key_down(int key)
{
    log_cb(RETRO_LOG_INFO, "key: %d\n", key);
}



int32_t WsInputGetState(core_crocods_t *core)
{
    // int32_t mode = 0;
    // static int16_t old_button_state[18];

    /* enum     KEYPAD_BITS {
     * KEY_A = BIT(0), KEY_B = BIT(1), KEY_SELECT = BIT(2), KEY_START = BIT(3),
     * KEY_RIGHT = BIT(4), KEY_LEFT = BIT(5), KEY_UP = BIT(6), KEY_DOWN = BIT(7),
     * KEY_R = BIT(8), KEY_L = BIT(9), KEY_X = BIT(10), KEY_Y = BIT(11),
     * KEY_TOUCH = BIT(12), KEY_LID = BIT(13), KEY_R2 = BIT(14), KEY_L2 = BIT(15),
     * }; */

    /*
     * 0: PAD_LEFT;
     * 1: PAD_RIGHT;
     * 2: PAD_UP;
     * 3: PAD_DOWN;
     * 4: PAD_A;
     * 5: PAD_B;
     * 6: PAD_X;
     * 7: PAD_Y;
     * 8: PAD_L;
     * 9: PAD_R;
     * 10: PAD_START;
     * 11: PAD_SELECT;
     * 12: PAD_QUIT;
     * 13: PAD_L2;
     * 14: PAD_R2;
     */

// Hardware keyboard

    memset(gb.clav, 0xFF, 16);

    int i;
    for (i = 0; i < RETROK_LAST; i++) {
        int scanCode = KeySymToCPCKey[i];

        if (scanCode != CPC_NIL) {

            Core_Key_Sate[i] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, i);

            if (Core_Key_Sate[i] != 0) {
                log_cb(RETRO_LOG_INFO, "hard key down: %d (scan: %d) %d\n", i, scanCode, Core_Key_Sate[i]);

                CPC_SetScanCode(&gb, scanCode);
            }
        }
    }



    // int8_t szFile[256];
    int32_t button = 0;




    // int i;

    // for (i = 0; i < sizeof(crocokeymap) / sizeof(struct CrocoKeyMap); i++) {
    //     int scanCode = crocokeymap[i].scanCode;

    //     if (scanCode != CPC_NIL) {

    //         if (input_state_cb(crocokeymap[i].port, RETRO_DEVICE_JOYPAD, 0, crocokeymap[i].index)) {
    //             log_cb(RETRO_LOG_INFO, "joy key down: %d (scan: %d)\n", crocokeymap[i].index, crocokeymap[i].scanCode);
    //         }
    //     }
    // }

    /* Button A	*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A)) ? (1 << 0) : 0;
    /* Button B	*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B)) ? (1 << 1) : 0;

    /* SELECT BUTTON */
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT)) ? (1 << 2) : 0;

    /* START BUTTON */
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START)) ? (1 << 3) : 0;

    /* RIGHT -> X1	*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT)) ? (1 << 4) : 0;
    /* LEFT -> X1	*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT)) ? (1 << 5) : 0;
    /* UP -> X1		*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP)) ? (1 << 6) : 0;
    /* DOWN -> X1	*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN)) ? (1 << 7) : 0;

    /* R1	*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R)) ? (1 << 8) : 0;
    /* L1	*/
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L)) ? (1 << 9) : 0;

    /* X    */
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X)) ? (1 << 10) : 0;
    /* Y    */
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y)) ? (1 << 11) : 0;

    /* R2    */
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2)) ? (1 << 14) : 0;
    /* L2    */
    button |= (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2)) ? (1 << 15) : 0;

    // if (button != 0) {
    //     printf("Button: %d\n", button);
    // }

    return button;
} /* WsInputGetState */

char framebuf[128];

int frame = 0;
static int old_width;
static int old_height;

void retro_run(void)
{
    frame++;

    // printf("retro_run %d\n", frame);


    static bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated) {
        updateFromEnvironnement();
    }

    input_poll_cb();



/*
 *  if ((gb.AutoType.nFlags & (AUTOTYPE_ACTIVE | AUTOTYPE_WAITING)) == 0) {
 *
 *      memset(gb.clav, 0xFF, 16);
 *
 *      int i;
 *      for (i = 0; i < RETROK_LAST; i++) {
 *          int scanCode = KeySymToCPCKey[i];
 *
 *          if (scanCode != CPC_NIL) {
 *
 *              Core_Key_Sate[i] = input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0, i);
 *
 *              if (Core_Key_Sate[i] != 0) {
 *                  log_cb(RETRO_LOG_INFO, "hard key down: %d (scan: %d) %d\n", i, scanCode, Core_Key_Sate[i]);
 *
 *                  CPC_SetScanCode(&gb, scanCode);
 *              }
 *          }
 *      }
 *
 *      for (i = 0; i < sizeof(crocokeymap) / sizeof(struct CrocoKeyMap); i++) {
 *          int scanCode = crocokeymap[i].scanCode;
 *
 *          if (scanCode != CPC_NIL) {
 *
 *              if (input_state_cb(crocokeymap[i].port, RETRO_DEVICE_JOYPAD, 0, crocokeymap[i].index)) {
 *                  log_cb(RETRO_LOG_INFO, "joy key down: %d (scan: %d)\n", crocokeymap[i].index, crocokeymap[i].scanCode);
 *
 *                  CPC_SetScanCode(&gb, scanCode);
 *              }
 *          }
 *      }
 *
 *  }
 */

    // game_update(frame_time, &ks);
    // game_render();

    int sndSamplerToPlay = 44100 / 50;
    int byCycle = 0;

    // Crocods
    // while (1) {

    if (!gb.isPaused) {     // Pause only the Z80
        croco_cpu_doFrame(&gb);
    }



    u16 keys_pressed = WsInputGetState(&gb);

    if (gb.runApplication != NULL) {
        gb.runApplication(&gb, (gb.wait_key_released == 1) ? 0 : keys_pressed);
    }

    // printf("UpdateScreen\n");

    UpdateScreen(&gb);

    if (keys_pressed == 0) {
        gb.wait_key_released = 0;
    }

    if ((gb.runApplication == NULL)  && (gb.wait_key_released == 0)) {
        gb.ipc.keys_pressed = keys_pressed;
    }

    // printf("Readkey\n");

    nds_ReadKey(&gb);


    int width, height;
    float ratio;

    width = gb.screenBufferWidth;
    height = gb.screenBufferHeight;

    ratio = (float)width / (float)height;

    if (gb.lastMode == 2) {
        width = width * 2;
    }

    if (gb.changeFilter != 0) {

        struct retro_game_geometry geometry;

        geometry.base_width = width;
        geometry.base_height = height;
        geometry.max_width = width;
        geometry.max_height = height;
        geometry.aspect_ratio = ratio;

        if (width != old_width || height != old_height)
                environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &geometry);
        old_width = width;
        old_height = height;
        gb.changeFilter = 0;
    }

    guestScreenDraw(&gb);


    updateScreenBuffer(&gb, gb.MemBitmap, gb.screenBufferWidth);        // gb.MemBitmap not used

    gb.overlayBitmap_width = 0;



    // if (byCycle>=maxByCycle) { // Sur 20000

    //     int snd_sampler = (44100/50/(20000/maxByCycle));
    //     GB_sample_t sample[snd_sampler];

    //     crocods_copy_sound_buffer(&gb, sample, snd_sampler);
    //     audio_batch_cb((int16_t*)&sample, snd_sampler);

    //     byCycle-=maxByCycle;
    //     sndSamplerToPlay -= snd_sampler;
    // }

    // }


    if (sndSamplerToPlay > 0) {

        GB_sample_t sample[sndSamplerToPlay];
        int x;

        crocods_copy_sound_buffer(&gb, sample, sndSamplerToPlay);
        audio_batch_cb((int16_t *)&sample, sndSamplerToPlay);
    }


} /* retro_run */

u16 scanlineMask[] = {0b1110111101011101,
                      0b1110011100011100,
                      0b1100011000011000,
                      0b1000010000010000};

u16 textureBytes[384 * 288 * 2];

u32 old_width1 = 0, old_height1 = 0, old_left1 = 0, old_top1 = 0, old_bpl1 = 0;
u16 old_width2 = 0, old_height2 = 0;

u32 *incX, *incY;


void guestInit(void)
{
// Set Video

    // printf("Guest Init\n");



    incX = (u32 *)malloc(384 * 2 * sizeof(u32));     // malloc the max width
    incY = (u32 *)malloc(272 * sizeof(u32));           // malloc the max height


} /* initSDL */

void guestBlit(core_crocods_t *core, u16 *memBitmap, u32 width1, u32 height1, u32 left1, u32 top1, u32 bpl1, u16 *buffer_scr, u16 width2, u16 height2)
{
    u16 myScanlineMask = 0;

    if ((core->scanline > 0) && (core->scanline < 5)) {
        myScanlineMask = scanlineMask[core->scanline - 1];
    }

    u32 x, y;

    if ((old_width1 != width1) || (old_height1 != height1) || (old_left1 != left1) || (old_top1 != top1) || (old_bpl1 != bpl1) || (old_width2 != width2) || (old_height2 != height2)) {

        if ((width2 > 384 * 2) || (height2 > 272)) { // Games/demos that use splits or others video hack
            ExecuteMenu(core, ID_SCREEN_OVERSCAN, NULL);
            return;
        }

        for (x = 0; x < width2; x++) {
            incX[x] = ((x * width1) / width2) + left1;
        }
        for (y = 0; y < height2; y++) {
            incY[y] = (((y * height1) / (height2)) + top1) *  bpl1;
        }
        old_width1 = width1;
        old_height1 = height1;
        old_left1 = left1;
        old_top1 = top1;
        old_bpl1 = bpl1;
        old_width2 = width2;
        old_height2 = height2;
    }

    if (myScanlineMask != 0) {
        for (y = 0; y < height2; y++) {
            if (((y % 2) == 1)  & (myScanlineMask != 0)) {
                for (x = 0; x < width2; x++) {
                    int pos = incX[x] + incY[y];

                    u16 color = memBitmap[pos] & myScanlineMask;

                    *buffer_scr = color;        /// AlphaBlendFast(memBitmap[pos],AlphaBlendFast(memBitmap[pos],AlphaBlendFast(memBitmap[pos], memBitmap[pos-bpl1])));
                    buffer_scr++;
                }
            } else {
                for (x = 0; x < width2; x++) {
                    int pos = incX[x] + incY[y];

                    *buffer_scr = memBitmap[pos];
                    buffer_scr++;
                }
            }
        }
    } else {
        for (y = 0; y < height2; y++) {
            for (x = 0; x < width2; x++) {
                int pos = incX[x] + incY[y];

                *buffer_scr = memBitmap[pos];
                buffer_scr++;
            }
        }
    }
} /* guestBlit */


void guestScreenDraw(core_crocods_t *core)
{

    uint16_t *buffer_scr = textureBytes;

    uint16_t *memBitmap;

    if (core->screenIsOptimized) {
        memBitmap = core->MemBitmap;
    } else {
        int ignoreRow = 0; // 3; // only in caprice32 actually
        memBitmap = core->MemBitmap + 384 * 2 * ignoreRow;
    }

    // printf("resize: %d\n", core->resize);

    // printf("%x\n", buffer_scr);



    u32 width2 = 0, height2 = 0;

    if (core->resize == 2) {  // 320x200
        width2 = 320;
        height2 = 200;
    }

    if (core->resize == 1) {  // AUTO
        width2 = core->screenBufferWidth;
        height2 = core->screenBufferHeight;

        if ((width2 < 256) || (height2 < 128)) {    // Games/demos that use splits or others video hack
            ExecuteMenu(core, ID_SCREEN_OVERSCAN, NULL);
        }
    }

    if (core->resize == 4) {                // Overscan
        width2 = 384;
        height2 = 272;
    }

//    if ((!core->screenIsOptimized) || (core->lastMode == 2)) {
//      width2 = width2 * 2;
//    }

    if (core->lastMode == 2) {
        width2 = width2 * 2;
    }

    // if (bufferWidth != width2) {
    //     bufferWidth = width2;
    //     bufferHeight = height2;
    // }


    int x, y;

    if (core->resize == 2) {
        // ID_SCREEN_320 x 200 - keep ratio

        if (core->screenIsOptimized) {
            int dbl;

            if (core->lastMode == 2) {
                dbl = 2;
            } else {
                dbl = 1;
            }

            // buffer_scr += 20 * actualScreen->w;      // TODO: add

            for (y = 0; y < height2; y++) {
                for (x = 0; x < width2; x++) {
                    int pos = (x * core->screenBufferWidth * dbl) / (width2) + ((y * core->screenBufferHeight) / (height2)) * core->screenBufferWidth * dbl;

                    *buffer_scr = memBitmap[pos];
                    buffer_scr++;
                }
            }

            // for (y = 0; y < 200; y++) {
            //     memcpy(buffer_scr + 320 * 20 + y * 320, core->MemBitmap + y * core->MemBitmap_width, 320 * 2);
            // }

            // for (y = 0; y < 240; y++) {
            //     for (x = 0; x < 320; x++) {
            //         int pos = (x * 320 * dbl) / 320 + ((y * 200) / 240) * core->screenBufferWidth;

            //         *buffer_scr = core->MemBitmap[pos];
            //         buffer_scr++;
            //     }
            // }


            #define PG_LBMASK565 0xF7DE

            #define AlphaBlendFast(pixel, backpixel) (((((pixel) & PG_LBMASK565) >> 1) | (((backpixel) & PG_LBMASK565) >> 1)))

            uint16_t *buffer_scr = textureBytes;
            // char pos[]={10,8,6,4,3,3,2,2,1,1};
            char pos[] = {10, 7, 5, 4, 3, 2, 2, 1, 1, 1};

            // uint16_t col = core->MemBitmap[0];

            uint16_t col = core->BG_PALETTE[core->TabCoul[ 16 ]];   // Border color

            for (y = 0; y < 20; y++) {
                for (x = 0; x < 320; x++) {
                    buffer_scr[x + y * 320] = col;
                    buffer_scr[x + (y + 220) * 320] = col;
                }
            }

            for (y = 0; y < 10; y++) {
                for (x = 0; x < pos[y]; x++) {
                    buffer_scr[x + y * 320] = 0;
                    buffer_scr[(319 - x) + y * 320] = 0;
                    buffer_scr[x + (239 - y) * 320] = 0;
                    buffer_scr[(319 - x) + (239 - y) * 320] = 0;
                }
            }
        } else {
            u32 left1 = 32 * 2;
            u32 top1 = 40;
            u32 width1 = 320 * 2;
            u32 height1 = 200;
            u32 bpl1 = 768;     // byte per line

            // TODO: fix
            if (height2 == 240) {  // Round border on GCW
                u32 x, y;

                char pos[] = {10, 7, 5, 4, 3, 2, 2, 1, 1, 1};

                u16 myScanlineMask = 0;

                if ((core->scanline > 0) && (core->scanline < 5)) {
                    myScanlineMask = scanlineMask[core->scanline - 1];
                }

                uint16_t col = core->BG_PALETTE[core->TabCoul[ 16 ]];   // Border color

                for (y = 0; y < 20; y++) {
                    for (x = 0; x < 320; x++) {
                        if (((y % 2) == 1)  & (myScanlineMask != 0)) {
                            u16 color = col & myScanlineMask;

                            buffer_scr[x + y * 320] = color;
                            buffer_scr[x + (y + 220) * 320] = color;
                        } else {
                            buffer_scr[x + y * 320] = col;
                            buffer_scr[x + (y + 220) * 320] = col;
                        }
                    }
                }

                for (y = 0; y < 10; y++) {
                    for (x = 0; x < pos[y]; x++) {
                        buffer_scr[x + y * 320] = 0;
                        buffer_scr[(319 - x) + y * 320] = 0;
                        buffer_scr[x + (239 - y) * 320] = 0;
                        buffer_scr[(319 - x) + (239 - y) * 320] = 0;
                    }
                }

                height2 = 200;
                buffer_scr += 20 * width2;
            }

            guestBlit(core, memBitmap, width1, height1, left1, top1, bpl1,
                      buffer_scr, width2, height2);
        }
    }

    // printf("bef resize 1\n");

    if (core->resize == 1) {   // TODO: improve resize
        // ID_SCREEN_AUTO

        // int width = core->screenBufferWidth;
        // if (width > 320)
        //  width = 320;

        // for (y = 0; y < 200; y++)
        // {
        //  memcpy(buffer_scr + 320 * 20 + y * 320, core->MemBitmap + y * core->MemBitmap_width, width * 2);
        // }

        // printf("resize 1\n");

        // printf("opt: %d\n", core->screenIsOptimized);

        if (core->screenIsOptimized) {
            int dbl;

            if (core->lastMode == 2) {
                dbl = 2;
            } else {
                dbl = 1;
            }

            for (y = 0; y < height2; y++) {
                for (x = 0; x < width2; x++) {
                    int pos = (x * core->screenBufferWidth * dbl) / (width2) + ((y * core->screenBufferHeight) / (height2)) * core->screenBufferWidth * dbl;

                    *buffer_scr = memBitmap[pos];
                    buffer_scr++;
                }
            }
        } else {
            // Copy 1 to 2

            // printf("draw %p %p %dx%d\n", incX, incY, width2, height2);

            u32 left1 = core->x0 * 2;
            u32 top1 = core->y0;
            u32 width1 = core->screenBufferWidth * 2;
            u32 height1 = core->screenBufferHeight;
            u32 bpl1 = 768;     // byte per line

            guestBlit(core, memBitmap, width1, height1, left1, top1, bpl1,
                      buffer_scr, width2, height2);

            /*
             * // printf(" incx\n");
             *
             * for (x = 0; x < width2; x++) {
             *  incX[x] = ((x * width1) / width2) + left1;
             * }
             *
             * // printf("end incx\n");
             *
             * for (y = 0; y < height2; y++) {
             *  incY[y] = (((y * height1) / (height2)) + top1) *  bpl1;
             * }
             *
             * // printf("end incy\n");
             * // printf("print to %p from %p\n", buffer_scr, memBitmap);
             *
             * for (y = 0; y < height2; y++) {
             *  if (((y % 2) == 1)  & (myScanlineMask != 0)) {
             *      for (x = 0; x < width2; x++) {
             *          int pos = incX[x] + incY[y];
             *
             * buffer_scr = memBitmap[pos] & myScanlineMask;
             *          buffer_scr++;
             *      }
             *  } else {
             *      for (x = 0; x < width2; x++) {
             *          int pos = incX[x] + incY[y];
             *
             * buffer_scr = memBitmap[pos];
             *          buffer_scr++;
             *      }
             *  }
             * }
             */

            // printf("end of draw\n");
        }
    }

    if (core->resize == 4) {   // TODO: improve resize
        // ID_SCREEN_OVERSCAN

        if (core->screenIsOptimized) {
            int dbl;

            if (core->lastMode == 2) {
                dbl = 2;
            } else {
                dbl = 1;
            }

            for (y = 0; y < height2; y++) {
                for (x = 0; x < width2; x++) {
                    int pos = (x * 384 * dbl) / (width2) + ((y * 272) / (height2)) * 384 * dbl;

                    *buffer_scr = memBitmap[pos];
                    buffer_scr++;
                }
            }
        } else {
            u32 left1 = 0;
            u32 top1 = 0;
            u32 width1 = 768;
            u32 height1 = 272;
            u32 bpl1 = 768;     // byte per line

            // u32 left2 = 0;
            // u32 top2 = 0;

            guestBlit(core, memBitmap, width1, height1, left1, top1, bpl1,
                      buffer_scr, width2, height2);

            /*
             *
             * for (x = 0; x < width2; x++) {
             *  incX[x] = ((x * width1) / width2) + left1;
             * }
             * for (y = 0; y < height2; y++) {
             *  incY[y] = (((y * height1) / (height2)) + top1) *  bpl1;
             * }
             *
             * for (y = 0; y < height2; y++) {
             *  if (((y % 2) == 1)  & (myScanlineMask != 0)) {
             *      for (x = 0; x < width2; x++) {
             *          int pos = incX[x] + incY[y];
             *
             * buffer_scr = memBitmap[pos] & myScanlineMask;
             *          buffer_scr++;
             *      }
             *  } else {
             *      for (x = 0; x < width2; x++) {
             *          int pos = incX[x] + incY[y];
             *
             * buffer_scr = memBitmap[pos];
             *          buffer_scr++;
             *      }
             *  }
             * }
             */
        }
    }

    /*
     * static char buffer[4];
     * if (GameConf.m_DisplayFPS)
     * {
     *      if (GameConf.m_ScreenRatio == 2 || GameConf.m_ScreenRatio == 0)
     *      {
     *              SDL_Rect pos;
     *              pos.x = 0;
     *              pos.y = 0;
     *              pos.w = 17;
     *              pos.h = 16;
     *              SDL_FillRect(actualScreen, &pos, 0);
     *      }
     *      sprintf(buffer,"%d",FPS);
     *      print_string_video(2,2,buffer);
     * }
     */

    // printf("Drawicon\n");


    if ((core->iconTimer > 0) || (core->overlayBitmap_width != 0)) {

        int dbl_x = (core->lastMode == 2) ? 2 : 1;
        int dbl_y = 1;

        dbl_x = (width2 / 320);
        dbl_y = (height2 / 200);

        if (dbl_x < 1) {
            dbl_x = 1;
        }
        if (dbl_y < 1) {
            dbl_y = 1;
        }
        // Draw icon

        if (core->iconTimer > 0) {
            int x, y;
            int y0;

            int dispiconX = core->iconToDislay / 16;
            int dispiconY = core->iconToDislay % 16;


            uint16_t *buffer_scr = textureBytes;

            buffer_scr += +(8 * width2) + 8;

            for (y = 0; y < 32; y++) {
                for (y0 = 0; y0 < dbl_y; y0++) {
                    int step_x = 0;
                    u16 *src = core->icons + (dispiconX * 32) + (y + dispiconY * 32) * 448;

                    for (x = 0; x < 32 * dbl_x; x++) {
                        u16 car;
                        car = *src;
                        if (car != 33840) {
                            *buffer_scr = car;
                        }
                        buffer_scr++;
                        step_x++;
                        if (step_x == dbl_x) {
                            step_x = 0;
                        }
                        if (step_x == 0) {
                            src++;
                        }
                    }

                    buffer_scr += (width2 - 32 * dbl_x);
                }
            }

            // dispIcon(core, 0, 0, core->iconToDislay / 16, core->iconToDislay % 16, 0);
            core->iconTimer--;
        } // End of icon

        if (core->overlayBitmap_width != 0) {
            int y0;


            uint16_t *buffer_scr = textureBytes;

            for (y = 0; y < core->overlayBitmap_height; y++) {
                for (y0 = 0; y0 < dbl_y; y0++) {
                    u16 *dest = buffer_scr;
                    if (core->overlayBitmap_center != 1) {
                        dest += width2 * core->overlayBitmap_posy + core->overlayBitmap_posx + (y * dbl_y + y0) * width2;
                    } else {
                        dest += width2 * ((height2 -  core->overlayBitmap_height * dbl_y) / 2)  + ((width2 -  core->overlayBitmap_width * dbl_x) / 2) + (y * dbl_y + y0) * width2;
                    }

                    // +actualScreen->w * core->overlayBitmap_posy + core->overlayBitmap_posx + y * actualScreen->w;
                    u16 *src = core->overlayBitmap + y * 320;
                    int step_x = 0;
                    for (x = 0; x < core->overlayBitmap_width * dbl_x; x++) {
                        u16 car = *src;
                        if (car != 63519) {       // RGB565(255,0,255)
                            *dest = car;
                        }
                        dest++;
                        if (step_x == 0) {
                            src++;
                        }
                        step_x++;
                        if (step_x == dbl_x) {
                            step_x = 0;
                        }
                    }
                }
            }
        } // End of overlay


    }


    video_cb(textureBytes, width2, height2, width2 * 2);


} /* screen_draw */

void guestExit(void)
{

}

void guestGetAllKeyPressed(core_crocods_t *core, char *string)
{

}

void guestGetJoystick(core_crocods_t *core, char *string)
{

}

void setVariable(char *key, char *value)
{
    char isMap = false;
    unsigned index;
    unsigned port;

    log_cb(RETRO_LOG_INFO, "setVariable: %s=%s\n", key, value);

    CPC_SCANCODE scanCode;

    if (!strcmp(key, "input_player1_a")) {
        isMap = true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_A;
    } else if (!strcmp(key, "input_player1_b")) {
        isMap = true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_B;
    } else if (!strcmp(key, "input_player1_x")) {
        isMap = true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_X;
    } else if (!strcmp(key, "input_player1_y")) {
        isMap = true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_Y;
    } else if (!strcmp(key, "input_player1_l")) {
        isMap = true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_L;
    } else if (!strcmp(key, "input_player1_r")) {
        isMap = true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_R;
    } else if (!strcmp(key, "input_player1_left")) {
        isMap = true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_LEFT;
    } else if (!strcmp(key, "input_player1_right")) {
        isMap = true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_RIGHT;
    } else if (!strcmp(key, "input_player1_up")) {
        isMap = true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_UP;
    } else if (!strcmp(key, "input_player1_down")) {
        isMap = true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_DOWN;
    } else if (!strcmp(key, "input_player1_select")) {
        isMap = true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_SELECT;
    } else if (!strcmp(key, "input_player1_start")) {
        isMap = true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_START;
    }

    if (!strcmp(key, "input_player2_a")) {
        isMap = true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_A;
    } else if (!strcmp(key, "input_player2_b")) {
        isMap = true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_B;
    } else if (!strcmp(key, "input_player2_x")) {
        isMap = true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_X;
    } else if (!strcmp(key, "input_player2_y")) {
        isMap = true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_Y;
    } else if (!strcmp(key, "input_player2_l")) {
        isMap = true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_L;
    } else if (!strcmp(key, "input_player2_r")) {
        isMap = true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_R;
    } else if (!strcmp(key, "input_player2_left")) {
        isMap = true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_LEFT;
    } else if (!strcmp(key, "input_player2_right")) {
        isMap = true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_RIGHT;
    } else if (!strcmp(key, "input_player2_up")) {
        isMap = true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_UP;
    } else if (!strcmp(key, "input_player2_down")) {
        isMap = true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_DOWN;
    } else if (!strcmp(key, "input_player2_select")) {
        isMap = true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_SELECT;
    } else if (!strcmp(key, "input_player2_start")) {
        isMap = true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_START;
    }


    //  CURSOR_UP, CURSOR_RIGHT, CURSOR_DOWN, F9, F6, F3, SMALL_ENTER, FDOT, CURSOR_LEFT, COPY, F7, F8, F5, F1, F2, F0, CLR, OPEN_SQUARE_BRACKET, RETURN, CLOSE_SQUARE_BRACKET, F4, SHIFT, FORWARD_SLASH, CONTROL, HAT, MINUS, AT, P, SEMICOLON, COLON, BACKSLASH, DOT, ZERO, 9, O, I, L, K, M, COMMA, 8, 7, U, Y, H, J, N, SPACE, 6, 5, R, T, G, F, B, V, 4, 3, E, W, S, D, C, X, 1, 2, ESC, Q, TAB, A, CAPS_LOCK, Z, JOY_UP, JOY_DOWN, JOY_LEFT, JOY_RIGHT, JOY_FIRE1, JOY_FIRE2, SPARE, DEL */


    if (!strcmp(value, "CURSOR_UP")) {
        scanCode = CPC_CURSOR_UP;
    } else if (!strcmp(value, "CURSOR_RIGHT")) {
        scanCode = CPC_CURSOR_RIGHT;
    } else if (!strcmp(value, "CURSOR_DOWN")) {
        scanCode = CPC_CURSOR_DOWN;
    } else if (!strcmp(value, "F9")) {
        scanCode = CPC_F9;
    } else if (!strcmp(value, "F6")) {
        scanCode = CPC_F6;
    } else if (!strcmp(value, "F3")) {
        scanCode = CPC_F3;
    } else if (!strcmp(value, "SMALL_ENTER")) {
        scanCode = CPC_SMALL_ENTER;
    } else if (!strcmp(value, "FDOT")) {
        scanCode = CPC_FDOT;
    } else if (!strcmp(value, "CURSOR_LEFT")) {
        scanCode = CPC_CURSOR_LEFT;
    } else if (!strcmp(value, "COPY")) {
        scanCode = CPC_COPY;
    } else if (!strcmp(value, "F7")) {
        scanCode = CPC_F7;
    } else if (!strcmp(value, "F8")) {
        scanCode = CPC_F8;
    } else if (!strcmp(value, "F5")) {
        scanCode = CPC_F5;
    } else if (!strcmp(value, "F1")) {
        scanCode = CPC_F1;
    } else if (!strcmp(value, "F2")) {
        scanCode = CPC_F2;
    } else if (!strcmp(value, "F0")) {
        scanCode = CPC_F0;
    } else if (!strcmp(value, "CLR")) {
        scanCode = CPC_CLR;
    } else if (!strcmp(value, "OPEN_SQUARE_BRACKET")) {
        scanCode = CPC_OPEN_SQUARE_BRACKET;
    } else if (!strcmp(value, "RETURN")) {
        scanCode = CPC_RETURN;
    } else if (!strcmp(value, "CLOSE_SQUARE_BRACKET")) {
        scanCode = CPC_CLOSE_SQUARE_BRACKET;
    } else if (!strcmp(value, "F4")) {
        scanCode = CPC_F4;
    } else if (!strcmp(value, "SHIFT")) {
        scanCode = CPC_SHIFT;
    } else if (!strcmp(value, "FORWARD_SLASH")) {
        scanCode = CPC_FORWARD_SLASH;
    } else if (!strcmp(value, "CONTROL")) {
        scanCode = CPC_CONTROL;
    } else if (!strcmp(value, "HAT")) {
        scanCode = CPC_HAT;
    } else if (!strcmp(value, "MINUS")) {
        scanCode = CPC_MINUS;
    } else if (!strcmp(value, "AT")) {
        scanCode = CPC_AT;
    } else if (!strcmp(value, "P")) {
        scanCode = CPC_P;
    } else if (!strcmp(value, "SEMICOLON")) {
        scanCode = CPC_SEMICOLON;
    } else if (!strcmp(value, "COLON")) {
        scanCode = CPC_COLON;
    } else if (!strcmp(value, "BACKSLASH")) {
        scanCode = CPC_BACKSLASH;
    } else if (!strcmp(value, "DOT")) {
        scanCode = CPC_DOT;
    } else if (!strcmp(value, "ZERO")) {
        scanCode = CPC_ZERO;
    } else if (!strcmp(value, "9")) {
        scanCode = CPC_9;
    } else if (!strcmp(value, "O")) {
        scanCode = CPC_O;
    } else if (!strcmp(value, "I")) {
        scanCode = CPC_I;
    } else if (!strcmp(value, "L")) {
        scanCode = CPC_L;
    } else if (!strcmp(value, "K")) {
        scanCode = CPC_K;
    } else if (!strcmp(value, "M")) {
        scanCode = CPC_M;
    } else if (!strcmp(value, "COMMA")) {
        scanCode = CPC_COMMA;
    } else if (!strcmp(value, "8")) {
        scanCode = CPC_8;
    } else if (!strcmp(value, "7")) {
        scanCode = CPC_7;
    } else if (!strcmp(value, "U")) {
        scanCode = CPC_U;
    } else if (!strcmp(value, "Y")) {
        scanCode = CPC_Y;
    } else if (!strcmp(value, "H")) {
        scanCode = CPC_H;
    } else if (!strcmp(value, "J")) {
        scanCode = CPC_J;
    } else if (!strcmp(value, "N")) {
        scanCode = CPC_N;
    } else if (!strcmp(value, "SPACE")) {
        scanCode = CPC_SPACE;
    } else if (!strcmp(value, "6")) {
        scanCode = CPC_6;
    } else if (!strcmp(value, "5")) {
        scanCode = CPC_5;
    } else if (!strcmp(value, "R")) {
        scanCode = CPC_R;
    } else if (!strcmp(value, "T")) {
        scanCode = CPC_T;
    } else if (!strcmp(value, "G")) {
        scanCode = CPC_G;
    } else if (!strcmp(value, "F")) {
        scanCode = CPC_F;
    } else if (!strcmp(value, "B")) {
        scanCode = CPC_B;
    } else if (!strcmp(value, "V")) {
        scanCode = CPC_V;
    } else if (!strcmp(value, "4")) {
        scanCode = CPC_4;
    } else if (!strcmp(value, "3")) {
        scanCode = CPC_3;
    } else if (!strcmp(value, "E")) {
        scanCode = CPC_E;
    } else if (!strcmp(value, "W")) {
        scanCode = CPC_W;
    } else if (!strcmp(value, "S")) {
        scanCode = CPC_S;
    } else if (!strcmp(value, "D")) {
        scanCode = CPC_D;
    } else if (!strcmp(value, "C")) {
        scanCode = CPC_C;
    } else if (!strcmp(value, "X")) {
        scanCode = CPC_X;
    } else if (!strcmp(value, "1")) {
        scanCode = CPC_1;
    } else if (!strcmp(value, "2")) {
        scanCode = CPC_2;
    } else if (!strcmp(value, "ESC")) {
        scanCode = CPC_ESC;
    } else if (!strcmp(value, "Q")) {
        scanCode = CPC_Q;
    } else if (!strcmp(value, "TAB")) {
        scanCode = CPC_TAB;
    } else if (!strcmp(value, "A")) {
        scanCode = CPC_A;
    } else if (!strcmp(value, "CAPS_LOCK")) {
        scanCode = CPC_CAPS_LOCK;
    } else if (!strcmp(value, "Z")) {
        scanCode = CPC_Z;
    } else if (!strcmp(value, "JOY_UP")) {
        scanCode = CPC_JOY_UP;
    } else if (!strcmp(value, "JOY_DOWN")) {
        scanCode = CPC_JOY_DOWN;
    } else if (!strcmp(value, "JOY_LEFT")) {
        scanCode = CPC_JOY_LEFT;
    } else if (!strcmp(value, "JOY_RIGHT")) {
        scanCode = CPC_JOY_RIGHT;
    } else if (!strcmp(value, "JOY_FIRE1")) {
        scanCode = CPC_JOY_FIRE1;
    } else if (!strcmp(value, "JOY_FIRE2")) {
        scanCode = CPC_JOY_FIRE2;
    } else if (!strcmp(value, "SPARE")) {
        scanCode = CPC_SPACE;
    } else if (!strcmp(value, "DEL")) {
        scanCode = CPC_DEL;
    }


    if (isMap) {
        int i;
        for (i = 0; i < sizeof(crocokeymap) / sizeof(struct CrocoKeyMap); i++) {

            if ((crocokeymap[i].port == port) && (crocokeymap[i].index == index)) {
                crocokeymap[i].scanCode = scanCode;
            }
        }

    }
} /* setVariable */

bool loadGame(void)
{
    ExecuteMenu(&gb, ID_AUTORUN, NULL);                             // Do: open disk & autorun // TODO
    return 1;

} /* loadGame */




bool retro_load_game(const struct retro_game_info *info)
{
    struct retro_frame_time_callback frame_cb;
    struct retro_input_descriptor desc[] = {
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,   "Left"  },
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,     "Up"    },
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,   "Down"  },
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT,  "Right" },
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,  "Start" },
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Pause" },
        {0},
    };

    old_width = 0;
    old_height = 0;

    log_cb(RETRO_LOG_INFO, "begin of load games\n");

    environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

    // Init pixel format

    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
        if (log_cb)
            log_cb(RETRO_LOG_INFO, "XRGG565 is not supported.\n");
        return 0;
    }

    // Load .dsk or .sna

    strcpy(gb.openFilename, info->path);
    ExecuteMenu(&gb, ID_AUTORUN, NULL);                         // Do: open disk & autorun // TODO

    log_cb(RETRO_LOG_INFO, "open file: %s\n", info->path);

    return 1;
} /* retro_load_game */

void retro_unload_game(void)
{
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_PAL;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
    (void)type;
    (void)info;
    (void)num;
    return false;
}

size_t retro_serialize_size(void)
{
    return getSnapshotSize(&gb);
}

bool retro_serialize(void *data_, size_t size)
{
    int len;
    char *buffer = getSnapshot(&gb, &len);

    if (buffer != NULL) {
        if (len > size) {
            free(buffer);
            return false;
        }

        memcpy(data_, buffer, len);
        free(buffer);

        return true;
    }
    return false;
}

bool retro_unserialize(const void *data_, size_t size)
{
    LireSnapshotMem(&gb, (u8 *)data_);

    return true;
}

void * retro_get_memory_data(unsigned id)
{
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    return 0;
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
    (void)index;
    (void)enabled;
    (void)code;
}

