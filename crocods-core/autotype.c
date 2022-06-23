#include "autotype.h"

#include "platform.h"
#include "vga.h"
#include "ppi.h"

#include <string.h>

// this table converts from ASCII to CPC keyboard codes not exactly the same as converting from host keyboard scan codes to CPC keys

typedef struct 
{
  int        nASCII;
  CPC_SCANCODE cpcKey1;
  CPC_SCANCODE cpcKey2;
} ASCII_TO_CPCKEY_MAP;

// this table is not complete

static ASCII_TO_CPCKEY_MAP ASCIIToCPCMapQwerty[]=
{	   
	{ '1'       , CPC_1    , CPC_NIL } ,
	{ '!'       , CPC_1    , CPC_SHIFT} ,
	   
	{ '2'       , CPC_2    , CPC_NIL } ,
	{ '"'       , CPC_2    , CPC_SHIFT} ,
	   
	{ '3'       , CPC_3    , CPC_NIL } ,
	{ '#'       , CPC_3    , CPC_SHIFT} ,
	   
	{ '4'       , CPC_4    , CPC_NIL } ,
	{ '$'       , CPC_4    , CPC_SHIFT} ,
	   
	{ '5'       , CPC_5    , CPC_NIL } ,
	{ '%'       , CPC_5    , CPC_SHIFT} , 
	   
	{ '6'       , CPC_6    , CPC_NIL } ,
	{ '&'       , CPC_6    , CPC_SHIFT} ,

	{ '7'       , CPC_7    , CPC_NIL } ,
	{ '\''      , CPC_7    , CPC_SHIFT} ,

	{ '8'       , CPC_8    , CPC_NIL } ,
	{ '('       , CPC_8    , CPC_SHIFT} ,
	   
	{ '9'       , CPC_9    , CPC_NIL } ,
	{ ')'       , CPC_9    , CPC_SHIFT} ,
	   
	{ '0'       , CPC_ZERO , CPC_NIL } ,
	{ '_'       , CPC_ZERO , CPC_SHIFT} , 
	   
	{ '-'       , CPC_MINUS , CPC_NIL } ,
	{ '='       , CPC_MINUS , CPC_SHIFT} ,
	   
	{ '^'       , CPC_HAT  , CPC_NIL } ,
	{ 156       , CPC_HAT  , CPC_SHIFT} ,  // £

	{ '\t'		,CPC_TAB  , CPC_NIL } ,

	{ 'q'       , CPC_Q    , CPC_NIL } ,
	{ 'Q'       , CPC_Q    , CPC_SHIFT} ,

	{ 'w'       , CPC_W    , CPC_NIL } ,
	{ 'W'       , CPC_W    , CPC_SHIFT} , 

	{ 'e'       , CPC_E    , CPC_NIL } ,
	{ 'E'       , CPC_E    , CPC_SHIFT} ,

	{ 'r'       , CPC_R    , CPC_NIL } ,
	{ 'R'       , CPC_R    , CPC_SHIFT} ,

	{ 't'       , CPC_T    , CPC_NIL } ,
	{ 'T'       , CPC_T    , CPC_SHIFT} ,

	{ 'y'       , CPC_Y    , CPC_NIL } ,
	{ 'Y'       , CPC_Y    , CPC_SHIFT} ,

	{ 'u'       , CPC_U    , CPC_NIL } ,
	{ 'U'       , CPC_U    , CPC_SHIFT} , 

	{ 'i'       , CPC_I    , CPC_NIL } ,
	{ 'I'       , CPC_I    , CPC_SHIFT} ,

	{ 'o'       , CPC_O    , CPC_NIL } ,
	{ 'O'       , CPC_O    , CPC_SHIFT} ,

	{ 'p'       , CPC_P    , CPC_NIL } ,
	{ 'P'       , CPC_P    , CPC_SHIFT} ,
	   
	{ '@'       , CPC_AT   , CPC_NIL } ,
	{ '|'       , CPC_AT   , CPC_SHIFT } ,
	   
	{ '['       , CPC_OPEN_SQUARE_BRACKET , CPC_NIL } ,
	{ '{'       , CPC_OPEN_SQUARE_BRACKET , CPC_SHIFT} ,
	   
	{ '\n', CPC_RETURN , CPC_NIL } ,
    { '\r', CPC_RETURN , CPC_NIL } ,

	{ 'a'       , CPC_A    , CPC_NIL } ,
	{ 'A'       , CPC_A    , CPC_SHIFT} ,
	   
	{ 's'       , CPC_S    , CPC_NIL } ,
	{ 'S'       , CPC_S    , CPC_SHIFT} ,

	{ 'd'       , CPC_D    , CPC_NIL } ,
	{ 'D'       , CPC_D    , CPC_SHIFT} ,
	   
	{ 'f'       , CPC_F    , CPC_NIL } ,
	{ 'F'       , CPC_F    , CPC_SHIFT} ,

	{ 'g'       , CPC_G    , CPC_NIL } ,
	{ 'G'       , CPC_G    , CPC_SHIFT} ,

	{ 'h'       , CPC_H    , CPC_NIL } ,
	{ 'H'       , CPC_H    , CPC_SHIFT} ,

	{ 'j'       , CPC_J    , CPC_NIL } ,
	{ 'J'       , CPC_J    , CPC_SHIFT} ,

	{ 'k'       , CPC_K    , CPC_NIL } ,
	{ 'K'       , CPC_K    , CPC_SHIFT} ,

	{ 'l'       , CPC_L    , CPC_NIL } ,
	{ 'L'       , CPC_L    , CPC_SHIFT} ,
	   
	{ ':'       , CPC_COLON , CPC_NIL } ,
	{ '*'       , CPC_COLON , CPC_SHIFT} ,

	{ ';'       , CPC_SEMICOLON , CPC_NIL } ,
	{ '+'       , CPC_SEMICOLON , CPC_SHIFT} ,

	{ ']'       , CPC_CLOSE_SQUARE_BRACKET , CPC_NIL } ,
	{ '}'       , CPC_CLOSE_SQUARE_BRACKET , CPC_SHIFT} ,
	   
	{ '/'      , CPC_BACKSLASH , CPC_NIL } ,
	{ '`'      , CPC_BACKSLASH , CPC_SHIFT} ,

	{ 'z'       , CPC_Z    , CPC_NIL } ,
	{ 'Z'       , CPC_Z    , CPC_SHIFT} ,

	{ 'x'       , CPC_X    , CPC_NIL } ,
	{ 'X'       , CPC_X    , CPC_SHIFT} ,

	{ 'c'       , CPC_C    , CPC_NIL } ,
	{ 'C'       , CPC_C    , CPC_SHIFT} ,

	{ 'v'       , CPC_V    , CPC_NIL } ,
	{ 'V'       , CPC_V    , CPC_SHIFT} ,

	{ 'b'       , CPC_B    , CPC_NIL } ,
	{ 'B'       , CPC_B    , CPC_SHIFT} ,

	{ 'n'       , CPC_N    , CPC_NIL } ,
	{ 'N'       , CPC_N    , CPC_SHIFT} ,

	{ 'm'       , CPC_M    , CPC_NIL } ,
	{ 'M'       , CPC_M    , CPC_SHIFT} ,

	{ ','       , CPC_COMMA, CPC_NIL } ,
	{ '<'       , CPC_COMMA, CPC_SHIFT} ,
      
	{ '.'       , CPC_DOT  , CPC_NIL } ,
	{ '>'       , CPC_DOT  , CPC_SHIFT} ,
	   
	{ '\\'      , CPC_FORWARD_SLASH , CPC_NIL } ,
	{ '?'       , CPC_BACKSLASH, CPC_SHIFT} ,
    
    { '\b', CPC_DEL, CPC_NIL },

	{ ' ', CPC_SPACE, CPC_NIL } ,
    
    { 28, CPC_CURSOR_DOWN, CPC_NIL } ,
    { 29, CPC_CURSOR_UP, CPC_NIL } ,
    { 30, CPC_CURSOR_LEFT, CPC_NIL } ,
    { 31, CPC_CURSOR_RIGHT, CPC_NIL } ,
    
    { 27 , CPC_ESC, CPC_NIL }
};


