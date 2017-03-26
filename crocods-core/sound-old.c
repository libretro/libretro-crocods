#include "sound.h"
#include "plateform.h"

#include "emu2149.h"

#ifdef SOUNDV2


void ProcSound(void);


PSG psg;

#define CPC_CLK 2000000


u8 PlaySound( core_crocods_t *gb )
{
    return 1;
}

void PauseSound( core_crocods_t *gb )
{
}


void Reset8912( core_crocods_t *gb )
{
    PSG_reset(&psg);
}

void Write8912( core_crocods_t *gb, int reg, int val )
{
    
    PSG_writeReg(&psg, reg, val);
    
    return;
}

int Read8912( core_crocods_t *gb, int r )
{
    return (u8) (psg.reg[r & 0x1f]);
}


void initSound(core_crocods_t *gb, int r) {
    
    printf("\nSound V2vi\n");
    
    // PSG initialize
    
    PSG *_psg = PSG_new(CPC_CLK,r);
    
    memcpy(&psg, _psg, sizeof(PSG));
    
    PSG_reset( &psg );
}


void crocods_copy_sound_buffer(core_crocods_t *gb, GB_sample_t *dest, unsigned int snd_bufsize) {
    
    
    int i;
    
    for ( i = 0; i < snd_bufsize; i++ )
    {
        e_uint8 cl,cr,cc;
        
        PSG_calc(&psg, &cl, &cc, &cr);
        
//        cc=cc/2;
        
        dest[i].left = (cl + cc) * 100; // ( (Wave[2][i] + Wave[1][i]) * SND_VOLUME ) >> 8 ;
        dest[i].right =  (cr + cc) * 100; // ( (Wave[0][i] + Wave[1][i]) * SND_VOLUME ) >> 8 ;
        //
        //
        //        dest[i].left = canal[0] ;
        //        dest[i].right = canal[1] ;
    }
    
    
}

void procSound(core_crocods_t *gb, int us) {
    
    u16 canal[2];
    
//    PSG_calc(&psg, canal);
    
    //    if (wptr>= SOUNDBUFCNT ) {
    //        return;
    //    }
    //
    //    *sbuf = canal[0];
    //	sbuf ++;
    //
    //    *sbuf = canal[1];
    //    sbuf ++;
    //
    //    wptr ++ ;
    //
}


#endif






