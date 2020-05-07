/******************************************************************************/
/* Configuration pour l'archivage des diff�rents �l�ments du fichier source   */
/******************************************************************************/
// !CONFIG!=/L/* /R/* /W"* Nom : "
// D�finition du syst�me       !CONFIG!=/V1!EMULATEUR CPC!
// D�finition du sous syst�me  !CONFIG!=/V2!WIN-CPC!
// D�finition du sous ensemble !CONFIG!=/V3!Chips!
// D�finition du module        !CONFIG!=/V4!UPD 765!
/******************************************************************************/

/********************************************************* !NAME! **************
* !./FLE!
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\Fichiers
********************************************************** !0! *****************
* ------------------------------------------------------------------------------
*          SYSTEME         |      SOUS SYSTEME       |      SOUS ENSEMBLE
* ------------------------------------------------------------------------------
*  EMULATEUR CPC           | WIN-CPC                 | Chips
* ------------------------------------------------------------------------------
*  Fichier     : UPD.C                 | Version : 0.1w
* ------------------------------------------------------------------------------
*  Date        : 05/11/2002            | Auteur  : L.DEPLANQUE
* ------------------------------------------------------------------------------
*  Description : Emulation du UPD 765
*
* ------------------------------------------------------------------------------
*  Historique  :
*           Date           |         Auteur          |       Description
* ------------------------------------------------------------------------------
*  05/11/2002              | L.DEPLANQUE             | creation
* ------------------------------------------------------------------------------
*  06/02/2003              | L.DEPLANQUE             | Version 0.1v : m�morise
*                          |                         | la longueur du fichier lu
*                          |                         | pour r��crire
*                          |                         | si n�cessaire
* ------------------------------------------------------------------------------
*  18/02/2003              | L.DEPLANQUE             | Version 0.1w : prise en
*                          |                         | compte de l'information
*                          |                         | disc missing dans le
*                          |                         | registre ST0
* ------------------------------------------------------------------------------
********************************************************** !END! **************/

#include "plateform.h"

#include "upd.h"

//
// Constantes generales...
//
#define SECTSIZE   512
#define NBSECT     9


// Bits de Status
#define STATUS_CB  0x10
#define STATUS_NDM 0x20
#define STATUS_DIO 0x40
#define STATUS_RQM 0x80

// ST0
#define ST0_NR     0x08
#define ST0_SE     0x20
#define ST0_IC1    0x40
#define ST0_IC2    0x80

// ST1
#define ST1_ND     0x04
#define ST1_EN     0x80

// ST3
#define ST3_TS     0x08
#define ST3_T0     0x10
#define ST3_RY     0x20





/********************************************************* !NAME! **************
* Nom : RechercheSecteur
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Recherche un secteur sur la piste courrante
*
* R�sultat    : L'index du secteur trouv�
*
* Variables globales modifi�es : ST0, ST1
*
********************************************************** !0! ****************/
static int RechercheSecteur(core_crocods_t *core, int newSect, int *pos)
{
    int i;

    *pos = 0;
    for ( i = 0; i < core->CurrTrackDatasDSK.NbSect; i++ ) {
        if ( core->CurrTrackDatasDSK.Sect[ i ].R == newSect )
            return( ( UBYTE )i );
        else
            *pos += retro_le_to_cpu16(core->CurrTrackDatasDSK.Sect[ i ].SectSize);
    }

    core->ST0 |= ST0_IC1;
    core->ST1 |= ST1_ND;

    //    sprintf( MsgLog, "secteur (C:%02X,H:%02X,R:%02X) non trouv�", C, H, R );

    return( 0 );
}


/********************************************************* !NAME! **************
* Nom : ReadCHRN
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Renvoie les param�tres C, H, R, N de la piste courante
*
* R�sultat    : /
*
* Variables globales modifi�es : C, H, R, N, IndexSecteur
*
********************************************************** !0! ****************/
static void ReadCHRN(core_crocods_t *core)
{
    core->C = core->CurrTrackDatasDSK.Sect[ core->IndexSecteur ].C;
    core->H = core->CurrTrackDatasDSK.Sect[ core->IndexSecteur ].H;
    core->R = core->CurrTrackDatasDSK.Sect[ core->IndexSecteur ].R;
    core->N = core->CurrTrackDatasDSK.Sect[ core->IndexSecteur ].N;
    if ( ++core->IndexSecteur == core->CurrTrackDatasDSK.NbSect ) {
        core->IndexSecteur = 0;
    }
}