static ASCII_TO_CPCKEY_MAP ASCIIToCPCMapAzerty[]=
{
    { '1'       , CPC_1    , CPC_SHIFT } ,
    { '!'       , CPC_8    , CPC_NIL} ,
	   
    { '2'       , CPC_2    , CPC_SHIFT } ,
    { '"'       , CPC_3    , CPC_NIL} ,
	   
    { '3'       , CPC_3    , CPC_SHIFT } ,
    { '#'       , CPC_CLOSE_SQUARE_BRACKET    , CPC_NIL} ,
	   
    { '4'       , CPC_4    , CPC_SHIFT } ,
    { '$'       , CPC_FORWARD_SLASH  , CPC_NIL} ,
	   
    { '5'       , CPC_5    , CPC_SHIFT } ,
    { '%'       , CPC_SEMICOLON    , CPC_SHIFT} ,
	   
    { '6'       , CPC_6    , CPC_SHIFT } ,
    { '&'       , CPC_1    , CPC_NIL} ,
    
    { '7'       , CPC_7    , CPC_SHIFT } ,
    { '\''      , CPC_4    , CPC_NIL} ,
    
    { '8'       , CPC_8    , CPC_SHIFT } ,
    { '('       , CPC_5    , CPC_NIL} ,
	   
    { '9'       , CPC_9    , CPC_SHIFT } ,
    { ')'       , CPC_MINUS    , CPC_NIL} ,
	   
    { '0'       , CPC_ZERO , CPC_SHIFT } ,
    { '_'       , CPC_HAT ,  CPC_SHIFT} ,
	   
    { '-'       , CPC_HAT , CPC_NIL } ,
    { '='       , CPC_BACKSLASH , CPC_NIL} ,
	   
    { '^'       , CPC_HAT  , CPC_NIL } ,
    { 156       , CPC_HAT  , CPC_SHIFT} ,  // £
    
    { '\t'		,CPC_TAB  , CPC_NIL } ,
    
    { 'q'       , CPC_A    , CPC_NIL } ,
    { 'Q'       , CPC_A    , CPC_SHIFT} ,
    
    { 'w'       , CPC_Z    , CPC_NIL } ,
    { 'W'       , CPC_Z    , CPC_SHIFT} ,
    
    { 'e'       , CPC_E    , CPC_NIL } ,
    { 'E'       , CPC_E    , CPC_SHIFT} ,
    
    { 'r'       , CPC_R    , CPC_NIL } ,
    { 'R'       , CPC_R    , CPC_SHIFT} ,
    
    { 't'       , CPC_T    , CPC_NIL } ,
    { 'T'       , CPC_T    , CPC_SHIFT} ,
    
    { 'y'       , CPC_Y    , CPC_NIL } ,
    { 'Y'       , CPC_Y    , CPC_SHIFT} ,
    
    { 'u'       , CPC_U    , CPC_NIL } ,
    { 'U'       , CPC_U    , CPC_SHIFT} ,
    
    { 'i'       , CPC_I    , CPC_NIL } ,
    { 'I'       , CPC_I    , CPC_SHIFT} ,
    
    { 'o'       , CPC_O    , CPC_NIL } ,
    { 'O'       , CPC_O    , CPC_SHIFT} ,
    
    { 'p'       , CPC_P    , CPC_NIL } ,
    { 'P'       , CPC_P    , CPC_SHIFT} ,
	   
    { '@'       , CPC_FORWARD_SLASH   , CPC_NIL } , // ???
    { '|'       , CPC_AT   , CPC_SHIFT } ,
	   
    { '['       , CPC_OPEN_SQUARE_BRACKET , CPC_NIL } ,
    { '{'       , CPC_OPEN_SQUARE_BRACKET , CPC_SHIFT} ,
	   
    { '\n', CPC_RETURN , CPC_NIL } ,
    { '\r', CPC_RETURN , CPC_NIL } ,
    
    { 'a'       , CPC_Q    , CPC_NIL } ,
    { 'A'       , CPC_Q    , CPC_SHIFT} ,
	   
    { 's'       , CPC_S    , CPC_NIL } ,
    { 'S'       , CPC_S    , CPC_SHIFT} ,
    
    { 'd'       , CPC_D    , CPC_NIL } ,
    { 'D'       , CPC_D    , CPC_SHIFT} ,
	   
    { 'f'       , CPC_F    , CPC_NIL } ,
    { 'F'       , CPC_F    , CPC_SHIFT} ,
    
    { 'g'       , CPC_G    , CPC_NIL } ,
    { 'G'       , CPC_G    , CPC_SHIFT} ,
    
    { 'h'       , CPC_H    , CPC_NIL } ,
    { 'H'       , CPC_H    , CPC_SHIFT} ,
    
    { 'j'       , CPC_J    , CPC_NIL } ,
    { 'J'       , CPC_J    , CPC_SHIFT} ,
    
    { 'k'       , CPC_K    , CPC_NIL } ,
    { 'K'       , CPC_K    , CPC_SHIFT} ,
    
    { 'l'       , CPC_L    , CPC_NIL } ,
    { 'L'       , CPC_L    , CPC_SHIFT} ,
	   
    { ':'       , CPC_DOT , CPC_NIL } ,
    { '*'       , CPC_OPEN_SQUARE_BRACKET , CPC_NIL} ,
    
    { ';'       , CPC_COMMA , CPC_NIL } ,
    { '+'       , CPC_BACKSLASH , CPC_SHIFT} ,
    
    { ']'       , CPC_CLOSE_SQUARE_BRACKET , CPC_NIL } ,
    { '}'       , CPC_CLOSE_SQUARE_BRACKET , CPC_SHIFT} ,
	   
    { '/'      , CPC_BACKSLASH , CPC_NIL } ,
    { '`'      , CPC_BACKSLASH , CPC_SHIFT} ,
    
    { 'z'       , CPC_W    , CPC_NIL } ,
    { 'Z'       , CPC_W    , CPC_SHIFT} ,
    
    { 'x'       , CPC_X    , CPC_NIL } ,
    { 'X'       , CPC_X    , CPC_SHIFT} ,
    
    { 'c'       , CPC_C    , CPC_NIL } ,
    { 'C'       , CPC_C    , CPC_SHIFT} ,
    
    { 'v'       , CPC_V    , CPC_NIL } ,
    { 'V'       , CPC_V    , CPC_SHIFT} ,
    
    { 'b'       , CPC_B    , CPC_NIL } ,
    { 'B'       , CPC_B    , CPC_SHIFT} ,
    
    { 'n'       , CPC_N    , CPC_NIL } ,
    { 'N'       , CPC_N    , CPC_SHIFT} ,
    
    { 'm'       , CPC_COLON    , CPC_NIL } ,
    { 'M'       , CPC_COLON    , CPC_SHIFT} ,
    
    { ','       , CPC_M, CPC_NIL } ,
    { '<'       , CPC_OPEN_SQUARE_BRACKET, CPC_SHIFT} ,
    
    { '.'       , CPC_COMMA , CPC_SHIFT } ,
    { '>'       , CPC_CLOSE_SQUARE_BRACKET  , CPC_SHIFT} ,
	   
    { '\\'      , CPC_FORWARD_SLASH , CPC_NIL } ,
    { '?'       , CPC_BACKSLASH, CPC_SHIFT} ,
    
    { '\b', CPC_DEL, CPC_NIL },
    
    { ' ', CPC_SPACE, CPC_NIL } ,
    
    { 28, CPC_CURSOR_DOWN, CPC_NIL } ,
    { 29, CPC_CURSOR_UP, CPC_NIL } ,
    { 30, CPC_CURSOR_LEFT, CPC_NIL } ,
    { 31, CPC_CURSOR_RIGHT, CPC_NIL } ,
    
    { 27 , CPC_ESC, CPC_NIL }
};

