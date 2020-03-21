#include "plateform.h"
#include "z80_cap32.h"

#include <string.h>
#include <stdarg.h>

extern t_z80regs z80;

void DispDebugger(core_crocods_t *core, u16 keys_pressed0);

#define     USER_DELETED 0xE5

#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

int apps_debugger_files_count = 0;
int apps_debugger_files_begin = 0;
int apps_debugger_files_selected = 0;

int apps_debugger_files_flag = 0;

void apps_debugger_end(core_crocods_t *core)
{
    core->runApplication = NULL;

    core->wait_key_released = 1;
}

void apps_debugger_init(core_crocods_t *core, int flag)
{
    printf("apps_debugger_init: %s\n", core->openFilename);

    core->runApplication = DispDebugger;
    apps_debugger_files_flag = flag;

    apps_debugger_files_count = 0;
    apps_debugger_files_begin = 0;
    apps_debugger_files_selected = 0;
} /* apps_debugger_init */

void apps_debugger_printat(core_crocods_t *core, int x, int y, u16 color, const char *fmt, ...)
{
    char tmp[512];

    va_list args;

    va_start(args, fmt);
    vsprintf(tmp, fmt, args);
    va_end(args);

    cpcprint16(core, core->overlayBitmap, 320, x, y, tmp, color, RGB565(0x00, 0x00, 0x00), 1, 1);
} /* mydebug */

void DispDebugger(core_crocods_t *core, u16 keys_pressed0)
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
        memcpy(pdwAddr, core->select + y * 256, 256 * 2);
        pdwAddr += 320;
    }

    apps_debugger_printat(core, 0, 0, RGB565(0xFF, 0x00, 0x00), "  A:");
    apps_debugger_printat(core, 24, 0, RGB565(0xFF, 0x00, 0x00), "#%02x", z80.AF.b.h);
    apps_debugger_printat(core, 0, 8, RGB565(0xFF, 0x00, 0x00), "  B:");
    apps_debugger_printat(core, 24, 8, RGB565(0xFF, 0x00, 0x00), "#%02x", z80.BC.b.h);

    apps_debugger_printat(core, 0, 80, RGB565(0xFF, 0x00, 0x00), " SP:");
    apps_debugger_printat(core, 24, 80, RGB565(0xFF, 0x00, 0x00), "#%04x", z80.SP.w.l);

    if (keys_pressed & KEY_A) {
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;

        apps_debugger_end(core);
    }

    if (((keys_pressed & KEY_B) == KEY_B) || ((keys_pressed & KEY_R) == KEY_R)) {
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;
        // ExecuteMenu(core, ID_MENU_EXIT, NULL);
    }

    if ((keys_pressed & KEY_UP) == KEY_UP) {
    }

    if ((keys_pressed & KEY_DOWN) == KEY_DOWN) {
    }
} /* DispKeyboard */
