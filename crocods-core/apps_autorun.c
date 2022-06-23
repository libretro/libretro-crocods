#include "platform.h"

#ifdef ZIP_SUPPORT
#include "miniz.h"
#endif

#include "idsk_lite.h"

void DispAutorun(core_crocods_t *core, u16 keys_pressed0);
void LireDiskMem(core_crocods_t *core, u8 *rom, u32 romsize);

#define     USER_DELETED 0xE5

#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

#pragma pack(1)

typedef struct {
    unsigned char User;
    char Nom[ 8 ];
    char Ext[ 3 ];
    unsigned char NumPage;
    unsigned char Unused[ 2 ];
    unsigned char NbPages;
    unsigned char Blocks[ 16 ];
} StDirEntry;

#pragma pack()

typedef struct {
    unsigned char user;
    char name[ 8 + 1 ];
    char ext[ 3 + 1 ];
    int nbpages;
} StDirEntryCroco;

StDirEntryCroco apps_autorun_files[64];
int apps_autorun_files_count = 0;
int apps_autorun_files_begin = 0;
int apps_autorun_files_selected = 0;

int apps_autorun_files_flag = 0;

void apps_autorun_end(core_crocods_t *core)
{
    core->runApplication = NULL;

    core->wait_key_released = 1;
}

