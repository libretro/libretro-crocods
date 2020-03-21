/******************************************************************************/
/* Configuration pour l'archivage des diff�rents �l�ments du fichier source   */
/******************************************************************************/
// !CONFIG!=/L/* /R/* /W"* Nom : "
// D�finition du syst�me       !CONFIG!=/V1!EMULATEUR CPC!
// D�finition du sous syst�me  !CONFIG!=/V2!PC-CPC!
// D�finition du sous ensemble !CONFIG!=/V3!Chips!
// D�finition du module        !CONFIG!=/V4!PSG AY3-8912!
/******************************************************************************/

/********************************************************* !NAME! **************
* !./FLE!
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*          SYSTEME         |      SOUS SYSTEME       |      SOUS ENSEMBLE
* ------------------------------------------------------------------------------
*  EMULATEUR CPC           | PC-CPC                  | Chips
* ------------------------------------------------------------------------------
*  Fichier     : AY8912.C              | Version : 0.1an
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Emulation du PSG AY3 8912
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  06/11/2002              | L.DEPLANQUE             | optimisations minimes
* ------------------------------------------------------------------------------
*  03/12/2002              | L.DEPLANQUE             | correction bugg
*                          |                         | (r�gr�ssion) dans la
*                          |                         | g�n�ration d'enveloppes
* ------------------------------------------------------------------------------
*  08/12/2005              | L.DEPLANQUE             | Version 0.1ai :
*                          |                         | Correction d'un bugg
*                          |                         | (r�gression) dans la
*                          |                         | g�n�ration des bruits
* ------------------------------------------------------------------------------
*  20/07/2006              | L.DEPLANQUE             | Version 0.1al :
*                          |                         | Correction d'un bugg
*                          |                         | dans l'utilisation des
*                          |                         | enveloppes de volume.
*                          |                         | Ajout d'un indicateur
*                          |                         | d'�criture du registre 13
* ------------------------------------------------------------------------------
*  15/11/2006              | L.DEPLANQUE             | Version 0.1am :
*                          |                         | Correction d'un bugg
*                          |                         | dans la restitution des
*                          |                         | fr�quences pour chaque
*                          |                         | canal
* ------------------------------------------------------------------------------
*  13/05/2007              | L.DEPLANQUE             | Version 0.1an :
*                          |                         | Optimisations mineures
* ------------------------------------------------------------------------------
********************************************************** !END! **************/

#include  "sound.h"

#include  <stdio.h>

#include  "plateform.h"


#ifdef SOUNDV1

#define AY8912_FMAX    62500        // Fr�quence maxi. en sortie

#define SND_RATE       44100
#define SND_CHANNELS   6
#define SND_NB_BUFFERS 12
#define SND_VOLUME     50





int Read8912(core_crocods_t *core, int r)
{
    return (u8)(core->RegsPSG[r & 0x1f]);
}


