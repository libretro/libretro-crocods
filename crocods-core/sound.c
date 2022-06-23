#include "sound.h"
#include "platform.h"

#include "emu2149.h"

#ifdef SOUNDV2

PSG psg;

#define CPC_CLK 1000000

GB_sample_t *sndbuf;
int sndbufend, sndbufbeg;

u8 PlaySound(core_crocods_t *gb)
{
    return 1;
}

void PauseSound(core_crocods_t *gb)
{
}

void Reset8912(core_crocods_t *gb)
{
    PSG_reset(&psg);
}

void Write8912(core_crocods_t *gb, int reg, int val)
{
    PSG_writeReg(&psg, reg, val);

    return;
}

int Read8912(core_crocods_t *gb, int r)
{
    return (u8)(psg.reg[r & 0x1f]);
}

#define SNDBUFSIZE (1024 * 16)

void initSound(core_crocods_t *gb, int r)
{
    // PSG initialize

    sndbuf = malloc(sizeof(GB_sample_t) * SNDBUFSIZE);
    sndbufbeg = 0;
    sndbufend = 0;

    gb->snd_cycle_count_init.both = (int64_t)(4000000.0 / 44100 * 4294967296.0); // number of Z80 cycles per sample

    PSG *_psg = PSG_new(CPC_CLK, r);
    memcpy(&psg, _psg, sizeof(PSG));

//    PSG_set_quality(&psg, 1);

    PSG_reset(&psg);
}

// snd_bufsize is number of sample (not bytes)

char useProcSound = 0;

void crocods_copy_sound_buffer(core_crocods_t *gb, GB_sample_t *dest, unsigned int snd_bufsize)
{
    if (gb->soundEnabled == 0) {
        return;
    }

    if (useProcSound == 0) {     // procsound never used
        int i;
//        u8 cl, cr, cc;

        for (i = 0; i < snd_bufsize; i++) {
            s32 left, right;

            PSG_calc_stereo(&psg, &left, &right);

            dest[i].left = (s16)left;
            dest[i].right = (s16)right;

//            PSG_calc_stereo(&psg, &dest[i].left, &dest[i].right);

//            PSG_calc(&psg, &cr, &cc, &cl);

//            // <<6 because gbaudioclient wait for signed.... (<<7 for unsigned)
//
////            dest[i].left = (((e_uint16)cl) + (((e_uint16)cc) >> 1)) << 7;
////            dest[i].right =  (((e_uint16)cr) + (((e_uint16)cc) >> 1)) << 7;
//            dest[i].left = (((e_uint16)cl) + (((e_uint16)cc) << 1)) << 6;
//            dest[i].right =  (((e_uint16)cr) + (((e_uint16)cc) << 1)) << 6;
        }
    } else {
        int i;
        int sndbufpos = sndbufbeg;

        for (i = 0; i < snd_bufsize; i++) {
            dest[i].left = sndbuf[sndbufpos].left;
            dest[i].right =  sndbuf[sndbufpos].right;

            sndbufpos++;
            if (sndbufpos == SNDBUFSIZE) {
                sndbufpos = 0;
            }
            if (sndbufpos == sndbufend) {  // Hope that never occurs
                if (sndbufpos >0)
                    sndbufpos--;
            }
        }

        sndbufbeg = sndbufpos;
    }
} /* crocods_copy_sound_buffer */

void procSound(core_crocods_t *gb)
{
    // TODO: FIX! Slow down CPU if too much sound sample are available - Not needed if cpu accuracy was good!

    if (/* DISABLES CODE */ (1) == 0) {
        u32 have;

        do {
            have = (sndbufend + SNDBUFSIZE - sndbufbeg) % SNDBUFSIZE;
        } while (have > SNDBUFSIZE / 4); // Wait if too much sample
    }

    useProcSound = 1;
    sndbufend++;

    if (sndbufend == SNDBUFSIZE) {
        sndbufend = 0;
    }

    s32 left, right;

    PSG_calc_stereo(&psg, &left, &right);

#ifdef TARGET_OS_MAC
// TODO: Verify big endian
    s16 left16 = (left >> 8) | (left << 8);
    s16 right16 = (right >> 8) | (right << 8);
#else
    s16 left16 = (s16)left;
    s16 right16 = (s16)right;
#endif

    sndbuf[sndbufend].left = left16;
    sndbuf[sndbufend].right = right16;
} /* procSound */

#endif /* ifdef SOUNDV2 */
