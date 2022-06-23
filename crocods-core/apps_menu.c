#include "apps_menu.h"

struct kmenu root;
struct kmenu *menuRomId;
struct kmenu *FirstROM;
struct kmenu *keyMenu;

struct kmenu * AddMenu(struct kmenu *parent,int id, int x, int y);

extern char *keyname0[];
extern CPC_SCANCODE keyown[13];
extern int keymenu[13];
extern int keymap[];

void apps_menu_updateKeyMenu(void);

apps_menu_publicmenu apps_menu_publicmenus[APPS_MENU_PUBLICMENU_COUNT] = {
    {ID_SHOW_INFOPANEL, "About", NULL, 0},
    {ID_NULL, "Empty",     "empty",     1 },
    {ID_RESET,    "Reset",     "reset",     1 },
    {ID_AUTODISK, "Insert Media & Autostart",  "autodisk",  1 },
    {ID_SHOW_VIRTUALKEYBOARD, "Show virtual keyboard", "virtualkeyboard",   1 },
    {ID_SCREEN_NEXT,  "Next screen format",    "screennext",    1 },
    {ID_DISK_MENU,    "Media",     NULL,    0 },
    {ID_DISK, "Insert Media",  NULL,    0 },
    {ID_SAVE_LOCALSETTINGS,   "Save local settings",   NULL,    0 },
    {ID_SHOW_BROWSER, "Browser",   NULL,    0 },
    {ID_MONITOR_MENU, "Monitor",   NULL,    0 },
    {ID_COLORMONITOR_MENU,    "Color",     NULL,    0 },
    {ID_COLOR_MONITOR,    "Color Monitor",     NULL,    0 },
    {ID_GREEN_MONITOR,    "Green Monitor",     NULL,    0 },
    {ID_SCREEN_MENU,  "Size",  NULL,    0 },
    {ID_SCREEN_AUTO,  "Auto",  NULL,    0 },
    {ID_SCREEN_320,   "320x200",   NULL,    0 },
    {ID_SCREEN_OVERSCAN,  "Overscan",  NULL,    0 },
    {ID_SCANLINEMENU, "Scanline",  NULL,    0 },
    {ID_NO_SCANLINE,  "No scanline",   NULL,    0 },
    {ID_SCANLINE_5,   "5%",    NULL,    0 },
    {ID_SCANLINE_10,  "10%",   NULL,    0 },
    {ID_SCANLINE_15,  "15%",   NULL,    0 },
    {ID_SCANLINE_20,  "20%",   NULL,    0 },
    {ID_KEY_MENU, "Keyboard",  NULL,    0 },
    {ID_KEY_KEYBOARD, "Use Keyboard",  NULL,    0 },
    {ID_KEY_JOYSTICK, "Set to Joystick",   NULL,    0 },
    {ID_KEY_KEYPAD,   "Set to Keypad",     NULL,    0 },
    {ID_SHOW_VIRTUALKEYBOARD, "Show Virtual Keyboard", NULL,    0 },
    {ID_REDEFINE_MENU,    "Redefine Key",  NULL,    0 },
    {ID_REDEFINE_UP,  "Up: XXXXXXXXXX",    NULL,    0 },
    {ID_REDEFINE_DOWN,    "Down: XXXXXXXXXX",  NULL,    0 },
    {ID_REDEFINE_LEFT,    "Left: XXXXXXXXXX",  NULL,    0 },
    {ID_REDEFINE_RIGHT,   "Right: XXXXXXXXXX", NULL,    0 },
    {ID_REDEFINE_A,   "A: XXXXXXXXXX",     NULL,    0 },
    {ID_REDEFINE_B,   "B: XXXXXXXXXX",     NULL,    0 },
    {ID_REDEFINE_X,   "X: XXXXXXXXXX",     NULL,    0 },
    {ID_REDEFINE_Y,   "Y: XXXXXXXXXX",     NULL,    0 },
    {ID_REDEFINE_START,   "START: XXXXXXXXXX", NULL,    0 },
    {ID_REDEFINE_L,   "L: XXXXXXXXXX",     NULL,    0 },
    {ID_REDEFINE_R,   "R: XXXXXXXXXX",     NULL,    0 },
    {ID_REDEFINE_L2,  "L2: XXXXXXXXXX",    NULL,    0 },
    {ID_REDEFINE_R2,  "R2: XXXXXXXXXX",    NULL,    0 },
    {ID_DEBUG_MENU,   "Debug",     NULL,    0 },
    {ID_FRAMERATEMENU,    "Display framerate", NULL,    0 },
    {ID_DISPFRAMERATE,    "Enable",    NULL,    0 },
    {ID_NODISPFRAMERATE,  "Disable",   NULL,    0 },
    {ID_HACKMENU, "Hack",  NULL,    0 },
    {ID_ACTIONMENU,   "Action",    NULL,    0 },
    {ID_HACK_TABCOUL, "Only one ink refresh per frame: N", NULL,    0 },
    {ID_NOHACK_TABCOUL,   "Normal ink refresh per frame: N",   NULL,    0 },
    {ID_RESET,    "Reset CPC",     NULL,    0 },
    {ID_SNAP_MENU,    "State",     NULL,    0 },
    {ID_SAVESNAP, "Save State",    NULL,    0 },
    {ID_LOADSNAP, "Load State",    NULL,    0 },
    {ID_MENU, "Action",    NULL,    0 },
    {ID_RESET,    "Reset",     NULL,    0 },
    {ID_ADVANCED_MENU,    "Advanced",  NULL,    0 },
    {ID_TURBOMENU,    "Turbo",     NULL,    0 },
    {ID_ENABLE_TURBO, "Enable",    NULL,    0 },
    {ID_DISABLE_TURBO,    "Disable",   NULL,    0 },
    {ID_SOUNDMENU,    "Sound",     NULL,    0 },
    {ID_SOUND_ENABLE, "Enable",    NULL,    0 },
    {ID_SOUND_DISABLE,    "Disable",   NULL,    0 },
    {ID_DEVMENU,  "Developer",     NULL,    0 },
    {ID_SHOW_GUESTINFO,   "Guest Info",    NULL,    0 },
    {ID_EXIT, "Quit",  NULL,    0 }
};


