
#include "libretro.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/time.h>

#include "zip/ziptool.h"

#include "crocods-core/plateform.h"

extern const unsigned char icons_gif[];
extern const unsigned char cpc6128_bin[];
extern const unsigned char romdisc_bin[];

bool loadGame(void);
void updateFromEnvironnement();

u16 pixels[320*200*8];   // Verifier taille

static int KeySymToCPCKey[RETROK_LAST];

retro_log_printf_t log_cb;
retro_video_refresh_t video_cb;

static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;

retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

// float frame_time = 0;

struct CrocoKeyMap {
    unsigned port;
    unsigned index;

    CPC_SCANCODE scanCode;
} crocokeymap[] = {
    { 0, RETRO_DEVICE_ID_JOYPAD_A, CPC_JOY_FIRE1},
    { 0, RETRO_DEVICE_ID_JOYPAD_B, CPC_JOY_FIRE2},
    { 0, RETRO_DEVICE_ID_JOYPAD_UP, CPC_JOY_UP},
    { 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, CPC_JOY_RIGHT},
    { 0, RETRO_DEVICE_ID_JOYPAD_LEFT, CPC_JOY_LEFT},
    { 0, RETRO_DEVICE_ID_JOYPAD_DOWN, CPC_JOY_DOWN},
    { 0, RETRO_DEVICE_ID_JOYPAD_X, CPC_NIL},
    { 0, RETRO_DEVICE_ID_JOYPAD_Y, CPC_NIL},
    { 0, RETRO_DEVICE_ID_JOYPAD_L, CPC_NIL},
    { 0, RETRO_DEVICE_ID_JOYPAD_R, CPC_NIL},
    { 0, RETRO_DEVICE_ID_JOYPAD_SELECT, CPC_SPACE},
    { 0, RETRO_DEVICE_ID_JOYPAD_START, CPC_RETURN},

    { 1, RETRO_DEVICE_ID_JOYPAD_A, CPC_SPACE},
    { 1, RETRO_DEVICE_ID_JOYPAD_B, CPC_SPACE},
    { 1, RETRO_DEVICE_ID_JOYPAD_UP, CPC_CURSOR_UP},
    { 1, RETRO_DEVICE_ID_JOYPAD_RIGHT, CPC_CURSOR_RIGHT},
    { 1, RETRO_DEVICE_ID_JOYPAD_LEFT, CPC_CURSOR_LEFT},
    { 1, RETRO_DEVICE_ID_JOYPAD_DOWN, CPC_CURSOR_DOWN},
    { 1, RETRO_DEVICE_ID_JOYPAD_X, CPC_NIL},
    { 1, RETRO_DEVICE_ID_JOYPAD_Y, CPC_NIL},
    { 1, RETRO_DEVICE_ID_JOYPAD_L, CPC_NIL},
    { 1, RETRO_DEVICE_ID_JOYPAD_R, CPC_NIL},
    { 1, RETRO_DEVICE_ID_JOYPAD_SELECT, CPC_SPACE},
    { 1, RETRO_DEVICE_ID_JOYPAD_START, CPC_RETURN}

};

// Crocods variable

