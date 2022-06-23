#include "platform.h"

void guestGetAllKeyPressed(core_crocods_t *core, char *string); // In guest.h
void guestGetJoystick(core_crocods_t *core, char *string);

void DispGuestinfo(core_crocods_t *core, u16 keys_pressed0);

#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

int apps_guestinfo_files_count = 0;
int apps_guestinfo_files_begin = 0;
int apps_guestinfo_files_selected = 0;

int apps_guestinfo_files_flag = 0;

int apps_guestinfo_pos = 0;
int apps_guestinfo_speed_about = 0;
int apps_guestinfo_dir_about = 1;

void apps_guestinfo_end(core_crocods_t *core)
{
    core->inKeyboard = 0;

    core->runApplication = NULL;

    core->wait_key_released = 1;

    ExecuteMenu(core, ID_MENU_EXIT, NULL);
}

void apps_guestinfo_init(core_crocods_t *core, int flag)
{
    printf("apps_guestinfo_init: %s\n", core->openFilename);

    core->runApplication = DispGuestinfo;
    apps_guestinfo_files_flag = flag;

    apps_guestinfo_files_count = 0;
    apps_guestinfo_files_begin = 0;
    apps_guestinfo_files_selected = 0;

//    if (error) {
//        appendIcon(core, 0, 4, 60);
//        apps_guestinfo_end(core);
//        return;
//    }
} /* apps_guestinfo_init */

void DispGuestinfo(core_crocods_t *core, u16 keys_pressed0)
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

    dispIcon(core, 219, 30, 6, 4, 0);

    cpcprint16(core, core->overlayBitmap, 320, 10, 53, "Keys:", RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);

    char keyString[256];
    guestGetAllKeyPressed(core, keyString);

    cpcprint16(core, core->overlayBitmap, 320, 10 + 48, 53, keyString, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);
    
    cpcprint16(core, core->overlayBitmap, 320, 10, 53+8, "Joy: ", RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);

    guestGetJoystick(core, keyString);

    cpcprint16(core, core->overlayBitmap, 320, 10 + 48, 53+8, keyString, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);
    
    
     cpcprint16(core, core->overlayBitmap, 320, 10, 103, "Select to quit", RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);

    if (keys_pressed & KEY_SELECT) {
        apps_guestinfo_end(core);
    }
} /* DispKeyboard */