/********************************************************* !NAME! **************
* Nom : SetST0
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Calcul des registres ST0, ST1, ST2
*
* R�sultat    : /
*
* Variables globales modifi�es : ST0, ST1, ST2
*
********************************************************** !0! ****************/
static void SetST0(core_crocods_t *core)
{
    core->ST0 = 0; // drive A
    if ( !core->Moteur || core->Drive || !core->Image )
        core->ST0 |= ST0_IC1 | ST0_NR;

    core->ST1 = 0;
    core->ST2 = 0;
}


/********************************************************* !NAME! **************
* Nom : Rien
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction non reconnue du UPD
*
* R�sultat    : /
*
* Variables globales modifi�es : etat, ST0
*
********************************************************** !0! ****************/
static int Rien(core_crocods_t *core, int val)
{
    core->Status &= ~STATUS_CB & ~STATUS_DIO;
    core->etat = 0;
    core->ST0 = ST0_IC2;

    //    Log( "Appel fonction FDC Rien", LOG_DEBUG );

    return( core->Status );
}


/********************************************************* !NAME! **************
* Nom : ReadST0
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD ReadST0
*
* R�sultat    : ST0, C
*
* Variables globales modifi�es : etat, ST0, ST1, Inter, Busy
*
********************************************************** !0! ****************/
static int ReadST0(core_crocods_t *core, int val)
{
    if ( !core->Inter )
        core->ST0 = ST0_IC2;
    else {
        core->Inter = 0;
        if ( core->Busy ) {
            core->ST0 = ST0_SE;
            core->Busy = 0;
        } else
            core->ST0 |= ST0_IC1 | ST0_IC2;
    }
    if ( core->Moteur && core->Image && !core->Drive )
        core->ST0 &= ~ST0_NR;
    else {
        core->ST0 |= ST0_NR;
        if ( !core->Image )
            core->ST0 |= ( ST0_IC1 | ST0_IC2 );
    }

    if ( core->etat++ == 1 ) {
        core->Status |= STATUS_DIO;
        return( core->ST0 );
    }

    core->etat = val = 0;
    core->Status &= ~STATUS_CB & ~STATUS_DIO;
    core->ST0 &= ~ST0_IC1 & ~ST0_IC2;
    core->ST1 &= ~ST1_ND;
    return( core->C );
} /* ReadST0 */


/********************************************************* !NAME! **************
* Nom : ReadST3
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD ReadST3
*
* R�sultat    : 0, ST3
*
* Variables globales modifi�es : etat, Status, ST3
*
********************************************************** !0! ****************/
static int ReadST3(core_crocods_t *core, int val)
{
    if ( core->etat++ == 1 ) {
        core->Drive = val;
        core->Status |= STATUS_DIO;
        return( 0 );
    }
    core->etat = 0;
    core->Status &= ~STATUS_CB & ~STATUS_DIO;
    if ( core->Moteur && !core->Drive )
        core->ST3 |= ST3_RY;
    else
        core->ST3 &= ~ST3_RY;

    return( core->ST3 );
}


/********************************************************* !NAME! **************
* Nom : Specify
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD Specify, non �mul�e
*
* R�sultat    : 0
*
* Variables globales modifi�es : etat, Status
*
********************************************************** !0! ****************/
static int Specify(core_crocods_t *core, int val)
{
    if ( core->etat++ == 1 )
        return( 0 );

    core->etat = val = 0;
    core->Status &= ~STATUS_CB & ~STATUS_DIO;
    return( 0 );
}


