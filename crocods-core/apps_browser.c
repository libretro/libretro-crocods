#include "plateform.h"

#if defined (_WIN32) || defined (_3DS)

void apps_browser_init(core_crocods_t *core, int flag) {
}

#else

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

void guestGetAllKeyPressed(core_crocods_t *core, char *string); // In guest.h
void guestGetJoystick(core_crocods_t *core, char *string);

void DispBrowser(core_crocods_t *core, u16 keys_pressed0);

void UseResources(void *core, void *bytes, int len); // In plateform.h

#define RGB565(R, G, B) ((((R) & 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))

int apps_browser_files_count = 0;
int apps_browser_files_begin = 0;
int apps_browser_files_selected = 0;

int apps_browser_files_flag = 0;

int apps_browser_pos = 0;
int apps_browser_speed_about = 0;
int apps_browser_dir_about = 1;

char *apps_browser_buf;

typedef struct {
    char *id;
    char *media_id;
    char *title;
} apps_browser_entry;

apps_browser_entry *apps_browser_files;

void apps_browser_end(core_crocods_t *core)
{
    free(apps_browser_buf);

    core->inKeyboard = 0;

    core->runApplication = NULL;

    core->wait_key_released = 1;

    free(apps_browser_files);

    ExecuteMenu(core, ID_MENU_EXIT, NULL);
}

// BDDBrowser/2.9.5d

// Need to free after
char * apps_browser_get_url(core_crocods_t *core, char *url, char *hostname, int *len)
{
    struct sockaddr_in servaddr;
    int sock;
    char *buf = (char *)malloc(256);

    *len = 0;

    memset(&servaddr, 0, sizeof(servaddr));

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Wifi connect: Socket error !");
        printf("socket error\n");
        return NULL;
    }

    struct hostent *hostent;

    hostent = gethostbyname(hostname);
    if (hostent == NULL) {
        printf("error: gethostbyname(\"%s\")\n", hostname);
        return NULL;
    }
    
    memcpy(&servaddr.sin_addr, hostent->h_addr_list[0], hostent->h_length);

//    in_addr_t in_addr;
//    in_addr = inet_addr(inet_ntoa(*(struct in_addr *)*(hostent->h_addr_list)));
//    servaddr.sin_addr.s_addr = in_addr;

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(80);

    printf("Wifi contact server ...");
    if (connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) == 0) {
        printf("Try to connect ...!\n");
        fcntl(sock, F_SETFL, O_NONBLOCK);

//            int i = 1;
//            ioctl(sock, FIONBIO, &i);
        printf("Connected successfully!\n");
    } else {
        printf("Connected not done ...\n");
        return NULL;
    }

    char szBuff[512];
    ssize_t recvd_len;

    // Grab the image
    // 1) grad image URL
    printf("Wifi get image ...");
//    sprintf(szBuff, "GET %s\r\nHost: %s\r\nUser-Agent: BDDBrowser/2.9.7c Java/1.8.0_192\r\nAccept: */*\r\n\r\n", url, hostname);
//    printf("Send %s\n", szBuff);

    sprintf(szBuff,"GET %s HTTP/1.1\r\nUser-Agent: BDDBrowser/2.9.7c Java/1.8.0_192\r\nHost: %s\r\nAccept: text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2\r\nConnection: keep-alive\r\n\r\n", url, hostname);
//
//    printf("Send %s\n", szBuff);

    send(sock, szBuff, strlen(szBuff), 0);

    while ( (recvd_len = recv(sock, szBuff, 255, 0) ) != 0) {
        if (recvd_len > 0) {
            buf = realloc(buf, (*len) + recvd_len + 1);
            memcpy(buf + (*len), szBuff, recvd_len);
            (*len) += recvd_len;
        } else if (recvd_len < 0) {
            perror("recv");
        }
    }
    buf[(*len)] = 0;

    return buf;
} /* apps_browser_get_url */

#define BASE_URL "cpc.devilmarkus.de"

char * xml_extract(char *xml, char *from, char *to, char *stop, char **result)
{
    char *beg = strstr(xml, from);

    if (stop != NULL) {
        char *stopstr = strstr(xml, stop);
        if (stopstr < beg) {
            return NULL;
        }
    }
    xml = beg;

    if (xml != NULL) {
        xml += strlen(from);
        char *endstr = strstr(xml, to);

        if (endstr != NULL) {
            *endstr = 0;
            *result = xml;
            xml = endstr + 1;
        } else {
            xml = NULL;
        }
    }
    return xml;
}

void apps_browser_use(core_crocods_t *core, int id)
{
    // Download resource
    int len;
    char url[512];
    strcpy(url, "/games/api.php?action=get&id=");
    strcat(url, apps_browser_files[id].media_id);

    char *res = apps_browser_get_url(core, url, BASE_URL, &len);
    
    res=strstr(res,"\r\n\r\n");
    if (res!=NULL) {
        res+=4;
        res=strstr(res,"\r\n");
        if (res!=NULL) {
            res+=2;
            
            apps_browser_end(core);
            UseResources(core, res, len);
            
            ExecuteMenu(core, ID_AUTORUN, NULL);

            return;
        }
    }


}