void Write8912(core_crocods_t *core, int Reg, int Val)
{
    int i, j, Reg7Mod;

    if ((Reg == 14) || (Reg == 15)) {
        // PSG_RefreshPortOutputs(ay,ay->PSG_SelectedRegister-14);
    }

    if ( ( core->RegsPSG[ Reg ] != Val ) || ( ( Reg > 7 ) && ( Reg < 11 ) ) ) {
        Reg7Mod = ( Val ^ core->RegsPSG[ 7 ] ) & 0x3F;
        core->RegsPSG[ Reg ] = Val;
        switch ( Reg ) {
            //
            // Registres de fr�quence
            //
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                if ( !( core->RegsPSG[ 7 ] & ( 1 << ( Reg >> 1 ) ) ) ) {
                    i = Reg & 0xFE;
                    j = ( ( core->RegsPSG[ i + 1 ] & 0x0F ) << 8 ) + core->RegsPSG[ i ];
                    i >>= 1;
                    core->Freq[ i ] = AY8912_FMAX / ( j + 1 );
                }
                break;

            //
            // Registre de p�riode de bruit
            //
            case 6:
                if ( ~core->RegsPSG[ 7 ] & 0x38 ) {
                    j = AY8912_FMAX / ( ( Val & 0x1F ) + 1 );
                    if ( !( core->RegsPSG[ 7 ] & 0x08 ) )
                        core->Freq[ 3 ] = j;

                    if ( !( core->RegsPSG[ 7 ] & 0x10 ) )
                        core->Freq[ 4 ] = j;

                    if ( !( core->RegsPSG[ 7 ] & 0x20 ) )
                        core->Freq[ 5 ] = j;
                }
                break;

            //
            // Registre de mixage
            //
            case 7:
                for ( j = 0; j < SOUND_CHANNELS; j++ ) {
                    if ( Reg7Mod & 1 ) {
                        if ( Val & 1 )
                            core->Freq[ j ] = 0;
                        else {
                            if ( j < 3 )
                                i = ( ( core->RegsPSG[ ( j << 1 ) + 1 ] & 0x0F ) << 8 ) + core->RegsPSG[ j << 1 ];
                            else
                                i = core->RegsPSG[ 6 ] & 0x1F;

                            core->Freq[ j ] = AY8912_FMAX / ( i + 1 );
                        }
                    }
                    Reg7Mod >>= 1;
                    Val >>= 1;
                }
                break;

            //
            // Registres de volume
            //
            case 8:
            case 9:
            case 10:
                Reg -= 8;
                core->Env[ Reg ] = ( Val & 0x10 ) >> 4;
                if ( core->Env[ Reg ] )
                    core->Vol[ Reg + 3 ] = core->Vol[ Reg ] = ( ( !( core->EnvType & 4 ) ) * 0x0F ) * 17;
                else
                    core->Vol[ Reg + 3 ] = core->Vol[ Reg ] = ( Val & 0x0F ) * 17;
                break;

            //
            // Registres de p�riode d'enveloppe
            //
            case 11:
            case 12:
                core->EPeriod = core->RegsPSG[ 11 ] + ( core->RegsPSG[ 12 ] << 8 );
                core->ECount = 0;
                break;

            //
            // Registre de forme d'enveloppe
            //
            case 13:
                core->WritePSGReg13 = TRUE;
                core->EnvType = Val & 0x0F;
                if ( core->Env[ 0 ] )
                    core->Vol[ 3 ] = core->Vol[ 0 ] = ( !( core->EnvType & 4 ) ) * 0x0F;

                if ( core->Env[ 1 ] )
                    core->Vol[ 4 ] = core->Vol[ 1 ] = ( !( core->EnvType & 4 ) ) * 0x0F;

                if ( core->Env[ 2 ] )
                    core->Vol[ 5 ] = core->Vol[ 2 ] = ( !( core->EnvType & 4 ) ) * 0x0F;
                break;

            default:



                printf("Unknown Registre: %d = %d\n", Reg, Val);
        } /* switch */
    }
} /* Write8912 */


/********************************************************* !NAME! **************
* Nom : IsWritePSGReg13
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Indique si le registre 13 a �t� �cris. Remet ensuite � z�ro
*               l'indicateur d'�criture du registre 13
*
* R�sultat    : Valeur de la variable WritePSGReg13
*
* Variables globales modifi�es : WritePSGReg13
*
********************************************************** !0! ****************/
BOOL IsWritePSGReg13(core_crocods_t *core)
{
    BOOL Ret = core->WritePSGReg13;

    core->WritePSGReg13 = FALSE;
    return( Ret );
}


