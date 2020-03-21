#include "crocods.h"
/* emu2149.h */
#ifndef _EMU2149_H_
#define _EMU2149_H_

/*#ifdef EMU2149_DLL_EXPORTS
#define EMU2149_API __declspec(dllexport)
#elif  EMU2149_DLL_IMPORTS
#define EMU2149_API __declspec(dllimport)
#else*/
#define EMU2149_API
//#endif

#define EMU2149_VOL_DEFAULT 1
#define EMU2149_VOL_YM2149 0
#define EMU2149_VOL_AY_3_8910 1

#define EMU2149_ZX_STEREO            0x80

#define PSG_MASK_CH(x) (1<<(x))

/*#ifdef __cplusplus
extern "C"
{
#endif*/

  typedef struct __PSG
  {

    /* Volume Table */
    u32 *voltbl;

    u8 reg[0x20];
    s32 out;
    s32 cout[3];

    u32 clk, rate, base_incr, quality;

    u32 count[3];
    u32 volume[3];
    u32 freq[3];
    u32 edge[3];
    u32 tmask[3];
    u32 nmask[3];
    u32 mask;
    u32 stereo_mask[3];

    u32 base_count;

    u32 env_volume;
    u32 env_ptr;
    u32 env_face;

    u32 env_continue;
    u32 env_attack;
    u32 env_alternate;
    u32 env_hold;
    u32 env_pause;
    u32 env_reset;

    u32 env_freq;
    u32 env_count;

    u32 noise_seed;
    u32 noise_count;
    u32 noise_freq;

    /* rate converter */
    u32 realstep;
    u32 psgtime;
    u32 psgstep;
    s32 prev, next;
    s32 sprev[2], snext[2];

    /* I/O Ctrl */
    u32 adr;

  }
  PSG;

  EMU2149_API void PSG_set_quality (PSG * psg, u32 q);
  EMU2149_API void PSG_set_clock(PSG * psg, u32 c);
  EMU2149_API void PSG_set_rate (PSG * psg, u32 r);
  EMU2149_API PSG *PSG_new (u32 clk, u32 rate);
  EMU2149_API void PSG_reset (PSG *);
  EMU2149_API void PSG_delete (PSG *);
  EMU2149_API void PSG_writeReg (PSG *, u32 reg, u32 val);
  EMU2149_API void PSG_writeIO (PSG * psg, u32 adr, u32 val);
  EMU2149_API u8 PSG_readReg (PSG * psg, u32 reg);
  EMU2149_API u8 PSG_readIO (PSG * psg);
  EMU2149_API s16 PSG_calc (PSG *);
EMU2149_API void PSG_calc_stereo (PSG * psg, s32 *bufMO, s32 *bufRO);
  EMU2149_API void PSG_setFlags (PSG * psg, u8 flags);
  EMU2149_API void PSG_setVolumeMode (PSG * psg, int type);
  EMU2149_API u32 PSG_setMask (PSG *, u32 mask);
  EMU2149_API u32 PSG_toggleMask (PSG *, u32 mask);
  EMU2149_API void PSG_setStereoMask (PSG *psg, u32 mask);
    
/*#ifdef __cplusplus
}
#endif*/

#endif
