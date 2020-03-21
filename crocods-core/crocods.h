/**
 * @file
 * @author  Miguel Vanhove / Kyuran <crocods@kyuran.be>
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
 * Types
 */

#define CROCOVERSION "2020-02-04"

#ifndef crocods_nds_h
#define crocods_nds_h

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

struct core_crocods_s;
typedef struct core_crocods_s core_crocods_t;

typedef unsigned short USHORT;
typedef signed short SHORT;
typedef unsigned char UBYTE;
//typedef unsigned long           ULONG;

#ifndef USEDBOOL
typedef signed char BOOL;
#endif

#ifndef FALSE
enum
{
    FALSE,
    TRUE
};
#endif

#define CPC_VISIBLE_SCR_WIDTH 256
#define CPC_VISIBLE_SCR_HEIGHT 240

#define USE_DEBUG
#undef USE_TAPE
#undef USE_MULTIFACE
#define USE_SOUND
#undef USE_SOUND_CAS
#define USE_SNAPSHOT

#define USE_CONSOLE

#define BIT16(n) (1 << (n))

enum KEYPAD_BITS
{
    KEY_A = BIT16(0),
    KEY_B = BIT16(1),
    KEY_SELECT = BIT16(2),
    KEY_START = BIT16(3),
    KEY_RIGHT = BIT16(4),
    KEY_LEFT = BIT16(5),
    KEY_UP = BIT16(6),
    KEY_DOWN = BIT16(7),
    KEY_R = BIT16(8),
    KEY_L = BIT16(9),
    KEY_X = BIT16(10),
    KEY_Y = BIT16(11),
    KEY_TOUCH = BIT16(12),
    KEY_LID = BIT16(13),
    KEY_R2 = BIT16(14),
    KEY_L2 = BIT16(15)
};

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;

#endif