/********************************************************* !NAME! **************
* Nom : Loop8912
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction de g�n�ration des sons du PSG, a appeler � intervalles
*               r�guliers
*
* R�sultat    : /
*
* Variables globales modifi�es : Vol, ECount, Env
*
********************************************************** !0! ****************/
void Loop8912(core_crocods_t *core, int uS)
{
    int c, v, IncVol;

    if ( core->EPeriod ) {
        core->ECount += uS;
        if ( core->ECount >= core->EPeriod ) {
            core->ECount -= core->EPeriod;
            IncVol = uS >= core->EPeriod ? uS / core->EPeriod : 1;
            for ( c = 0; c < 3; c++ ) {
                if ( core->Env[ c ] && ( core->RegsPSG[ c + 8 ] & 0x10 ) ) {
                    //
                    // Cr�ation des enveloppes de volume
                    //
                    switch ( core->RegsPSG[ 13 ] ) {
                        case 0x00:
                        case 0x01:
                        case 0x02:
                        case 0x03:
                        case 0x09:
                            v = core->Vol[ c ] - IncVol;
                            core->Vol[ c + 3 ] = core->Vol[ c ] = v = v >= 0 ? v : 0;
                            if ( !v )
                                core->Env[ c ] = 0;

                            break;

                        case 0x04:
                        case 0x05:
                        case 0x06:
                        case 0x07:
                        case 0x0F:
                            v = core->Vol[ c ] + IncVol;
                            core->Vol[ c + 3 ] = core->Vol[ c ] = v = v < 256 ? v : 0;
                            if ( !v )
                                core->Env[ c ] = 0;

                            break;

                        case 0x08:
                            v = core->Vol[ c ] - IncVol;
                            core->Vol[ c + 3 ] = core->Vol[ c ] = v >= 0 ? v : 255;
                            break;

                        case 0x0A:
                            if ( core->Env[ c ] == 2 ) {
                                v = core->Vol[ c ] + IncVol;
                                core->Vol[ c + 3 ] = core->Vol[ c ] = v = v < 256 ? v : 255;
                                if ( v == 255 )
                                    core->Env[ c ] = 1;
                            } else {
                                v = core->Vol[ c ] - IncVol;
                                core->Vol[ c + 3 ] = core->Vol[ c ] = v = v >= 0 ? v : 0;
                                if ( !v )
                                    core->Env[ c ] = 2;
                            }
                            break;

                        case 0x0B:
                            v = core->Vol[ c ] - IncVol;
                            core->Vol[ c + 3 ] = core->Vol[ c ] = v = v >= 0 ? v : 255;
                            if ( v == 255 )
                                core->Env[ c ] = 0;

                            break;

                        case 0x0C:
                            v = core->Vol[ c ] + IncVol;
                            core->Vol[ c + 3 ] = core->Vol[ c ] = v < 256 ? v : 0;
                            break;

                        case 0x0D:
                            v = core->Vol[ c ] + IncVol;
                            core->Vol[ c + 3 ] = core->Vol[ c ] = v < 256 ? v : 255;
                            if ( v == 255 )
                                core->Env[ c ] = 0;

                            break;

                        case 0x0E:
                            if ( core->Env[ c ] == 2 ) {
                                v = core->Vol[ c ] - IncVol;
                                core->Vol[ c + 3 ] = core->Vol[ c ] = v = v >= 0 ? v : 0;
                                if ( !v )
                                    core->Env[ c ] = 1;
                            } else {
                                v = core->Vol[ c ] + IncVol;
                                core->Vol[ c + 3 ] = core->Vol[ c ] = v = v < 256 ? v : 255;
                                if ( v == 255 )
                                    core->Env[ c ] = 2;
                            }
                            break;
                    } /* switch */
                }
            }
        }
    }
} /* Loop8912 */


/********************************************************* !NAME! **************
* Nom : Reset8912
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Effectue un reset du 8912
*
* R�sultat    : /
*
* Variables globales modifi�es : RegsPSG
*
********************************************************** !0! ****************/
void Reset8912(core_crocods_t *core)
{
    int c;

    core->RegsPSG[ 7 ] = 0xFD;
    core->RegsPSG[ 14 ] = 0xFF;
    for ( c = 0; c < SOUND_CHANNELS; c++ ) {
        core->Freq[ c ] = 0;
    }
}


/********************************************************* !NAME! **************
* Nom : Sound
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Modifie la fr�quence/volume d'un canal
*
* R�sultat    : /
*
* Variables globales modifi�es : TabChannel
*
********************************************************** !0! ****************/
void SetSound(core_crocods_t *core, int c, int f, int v)
{
    core->Freq[ c ] = f;
    core->Vol[ c ] = v;
}








void initSound(core_crocods_t *gb, int r)
{

    printf("\nSound V1\n");

    if (gb->audio_buffer) {
        free(gb->audio_buffer);
    }

    int sample_rate = r;

    gb->buffer_size = sample_rate / 25; // 40ms delay
    gb->audio_buffer = malloc(gb->buffer_size * sizeof(*gb->audio_buffer));
    gb->sample_rate = sample_rate;
    gb->audio_position = 0;


    Reset8912(gb);
}

