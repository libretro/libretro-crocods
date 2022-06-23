#ifndef APPSSHARED_H
#define APPSSHARED_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "platform.h"

#include "apps_keyboard.h"
#include "apps_tapeplayer.h"
#include "apps_menu.h"
#include "apps_autorun.h"
#include "apps_disk.h"
#include "apps_infopanel.h"
#include "apps_guestinfo.h"
#include "apps_browser.h"

    u16 appli_begin(core_crocods_t *core, u16 keys_pressed);

#ifdef __cplusplus
}
#endif

#endif // ifndef APPSSHARED_H
