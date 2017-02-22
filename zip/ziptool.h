/* ziptool.h internal header */
#ifndef _ZIPTOOL
#define _ZIPTOOL

#ifdef __cplusplus
extern "C" {
#endif 

unsigned char *unzip(unsigned char *zipbuf, unsigned int zipsize, char *filename, unsigned int *size);
// void FS_zipgetFileList(FS_AddFile AddFile, char *zipfile, unsigned char *zipbuf, unsigned int zipsize);

#ifdef __cplusplus
}
#endif 


#endif