void apps_autorun_init(core_crocods_t *core, int flag)
{
    if ((core->openFilename[0] == 0) && (core->resources == NULL)) {
        apps_autorun_end(core);
        return;
    }

    core->runApplication = DispAutorun;
    apps_autorun_files_flag = flag;

    apps_autorun_files_count = 0;
    apps_autorun_files_begin = 0;
    apps_autorun_files_selected = 0;

    u8 *snapshot = NULL;
    u32 snapshotLength;

    u8 *dsk;
    long dsk_size;

    if (core->resources != NULL)
    {
        dsk_size = core->resources_len;

        dsk = (u8 *)malloc(core->resources_len);
        memcpy(dsk, core->resources, core->resources_len);
    } else {
        FILE *fic = fopen(core->openFilename, "rb");
        if (fic == NULL) {
            appendIcon(core, 0, 4, 60);
            apps_autorun_end(core);
            return;
        }
        fseek(fic, 0, SEEK_END);
        dsk_size = ftell(fic);
        fseek(fic, 0, SEEK_SET);

        dsk = (u8 *)malloc(dsk_size);
        if (dsk == NULL) {
            appendIcon(core, 0, 4, 60);
            apps_autorun_end(core);
            return;
        }
        fread(dsk, 1, dsk_size, fic);
        fclose(fic);

        char *p = strrchr(core->openFilename, '/');

        strcpy(core->filename, p ? p + 1 : (char *)core->openFilename);

        // Copy basename of filename to current directory

        char directory[2048];
        strcpy(directory, core->openFilename);
        apps_disk_path2Abs(directory, "..");

        core->file_dir = (char *)realloc(core->file_dir, strlen(directory) + 1);
        strcpy(core->file_dir, directory);
    }

    char *ext = strrchr(core->openFilename, '.');
    if (ext != NULL) {
        ext++;
        if (!strcasecmp(ext, "bas")) {
            u8 *ImgDsk = idsk_createNewDisk();

            // TODO: check 0x0A, 0x0D for carriage return

            idsk_importFile(ImgDsk, dsk, (u32)dsk_size, "autorun.bas");

            u32 length;
            u8 *buf = (u8 *)idsk_getDiskBuffer(ImgDsk, &length);

            BOOL autoStart = apps_autorun_files_flag;

            LireDiskMem(core, buf, length);
            loadIni(core, 1);           // Load local in file

            if (!autoStart) {
                apps_autorun_end(core);
            }
            return;
        }
        
        if (!strcasecmp(ext, "rom")) {
            
//                memcpy(core->ROMINF, cpc6128_bin, sizeof(core->ROMINF));
//                memcpy(core->ROMEXT[0], cpc6128_bin + 0x4000, sizeof(core->ROMEXT[0]));
//                memcpy(core->ROMEXT[7], romdisc_bin, sizeof(core->ROMEXT[7]));
            memcpy(core->ROMEXT[7], dsk, dsk_size);

            SoftResetCPC(core);

            apps_autorun_end(core);
            return;
        }
    }

    char header[32];

    if (dsk_size < 32) {
        appendIcon(core, 0, 4, 60);
        apps_autorun_end(core);
        return;
    }

    memcpy(header, dsk, 32);

    if (!memcmp(header, "MV - SNA", 8)) {  // .sna file
        snapshotLength = (u32)dsk_size;

        snapshot = (u8 *)malloc(snapshotLength);
        memcpy(snapshot, dsk, snapshotLength);

        if (snapshot != NULL) {
            LireSnapshotMem(core, snapshot);
            loadIni(core, 1);       // Load local in file
        }

        apps_autorun_end(core);
        return;
    }

    if (!memcmp(header, "RIFF", 4)) {  // .cpr file
        if (memcmp(header + 8, "AMS!", 4) != 0) {
            appendIcon(core, 0, 4, 60);
            apps_autorun_end(core);

            return;
        }

        u8 chunkid[4];  // chunk ID (4 character code - cb00, cb01, cb02... upto cb31 (max 512kB), other chunks are ignored)
        u8 chunklen[4];  // chunk length (always little-endian)
        u32 chunksize;  // chunk length, calcaulated from the above
        u32 ramblock;  // 16k RAM block chunk is to be loaded in to
        u32 pos = 0;

        u32 bytes_to_read = header[4] + (header[5] << 8) + (header[6] << 16) + (header[7] << 24);
        bytes_to_read -= 4;  // account for AMS! header

        pos = 12;

        while (bytes_to_read > 0) {
            if (pos + 4 > dsk_size)
                return; // TODO: fix return
            memcpy(chunkid, dsk + pos, 4);
            pos += 4;
            bytes_to_read -= 4;

            if (pos + 4 > dsk_size)
                return;
            memcpy(chunklen, dsk + pos, 4);
            pos += 4;
            bytes_to_read -= 4;

            // calculate little-endian value, just to be sure
            chunksize = chunklen[0] + (chunklen[1] << 8) + (chunklen[2] << 16) + (chunklen[3] << 24);

            if (!memcmp(chunkid, "cb", 2)) {
                // load chunk into RAM
                // find out what block this is
                ramblock = (chunkid[2] - 0x30) * 10;
                ramblock += chunkid[3] - 0x30;

                if (ramblock >= 0 && ramblock < 32) {
                    if (chunksize > 16384) chunksize = 16384;
                    // clear RAM block

                    if (ramblock == 0) {
                        memset(core->ROMINF, 0, 0x4000);
                        memcpy(core->ROMINF, dsk + pos, chunksize);

                        memset(core->MemCPC, 0, 0x4000);
                        memcpy(core->MemCPC, dsk + pos, chunksize);
                    }

                    memset(core->ROMEXT[ramblock], 0, 0x4000);
                    memcpy(core->ROMEXT[ramblock], dsk + pos, chunksize);

                    pos += chunksize;
                    bytes_to_read -= chunksize;
                }
            } else {
                if (chunksize != 0) {
                    pos += chunksize;
                    bytes_to_read -= chunksize;
                }
            }
        }

        SoftResetCPC(core);
        core->NumRomExt = 32; // TODO: fix

        apps_autorun_end(core);
        return;
    }

#ifdef ZIP_SUPPORT
    if (!memcmp(header, "PK", 2)) { // .zip & .kcr files (kcr not supported yet)
        mz_zip_archive zip_archive;
        memset(&zip_archive, 0, sizeof(zip_archive));

        if (mz_zip_reader_init_mem(&zip_archive, dsk, dsk_size, 0) == MZ_TRUE) {
            int i;
            char isKcr = 0;

            for (i = 0; i < (int)mz_zip_reader_get_num_files(&zip_archive); i++) {
                mz_zip_archive_file_stat file_stat;
                if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat)) {
                    mz_zip_reader_end(&zip_archive);
                    break;
                }

                if (!strcasecmp(file_stat.m_filename, "settings.ini")) {
                    isKcr = 1;
                }
            }

            // TODO: handle isKcr flag!

            for (i = 0; i < (int)mz_zip_reader_get_num_files(&zip_archive); i++) {
                mz_zip_archive_file_stat file_stat;
                mz_zip_reader_file_stat(&zip_archive, i, &file_stat);  // Return has been tested in the precedent loop

                char *ext = strrchr(file_stat.m_filename, '.');
                if (ext != NULL) {
                    ext++;
                }

                if ((ext != NULL) &&
                    ((!strcasecmp(ext, "sna")) ||
                     (!strcasecmp(ext, "dsk")))) {
                    unsigned char *undsk = (unsigned char *)malloc(file_stat.m_uncomp_size);
                    mz_zip_reader_extract_to_mem(&zip_archive, 0, undsk, (uint)file_stat.m_uncomp_size, 0);

                    BOOL autoStart = apps_autorun_files_flag;

                    LireDiskMem(core, undsk, (u32)file_stat.m_uncomp_size);
                    loadIni(core, 1); // Load local in file

                    if (!autoStart) {
                        apps_autorun_end(core);
                    }
                    return;
                }
            } // End for
              // No .dsk or .sna found :(
        }

        appendIcon(core, 0, 4, 60);
        apps_autorun_end(core);

        return;
    }