/********************************************************* !NAME! **************
* Nom : ReadID
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD ReadID
*
* R�sultat    : ST0, ST1, ST2, C, H, R, N
*
* Variables globales modifi�es : Drive, Status, Inter, etat
*
********************************************************** !0! ****************/
static int ReadID(core_crocods_t *core, int val)
{
    switch ( core->etat++ ) {
        case 1:
            core->Drive = val;
            core->Status |= STATUS_DIO;
            core->Inter = 1;
            break;

        case 2:
            return( 0 /*ST0*/ );

        case 3:
            return( core->ST1 );

        case 4:
            return( core->ST2 );

        case 5:
            ReadCHRN(core);
            return( core->C );

        case 6:
            return( core->H );

        case 7:
            return( core->R );

        case 8:
            core->etat = 0;
            core->Status &= ~STATUS_CB & ~STATUS_DIO;
            return( core->N );
    } /* switch */
    return( 0 );
} /* ReadID */



/********************************************************* !NAME! **************
* Nom : FormatTrack
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD FormatTrack, non �mul�e
*
* R�sultat    : 0
*
* Variables globales modifi�es : etat, Status
*
********************************************************** !0! ****************/
static int FormatTrack(core_crocods_t *core, int val)
{
    //    Log( "Appel fonction FDC format", LOG_DEBUG );

    core->etat = val = 0;
    core->Status &= ~STATUS_CB & ~STATUS_DIO;
    return( 0 );
}


/********************************************************* !NAME! **************
* Nom : Scan
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD Scan, non �mul�e
*
* R�sultat    : 0
*
* Variables globales modifi�es : etat
*
********************************************************** !0! ****************/
static int Scan(core_crocods_t *core, int val)
{
    //    Log( "Appel fonction FDC scan", LOG_DEBUG );
    core->etat = val = 0;
    return( 0 );
}


/********************************************************* !NAME! **************
* Nom : ChangeCurrTrack
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Changement de la piste courrante
*
* R�sultat    : /
*
* Variables globales modifi�es : C, H, R, N, ST3
*
********************************************************** !0! ****************/
void ChangeCurrTrack(core_crocods_t *core, int newTrack)
{
    uint32_t Pos = 0;
    int t, s;

    if ( !core->Infos.DataSize ) {
        memcpy(&core->CurrTrackDatasDSK, core->ImgDsk, sizeof( core->CurrTrackDatasDSK ) );
        for ( t = 0; t < newTrack; t++ ) {
            for ( s = 0; s < core->CurrTrackDatasDSK.NbSect; s++ ) {
	         Pos += retro_le_to_cpu16(core->CurrTrackDatasDSK.Sect[ s ].SectSize);
            }

            Pos += sizeof( core->CurrTrackDatasDSK );
            memcpy(&core->CurrTrackDatasDSK, &core->ImgDsk[ Pos ], sizeof( core->CurrTrackDatasDSK ) );
        }
    } else
        Pos += retro_le_to_cpu16(core->Infos.DataSize) * newTrack;

    memcpy(&core->CurrTrackDatasDSK, &core->ImgDsk[ Pos ], sizeof( core->CurrTrackDatasDSK ) );

    core->PosData = Pos + sizeof( core->CurrTrackDatasDSK );
    core->IndexSecteur = 0;
    ReadCHRN(core);

    if ( !newTrack )
        core->ST3 |= ST3_T0;
    else
        core->ST3 &= ~ST3_T0;
} /* ChangeCurrTrack */


/********************************************************* !NAME! **************
* Nom : MoveTrack
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD MoveTrack
*
* R�sultat    : 0
*
* Variables globales modifi�es : etat, Drive, Status, Busy, Inter
*
********************************************************** !0! ****************/
static int MoveTrack(core_crocods_t *core, int val)
{
    switch ( core->etat++ ) {
        case 1:
            core->Drive = val;
            SetST0(core);
            core->Status |= STATUS_NDM;
            break;

        case 2:
            ChangeCurrTrack(core, core->C = val);
            core->etat = 0;
            core->Status &= ~STATUS_CB & ~STATUS_DIO & ~STATUS_NDM;
            core->Busy = 1;
            core->Inter = 1;
            break;
    }
    return( 0 );
}