core_crocods_t gb;
int bx,by;

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

    environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &savedir);


    char oldOpenFilename[PATH_MAX];
    strcpy(oldOpenFilename, gb.openFilename);

    memset(&gb,0,sizeof(gb));

    strcpy(gb.openFilename,oldOpenFilename);

    // Get map layout

    gb.keyboardLayout = 1; // French

    for (i=0; i<RETROK_LAST; i++)
    {
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
    KeySymToCPCKey[RETROK_RIGHTBRACKET] =CPC_OPEN_SQUARE_BRACKET;

    KeySymToCPCKey[RETROK_TAB] = CPC_TAB;
    KeySymToCPCKey[RETROK_RETURN] = CPC_RETURN;
    KeySymToCPCKey[RETROK_BACKSPACE] = CPC_DEL;
    KeySymToCPCKey[RETROK_ESCAPE] = CPC_ESC;

    //KeySymToCPCKey[RETROK_Equals & 0x0ff)] = CPC_CLR;

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
        if (InitMemCPC(&gb, (char*)&(cpc6128_bin[0]), (char*)&(romdisc_bin[0]))) {

            ResetZ80(&gb);
            ResetUPD(&gb);
            ResetCRTC(&gb);
            InitPlateforme(&gb, pixels, 320);

            Autoexec(&gb);
        }
        else {
            //  NSLog(@"Roms du CPC non trouvÈes");
        }
    }

    else {
        //   NSLog(@"Fichier de configuration du CPC non trouvÈ.");
    }

    initSound(&gb, 44100);

}

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
    info->library_name     = "crocods";
    info->need_fullpath    = false;
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

    info->geometry.base_width   = 320;
    info->geometry.base_height  = 200;

    info->geometry.max_width    = 320;
    info->geometry.max_height   = 200;
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

    static const struct retro_variable vars[] = {
        // { "crocods_computertype", "Machine Type (Restart); CPC 464|CPC 6128" },
        // { "crocods_vdpsync", "VDP Sync Type (Restart); Auto|50Hz|60Hz" },
        { "crocods_greenmonitor", "Color Monitor; color|green" },
        { "crocods_resize", "Resize; Auto|320x200|Overscan" },
        { "crocods_hack", "Speed hack; no|yes" },
        { NULL, NULL },
    };

    cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars);


    static const struct retro_controller_description port[] = {
        { "RetroPad", RETRO_DEVICE_JOYPAD },
        { "RetroKeyboard", RETRO_DEVICE_KEYBOARD },
    };

    static const struct retro_controller_info ports[] = {
        { port, 2 },
        { port, 2 },
        { NULL, 0 },
    };

    cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);

}

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
    ResetZ80(&gb);
    ResetUPD(&gb);
    ResetCRTC(&gb);

    loadGame();
}

// static void frame_time_cb(retro_usec_t usec)
// {
//     frame_time = usec / 1000000.0;

// }

long nds_GetTicks(void) {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

void readIni(void) {
    /*
       CURSOR_UP, CURSOR_RIGHT, CURSOR_DOWN, F9, F6, F3, SMALL_ENTER, FDOT, CURSOR_LEFT, COPY, F7, F8, F5, F1, F2, F0, CLR, OPEN_SQUARE_BRACKET, RETURN, CLOSE_SQUARE_BRACKET, F4, SHIFT, FORWARD_SLASH, CONTROL, HAT, MINUS, AT, P, SEMICOLON, COLON, BACKSLASH, DOT, ZERO, 9, O, I, L, K, M, COMMA, 8, 7, U, Y, H, J, N, SPACE, 6, 5, R, T, G, F, B, V, 4, 3, E, W, S, D, C, X, 1, 2, ESC, Q, TAB, A, CAPS_LOCK, Z, JOY_UP, JOY_DOWN, JOY_LEFT, JOY_RIGHT, JOY_FIRE1, JOY_FIRE2, SPARE, DEL */

}

void updateFromEnvironnement() {
    struct retro_variable pk1var = { "crocods_greenmonitor" };
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &pk1var) && pk1var.value)
    {
        if (!strcmp(pk1var.value, "green")) {
            ExecuteMenu(&gb, ID_GREEN_MONITOR, NULL);
        } else if (!strcmp(pk1var.value, "color")) {
            ExecuteMenu(&gb, ID_COLOR_MONITOR, NULL);
        }
    }

    struct retro_variable pk2var = { "crocods_resize" };
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &pk2var) && pk2var.value)
    {
        if (!strcmp(pk2var.value, "Auto")) {
            ExecuteMenu(&gb, ID_SCREEN_AUTO, NULL);
        } else if (!strcmp(pk2var.value, "320x200")) {
            ExecuteMenu(&gb, ID_SCREEN_320, NULL);
        } else if (!strcmp(pk2var.value, "Overscan")) {
            ExecuteMenu(&gb, ID_SCREEN_OVERSCAN, NULL);
        }
    }

    struct retro_variable pk3var = { "crocods_hack" };
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &pk3var) && pk3var.value)
    {
        if (!strcmp(pk3var.value, "no")) {
            ExecuteMenu(&gb, ID_NOHACK_TABCOUL, NULL);
        } else if (!strcmp(pk3var.value, "yes")) {
            ExecuteMenu(&gb, ID_HACK_TABCOUL, NULL);
        }
    }

}

void retro_key_down(int key)
{
    printf("key: %d\n", key);
}


unsigned long TimeOut = 0, OldTime = 0;
long tz80 = 0;
char framebuf[128];

char Core_Key_Sate[512];
char Core_old_Key_Sate[512];

