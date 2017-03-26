#include  "rompack.h"

#include  "plateform.h"
#include  "config.h"
#include  "sound.h"
#include  "crtc.h"
#include  "ppi.h"
#include  "vga.h"
#include  "z80.h"


#define     MODULENAME      "Rompack"



int ED_00( core_crocods_t *core )
{
    core->Z80.AF.Byte.High = 0xAA;
    return( 2 );
}

int ED_01( core_crocods_t *core ) { // PACKSCR
    
    
    
    
    return 2;
}

int ED_02( core_crocods_t *core ) { return 2; }

int ED_03( core_crocods_t *core ) { return 2; }

int ED_04( core_crocods_t *core ) { return 2; }

int ED_05( core_crocods_t *core ) { return 2; }

int ED_06( core_crocods_t *core ) { return 2; }

int ED_07( core_crocods_t *core ) { return 2; }

int ED_08( core_crocods_t *core ) { return 2; }

int ED_09( core_crocods_t *core ) {
    return 2;
}

int ED_0A( core_crocods_t *core ) { // SSH
    static char Nom[ 258 ];
    static char Argument[ 258 ];
    int i;
    
    for ( i = 0; i < RegB; i++ )
        Nom[ i ] = Peek8Ext( core, ( USHORT )( RegHL + i ) );
    
    Nom[ i ] = 0;
    
    for ( i = 0; i < RegC; i++ )
        Argument[ i ] = Peek8Ext( core, ( USHORT )( RegDE + i ) );
    
    Argument[ i ] = 0;
    
    printf("%s\n%s\n", Nom, Argument);
    
    memcpy(core->runParam[0], Nom, 258);
    memcpy(core->runParam[1], Argument, 258);
    strcpy(core->runApp, "ssh");
    
    core->runStartApp = 1;
    return 2;
}

int ED_0B( core_crocods_t *core ) { return 2; }

int ED_0C( core_crocods_t *core ) { return 2; }

int ED_0D( core_crocods_t *core ) { return 2; }

int ED_0E( core_crocods_t *core ) { return 2; }

int ED_0F( core_crocods_t *core ) { return 2; }

int ED_10( core_crocods_t *core ) { return 2; }

int ED_11( core_crocods_t *core ) { return 2; }

int ED_12( core_crocods_t *core ) { return 2; }

int ED_13( core_crocods_t *core ) { return 2; }

int ED_14( core_crocods_t *core ) { return 2; }

int ED_15( core_crocods_t *core ) { return 2; }

int ED_16( core_crocods_t *core ) { return 2; }

int ED_17( core_crocods_t *core ) { return 2; }

int ED_18( core_crocods_t *core ) { return 2; }

int ED_19( core_crocods_t *core ) { return 2; }

int ED_1A( core_crocods_t *core ) { return 2; }

int ED_1B( core_crocods_t *core ) { return 2; }

int ED_1C( core_crocods_t *core ) { return 2; }

int ED_1D( core_crocods_t *core ) { return 2; }

int ED_1E( core_crocods_t *core ) { return 2; }

int ED_1F( core_crocods_t *core )
{
    return( 2 );
}