#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

void DispMenu(core_crocods_t *core, u16 keys_pressed0)
{
  if (core->redefineKey) {
  DispKeyboard(core, keys_pressed0);
  if (!core->redefineKey) {
  if (core->redefineKeyRetour != -1) {
 keyown[core->redefineKeyKey] = keymap[core->redefineKeyRetour];
  }
  apps_menu_updateKeyMenu();
  }
  return;
  }
  u16 keys_pressed = appli_begin(core, keys_pressed0);

  core->overlayBitmap_width = 256;
  core->overlayBitmap_height = 168;
  core->overlayBitmap_posx = 0; //  320 - 6 * 32;
  core->overlayBitmap_posy = 0;
  core->overlayBitmap_center = 1;

  // u16 n;
  // for (n = 0; n < 320 * 192; n++) {
  // core->overlayBitmap[n] = 63519;
  // }

  u16 *pdwAddr = core->overlayBitmap; // + ((j * 32) * core->MemBitmap_width) + (i * 32);
  u16 y;

  for (y = 0; y < 168; y++) {
  memcpy(pdwAddr, core->menu + y * 256, 256 * 2);
  pdwAddr += 320;
  }

  int i;
  struct kmenu *folder = core->selectedMenu->parent;
  struct kmenu *firstMenu = folder->firstchild;
  struct kmenu *selectedChildMenu = NULL;

  for (i = 0; i < folder->nbr; i++) {
  if (firstMenu == core->selectedMenu) {
  cpcprint16(core, core->overlayBitmap, 320, 9, 34 + i * 9, firstMenu->title, RGB565(0x00, 0xFF, 0xFF), RGB565(0x00, 0x00, 0xFF), 1, 0);
  } else {
  cpcprint16(core, core->overlayBitmap, 320, 9, 34 + i * 9, firstMenu->title, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0x00), 1, 0);
  }

  if (firstMenu->firstchild != NULL) {
  dispIcon8(core, 9 + (int)strlen(firstMenu->title) * 8, 34 + i * 9, 3);
  }

  firstMenu = firstMenu->nextSibling;
  }

  dispIcon(core, 219, 30, core->selectedMenu->x, core->selectedMenu->y, 0);

  int z = 0;
  switch (core->selectedMenu->id) {
  case ID_MENU:
  z = ID_AUTODISK;
  break;
  case ID_COLORMONITOR_MENU:
  z = (core->lastcolour == 0) ? ID_GREEN_MONITOR : ID_COLOR_MONITOR;
  break;
  case ID_SCREEN_MENU:
  if (core->resize == 1) { // AUTO
 z = ID_SCREEN_AUTO;
  } else if (core->resize == 2) { // 320
 z = ID_SCREEN_320;
  } else if (core->resize == 3) { // NO-RESIZE
 z = ID_SCREEN_NORESIZE; // Not used
  } else if (core->resize == 4) { // OVERSCAN
 z = ID_SCREEN_OVERSCAN;
  }
  break;
  case ID_SCANLINEMENU:
  if (core->scanline == 0) {
 z = ID_NO_SCANLINE;
  } else if (core->scanline == 1) {
 z = ID_SCANLINE_5;
  } else if (core->scanline == 2) {
 z = ID_SCANLINE_10;
  } else if (core->scanline == 3) {
 z = ID_SCANLINE_15;
  } else if (core->scanline == 4) {
 z = ID_SCANLINE_20;
  }
  break;
  case ID_TURBOMENU:
  if (core->turbo == 0) {
 z = ID_DISABLE_TURBO;
  } else if (core->turbo == 1) {
 z = ID_ENABLE_TURBO;
  }
  break;
  case ID_SOUNDMENU:
  if (core->soundEnabled == 0) {
 z = ID_SOUND_DISABLE;
  } else if (core->soundEnabled == 1) {
 z = ID_SOUND_ENABLE;
  }
  break;
  case ID_KEY_MENU:
  if (core->keyEmul == 2) {
 z = ID_KEY_KEYBOARD;
  } else if (core->keyEmul == 3) {
 if (keyown[0] == CPC_JOY_UP) {
 z = ID_KEY_JOYSTICK;
 } else if (keyown[0] == CPC_CURSOR_UP) {
 z = ID_KEY_KEYPAD;
 }
  }
  break;
  case ID_FRAMERATEMENU:
  z = core->dispframerate ? ID_DISPFRAMERATE : ID_NODISPFRAMERATE;
  break;
  default: // Menu without selector
  z = 0;
  break;
  } /* switch */

  if (z != 0) {
  struct kmenu *folder = core->selectedMenu;
  struct kmenu *firstMenu = folder->firstchild;

  for (i = 0; i < folder->nbr; i++) {
  if (firstMenu->id == z) {
 selectedChildMenu = firstMenu;
 dispIcon(core, 219, 30, firstMenu->x, firstMenu->y, 0);
 break;
  }

  firstMenu = firstMenu->nextSibling;
  }
  }

  if ((keys_pressed & KEY_UP) == KEY_UP) {
  if (core->selectedMenu->previousSibling != NULL) {
  core->selectedMenu = core->selectedMenu->previousSibling;
  } else {
  core->selectedMenu = core->selectedMenu->parent->lastchild;
  }
  }

  if ((keys_pressed & KEY_DOWN) == KEY_DOWN) {
  if (core->selectedMenu->nextSibling != NULL) {
  core->selectedMenu = core->selectedMenu->nextSibling;
  } else {
  core->selectedMenu = core->selectedMenu->parent->firstchild;
  }
  }

  if ((keys_pressed & KEY_A) == KEY_A) {
  CPC_ClearScanCode(core, keyown[1]);

  if (core->selectedMenu->firstchild == NULL) {
  int ret = ExecuteMenu(core, core->selectedMenu->id, NULL);

  // 0 -> return to parent
  // 1 -> return emulator
  // 2 -> return to item (switch)
  switch (ret) {
 case 0:
 core->selectedMenu = core->selectedMenu->parent;
 break;
 case 1:
 ExecuteMenu(core, ID_MENU_EXIT, NULL);
 break;
 case 2:
 break;
  }
  } else {
  if (selectedChildMenu != NULL) {
 core->selectedMenu = selectedChildMenu;
  } else {
 core->selectedMenu = core->selectedMenu->firstchild;
  }
  }
  }

  if (((keys_pressed & KEY_B) == KEY_B) || ((keys_pressed & KEY_SELECT) == KEY_SELECT)) {
  if (core->selectedMenu->parent == &root) {
  ExecuteMenu(core, ID_MENU_EXIT, NULL);
  return;
  }
  core->selectedMenu = core->selectedMenu->parent;
  }
} /* DispIcons */

