#include "plateform.h"

void DispInfopanel(core_crocods_t *core, u16 keys_pressed0);

#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

int apps_infopanel_files_count = 0;
int apps_infopanel_files_begin = 0;
int apps_infopanel_files_selected = 0;

int apps_infopanel_files_flag = 0;

int apps_infopanel_pos = 0;
int apps_infopanel_speed_about = 0;
int apps_infopanel_dir_about = 1;

#define apps_infopanel_num_stars 200
float apps_infopanel_starX[apps_infopanel_num_stars];
float apps_infopanel_starY[apps_infopanel_num_stars];
float apps_infopanel_starZ[apps_infopanel_num_stars];

char *apps_infopanel_about =
      "CrocoDS\n"
      "v" CROCOVERSION "\n"
      "by RedBug/Kyuran Knights\n"
      "\n"
    "*Credits*\n\nEmulator guys\nDemoniak/Win-CPC\nUlrich Doewich/CaPriCe\nColin Pitrat/Caprice32\nKevin Thacker/Arnold\n\nGFX\nKukulcan\n\n*Greets - no particular order*\n\nGryzor\nTarghan\nPascal Visa\nAlexis Koopa\nCedric LeZone\nGenesis8\nJuan Jose Martinez\nRoudoudou\nAlekmaul\n\nThat's all folks\n\n";

void apps_infopanel_end(core_crocods_t *core)
{
    core->inKeyboard = 0;

    core->runApplication = NULL;

    core->wait_key_released = 1;

    ExecuteMenu(core, ID_MENU_EXIT, NULL);
}

void apps_infopanel_initstar(int n)
{
    float m_spread = 64.0f;

    apps_infopanel_starX[n] = 2 * ((float)((double)rand() / (double)RAND_MAX) - 0.5f) * m_spread;
    apps_infopanel_starY[n] = 2 * ((float)((double)rand() / (double)RAND_MAX) - 0.5f) * m_spread;

    // For Z, the random value is only adjusted by a small amount to stop
    // a star from being generated at 0 on Z.
    apps_infopanel_starZ[n] = ((float)((double)rand() / (double)RAND_MAX) + 0.00001f) * m_spread;
}

void apps_infopanel_init(core_crocods_t *core, int flag)
{
    printf("apps_infopanel_init: %s\n", core->openFilename);

    core->runApplication = DispInfopanel;
    apps_infopanel_files_flag = flag;

    apps_infopanel_files_count = 0;
    apps_infopanel_files_begin = 0;
    apps_infopanel_files_selected = 0;

    apps_infopanel_pos = 0;
    apps_infopanel_speed_about = 100;
    apps_infopanel_dir_about = 1;

    int n;
    for (n = 0; n < apps_infopanel_num_stars; n++) {
        apps_infopanel_initstar(n);
    }

//    if (error) {
//        appendIcon(core, 0, 4, 60);
//        apps_infopanel_end(core);
//        return;
//    }
} /* apps_infopanel_init */