/********************************************************* !NAME! **************
* Nom : MoveTrack0
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD MoveTrack0
*
* R�sultat    : 0
*
* Variables globales modifi�es : etat, C, Status, Busy, Inter
*
********************************************************** !0! ****************/
static int MoveTrack0(core_crocods_t *core, int val)
{
    core->Drive = val;
    ChangeCurrTrack(core, core->C = 0);
    core->etat = 0;
    core->Status &= ~STATUS_CB & ~STATUS_DIO & ~STATUS_NDM;
    SetST0(core);
    core->Busy = 1;
    core->Inter = 1;
    return( 0 );
}


/********************************************************* !NAME! **************
* Nom : ReadData
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD ReadData
*
* R�sultat    : Datas, ST0, ST1, ST2, C, H, R, N
*
* Variables globales modifi�es : C, H, R, N, EOT, etat, Status
*
********************************************************** !0! ****************/
static int ReadData(core_crocods_t *core, int val)
{


    switch ( core->etat++ ) {
        case 1:
            core->Drive = val;
            SetST0(core);
            break;

        case 2:
            core->C = val;
            break;

        case 3:
            core->H = val;
            break;

        case 4:
            core->R = val;
            break;

        case 5:
            core->N = val;
            break;

        case 6:
            core->EOT = val;
            break;

        case 7:
            core->updRsect = RechercheSecteur(core, core->R, &core->updRnewPos);
            if (core->updRsect != -1) {
                core->updRTailleSect = 128 << core->CurrTrackDatasDSK.Sect[ core->updRsect ].N;
                if ( !core->updRnewPos )
                    core->updRcntdata = ( core->updRsect * core->CurrTrackDatasDSK.SectSize ) << 8;
                else
                    core->updRcntdata = core->updRnewPos;
            }
            break;

        case 8:
            core->Status |= STATUS_DIO | STATUS_NDM;
            break;

        case 9:
            if ( !( core->ST0 & ST0_IC1 ) ) {
                if ( --core->updRTailleSect )
                    core->etat--;
                else {
                    if ( core->R++ < core->EOT )
                        core->etat = 7;
                    else
                        core->Status &= ~STATUS_NDM;
                }
                return( core->ImgDsk[ core->PosData + core->updRcntdata++ ] );
            }
            core->Status &= ~STATUS_NDM;
            return( 0 );

        case 10:
            return( core->ST0 );

        case 11:
            return( core->ST1 | ST1_EN );       // ### ici PB suivant logiciels... ###

        case 12:
            return(core->ST2 );

        case 13:
            return( core->C );

        case 14:
            return( core->H );

        case 15:
            return( core->R );

        case 16:
            core->etat = 0;
            core->Status &= ~STATUS_CB & ~STATUS_DIO;
            return( core->N );
    } /* switch */
    return( 0 );
} /* ReadData */


/********************************************************* !NAME! **************
* Nom : WriteData
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Fonction UPD WriteData
*
* R�sultat    : ST0, ST1, ST2, C, H, R, N
*
* Variables globales modifi�es : C, H, R, N, EOT, etat, Status
*
********************************************************** !0! ****************/
static int WriteData(core_crocods_t *core, int val)
{


    switch ( core->etat++ ) {
        case 1:
            core->Drive = val;
            SetST0(core);
            break;

        case 2:
            core->C = val;
            break;

        case 3:
            core->H = val;
            break;

        case 4:
            core->R = val;
            break;

        case 5:
            core->N = val;
            break;

        case 6:
            core->EOT = val;
            break;

        case 7:
            core->updWsect = RechercheSecteur(core, core->R, &core->updWnewPos);
            if (core->updWsect != -1) {

                core->updWTailleSect = 128 << core->CurrTrackDatasDSK.Sect[ core->updWsect ].N;
                if ( !core->updWnewPos )
                    core->updWcntdata = ( core->updWsect * core->CurrTrackDatasDSK.SectSize ) << 8;
                else
                    core->updWcntdata = core->updWnewPos;
            }
            break;

        case 8:
            core->Status |= STATUS_DIO | STATUS_NDM;
            break;

        case 9:
            if ( !( core->ST0 & ST0_IC1 ) ) {
                core->ImgDsk[ core->PosData + core->updWcntdata++ ] = ( UBYTE )val;
                if ( --core->updWTailleSect )
                    core->etat--;
                else {
                    if ( core->R++ < core->EOT )
                        core->etat = 7;
                    else
                        core->Status &= ~STATUS_NDM;
                }
                return( 0 );
            }
            core->Status &= ~STATUS_NDM;
            return( 0 );

        case 10:
            if ( !( core->ST0 & ST0_IC1 ) )
                core->FlagWrite = 1;

            return( core->ST0 );

        case 11:
            return( core->ST1 );

        case 12:
            return( core->ST2 );

        case 13:
            return( core->C );

        case 14:
            return( core->H );

        case 15:
            return( core->R );

        case 16:
            core->etat = 0;
            core->Status &= ~STATUS_CB & ~STATUS_DIO;
            return( core->N );
    } /* switch */
    return( 0 );
} /* WriteData */