void apps_browser_init(core_crocods_t *core, int flag)
{
    printf("apps_browser_init: %s\n", core->openFilename);

    core->runApplication = DispBrowser;
    apps_browser_files_flag = flag;

    apps_browser_files_count = 0;
    apps_browser_files_begin = 0;
    apps_browser_files_selected = 0;

    apps_browser_files = NULL;
    apps_browser_files_count = 0;

    char *url = "/games/api.php?action=detailist";

    int len;
    apps_browser_buf = apps_browser_get_url(core, url, "cpc.devilmarkus.de", &len);
    if (apps_browser_buf != NULL) {
        char *xml = apps_browser_buf;
        while (xml != NULL) {
            char *id, *title;

            xml =  xml_extract(xml, "<game id=\"", "\"", NULL, &id);
            if (xml == NULL) {
                break;
            }
            printf("id: %s\n", id);
            xml =  xml_extract(xml, "title=\"", "\"", NULL, &title);
            printf("title: %s\n", title);
            while (1) {
                char *media;
                char *media_id, *media_type;
                media = xml_extract(xml, "<media id=\"", "\"", "</game>", &media_id);
                if (media == NULL) {
                    break;
                }
                xml = xml_extract(media, "type=\"", "\"", NULL, &media_type);
                printf("media: %s - %s\n", media_type, media_id);

                if (!strcmp(media_type, "Disquette")) {
                    // Create new entry

                    apps_browser_files_count++;
                    apps_browser_files = realloc(apps_browser_files, sizeof(apps_browser_entry) * apps_browser_files_count);

                    apps_browser_files[apps_browser_files_count - 1].title = title;
                    apps_browser_files[apps_browser_files_count - 1].media_id = media_id;
                    apps_browser_files[apps_browser_files_count - 1].id = id;
                }
            }
        }
    }

//    if (error) {
//        appendIcon(core, 0, 4, 60);
//        apps_browser_end(core);
//        return;
//    }
}         /* apps_browser_init */

void DispBrowser(core_crocods_t *core, u16 keys_pressed0)
{
//    static int key = 0;

    int y;

    u16 keys_pressed = appli_begin(core, keys_pressed0);

    core->overlayBitmap_width = 256;
    core->overlayBitmap_height = 168;
    core->overlayBitmap_posx = (320 - 256) / 2;
    core->overlayBitmap_posy = (240 - 168) / 2;
    core->overlayBitmap_center = 1;

    u16 *pdwAddr = core->overlayBitmap;         // + ((j * 32) * core->MemBitmap_width) + (i * 32);

    // Text begin in 12,36 (max 13 lines, 26 columns)

    for (y = 0; y < 168; y++) {
        memcpy(pdwAddr, core->select + y * 256, 256 * 2);
        pdwAddr += 320;
    }

    char *title = "Select file to run";
    cpcprint16(core, core->overlayBitmap, 320, (256 - (int)strlen(title) * 8) / 2, 15, title, RGB565(0xFF, 0x00, 0x00), RGB565(0x00, 0x00, 0x00), 1, 1);

    for (y = 0; y < 13; y++) {
        int n = y + apps_browser_files_begin;

        if (n < apps_browser_files_count) {
            char text[30+1];

            snprintf(text, 30, "%s", apps_browser_files[n].title);
            text[30]=0;
            
            if (n == apps_browser_files_selected) {
                cpcprint16(core, core->overlayBitmap, 320, 12, 36 + y * 8, text, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0xFF), 1, 0);
            } else {
                cpcprint16(core, core->overlayBitmap, 320, 12, 36 + y * 8, text, RGB565(0xFF, 0xFF, 0x00), RGB565(0x00, 0x00, 0x00), 1, 0);
            }
        }
    }

    if (keys_pressed & KEY_A) {
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;

        apps_browser_use(core, apps_browser_files_selected);
    }

    if (((keys_pressed & KEY_B) == KEY_B) || ((keys_pressed & KEY_R) == KEY_R)) {
        core->inKeyboard = 0;
        core->runApplication = NULL;

        core->wait_key_released = 1;
        // ExecuteMenu(core, ID_MENU_EXIT, NULL);
    }

    if ((keys_pressed & KEY_UP) == KEY_UP) {
        apps_browser_files_selected--;
        if (apps_browser_files_selected < 0) {
            apps_browser_files_selected = apps_browser_files_count - 1;
            apps_browser_files_begin = apps_browser_files_count - 13;
            if (apps_browser_files_begin < 0) {
                apps_browser_files_begin = 0;
            }
        }
        if (apps_browser_files_selected < apps_browser_files_begin) {
            apps_browser_files_begin--;
        }
    }

    if ((keys_pressed & KEY_DOWN) == KEY_DOWN) {
        apps_browser_files_selected++;
        if (apps_browser_files_selected >= apps_browser_files_count) {
            apps_browser_files_selected = 0;
            apps_browser_files_begin = 0;
        }
        if (apps_browser_files_selected > apps_browser_files_begin + 12) {
            apps_browser_files_begin++;
        }
    }
    
    if ((keys_pressed & KEY_LEFT) == KEY_LEFT) {
          apps_browser_files_selected -= 10;
         if ( apps_browser_files_selected < 0) {
              apps_browser_files_selected = 0;
              apps_browser_files_begin =  apps_browser_files_count - 13;
             if ( apps_browser_files_begin < 0) {
                  apps_browser_files_begin = 0;
             }
         }
         if ( apps_browser_files_selected <  apps_browser_files_begin) {
              apps_browser_files_begin =  apps_browser_files_selected;
         }
     }

     if ((keys_pressed & KEY_RIGHT) == KEY_RIGHT) {
          apps_browser_files_selected += 10;
         if ( apps_browser_files_selected >=  apps_browser_files_count) {
              apps_browser_files_selected =  apps_browser_files_count - 1;
         }
         if ( apps_browser_files_selected >  apps_browser_files_begin + 12) {
              apps_browser_files_begin =  apps_browser_files_selected - 12;
         }
     }

    if (keys_pressed & KEY_SELECT) {
        apps_browser_end(core);
    }
}         /* DispKeyboard */

#endif