struct kmenu * AddMenu(struct kmenu *parent, int id, int x, int y)
{
  struct kmenu *kcur;

  char *title = apps_menu_TitleFromID(id);

  kcur = (struct kmenu *)calloc(sizeof(struct kmenu), 1);
  kcur->parent = parent;
  kcur->firstchild = NULL;
  kcur->lastchild = NULL;
  kcur->nextSibling = NULL;
  kcur->previousSibling = NULL;
  kcur->nbr = 0;
  strcpy(kcur->title, title);
  kcur->id = id;
  kcur->x = x;
  kcur->y = y;

  if (kcur->parent->nbr == 0) {
  kcur->parent->firstchild = kcur;
  kcur->parent->lastchild = kcur;
  kcur->parent->nbr = 1;
  } else {
  struct kmenu *i, *i0;

  i0 = NULL;
  i = kcur->parent->firstchild;
  do {
  if (strcmp(kcur->title, i->title) < 0) {
 break; // placer kcur juste avant i
  }
  i0 = i;
  i = i->nextSibling;
  } while (i != NULL);

  i = NULL;

  if (i == NULL) {
  kcur->previousSibling = kcur->parent->lastchild;
  kcur->parent->lastchild->nextSibling = kcur;
  kcur->parent->lastchild = kcur;
  } else {
  kcur->nextSibling = i;
  if (i0 == NULL) {
 kcur->parent->firstchild = kcur;
  } else {
 i0->nextSibling = kcur;
  }
  }
  kcur->parent->nbr++;
  }