/********************************************************* !NAME! **************
* Nom : ReadUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Lecture d'un registre/donn�e du UPD
*
* R�sultat    : Valeur registre/donn�e du UPD
*
* Variables globales modifi�es : /
*
********************************************************** !0! ****************/
int ReadUPD(core_crocods_t *core, int port)
{
    if ( port & 1 )
        return( core->fct(core, port) );

    return( core->Status );
}


/********************************************************* !NAME! **************
* Nom : WriteUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Ecriture d'un registre/donn�e du UPD
*
* R�sultat    : /
*
* Variables globales modifi�es : Status, etat, fct, Moteur
*
********************************************************** !0! ****************/
void WriteUPD(core_crocods_t *core, int port, int val)
{
    appendIcon(core, 0, 3, 10); // Disk read

    if ( port == 0xFB7F ) {
        if ( core->etat )
            core->fct(core, val);
        else {
            core->Status |= STATUS_CB;
            core->etat = 1;
            switch ( val & 0x1F ) {
                case 0x03:
                    // Specify
                    core->fct = Specify;
                    break;

                case 0x04:
                    // Lecture ST3
                    core->fct = ReadST3;
                    break;

                case 0x05:
                    // Ecriture donn�es
                    core->fct = WriteData;
                    break;

                case 0x06:
                    // Lecture donn�es
                    core->fct = ReadData;
                    break;

                case 0x07:
                    // D�placement t�te piste 0
                    //                    Status |= STATUS_NDM;
                    core->fct = MoveTrack0;
                    break;

                case 0x08:
                    // Lecture ST0, track
                    core->Status |= STATUS_DIO;
                    core->fct = ReadST0;
                    break;

                case 0x09:
                    // Ecriture donn�es
                    core->fct = WriteData;
                    break;

                case 0x0A:
                    // Lecture champ ID
                    core->fct = ReadID;
                    break;

                case 0x0C:
                    // Lecture donn�es
                    core->fct = ReadData;
                    break;

                case 0x0D:
                    // Formattage piste
                    core->fct = FormatTrack;
                    break;

                case 0x0F:
                    // D�placement t�te
                    core->fct = MoveTrack;
                    break;

                case 0x11:
                    core->fct = Scan;
                    break;

                default:
                    core->Status |= STATUS_DIO;
                    core->fct = Rien;
            } /* switch */
        }
    } else
    if ( port == 0xFA7E )
        core->Moteur = val & 1;
} /* WriteUPD */


/********************************************************* !NAME! **************
* Nom : ResetUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Effectue un reset du UPD
*
* R�sultat    : /
*
* Variables globales modifi�es : Status, ST0, ST1, ST2, ST3, Busy, Inter, etat
*
********************************************************** !0! ****************/
void ResetUPD(core_crocods_t *core)
{
    core->Status = STATUS_RQM;
    core->ST0 = ST0_SE;
    core->ST1 = 0;
    core->ST2 = 0;
    core->ST3 = ST3_RY | ST3_TS;
    core->Busy = 0;
    core->Inter = 0;
    core->etat = 0;

    core->fct = Rien;
}