#ifndef _countof
   #define _countof(array) (sizeof(array)/sizeof((array)[0]))
#endif

void ASCII_to_CPC(core_crocods_t *core, int nASCII, BOOL bKeyDown)
{
   int i;
    ASCII_TO_CPCKEY_MAP *pMap;
    int nMap;
    
    if (core->keyboardLayout==0) {
    pMap = ASCIIToCPCMapQwerty;
    } else if (core->keyboardLayout==1) {
        pMap = ASCIIToCPCMapAzerty;
    } else {
        pMap = ASCIIToCPCMapQwerty;
    }
    nMap = _countof(ASCIIToCPCMapQwerty);
    
   for (i = 0; i < nMap; i++)
   {
      if (pMap->nASCII == nASCII)
      {
         if (bKeyDown)
         {
            if (pMap->cpcKey2 != CPC_NIL)
            {
               CPC_SetScanCode(core, pMap->cpcKey2);
            }
            CPC_SetScanCode(core, pMap->cpcKey1);
         }
         else 
         {
            CPC_ClearScanCode(core, pMap->cpcKey1);
            if (pMap->cpcKey2!= CPC_NIL)
            {
               CPC_ClearScanCode(core, pMap->cpcKey2);
            }
         }
         break;
      }
   
	  pMap++;
   }
}