void retro_run(void)
{

    static bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated) {
        updateFromEnvironnement();
    }

    input_poll_cb();

    if ((gb.AutoType.nFlags & (AUTOTYPE_ACTIVE|AUTOTYPE_WAITING))==0) {

        memset(gb.clav,0xFF,16);

		int i;
        for(i=0; i<RETROK_LAST; i++) {
            int scanCode=KeySymToCPCKey[i];

            if (scanCode!=CPC_NIL) {
                Core_Key_Sate[i]=input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0,i) ? 0x80 : 0;

                if(Core_Key_Sate[i]) {
                    CPC_SetScanCode(&gb, scanCode);
                }
            }
        }

        for(i=0; i<sizeof(crocokeymap)/sizeof(struct CrocoKeyMap); i++) {
            if (input_state_cb(crocokeymap[i].port, RETRO_DEVICE_JOYPAD, 0, crocokeymap[i].index)) {

                CPC_SetScanCode(&gb, crocokeymap[i].scanCode);
            }
        }

    }

    // game_update(frame_time, &ks);
    // game_render();

    int sndSamplerToPlay = 44100/50;
    int byCycle=0;

    // Crocods
    //
    //
    //
    //
    while(1) {


        if (gb.inMenu) {




        } else if (gb.isPaused) { // Pause only the Z80


        } else {
            tz80 -= nds_GetTicks(); // TODO("replace this function")
            byCycle += ExecInstZ80(&gb); // Fais tourner le CPU tant CptInstr < CYCLELIGNE
            tz80 += nds_GetTicks();

            procSound(&gb, byCycle);
        }


        TimeOut += gb.RegsCRTC[ 0 ] + 1;

        if (!CalcCRTCLine(&gb)) { // Arrive en fin d'ecran ?
            // Rafraichissement de l'ecran...


            // procSound(&gb, (int)TimeOut);


            //                static int frame=0;
            //                printf("frame: %d\n", frame++);

            if (gb.dispframerate) {
                cpcprint(&gb, 0, 192 - 8, framebuf, 1);
            }

            if (gb.inMenu) {
                DispIcons(&gb);
            }

            // Disk
            if (gb.DriveBusy > 0) {
                DispDisk(&gb, gb.DriveBusy);
                gb.DriveBusy--;
            }

            UpdateScreen(&gb);
            nds_ReadKey(&gb);




            if (gb.changeFilter!=0) {

                struct retro_game_geometry geometry;

                geometry.base_width = gb.screenBufferWidth;
                geometry.base_height = gb.screenBufferHeight;
                geometry.max_width = gb.screenBufferWidth;
                geometry.max_height = gb.screenBufferHeight;
                geometry.aspect_ratio = 0.0f;

                environ_cb( RETRO_ENVIRONMENT_SET_GEOMETRY, &geometry );
                gb.changeFilter=0;
            }

            video_cb(pixels, gb.screenBufferWidth, gb.screenBufferHeight, gb.screenBufferWidth*2);



            updateScreenBuffer(&gb, pixels, gb.screenBufferWidth);




            // Synchronisation de l'image ‡ 50 Hz
            // Pourcentage, temps espere, temps pris, temps z80, nombre de drawligne

            if (gb.DoResync) {
                //                    NSInteger Time;


                if ((!gb.turbo)  && ((gb.AutoType.nFlags & (AUTOTYPE_ACTIVE|AUTOTYPE_WAITING))==0)) {

                    // Wait Timeout - (Time - OldTime) milliseconds

                    float waitTime = TimeOut - (nds_GetTicks() - OldTime);
                    waitTime = waitTime / 1000000;

                    if (waitTime<1) {

                        // [NSThread sleepForTimeInterval:waitTime];
                    }

                }

                tz80 = 0;

                OldTime = nds_GetTicks();
                TimeOut = 0;

                break;
            }

        }

        int maxByCycle=400; // 50 fois par frame

        if (byCycle>=maxByCycle) { // Sur 20000

            int snd_sampler = (44100/50/(20000/maxByCycle));
            GB_sample_t sample[snd_sampler];
            int x;

            crocods_copy_sound_buffer(&gb, sample, snd_sampler);
            audio_batch_cb((int16_t*)&sample, snd_sampler);
            // for(x = 0; x < snd_sampler; x++) {
            //     audio_cb(sample[x].left,sample[x].right);
            // }
            //
            //
            //
            byCycle-=maxByCycle;
            sndSamplerToPlay -= snd_sampler;

        }

    }


    if (sndSamplerToPlay>0) {

        GB_sample_t sample[sndSamplerToPlay];
        int x;

        crocods_copy_sound_buffer(&gb, sample, sndSamplerToPlay);
        audio_batch_cb((int16_t*)&sample, sndSamplerToPlay);

        // for(x = 0; x < sndSamplerToPlay; x++) {
        //     audio_cb(sample[x].left,sample[x].right);
        // }



    }


