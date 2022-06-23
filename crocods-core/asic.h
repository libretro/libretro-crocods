/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
 * @author  Kevin Thacker for the original version on Arnold (1995-2001)
 * @version 2.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * https://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * CPC+ ASIC Emulation
 */

#include  "crocods.h"

#ifndef ASIC_H
#define ASIC_H

#ifdef __cplusplus
extern "C" {
#endif

BOOL InitASIC(core_crocods_t *core);



typedef struct
{
    BOOL PauseActive;
    int PauseCount;                                 /* pause current count */
    int PrescaleCount;                              /* channel prescalar current count */
    int LoopStart;                                  /* reload address for loop */
    int RepeatCount;                        /* number of times to repeat the loop */
} ASIC_DMA_CHANNEL;

void    ASIC_Finish(void);

void    ASIC_InitCart(void);
void    ASIC_Reset(void);
void    ASIC_EnableDisable(int);
int     ASIC_DMA_GetChannelAddr(int);
int     ASIC_DMA_GetChannelPrescale(int);
void    ASIC_SetRasterInterrupt(void);
void    ASIC_ClearRasterInterrupt(void);

unsigned long ASIC_BuildDisplayReturnMaskWithPixels(int Line, int HCount, /*int MonitorHorizontalCount, int ActualY,*/ int *pPixels);

void    ASIC_DoDMA(void);

void    ASIC_HSync(int,int);

/* asic functions to be executed when Htot reached */
void    ASIC_HTot(int);

int             ASIC_CalculateInterruptVector(void);

/* set lock state of ASIC (features locked/unlocked) for snapshot */
void ASIC_SetUnLockState(BOOL);

void    ASIC_SetSecondaryRomMapping(unsigned char Data);

/* reset gate array in ASIC */
void    ASIC_GateArray_Reset(void);

/* trap writes to asic ram */
void    ASIC_WriteRam(int Addr,int Data);

/* used when setting up ASIC in reset or from snapshots */
void    ASIC_WriteRamFull(int Addr, int Data);

int Cartridge_AttemptInsert(unsigned char *pCartridgeData, unsigned long CartridgeLength);
int     Cartridge_Insert(const unsigned char *pCartridgeData, const unsigned long CartridgeDataLength);
void    Cartridge_Autostart(void);
void    Cartridge_Remove(void);

BOOL    ASIC_RasterIntEnabled(void);

void    ASIC_DoRomSelection(void);
void    ASIC_AcknowledgeInterrupt(void);

void    ASIC_DMA_EnableChannels(unsigned char);

typedef struct
{
    /* width of sprite in 16-pixel wide columns */
    unsigned long WidthInColumns;
    /* HCount of column that min sprite x is in */
    unsigned long MinColumn;
    /* height of sprite in scan-lines */
    unsigned long HeightInLines;

    unsigned int XMagShift,YMagShift;
    unsigned long x, y;

//  unsigned long    SpriteMaxXPixel, SpriteMaxYPixel;
} ASIC_SPRITE_RENDER_INFO;

#define ASIC_RAM_ENABLED    0x0002
#define ASIC_ENABLED        0x0001

/* this structure represents what is stored in internal ASIC registers */
typedef struct
{
    union
    {
        unsigned short SpriteX_W;

#if RETRO_IS_LITTLE_ENDIAN
        struct
        {
            unsigned char l;
            unsigned char h;
        } SpriteX_B;
#elif RETRO_IS_BIG_ENDIAN
        struct
        {
            unsigned char h;
            unsigned char l;
        } SpriteX_B;
#else
#error Unknown endianness
#endif

    } SpriteX;

    union
    {
        unsigned short SpriteY_W;
#if RETRO_IS_LITTLE_ENDIAN
        struct
        {
            unsigned char l;
            unsigned char h;
        } SpriteY_B;
#else
        struct
        {
            unsigned char h;
            unsigned char l;
        } SpriteY_B;
#endif

    } SpriteY;

    unsigned char SpriteMag;

    unsigned char pad[3];
} ASIC_SPRITE_INFO;

typedef struct
{
    union
    {
        unsigned short Addr_W;
#if RETRO_IS_LITTLE_ENDIAN
        struct
        {
            unsigned char l;
            unsigned char h;
        } Addr_B;
#else
        struct
        {
            unsigned char h;
            unsigned char l;
        } Addr_B;
#endif

    } Addr;

    unsigned char Prescale;
    unsigned char pad;
} ASIC_DMA_INFO;

typedef struct
{
    /* status flags */
    unsigned long Flags;
    /* pointer to asic ram */
    unsigned char    *ASIC_Ram;
    /* pointer to asic ram for "re-thinking memory" */
    unsigned char    *ASIC_Ram_Adjusted;
    /* a mask used for memory paging */
    unsigned long ASIC_RamMask;

    /* SPRITES */
    unsigned long SpriteEnableMask;
    unsigned long SpriteEnableMaskOnLine;
    ASIC_SPRITE_INFO Sprites[16];
    ASIC_SPRITE_RENDER_INFO SpriteInfo[16];

    /* DMA */
    unsigned long DMAPauseActive;
    ASIC_DMA_INFO DMA[3];
    ASIC_DMA_CHANNEL DMAChannel[3];

    /* interrupt vector */
    unsigned char ASIC_InterruptVector;
    /* raster interrupt line */
    unsigned char ASIC_RasterInterruptLine;
    /* soft scroll */
    unsigned char ASIC_SoftScroll;
    /* raster split line */
    unsigned char ASIC_RasterSplitLine;

    /* Secondary Screen Address */
    union
    {
        unsigned short Addr_W;
#if RETRO_IS_LITTLE_ENDIAN
        struct
        {
            unsigned char l;
            unsigned char h;
        } Addr_B;
#else
        struct
        {
            unsigned char h;
            unsigned char l;
        } Addr_B;
#endif
    } ASIC_SecondaryScreenAddress;

    /* bit 7 = 1 if raster interrupt triggered */
    /* bit 6 = 1 if DMA channel 0 interrupt triggered */
    /* bit 5 = 1 if DMA channel 1 interrupt triggered */
    /* bit 4 = 1 if DMA channel 2 interrupt triggered */
    unsigned char InternalDCSR;
    unsigned char ASIC_DCSR2;

    unsigned char SecondaryRomMapping;

    unsigned char AnalogueInputs[8];

} ASIC_DATA;




#ifdef __cplusplus
}
#endif


#endif
