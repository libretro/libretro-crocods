#include "platform.h"

u16 appli_begin(core_crocods_t *core, u16 keys_pressed0)
{
    u16 keys_pressed;
    core->ipc.keys_pressed = 0;
    memset(core->clav, 0xFF, sizeof(core->clav));


    if (keys_pressed0 != core->last_keys_pressed)
    {
        keys_pressed = keys_pressed0 & (~core->last_keys_pressed);
        core->last_keys_pressed = keys_pressed0;
    }
    else
        keys_pressed = 0;

    return keys_pressed;
}
