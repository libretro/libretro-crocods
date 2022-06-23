#ifndef _APPS_MENU_H_
#define _APPS_MENU_H_

#include "platform.h"

typedef struct {
    int id;
    char *title;
    char *keyword;
    char keyable;
} apps_menu_publicmenu;

#define APPS_MENU_PUBLICMENU_COUNT 68
extern apps_menu_publicmenu apps_menu_publicmenus[APPS_MENU_PUBLICMENU_COUNT];

void DispMenu(core_crocods_t *core, u16 keys_pressed);
void apps_menu_init(core_crocods_t *core);

char * apps_menu_KeywordFromID(int menuId);
char * apps_menu_TitleFromID(int menuId);
int apps_menu_IDFromKeyword(const char *keyword);

#endif