void crocods_copy_sound_buffer(core_crocods_t *gb, GB_sample_t *dest, unsigned int snd_bufsize)
{
    s32 i, j;

    if ((!gb->inMenu) && (!gb->isPaused)) {

        static s32 Count[ SND_CHANNELS ];
        s32 k, l, v, f;

        for ( j = 0; j < SND_CHANNELS; j++ ) {
            v = gb->Vol[ j ];
            f = gb->Freq[ j ];
            if ( f && v ) {
                if ( j > 2 ) {
                    //
                    // Canaux "bruit blanc"
                    //
                    if ( f <= SND_RATE )
                        k = 0x10000 * f / SND_RATE;
                    else {
                        v = v * SND_RATE / f;
                        k = 0x10000;
                    }
                    l = Count[ j ];
                    for ( i = 0; i < snd_bufsize; i++ ) {
                        l += k;
                        if ( l & 0xFFFF0000 ) {
                            if ( ( gb->NoiseGen <<= 1 ) & 0x80000000 )
                                gb->NoiseGen ^= 0x08000001;

                            l &= 0xFFFF;
                        }
                        gb->Wave[j][ i ] = ( gb->NoiseGen & 1 ? 127 : -128 ) * v;
                    }
                    Count[ j ] = l;
                } else
                //
                // Canaux "standard"
                //
                if ( f < SND_RATE / 2 ) {
                    k = 0x10000 * f / SND_RATE;
                    l = Count[ j ];
                    for ( i = 0; i < snd_bufsize; i++ ) {
                        l += k;
                        gb->Wave[j][ i ] = ( l & 0x8000 ? 127 : -128 ) * v;
                    }

                    Count[ j ] = l & 0xFFFF;
                }
            } else {
                for ( i = 0; i < snd_bufsize; i++ ) {
                    gb->Wave[j][i] = 0x7FFF;
                }
            }
        }
    }

    for ( i = 0; i < snd_bufsize; i++ ) {
        dest[i].left = (gb->Wave[2][i] >> 2) + (gb->Wave[1][i] >> 2) + (gb->Wave[5][i] >> 2) + (gb->Wave[4][i] >> 2);
        dest[i].right = (gb->Wave[0][i] >> 2) + (gb->Wave[1][i] >> 2) + (gb->Wave[3][i] >> 2) + (gb->Wave[4][i] >> 2);

        for ( j = 0; j < SND_CHANNELS; j++ ) {
            gb->Wave[j][i] = 0x7FFF;
        }
    }

    //
    //    //    if( wptr >= SOUNDBUFCNT ) {  // Ne devrait pas arriver
    //    //        sbuf = soundbuf;
    //    //        wptr = 0;
    //    //    }
    //
    //
    //    printf("Before: %d %d %d ", SOUNDBUFCNT, wptr, count);
    //
    //    if (count > wptr) {
    //        printf( "(Audio underflow: miss %d)", count - wptr);
    //
    //        memset(dest + wptr, 0, (count - wptr) * sizeof(u16) * 2);
    //        count = wptr;
    //    }
    //
    //    memcpy(dest, soundbuf, count  * sizeof(u16) * 2);
    //    memmove(soundbuf, soundbuf+count * 2, (wptr - count) * sizeof(u16) * 2);
    //
    //    printf("After: %d %d %d\n", SOUNDBUFCNT, wptr, count);
    //
    //    wptr -= count;
    //    sbuf = soundbuf + wptr;
    // }

} /* crocods_copy_sound_buffer */

void procSound(core_crocods_t *gb, int uS)
{

    //    printf("(%d)", uS);


    Loop8912(gb, uS);

    //    u16 canal[2];
    //
    //    PSG_calc(&psg, canal);
    //
    //    if (wptr>= SOUNDBUFCNT ) {
    //        return;
    //    }
    //
    //    *sbuf = canal[0];
    //    sbuf ++;
    //
    //    *sbuf = canal[1];
    //    sbuf ++;
    //
    //    wptr ++ ;

} /* procSound */

#endif /* ifdef SOUNDV1 */

