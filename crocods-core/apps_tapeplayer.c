#include "apps_tapeplayer.h"

void DispTapePlayer(core_crocods_t *core, u16 keys_pressed0)
{
    int y;

    core->ipc.keys_pressed = 0;
    memset(core->clav, 0xFF, sizeof(core->clav));

    u16 keys_pressed;

    if (keys_pressed0 != core->last_keys_pressed) {
        keys_pressed = keys_pressed0;
        core->last_keys_pressed = keys_pressed;
    } else {
        keys_pressed = 0;
    }


    core->overlayBitmap_width = 256;
    core->overlayBitmap_height = 155;
    core->overlayBitmap_posx = (320 - 256) / 2;
    core->overlayBitmap_posy = (240 - 192) / 2;
    core->overlayBitmap_center = 1;

    u16 *pdwAddr = core->overlayBitmap; // + ((j * 32) * core->MemBitmap_width) + (i * 32);

    for (y = 0; y < 155; y++) {
        memcpy(pdwAddr, core->tape + y * 256, 256 * 2);
        pdwAddr += 320;
    }

    if (((keys_pressed & KEY_B) == KEY_B) || ((keys_pressed & KEY_R) == KEY_R)) {
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;
    }

}