  return kcur;
} /* AddMenu */

void apps_menu_geticon(core_crocods_t *core, int id, int *x, int *y)
{
}

void apps_menu_init(core_crocods_t *core)
{
  struct kmenu *menuMonitor = NULL;
  struct kmenu *menuAdvanced = NULL;
  struct kmenu *menuDiskId = NULL;

  printf("apps_menu_init\n");

  struct kmenu *id;

  root.nbr = 0;

  AddMenu(&root,  ID_SHOW_INFOPANEL, 6, 4);

  if (core->resources == NULL) {
  menuDiskId = AddMenu(&root, ID_DISK_MENU, 0, 0);
  AddMenu(menuDiskId, ID_AUTODISK, 0, 1);
  AddMenu(menuDiskId, ID_DISK, 0, 2);
  AddMenu(menuDiskId,  ID_SAVE_LOCALSETTINGS, 0, 5);
  #ifdef REDBUG
  AddMenu(menuDiskId, ID_SHOW_BROWSER, 8, 0);
  #endif
  }

  menuMonitor = AddMenu(&root,  ID_MONITOR_MENU, 2, 4);

  id = AddMenu(menuMonitor,  ID_COLORMONITOR_MENU, 0, 0);
  AddMenu(id, ID_COLOR_MONITOR, 1, 1);
  AddMenu(id, ID_GREEN_MONITOR, 1, 0);

  id = AddMenu(menuMonitor,  ID_SCREEN_MENU, 0, 0);
  AddMenu(id,  ID_SCREEN_AUTO, 2, 2);
  AddMenu(id, ID_SCREEN_320, 2, 3);
  AddMenu(id,  ID_SCREEN_OVERSCAN, 2, 0);

  id = AddMenu(menuMonitor,  ID_SCANLINEMENU, 0, 0);
  AddMenu(id,  ID_NO_SCANLINE, 2, 5);
  AddMenu(id,  ID_SCANLINE_5, 2, 6);
  AddMenu(id,  ID_SCANLINE_10, 2, 7);
  AddMenu(id, ID_SCANLINE_15, 2, 8);
  AddMenu(id,  ID_SCANLINE_20, 2, 9);

  id = AddMenu(&root, ID_KEY_MENU, 0, 0);

#if defined(_WIN32) || defined(TARGET_OS_MAC)
  AddMenu(id, ID_KEY_KEYBOARD, 3, 0); // Only on desktop
#endif
  AddMenu(id,  ID_KEY_JOYSTICK, 3, 1);
  AddMenu(id,  ID_KEY_KEYPAD, 3, 2);

  AddMenu(id,  ID_SHOW_VIRTUALKEYBOARD, 3, 0);

  keyMenu = AddMenu(id,  ID_REDEFINE_MENU, 0, 0);
  AddMenu(keyMenu,  ID_REDEFINE_UP, 0, 0);
  AddMenu(keyMenu, ID_REDEFINE_DOWN, 0, 0);
  AddMenu(keyMenu, ID_REDEFINE_LEFT, 0, 0);
  AddMenu(keyMenu, ID_REDEFINE_RIGHT, 0, 0);
  AddMenu(keyMenu, ID_REDEFINE_A, 0, 0);
  AddMenu(keyMenu,  ID_REDEFINE_B, 0, 0);
  AddMenu(keyMenu,  ID_REDEFINE_X, 0, 0);
  AddMenu(keyMenu,  ID_REDEFINE_Y, 0, 0);
  AddMenu(keyMenu,  ID_REDEFINE_START, 0, 0);
  AddMenu(keyMenu,  ID_REDEFINE_L, 0, 0);
  AddMenu(keyMenu, ID_REDEFINE_R, 0, 0);
  AddMenu(keyMenu, ID_REDEFINE_L2, 0, 0);
  AddMenu(keyMenu,  ID_REDEFINE_R2, 0, 0);

#ifdef REDBUG
  if (core->resources == NULL) {
  struct kmenu *menuDebug = NULL;

  menuDebug = AddMenu(&root,  ID_DEBUG_MENU, 0, 0);

  id = AddMenu(menuDebug,  ID_FRAMERATEMENU, -1, 0);
  AddMenu(id,  ID_DISPFRAMERATE, 4, 0);
  AddMenu(id,  ID_NODISPFRAMERATE, 4, 1);
  }
#endif

  //  id=AddMenu(&root,"Hack", ID_MENU, 0, 0);
  //  AddMenu(id, "Only one ink refresh per frame: N", ID_HACK_TABCOUL, 5, 0);
  //  AddMenu(id, "Normal ink refresh per frame: N", ID_NOHACK_TABCOUL, 5, 0);

  //  AddMenu(&root, "Reset CPC", ID_RESET);

  if (core->resources == NULL) {
  id = AddMenu(&root,  ID_SNAP_MENU, 6, 0);
  AddMenu(id, ID_SAVESNAP, 6, 1);
  AddMenu(id,  ID_LOADSNAP, 6, 2);
  }

  id = AddMenu(&root,  ID_ACTIONMENU, 7, 0);
  AddMenu(id, ID_RESET, 7, 1);

  menuAdvanced = AddMenu(&root,  ID_ADVANCED_MENU, 8, 0);
#ifdef REDBUG
  id = AddMenu(menuAdvanced,  ID_TURBOMENU, -1, 0);
  AddMenu(id,  ID_ENABLE_TURBO, 4, 2);
  AddMenu(id,  ID_DISABLE_TURBO, 4, 3);
#endif

  id = AddMenu(menuAdvanced, ID_SOUNDMENU, -1, 0);
  AddMenu(id,  ID_SOUND_ENABLE, 4, 2);
  AddMenu(id,  ID_SOUND_DISABLE, 4, 3);

#ifdef REDBUG
  if (1 == 1) {
  id = AddMenu(&root, ID_DEVMENU, 8, 0);
  AddMenu(id,  ID_SHOW_GUESTINFO, 8, 1);
  }
#endif

  AddMenu(&root,  ID_EXIT, 7, 2);

  if (core->resources == NULL) {
  core->selectedMenu = menuDiskId;
  } else {
  core->selectedMenu = menuMonitor;
  }

  apps_menu_updateKeyMenu();

  core->runApplication = DispMenu;
} /* apps_menu_init */

