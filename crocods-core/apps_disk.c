#include "platform.h"
#include <dirent.h>

void DispAppsDisk(core_crocods_t *core, u16 keys_pressed0);
void apps_disk_readdir(core_crocods_t *core);

void apps_disk_tpath2Abs(char *p, char *Ficname);
void apps_disk_path2Abs(char *p, const char *relatif);

#define     USER_DELETED 0xE5

#ifdef _WIN32
#define DEFSLASH         '\\'
#else
#define DEFSLASH         '/'
#endif

#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

typedef struct {
    char *filename;
    unsigned char folder;
} DirEntryCroco;

DirEntryCroco *apps_disk_files;
int apps_disk_files_count = 0;
int apps_disk_files_begin = 0;
int apps_disk_files_selected = 0;

int apps_disk_files_flag = 0;

void apps_disk_end(core_crocods_t *core)
{
    core->runApplication = NULL;

    core->wait_key_released = 1;
}

void apps_disk_init(core_crocods_t *core, int flag)
{
    apps_disk_files_flag = flag;
    core->runApplication = DispAppsDisk;

    apps_disk_files = NULL;

    apps_disk_readdir(core);
}

static int apps_disk_compare(void const *a, void const *b)
{
    DirEntryCroco *ad, *bd;

    ad = (DirEntryCroco *)a;
    bd = (DirEntryCroco *)b;

    if (ad->folder == bd->folder) {
        return strcasecmp(ad->filename, bd->filename);
    }

    if (ad->folder > bd->folder) {
        return -1;
    }
    return 1;
}

void apps_disk_readdir(core_crocods_t *core)
{
    apps_disk_files_count = 0;
    apps_disk_files_begin = 0;
    apps_disk_files_selected = 0;

    printf("Open dir %s\n", core->file_dir);

    DIR *d;
    struct dirent *dir;
    d = opendir(core->file_dir);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
//            printf("%s\n", dir->d_name);

            char filename[256];

            strcpy(filename, dir->d_name);

            char *ext = strrchr(filename, '.');
            if (ext != NULL) {
                ext++;
            }

            char ok = 0;
            char folder = 0;

            if ((ext != NULL) &&
                ((!strcasecmp(ext, "sna")) ||
                 (!strcasecmp(ext, "dsk")) ||
                 (!strcasecmp(ext, "bas")) ||
                 (!strcasecmp(ext, "kcr")) ||
                 (!strcasecmp(ext, "cpr")) ||
                 (!strcasecmp(ext, "rom")) ||
                 (!strcasecmp(ext, "zip")))) {
                ok = 1;
            }

            if (!ok) {
                char directory[2048];
                struct stat s;

                strcpy(directory, core->file_dir);
                apps_disk_path2Abs(directory, dir->d_name);

                stat(directory, &s);

                if (S_ISDIR(s.st_mode)) {
                    if ((filename[0] != '.') || (!strcmp(filename, ".."))) {
                        ok = 1;
                        folder = 1;
                    }
                }
            }

            if (ok) {
                apps_disk_files = (DirEntryCroco *)realloc(apps_disk_files, sizeof(DirEntryCroco) * (apps_disk_files_count + 1));

                apps_disk_files[apps_disk_files_count].filename = malloc(strlen(filename) + 1);
                apps_disk_files[apps_disk_files_count].folder = folder;

                strcpy(apps_disk_files[apps_disk_files_count].filename, filename);
                apps_disk_files_count++;
            }
        }
        closedir(d);

        qsort(apps_disk_files, apps_disk_files_count, sizeof(DirEntryCroco), apps_disk_compare);

        int n;
        for (n = 0; n < apps_disk_files_count; n++) {
            if (!strcasecmp(apps_disk_files[n].filename, core->filename)) {
                apps_disk_files_selected = n;
                if (apps_disk_files_selected > apps_disk_files_begin + 12) {
                    apps_disk_files_begin = apps_disk_files_selected - 12;
                }
            }
        }
    } else {
//        printf("Error failed to open input directory -%s\n",strerror(errno) );
    }

    if (apps_disk_files_count == 0) {
        char directory[2048];
        strcpy(directory, core->file_dir);
        apps_disk_path2Abs(directory, "..");

        core->file_dir = (char *)realloc(core->file_dir, strlen(directory) + 1);
        strcpy(core->file_dir, directory);
        
        apps_disk_readdir(core);
    }
} /* apps_disk_readdir */