#endif /* ifdef ZIP_SUPPORT */

    if ((!memcmp(header, "MV - CPC", 8)) || (!memcmp(header, "EXTENDED", 8))) {      // .dsk file
        BOOL autoStart = apps_autorun_files_flag;

        LireDiskMem(core, dsk, (u32)dsk_size);
        loadIni(core, 1);       // Load local in file

        if (!autoStart) {
            apps_autorun_end(core);
        }
        return;
    }

    appendIcon(core, 0, 4, 60);
    apps_autorun_end(core);
} /* apps_autorun_init */

void DispAutorun(core_crocods_t *core, u16 keys_pressed0)
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

    char *title = "Select file to run";
    cpcprint16(core, core->overlayBitmap, 320, (256 - (int)strlen(title) * 8) / 2, 15, title, RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);

    for (y = 0; y < 13; y++) {
        int n = y + apps_autorun_files_begin;

        if (n < apps_autorun_files_count) {
            char text[27 + 1];

            snprintf(text, 27, "   %8s %3s %05d %02x     ", apps_autorun_files[n].name, apps_autorun_files[n].ext, apps_autorun_files[n].nbpages, apps_autorun_files[n].user);
            text[27] = 0;

            if (n == apps_autorun_files_selected) {
                cpcprint16(core, core->overlayBitmap, 320, 12, 36 + y * 8, text, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0xFF), 1, 0);
            } else {
                cpcprint16(core, core->overlayBitmap, 320, 12, 36 + y * 8, text, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0x00), 1, 0);
            }
        }
    }

    if ((apps_autorun_files_count == 1) && (!strcasecmp(apps_autorun_files[0].name, "autorun"))) {
        keys_pressed = KEY_A;
    }

    if (keys_pressed & KEY_A) {
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;

        char usefile[256];
        char autoString[256];

        char *ext = apps_autorun_files[apps_autorun_files_selected].ext;

        strcpy(usefile, apps_autorun_files[apps_autorun_files_selected].name);

        if ((ext[0] != 0) & (ext[0] != 32)) {
            strcat(usefile, ".");
            strcat(usefile, ext);
        }

        sprintf(autoString, "run\"%s\n", usefile);
        AutoType_SetString(core, autoString, 1);        // Rest & Run
        apps_autorun_end(core);
    }

    if (((keys_pressed & KEY_B) == KEY_B) || ((keys_pressed & KEY_R) == KEY_R)) {
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;
        // ExecuteMenu(core, ID_MENU_EXIT, NULL);
    }

    if ((keys_pressed & KEY_UP) == KEY_UP) {
        apps_autorun_files_selected--;
        if (apps_autorun_files_selected < 0) {
            apps_autorun_files_selected = apps_autorun_files_count - 1;
            apps_autorun_files_begin = apps_autorun_files_count - 13;
            if (apps_autorun_files_begin < 0) {
                apps_autorun_files_begin = 0;
            }
        }
        if (apps_autorun_files_selected < apps_autorun_files_begin) {
            apps_autorun_files_begin--;
        }
    }

    if ((keys_pressed & KEY_DOWN) == KEY_DOWN) {
        apps_autorun_files_selected++;
        if (apps_autorun_files_selected >= apps_autorun_files_count) {
            apps_autorun_files_selected = 0;
            apps_autorun_files_begin = 0;
        }
        if (apps_autorun_files_selected > apps_autorun_files_begin + 12) {
            apps_autorun_files_begin++;
        }
    }
} /* DispKeyboard */

// Used in upd.c
int GetMinSect(u8 *imgDsk);
int GetPosData(u8 *imgDsk, int track, int sect, char SectPhysique);