/*
    printf("Bycycle: %d", byCycle);
    byCycle=0;

    {
        int snd_sampler = (44100/50);
        GB_sample_t sample[snd_sampler];
        int x;

        crocods_copy_sound_buffer(&gb, sample, snd_sampler);


        for(x = 0; x < snd_sampler; x++) {
            audio_cb(sample[x].left,sample[x].right);
        }
    }
 */

}

void setVariable(char *key, char *value) {
    char isMap=false;
    unsigned index;
    unsigned port;

    printf("setVariable: %s=%s\n", key, value);

    CPC_SCANCODE scanCode;

    if (!strcmp(key, "input_player1_a")) {
        isMap=true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_A;
    } else if (!strcmp(key, "input_player1_b")) {
        isMap=true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_B;
    } else if (!strcmp(key, "input_player1_x")) {
        isMap=true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_X;
    } else if (!strcmp(key, "input_player1_y")) {
        isMap=true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_Y;
    } else if (!strcmp(key, "input_player1_l")) {
        isMap=true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_L;
    } else if (!strcmp(key, "input_player1_r")) {
        isMap=true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_R;
    } else if (!strcmp(key, "input_player1_left")) {
        isMap=true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_LEFT;
    } else if (!strcmp(key, "input_player1_right")) {
        isMap=true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_RIGHT;
    } else if (!strcmp(key, "input_player1_up")) {
        isMap=true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_UP;
    } else if (!strcmp(key, "input_player1_down")) {
        isMap=true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_DOWN;
    } else if (!strcmp(key, "input_player1_select")) {
        isMap=true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_SELECT;
    } else if (!strcmp(key, "input_player1_start")) {
        isMap=true;
        port = 0;
        index = RETRO_DEVICE_ID_JOYPAD_START;
    }

    if (!strcmp(key, "input_player2_a")) {
        isMap=true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_A;
    } else if (!strcmp(key, "input_player2_b")) {
        isMap=true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_B;
    } else if (!strcmp(key, "input_player2_x")) {
        isMap=true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_X;
    } else if (!strcmp(key, "input_player2_y")) {
        isMap=true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_Y;
    } else if (!strcmp(key, "input_player2_l")) {
        isMap=true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_L;
    } else if (!strcmp(key, "input_player2_r")) {
        isMap=true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_R;
    } else if (!strcmp(key, "input_player2_left")) {
        isMap=true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_LEFT;
    } else if (!strcmp(key, "input_player2_right")) {
        isMap=true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_RIGHT;
    } else if (!strcmp(key, "input_player2_up")) {
        isMap=true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_UP;
    } else if (!strcmp(key, "input_player2_down")) {
        isMap=true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_DOWN;
    } else if (!strcmp(key, "input_player2_select")) {
        isMap=true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_SELECT;
    } else if (!strcmp(key, "input_player2_start")) {
        isMap=true;
        port = 1;
        index = RETRO_DEVICE_ID_JOYPAD_START;
    }


    //  CURSOR_UP, CURSOR_RIGHT, CURSOR_DOWN, F9, F6, F3, SMALL_ENTER, FDOT, CURSOR_LEFT, COPY, F7, F8, F5, F1, F2, F0, CLR, OPEN_SQUARE_BRACKET, RETURN, CLOSE_SQUARE_BRACKET, F4, SHIFT, FORWARD_SLASH, CONTROL, HAT, MINUS, AT, P, SEMICOLON, COLON, BACKSLASH, DOT, ZERO, 9, O, I, L, K, M, COMMA, 8, 7, U, Y, H, J, N, SPACE, 6, 5, R, T, G, F, B, V, 4, 3, E, W, S, D, C, X, 1, 2, ESC, Q, TAB, A, CAPS_LOCK, Z, JOY_UP, JOY_DOWN, JOY_LEFT, JOY_RIGHT, JOY_FIRE1, JOY_FIRE2, SPARE, DEL */


    if (!strcmp(value,"CURSOR_UP")) {
        scanCode = CPC_CURSOR_UP;
    } else if (!strcmp(value,"CURSOR_RIGHT")) {
        scanCode = CPC_CURSOR_RIGHT;
    } else if (!strcmp(value,"CURSOR_DOWN")) {
        scanCode = CPC_CURSOR_DOWN;
    } else if (!strcmp(value,"F9")) {
        scanCode = CPC_F9;
    } else if (!strcmp(value,"F6")) {
        scanCode = CPC_F6;
    } else if (!strcmp(value,"F3")) {
        scanCode = CPC_F3;
    } else if (!strcmp(value,"SMALL_ENTER")) {
        scanCode = CPC_SMALL_ENTER;
    } else if (!strcmp(value,"FDOT")) {
        scanCode = CPC_FDOT;
    } else if (!strcmp(value,"CURSOR_LEFT")) {
        scanCode = CPC_CURSOR_LEFT;
    } else if (!strcmp(value,"COPY")) {
        scanCode = CPC_COPY;
    } else if (!strcmp(value,"F7")) {
        scanCode = CPC_F7;
    } else if (!strcmp(value,"F8")) {
        scanCode = CPC_F8;
    } else if (!strcmp(value,"F5")) {
        scanCode = CPC_F5;
    } else if (!strcmp(value,"F1")) {
        scanCode = CPC_F1;
    } else if (!strcmp(value,"F2")) {
        scanCode = CPC_F2;
    } else if (!strcmp(value,"F0")) {
        scanCode = CPC_F0;
    } else if (!strcmp(value,"CLR")) {
        scanCode = CPC_CLR;
    } else if (!strcmp(value,"OPEN_SQUARE_BRACKET")) {
        scanCode = CPC_OPEN_SQUARE_BRACKET;
    } else if (!strcmp(value,"RETURN")) {
        scanCode = CPC_RETURN;
    } else if (!strcmp(value,"CLOSE_SQUARE_BRACKET")) {
        scanCode = CPC_CLOSE_SQUARE_BRACKET;
    } else if (!strcmp(value,"F4")) {
        scanCode = CPC_F4;
    } else if (!strcmp(value,"SHIFT")) {
        scanCode = CPC_SHIFT;
    } else if (!strcmp(value,"FORWARD_SLASH")) {
        scanCode = CPC_FORWARD_SLASH;
    } else if (!strcmp(value,"CONTROL")) {
        scanCode = CPC_CONTROL;
    } else if (!strcmp(value,"HAT")) {
        scanCode = CPC_HAT;
    } else if (!strcmp(value,"MINUS")) {
        scanCode = CPC_MINUS;
    } else if (!strcmp(value,"AT")) {
        scanCode = CPC_AT;
    } else if (!strcmp(value,"P")) {
        scanCode = CPC_P;
    } else if (!strcmp(value,"SEMICOLON")) {
        scanCode = CPC_SEMICOLON;
    } else if (!strcmp(value,"COLON")) {
        scanCode = CPC_COLON;
    } else if (!strcmp(value,"BACKSLASH")) {
        scanCode = CPC_BACKSLASH;
    } else if (!strcmp(value,"DOT")) {
        scanCode = CPC_DOT;
    } else if (!strcmp(value,"ZERO")) {
        scanCode = CPC_ZERO;
    } else if (!strcmp(value,"9")) {
        scanCode = CPC_9;
    } else if (!strcmp(value,"O")) {
        scanCode = CPC_O;
    } else if (!strcmp(value,"I")) {
        scanCode = CPC_I;
    } else if (!strcmp(value,"L")) {
        scanCode = CPC_L;
    } else if (!strcmp(value,"K")) {
        scanCode = CPC_K;
    } else if (!strcmp(value,"M")) {
        scanCode = CPC_M;
    } else if (!strcmp(value,"COMMA")) {
        scanCode = CPC_COMMA;
    } else if (!strcmp(value,"8")) {
        scanCode = CPC_8;
    } else if (!strcmp(value,"7")) {
        scanCode = CPC_7;
    } else if (!strcmp(value,"U")) {
        scanCode = CPC_U;
    } else if (!strcmp(value,"Y")) {
        scanCode = CPC_Y;
    } else if (!strcmp(value,"H")) {
        scanCode = CPC_H;
    } else if (!strcmp(value,"J")) {
        scanCode = CPC_J;
    } else if (!strcmp(value,"N")) {
        scanCode = CPC_N;
    } else if (!strcmp(value,"SPACE")) {
        scanCode = CPC_SPACE;
    } else if (!strcmp(value,"6")) {
        scanCode = CPC_6;
    } else if (!strcmp(value,"5")) {
        scanCode = CPC_5;
    } else if (!strcmp(value,"R")) {
        scanCode = CPC_R;
    } else if (!strcmp(value,"T")) {
        scanCode = CPC_T;
    } else if (!strcmp(value,"G")) {
        scanCode = CPC_G;
    } else if (!strcmp(value,"F")) {
        scanCode = CPC_F;
    } else if (!strcmp(value,"B")) {
        scanCode = CPC_B;
    } else if (!strcmp(value,"V")) {
        scanCode = CPC_V;
    } else if (!strcmp(value,"4")) {
        scanCode = CPC_4;
    } else if (!strcmp(value,"3")) {
        scanCode = CPC_3;
    } else if (!strcmp(value,"E")) {
        scanCode = CPC_E;
    } else if (!strcmp(value,"W")) {
        scanCode = CPC_W;
    } else if (!strcmp(value,"S")) {
        scanCode = CPC_S;
    } else if (!strcmp(value,"D")) {
        scanCode = CPC_D;
    } else if (!strcmp(value,"C")) {
        scanCode = CPC_C;
    } else if (!strcmp(value,"X")) {
        scanCode = CPC_X;
    } else if (!strcmp(value,"1")) {
        scanCode = CPC_1;
    } else if (!strcmp(value,"2")) {
        scanCode = CPC_2;
    } else if (!strcmp(value,"ESC")) {
        scanCode = CPC_ESC;
    } else if (!strcmp(value,"Q")) {
        scanCode = CPC_Q;
    } else if (!strcmp(value,"TAB")) {
        scanCode = CPC_TAB;
    } else if (!strcmp(value,"A")) {
        scanCode = CPC_A;
    } else if (!strcmp(value,"CAPS_LOCK")) {
        scanCode = CPC_CAPS_LOCK;
    } else if (!strcmp(value,"Z")) {
        scanCode = CPC_Z;
    } else if (!strcmp(value,"JOY_UP")) {
        scanCode = CPC_JOY_UP;
    } else if (!strcmp(value,"JOY_DOWN")) {
        scanCode = CPC_JOY_DOWN;
    } else if (!strcmp(value,"JOY_LEFT")) {
        scanCode = CPC_JOY_LEFT;
    } else if (!strcmp(value,"JOY_RIGHT")) {
        scanCode = CPC_JOY_RIGHT;
    } else if (!strcmp(value,"JOY_FIRE1")) {
        scanCode = CPC_JOY_FIRE1;
    } else if (!strcmp(value,"JOY_FIRE2")) {
        scanCode = CPC_JOY_FIRE2;
    } else if (!strcmp(value,"SPARE")) {
        scanCode = CPC_SPACE;
    } else if (!strcmp(value,"DEL")) {
        scanCode = CPC_DEL;
    }


    if (isMap) {
		int i;
        for(i=0; i<sizeof(crocokeymap)/sizeof(struct CrocoKeyMap); i++) {

            if ((crocokeymap[i].port == port) && (crocokeymap[i].index == index)) {
                crocokeymap[i].scanCode = scanCode;
            }
        }

    }
}