void DispAppsDisk(core_crocods_t *core, u16 keys_pressed0)
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

    // Text begin in 12,36 (max 13 lines, 30 columns)

    for (y = 0; y < 168; y++) {
        memcpy(pdwAddr, core->select + y * 256, 256 * 2);
        pdwAddr += 320;
    }

    char *title = "Open a disk or a snapshot";
    cpcprint16(core, core->overlayBitmap, 320, (256 - (int)strlen(title) * 8) / 2, 15, title, RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);

    for (y = 0; y < 13; y++) {
        int n = y + apps_disk_files_begin;

        if ((n < apps_disk_files_count) && (n >= 0)) {
            char text[27 + 1];
            char filename[2048];

            strcpy(filename, apps_disk_files[n].filename);

            char *ext;

            if (!strcmp(filename, "..")) {
                strcpy(filename, "[UP]");
                ext = NULL;
            } else {
                ext = strrchr(filename, '.');
                if (ext != NULL) {
                    *ext = 0;
                    ext++;
                }
            }

            if (apps_disk_files[n].folder == 1) {
                dispIcon8(core, 12 + 4, 36 + y * 8, 2);
            } else if ((ext != NULL) && (!strcasecmp(ext, "sna"))) {
                dispIcon8(core, 12 + 4, 36 + y * 8, 1);
            } else if ((ext != NULL) && (!strcasecmp(ext, "dsk"))) {
                dispIcon8(core, 12 + 4, 36 + y * 8, 0);
            } else if ((ext != NULL) && (!strcasecmp(ext, "zip"))) {
                dispIcon8(core, 12 + 4, 36 + y * 8, 4);
            }

            snprintf(text, 27, "%s", filename);
            text[27] = 0;

            if (n == apps_disk_files_selected) {
                cpcprint16(core, core->overlayBitmap, 320, 12 + 2 * 8, 36 + y * 8, text, RGB565(0x00, 0xFF, 0xFF), RGB565(0x00, 0x00, 0xFF), 1, 0);
            } else {
                cpcprint16(core, core->overlayBitmap, 320, 12 + 2 * 8, 36 + y * 8, text, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0x00), 1, 0);
            }
        }
    }

    if (keys_pressed & KEY_A) {
        if (apps_disk_files[apps_disk_files_selected].folder == 1) {
            char directory[2048];
            strcpy(directory, core->file_dir);
            apps_disk_path2Abs(directory, apps_disk_files[apps_disk_files_selected].filename);

            core->file_dir = (char *)realloc(core->file_dir, strlen(directory) + 1);
            strcpy(core->file_dir, directory);

            apps_disk_readdir(core);

            return;
        }
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;

        strcpy(core->openFilename, core->file_dir);
        apps_disk_path2Abs(core->openFilename, apps_disk_files[apps_disk_files_selected].filename);

        ExecuteMenu(core, ID_PAUSE_EXIT, NULL);

        if (apps_disk_files_flag == 1) {
            ExecuteMenu(core, ID_AUTORUN, NULL);
        } else {
            ExecuteMenu(core, ID_INSERTDISK, NULL);
        }
        return;
    }

    if (((keys_pressed & KEY_B) == KEY_B) || ((keys_pressed & KEY_R) == KEY_R)) {
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;
        // ExecuteMenu(core, ID_MENU_EXIT, NULL);
    }

    if ((keys_pressed & KEY_UP) == KEY_UP) {
        apps_disk_files_selected--;
        if (apps_disk_files_selected < 0) {
            apps_disk_files_selected = apps_disk_files_count - 1;
            apps_disk_files_begin = apps_disk_files_count - 13;
            if (apps_disk_files_begin < 0) {
                apps_disk_files_begin = 0;
            }
        }
        if (apps_disk_files_selected < apps_disk_files_begin) {
            apps_disk_files_begin--;
        }
    }

    if ((keys_pressed & KEY_DOWN) == KEY_DOWN) {
        apps_disk_files_selected++;
        if (apps_disk_files_selected >= apps_disk_files_count) {
            apps_disk_files_selected = 0;
            apps_disk_files_begin = 0;
        }
        if (apps_disk_files_selected > apps_disk_files_begin + 12) {
            apps_disk_files_begin++;
        }
    }

    if ((keys_pressed & KEY_LEFT) == KEY_LEFT) {
        apps_disk_files_selected -= 10;
        if (apps_disk_files_selected < 0) {
            apps_disk_files_selected = 0;
            apps_disk_files_begin = apps_disk_files_count - 13;
            if (apps_disk_files_begin < 0) {
                apps_disk_files_begin = 0;
            }
        }
        if (apps_disk_files_selected < apps_disk_files_begin) {
            apps_disk_files_begin = apps_disk_files_selected;
        }
    }

    if ((keys_pressed & KEY_RIGHT) == KEY_RIGHT) {
        apps_disk_files_selected += 10;
        if (apps_disk_files_selected >= apps_disk_files_count) {
            apps_disk_files_selected = apps_disk_files_count - 1;
        }
        if (apps_disk_files_selected > apps_disk_files_begin + 12) {
            apps_disk_files_begin = apps_disk_files_selected - 12;
        }
    }
} /* DispKeyboard */

