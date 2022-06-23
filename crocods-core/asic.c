#include "asic.h"

#include "z80.h"

#include "plateform.h"

#define ASIC_UNUSED_RAM_DATA 0x0b0


BOOL InitASIC(core_crocods_t *core) {


    core->ASIC_Data.ASIC_Ram = NULL;

    /* allocate ASIC Ram */
    core->ASIC_Data.ASIC_Ram = (unsigned char *)malloc(16384);

    if (core->ASIC_Data.ASIC_Ram==NULL)
        return FALSE;

    memset(core->ASIC_Data.ASIC_Ram, ASIC_UNUSED_RAM_DATA, 16384);

    core->ASIC_Data.ASIC_Ram_Adjusted = core->ASIC_Data.ASIC_Ram-(unsigned long)0x04000;

    /* Initialise cartridge pages */
    // ASIC_InitCart();

    /* initialise a table of colours and grey-scales. Given a ASIC
       colour, this will give the new colour to pass to the render part
       to give the appearance of a colour or grey-scale/paper-white display */
    // ASIC_InitialiseMonitorColourModes();

    /* initial analogue inputs */
    core->ASIC_Data.AnalogueInputs[0] = 0x03f;
    core->ASIC_Data.AnalogueInputs[1] = 0x03f;
    core->ASIC_Data.AnalogueInputs[2] = 0x03f;
    core->ASIC_Data.AnalogueInputs[3] = 0x03f;
    core->ASIC_Data.AnalogueInputs[4] = 0x03f;
    core->ASIC_Data.AnalogueInputs[5] = 0x000;
    core->ASIC_Data.AnalogueInputs[6] = 0x03f;
    core->ASIC_Data.AnalogueInputs[7] = 0x000;

    return TRUE;

}