bool loadGame(void) {
    FILE *fic;

    u8 *dsk;
    long dsk_size;

    fic=fopen(gb.openFilename,"rb");
    if (fic==NULL) {
        return 0;
    }
    fseek(fic,0,SEEK_END);
    dsk_size=ftell(fic);
    fseek(fic,0,SEEK_SET);

    dsk = (u8*)malloc(dsk_size);
    if (dsk==NULL) {
        return 0;
    }
    fread(dsk,1,dsk_size,fic);
    fclose(fic);

    char header[32];

    if (dsk_size<32) {
        return false;
    }

    memcpy(header, dsk, 32);

    BOOL isOk = false;

    if ( (!memcmp(header, "MV - CPCEMU", 11)) || (!memcmp(header,"EXTENDED", 8)) ) {
        diskLength = (u32)dsk_size;

        disk = (u8*)malloc(diskLength);
        memcpy(disk, dsk, diskLength);

        isOk = true;
    }

    if (!memcmp(header, "MV - SNA", 8)) {
        snapshotLength = (u32)dsk_size;

        snapshot = (u8*)malloc(snapshotLength);
        memcpy(snapshot, dsk, snapshotLength);

        isOk = true;
    }

    if (!memcmp(header, "PK", 2)) {

        snapshot = unzip(dsk, dsk_size, "snapshot.sna", &snapshotLength);
        disk = unzip(dsk, dsk_size, "disk.dsk", &diskLength);


        u32 settings_length;
        unsigned char *settings = unzip(dsk, dsk_size, "settings.ini", &settings_length);


        char key[128], value[128];
        int n=0, key_n=0, value_n = 0;
        char inkey=1;

        while(n<settings_length) {

            if (inkey==1) {
                if (settings[n]!='=') {

                    if ((settings[n]!=32) && (settings[n]!=10) && (settings[n]!=13) && (settings[n]!='"')) {
                        key[key_n] = settings[n];
                        key_n++;
                    }

                } else {
                    inkey=0;
                }
            } else {


                if ((settings[n]!=32) && (settings[n]!=10) && (settings[n]!=13) && (settings[n]!='"')) {
                    value[value_n] = settings[n];
                    value_n++;
                }

            }

            n++;

            if ((settings[n]==10) || (settings[n]==13) || (n==settings_length)) {
                key[key_n] = 0;
                value[value_n]=0;

                if ((key[0]!='#') && (strlen(key)>0)) {
                    setVariable(key, value);
                }

                value_n=0;
                key_n=0;

                inkey=1;
            }


        }


        if ((disk!=NULL) || (snapshot!=NULL)) {
            isOk = true;
        }

    }

    if (!isOk) {
        return false;
    }

    // Disk loaded
    //

    // frame_cb.callback  = frame_time_cb;
    // frame_cb.reference = 1000000 / 50;
    // frame_cb.callback(frame_cb.reference);
    // environ_cb(RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK, &frame_cb);

    // (void)info;



    BOOL autoStart=true;

    char autofile[256];
    if (disk!=NULL) {
        LireDiskMem( &gb, disk, diskLength, autofile);


        if ((autoStart) && (autofile[0]!=0) && (snapshot==NULL)) {
            sprintf(autoString,"run\"%s\n", autofile);
            if (autoString[0]!=0) {
                AutoType_SetString(&gb, autoString, true);
            }
            printf("\n%s\n", autoString);
        }

    }


    if (snapshot!=NULL) {
        LireSnapshotMem(&gb, snapshot);
    }


    printf("end of load games\n");

    return true;
}