/********************************************************* !NAME! **************
* Nom : EjectDiskUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Termine l'acc�s � une disquette (ejection)
*
* R�sultat    : /
*
* Variables globales modifi�es : /
*
********************************************************** !0! ****************/
void EjectDiskUPD(core_crocods_t *core)
{
    //    core->Image = 0;
    //
    //    char *dsk = (char*)malloc(sizeof( core->Infos ) + core->LongFic);
    //
    //    *length = sizeof(  core->Infos ) + core->LongFic;
    //
    //     if ( core->FlagWrite )
    //     {
    //         memcpy(dsk, &core->Infos, sizeof(core->Infos));
    //         memcpy(dsk+sizeof(core->Infos), core->ImgDsk, core->LongFic);
    //     core->FlagWrite = 0;
    //     }
    //
    //    return dsk;
    //
    /*
     * if (ImgDsk!=NULL) {
     * free(ImgDsk);
     * }
     * ImgDsk=NULL;
     */
}


/********************************************************* !NAME! **************
* Nom : SetDiskUPD
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Initialise l'acc�s � une disquette (insertion)
*
* R�sultat    : /
*
* Variables globales modifi�es : NomFic, Image, FlagWrite
*
********************************************************** !0! ****************/
void SetDiskUPD(core_crocods_t *core, char *n)
{
    /*
     * FILE * handle;
     *
     * EjectDiskUPD();
     * NomFic = n;
     * handle = fopen( NomFic, "rb" );
     * FAT_fseek(handle,0,SEEK_END);
     * LongFic = ftell(handle) - sizeof(Infos);
     * ImgDsk = (u8*)MyAlloc(dsksize, "UPD Disk");
     * FAT_fseek(handle,0,SEEK_SET);
     *
     * if ( handle )
     * {
     * fread( &Infos, sizeof( Infos ), 1, handle );
     * fread( ImgDsk, 1, LongFic, handle );
     * fclose( handle );
     * Image = 1;
     * }
     * FlagWrite = 0;
     * ChangeCurrTrack( 0 );
     */
}


/********************************************************* !NAME! **************
* Nom : GetCurrTrack
********************************************************** !PATHS! *************
* !./V1!\!./V2!\!./V3!\!./V4!\Fonctions
********************************************************** !1! *****************
*
* Fichier     : !./FPTH\/FLE!, ligne : !./LN!
*
* Description : Retourne la piste courrante
*
* R�sultat    : C
*
* Variables globales modifi�es : /
*
********************************************************** !0! ****************/
int GetCurrTrack(core_crocods_t *core)
{
    return( core->C );
}




int GetPosData(u8 *imgDsk, int track, int sect, char SectPhysique)
{
    // Recherche position secteur
    int Pos = 0;
    CPCEMUTrack *tr = ( CPCEMUTrack * )&imgDsk[ Pos ];
    short SizeByte;
    int t;

    for ( t = 0; t <= track; t++ ) {
        int s;
        Pos += sizeof( CPCEMUTrack );
        for ( s = 0; s < tr->NbSect; s++ ) {
            if ( t == track ) {
                if (  ( ( tr->Sect[ s ].R == sect ) && SectPhysique )
                      || ( ( s == sect ) && !SectPhysique )
                      )
                    break;
            }
            SizeByte = retro_le_to_cpu16 (tr->Sect[ s ].SectSize);
            if (SizeByte)
                Pos += SizeByte;
            else
                Pos += ( 128 << tr->Sect[ s ].N );
        }
    }
    return( Pos );
} /* GetPosData */

int GetMinSect(u8 *imgDsk)
{
    int Sect = 0x100;
    int s;

    CPCEMUTrack *tr = ( CPCEMUTrack * )&imgDsk[ 0 ];

    for ( s = 0; s < tr->NbSect; s++ ) {
        if ( Sect > tr->Sect[ s ].R )
            Sect = tr->Sect[ s ].R;
    }

    return( Sect );
}

#define     USER_DELETED 0xE5