void DispInfopanel(core_crocods_t *core, u16 keys_pressed0)
{
//    static int key = 0;

    int y;

    u16 keys_pressed = appli_begin(core, keys_pressed0);

    core->overlayBitmap_width = 256;
    core->overlayBitmap_height = 168;
    core->overlayBitmap_posx = (320 - 256) / 2;
    core->overlayBitmap_posy = (240 - 168) / 2;
    core->overlayBitmap_center = 1;

    u16 *pdwAddr = core->overlayBitmap; // + ((j * 32) * core->MemBitmap_width) + (i * 32);

    // Text begin in 12,36 (max 13 lines, 26 columns)

    for (y = 0; y < 168; y++) {
        memcpy(pdwAddr, core->menu + y * 256, 256 * 2);
        pdwAddr += 320;
    }

    float m_speed = 20.0f;
    float delta = 0.055;

    float halfWidth = (209 - 6) / 2.0f;
    float halfHeight = (146 - 32) / 2.0f;

    unsigned int i;

    for (i = 0; i < apps_infopanel_num_stars; i++) {
        // Update the Star.

        // Move the star towards the camera which is at 0 on Z.
        apps_infopanel_starZ[i] -= delta * m_speed;

        // std::cout << "delta * m_speed " << delta * m_speed << "\n";

        // If a star is at or behind the camera, generate a new position for it

        if (apps_infopanel_starZ[i] <= 0) {
            apps_infopanel_initstar(i);
        }

        // Render the Star.

        // Multiplying the position by (size/2) and then adding (size/2)
        // remaps the positions from range (-1, 1) to (0, size)
        int x = (int)((apps_infopanel_starX[i] / apps_infopanel_starZ[i]) * halfWidth + halfWidth);
        int y = (int)((apps_infopanel_starY[i] / apps_infopanel_starZ[i]) * halfHeight + halfHeight);

        // If the star is not within range of the screen, then generate a
        // new position for it.
        // if(x < 0 || x >= target.GetWidth() || (y < 0 || y >= target.GetHeight()))

        if (x < 0 || x >= (209 - 6) || (y < 0 || y >= (146 - 32))) {
            apps_infopanel_initstar(i);
        } else {
            // Otherwise, it is safe to draw this star to the screen.
            // target.DrawPixel(x, y, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF);

            // SDL_RenderDrawPoint(target, x, y);
//              display.setPixel(x,y);

            *(core->overlayBitmap + (y + 32) * 320 + (x + 6)) = RGB565(0x7F, 0x7F, 0x7F);

            /*
             * r.w = -m_starZ[i] * 0.04 + 4;
             * r.h = -m_starZ[i] * 0.04 + 4;
             *
             * r.x = x;
             * r.y = y;
             */

            // green_val = (r.w * 50) + 30;

            /*
             * SDL_SetRenderDrawColor(target, 32 + r.w * 25, green_val, 32 + r.w * 25, 255);
             * SDL_RenderFillRect( target, &r );
             */
        }
    }

    #define MAXLEN       40
    #define MAXLEN_ABOUT 34

    char title[MAXLEN + 1];

    if (strlen(core->openFilename) > MAXLEN) {
        static int ii = 0, speed = 6, dir = 1;

        speed--;
        if (speed <= 0) {
            speed = 6;
            if (dir == 1) {
                ii++;
                if (ii > strlen(core->openFilename) - MAXLEN) {
                    dir = -1;
                    ii = (int)strlen(core->openFilename) - MAXLEN;
                    speed = 48;
                }
            } else {
                ii--;
                if (ii <= 0) {
                    dir = 1;
                    ii = 0;
                    speed = 48;
                }
            }
        }

        strncpy(title, core->openFilename + ii, MAXLEN);
        title[MAXLEN] = 0;
    } else {
        strcpy(title, core->openFilename);
    }
    cpcprint16_6w(core, core->overlayBitmap, 320, 6, 156, title, RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);
    dispIcon(core, 219, 30, 6, 4, 0);

    BOOL blockDown = 0;

    u16 color = RGB565(0xFF, 0xFF, 0x00);

    if (1 == 1) {
        int end = 0, pos = 0, lastspace = 0, y = 0;
        char quit = 0;

        do {
            do {
                if (apps_infopanel_about[end] == 32) {
                    lastspace = end;
                }
                if (apps_infopanel_about[end] == '\n') {
                    break;
                }
                if (apps_infopanel_about[end] == 0) {
                    blockDown = 1;
                    quit = 1;
                    break;
                }
                if (end - pos + 1 >= MAXLEN_ABOUT) {
                    end = lastspace;
                    break;
                }
                end++;
            } while (1);

            strncpy(title, apps_infopanel_about + pos, end - pos);
            title[end - pos] = 0;

            end++;
            pos = end;

            int y0 = y  * 8 - apps_infopanel_pos;
            int x0 = 9 + (MAXLEN_ABOUT - (int)strlen(title)) * (6 / 2);

            if (y0 >= 13 * 8) {
                cpcprint16_6w_limit(core, core->overlayBitmap, 320, x0, 33 + y0, title, color, RGB565(0x00, 0x00, 0x00), 1, 1, 0, 8 - (y0 - 13 * 8));
            } else if (y0 >= 0) {
                cpcprint16_6w(core, core->overlayBitmap, 320, x0, 33 + y0, title, color, RGB565(0x00, 0x00, 0x00), 1, 1);
            } else if (y0 >= -7) {
                cpcprint16_6w_limit(core, core->overlayBitmap, 320, x0, 33 + y0, title, color, RGB565(0x00, 0x00, 0x00), 1, 1, -y0,  8);
            }

            if (apps_infopanel_about[end] == '*') {
                color = RGB565(0xFF, 0x8C, 0x00);
            } else {
                color = RGB565(0xFF, 0xFF, 0x00);
            }

            y++;
            if ((y - apps_infopanel_pos / 8) == 15) {
                break;
            }
        } while (!quit);
//        printf("%d\n", y);
    }

    apps_infopanel_speed_about--;
    if (apps_infopanel_speed_about <= 0) {
        if (apps_infopanel_dir_about == 1) {
            if (!blockDown) {
                apps_infopanel_pos++;
                apps_infopanel_speed_about = 4;
            } else {
                apps_infopanel_dir_about = -1;
                apps_infopanel_speed_about = 48;
            }
        } else {
            if (apps_infopanel_pos > 0) {
                apps_infopanel_pos--;
                apps_infopanel_speed_about = 4;
            } else {
                apps_infopanel_dir_about = 1;
                apps_infopanel_speed_about = 48;
            }
        }
    }

//   cpcprint16_6w(core, core->overlayBitmap, 320, 0, 45, "title (azerty)", RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);
//
//
//    cpcprint16(core, core->overlayBitmap, 320, 0, 53, "title (azerty)", RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);

    if (keys_pressed & KEY_A) {
        apps_infopanel_end(core);
    }

    if (((keys_pressed & KEY_B) == KEY_B) || ((keys_pressed & KEY_R) == KEY_R)) {
        apps_infopanel_end(core);

        // ExecuteMenu(core, ID_MENU_EXIT, NULL);
    }

    if ((keys_pressed0 & KEY_UP) == KEY_UP) {
        apps_infopanel_pos--;
        if (apps_infopanel_pos <= 0) {
            apps_infopanel_pos = 0;
        }
        apps_infopanel_speed_about = 200;
    }

    if ((keys_pressed0 & KEY_DOWN) == KEY_DOWN) {
        if (!blockDown) {
            apps_infopanel_pos++;
            apps_infopanel_speed_about = 200;
        }
    }
} /* DispKeyboard */