void apps_disk_path2Abs(char *p, const char *relatif)
{
    static char Ficname[256];
    int l, m, n;
    char car;

    // printf("Path2abs %s with %s\n", p, relatif);

    if (relatif[0] == 0) return;

    strcpy(Ficname, relatif);

    for (n = 0; n < strlen(p); n++) {
        if (p[n] == '/') p[n] = DEFSLASH;
    }

    for (n = 0; n < strlen(Ficname); n++) {
        if (Ficname[n] == '/') Ficname[n] = DEFSLASH;
    }

    m = 0;
    l = (int)strlen(Ficname);

    if ((Ficname[l - 1] == DEFSLASH) & (l != 1)) { // --- retire le dernier slash ---
        if (Ficname[l - 2] != ':') { // --- drive --------------------------------
            l--;
            Ficname[l] = 0;
        }
    }

    for (n = 0; n < l; n++) {
        if (Ficname[n] == DEFSLASH) {
            car = Ficname[n + 1];

            Ficname[n + 1] = 0;

            apps_disk_tpath2Abs(p, Ficname + m);

            Ficname[n + 1] = car;
            Ficname[n] = 0;

            m = n + 1;
        }
    }

    apps_disk_tpath2Abs(p, Ficname + m);

    if (p[0] == 0) {
        p[0] = DEFSLASH;
        p[1] = 0;
    }

    // printf("Path2abs result: %s\n", p);
} /* apps_disk_path2Abs */

void apps_disk_tpath2Abs(char *p, char *Ficname)
{
    int n;
    static char old[256]; // --- Path avant changement ------------------
    signed int deuxpoint;
    char defslash[2];

    defslash[0] = DEFSLASH;
    defslash[1] = 0;

    if (Ficname[0] == 0) return;

    memcpy(old, p, 256);

    if (p[strlen(p) - 1] == DEFSLASH) p[strlen(p) - 1] = 0;

    if ( (!strncmp(Ficname, "..", 2)) & (p[0] != 0) ) {
        for (n = (int)strlen(p); n > 0; n--) {
            if (p[n] == DEFSLASH) {
                p[n] = 0;
                break;
            }
        }
        if (p[strlen(p) - 1] == ':') strcat(p, defslash);
        return;
    }

    if ((Ficname[0] != '.') || (Ficname[1] != '.')) {
        deuxpoint = -1;

        for (n = 0; n < strlen(Ficname); n++) {
            if (Ficname[n] == ':') deuxpoint = n;
        }

        if (deuxpoint != -1) {
            if (Ficname[deuxpoint + 1] == DEFSLASH) {
                strcpy(p, Ficname);
                if (p[strlen(p) - 1] == ':') strcat(p, defslash);
                return;
            }
        }

        if (Ficname[0] == DEFSLASH) {
            if (p[1] == ':') {
                strcpy(p + 2, Ficname);
            } else {
                strcpy(p, Ficname);
            }
            if (p[strlen(p) - 1] == ':') strcat(p, defslash);
            return;
        }

        if (p[strlen(p) - 1] != DEFSLASH) strcat(p, defslash);
        strcat(p, Ficname);
    }

    if (p[strlen(p) - 1] == ':') strcat(p, defslash);
} /* apps_disk_tpath2Abs */