/* init the auto type functions */
void AutoType_Init(core_crocods_t *core)
{
	core->AutoType.nFlags = 0;
    core->AutoType.sString = NULL;
	core->AutoType.nPos = 0;
	core->AutoType.nFrames = 0;
	core->AutoType.nCountRemaining = 0;
}

BOOL AutoType_Active(core_crocods_t *core)
{
	/* if actively typing, or waiting for first keyboard scan
	before typing then auto-type is active */
	return ((core->AutoType.nFlags & (AUTOTYPE_ACTIVE|AUTOTYPE_WAITING))!=0);
}


/* set the string to auto type */
void AutoType_SetString(core_crocods_t *core, const char *sString, BOOL bWaitInput)
{
    if (core->AutoType.sString!=NULL) {
        free(core->AutoType.sString);
    }
    core->AutoType.sString = (char*)malloc(strlen(sString)+1);
	strcpy(core->AutoType.sString, sString);
	core->AutoType.ch = 0;
	core->AutoType.nPos = 0;
	core->AutoType.nFrames = 0;
	core->AutoType.nCountRemaining = (int)strlen(sString);
	if (bWaitInput)
	{
	    SoftResetCPC(core);

		/* wait for first keyboard */
		core->AutoType.nFlags|=AUTOTYPE_WAITING;
		core->AutoType.nFlags&=~AUTOTYPE_ACTIVE;
	}
	else
	{
		core->AutoType.nFlags |= AUTOTYPE_ACTIVE;
	}
}