void LireDiskMem(core_crocods_t *core, u8 *rom, u32 romsize)
{
    int j, pos;

    char isOk = 0;

    isOk = ( (!memcmp(rom, "MV - CPC", 8)) || (!memcmp(rom, "EXTENDED", 8)) );

    if (isOk == 0) {
        return;
    }

    EjectDiskUPD(core);

    core->LongFic = (int)romsize - sizeof(core->Infos);

    memcpy(&core->Infos, rom, sizeof(core->Infos));
    memcpy(core->ImgDsk, rom + sizeof(core->Infos), core->LongFic);

    core->Image = 1;
    core->FlagWrite = 0;

    ChangeCurrTrack(core, 0);  // Met a jour posdata

    apps_autorun_files_count = 0;

    pos = core->PosData;

    int NumDir;

    for (NumDir = 0; NumDir < 64; NumDir++) {
        static StDirEntry Dir;
        int MinSect = GetMinSect(core->ImgDsk);
        int s = (NumDir >> 4) + MinSect;
        int t = (MinSect == 0x41 ? 2 : 0);
        if (MinSect == 1) {
            t = 1;
        }

        pos = ( (NumDir & 15) << 5) + GetPosData(core->ImgDsk, t, s, 1);

        if ((pos < 0) || (pos + sizeof(StDirEntry) >= 1024 * 1024))
            break;

        memcpy(&Dir, &core->ImgDsk[pos], sizeof(StDirEntry) );

        char filename[9];
        char ext[4];

        filename[0] = 0;
        ext[0] = 0;

        for (j = 0; j < 8; j++) {
            filename[j] = Dir.Nom[j] & 0x7F;
            if (filename[j] == 32) filename[j] = 0;
        }
        filename[8] = 0;

        for (j = 0; j < 3; j++) {
            ext[j] = Dir.Ext[j] & 0x7F;
            if (ext[j] == 32) ext[j] = 0;
        }
        ext[3] = 0;

        if ((Dir.User != 0xE5) && (filename[0] != 0)) {
            signed int found = -1;
            int n;

            for (n = 0; n < apps_autorun_files_count; n++) {
                if ((!strcasecmp(apps_autorun_files[n].name, filename)) && (!strcasecmp(apps_autorun_files[n].ext, ext))) {
                    found = n;
                }
            }

            if (found == -1) {
                strcpy(apps_autorun_files[apps_autorun_files_count].name, filename);
                strcpy(apps_autorun_files[apps_autorun_files_count].ext, ext);
                apps_autorun_files[apps_autorun_files_count].nbpages = Dir.NbPages;
                apps_autorun_files[apps_autorun_files_count].user = Dir.User;
                apps_autorun_files_count++;
            } else {
                strcpy(apps_autorun_files[found].name, filename);
                strcpy(apps_autorun_files[found].ext, ext);
                apps_autorun_files[found].nbpages += Dir.NbPages;
                apps_autorun_files[found].user = Dir.User;
            }
        }
    }

    // Move to the first line following extension (empty, .bas & .bin)

    int place = 0;

    int n;

    for (n = 0; n < apps_autorun_files_count; n++) {
        if (!strcasecmp(apps_autorun_files[n].ext, "")) {
            if (place != n) {
                StDirEntryCroco tmp;
                memcpy(&tmp, &apps_autorun_files[n], sizeof(StDirEntryCroco));
                memcpy(&apps_autorun_files[n], &apps_autorun_files[place], sizeof(StDirEntryCroco));
                memcpy(&apps_autorun_files[place], &tmp, sizeof(StDirEntryCroco));
            }
            place++;
        }
    }

    for (n = 0; n < apps_autorun_files_count; n++) {
        if (!strcasecmp(apps_autorun_files[n].ext, "bas")) {
            if (place != n) {
                StDirEntryCroco tmp;
                memcpy(&tmp, &apps_autorun_files[n], sizeof(StDirEntryCroco));
                memcpy(&apps_autorun_files[n], &apps_autorun_files[place], sizeof(StDirEntryCroco));
                memcpy(&apps_autorun_files[place], &tmp, sizeof(StDirEntryCroco));
            }
            place++;
        }
    }

    for (n = 0; n < apps_autorun_files_count; n++) {
        if (!strcasecmp(apps_autorun_files[n].ext, "bin")) {
            if (place != n) {
                StDirEntryCroco tmp;
                memcpy(&tmp, &apps_autorun_files[n], sizeof(StDirEntryCroco));
                memcpy(&apps_autorun_files[n], &apps_autorun_files[place], sizeof(StDirEntryCroco));
                memcpy(&apps_autorun_files[place], &tmp, sizeof(StDirEntryCroco));
            }
            place++;
        }
    }
} /* LireDiskMem */