bool retro_load_game(const struct retro_game_info *info)
{
    struct retro_frame_time_callback frame_cb;
    struct retro_input_descriptor desc[] = {
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "Left" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "Up" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "Down" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start" },
        { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Pause" },
        { 0 },
    };

    printf("begin of load games\n");

    environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

    // Init pixel format

    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_RGB565;
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
    {
        if (log_cb)
            log_cb(RETRO_LOG_INFO, "XRGG565 is not supported.\n");
        return 0;
    }

    // Load .dsk or .sna

    strcpy(gb.openFilename, info->path);

    printf("open file: %s", info->path);


    return loadGame();
}

void retro_unload_game(void)
{
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
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
    int len;
    char *buffer = getSnapshot(&gb, &len);
    if (buffer != NULL) {
        free(buffer);
        return len;
    }
    return 0;
}

bool retro_serialize(void *data_, size_t size)
{
    printf("serialise\n");

    int len;
    char *buffer = getSnapshot(&gb, &len);
    if (buffer != NULL) {
        if (len>size) {
            free(buffer);
            return false;
        }

        memcpy(data_, buffer, len);
        free(buffer);

        printf("Data copied\n");

        // FILE * fp = fopen( "/Users/miguelvanhove/Downloads/save.sna", "wb" );

        // if (fp)  {
        //     fwrite( data_, 1, len, fp );
        // }
        // fclose(fp);

        return true;
    }
    return false;
}

bool retro_unserialize(const void *data_, size_t size)
{

    printf("unserialise\n");

    LireSnapshotMem(&gb, (u8*)data_);

    return true;
}

void *retro_get_memory_data(unsigned id)
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