void apps_menu_updateKeyMenu(void)
{
  struct kmenu *first;

  // keyMenu

  first = keyMenu->firstchild;

  while (first != NULL) {
  switch (first->id) {
  case ID_REDEFINE_UP:
 sprintf(first->title, "Up : %s", keyname0[keyown[0]]);
 break;
  case ID_REDEFINE_DOWN:
 sprintf(first->title, "Down : %s", keyname0[keyown[1]]);
 break;
  case ID_REDEFINE_LEFT:
 sprintf(first->title, "Left : %s", keyname0[keyown[2]]);
 break;
  case ID_REDEFINE_RIGHT:
 sprintf(first->title, "Right: %s", keyname0[keyown[3]]);
 break;
  case ID_REDEFINE_START:
 sprintf(first->title, "Start: %s", keyname0[keyown[4]]);
 break;
  case ID_REDEFINE_A:
 sprintf(first->title, "A  : %s", keyname0[keyown[5]]);
 break;
  case ID_REDEFINE_B:
 sprintf(first->title, "B  : %s", keyname0[keyown[6]]);
 break;
  case ID_REDEFINE_X:
 sprintf(first->title, "X  : %s", keyname0[keyown[7]]);
 break;
  case ID_REDEFINE_Y:
 sprintf(first->title, "Y  : %s", keyname0[keyown[8]]);
 break;
  case ID_REDEFINE_L:
 sprintf(first->title, "L  : %s", keyname0[keyown[9]]);
 break;
  case ID_REDEFINE_R:
 sprintf(first->title, "R  : %s", keyname0[keyown[10]]);
 break;
  case ID_REDEFINE_L2:
 sprintf(first->title, "L2 : %s", keyname0[keyown[11]]);
 break;
  case ID_REDEFINE_R2:
 sprintf(first->title, "R2 : %s", keyname0[keyown[12]]);
 break;
  } /* switch */

  first = first->nextSibling;
  }
} /* apps_menu_updateKeyMenu */

char * apps_menu_TitleFromID(int menuId)
{
  int n;

  for (n = 0; n < APPS_MENU_PUBLICMENU_COUNT; n++) {
  if (apps_menu_publicmenus[n].id == menuId) {
  return apps_menu_publicmenus[n].title;
  }
  }
    return "";
}

char * apps_menu_KeywordFromID(int menuId)
{
  int n;

  for (n = 0; n < APPS_MENU_PUBLICMENU_COUNT; n++) {
  if (apps_menu_publicmenus[n].id == menuId) {
  return apps_menu_publicmenus[n].keyword;
  }
  }
  return "empty";
}

int apps_menu_IDFromKeyword(const char *keyword)
{
  int n;

  for (n = 0; n < APPS_MENU_PUBLICMENU_COUNT; n++) {
      if ((apps_menu_publicmenus[n].keyword!=NULL) && (!strcasecmp(apps_menu_publicmenus[n].keyword, keyword))) {
  return apps_menu_publicmenus[n].id;
  }
  }
  return ID_NULL;
}