/* execute this every emulated frame; even if it will be skipped */
void AutoType_Update(core_crocods_t *core)
{
	if ((core->AutoType.nFlags & AUTOTYPE_ACTIVE)==0)
	{
		if ((core->AutoType.nFlags & AUTOTYPE_WAITING)!=0)
		{
			if (Keyboard_HasBeenScanned(core))
			{
				/* auto-type is now active */
				core->AutoType.nFlags |= AUTOTYPE_ACTIVE;
				/* no longer waiting */
				core->AutoType.nFlags &=~AUTOTYPE_WAITING;
			}
		}
	}
	else
	{
		/* auto-type is active */

		/* delay frames? */
		if (core->AutoType.nFrames!=0)
		{
			core->AutoType.nFrames--;
			return;
		}

		/* NOTES:
			- if SHIFT or CONTROL is pressed, then they must be released
			for at least one whole frame for the CPC operating system to recognise them 
			as released.
			
			- When the same key is pressed in sequence (e.g. press, release, press, release)
			then there must be at least two frames for the key to be recognised as released.
			The CPC operating system is trying to 'debounce' the key
		*/
		if (core->AutoType.nFlags & AUTOTYPE_RELEASE)
		{
			if (core->AutoType.nCountRemaining==0)
			{
				/* auto type is no longer active */
				core->AutoType.nFlags &=~AUTOTYPE_ACTIVE;
			}

			core->AutoType.nFlags &=~AUTOTYPE_RELEASE;

			if (core->AutoType.ch!=1)
			{
				/* release the key */
				ASCII_to_CPC(core, core->AutoType.ch, FALSE);
			}

			/* number of frames for release to be acknowledged */
			core->AutoType.nFrames = 1;
		}
		else
		{
			char ch;
        

			/* get the current character */
			ch = core->AutoType.sString[core->AutoType.nPos];

			/* update position in string */
			core->AutoType.nPos++;

			/* update count */
			core->AutoType.nCountRemaining--;

			core->AutoType.ch = ch;

			if (ch==1)
			{
				core->AutoType.nFrames = 2;
			}
			else
			{
				/* number of frames for key to be acknowledged */
				core->AutoType.nFrames=1;

				ASCII_to_CPC(core, ch, TRUE);
			}

			core->AutoType.nFlags |= AUTOTYPE_RELEASE;
		}
	}
}

