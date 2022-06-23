#include "platform.h"
#include "z80.h"
#include "crtc.h"
#include "vga.h"
#include "monitor.h"

/*
 *
 * CRTC type 0 : UM6845 ou Japonais (sur la plupart des  CPC6128 sortis entre 85 et  87).
 * CRTC type 1 : UM6845R ( sur la plupart des  CPC6128 sortis entre 88 et 90).
 * CRTC type 2 : UM6845S ( sur la plupart des  CPC464 et des CPC664).
 * CRTC type 3 : EmulŽ (ASIC) sur les 464 plus et 6128 plus.
 * CRTC type 4 : EmulŽ (ERSATZ PLUS) sur la plupart des  CPC6128 sortis en 90.
 *
 */

void GateArray_UpdateHsync(core_crocods_t *core, BOOL bState);
void GateArray_UpdateVsync(core_crocods_t *core, BOOL bState);

void GateArray_DoDispEnable(core_crocods_t *core, BOOL bState);

int CRTC_GetHorizontalSyncWidth(core_crocods_t *core);
int CRTC_GetVerticalSyncWidth(core_crocods_t *core);
int CRTC_GetRAOutput(core_crocods_t *core);

#define GET_MA (core->RegsCRTC[12] << 8) | (core->RegsCRTC[13])

void CRTC_DoLineChecks(core_crocods_t *core);
void arn_CRTC_DoLine(core_crocods_t *core);

/*-----------------------------------------------------------------------*/
/* HD6845S */
const unsigned char HD6845S_ReadMaskTable[32] =
{
    0x000,     /* Horizontal Total */
    0x000,     /* Horizontal Displayed */
    0x000,     /* Horizontal Sync Position */
    0x000,     /* Sync Widths */
    0x000,     /* Vertical Total */
    0x000,     /* Vertical Adjust */
    0x000,     /* Vertical Displayed */
    0x000,     /* Vertical Sync Position */
    0x000,     /* Interlace and Skew */
    0x000,     /* Maximum Raster Address */
    0x000,     /* Cursor Start */
    0x000,     /* Cursor End */
    0x0ff,     /* Screen Addr (H) */
    0x0ff,     /* Screen Addr (L) */
    0x0ff,     /* Cursor (H) */
    0x0ff,     /* Cursor (L) */

    0x0ff,     /* Light Pen (H) */
    0x0ff,     /* Light Pen (L) */
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
    0x000,
};

/* these are anded before data is written */
const unsigned char HD6845S_WriteMaskTable[32] =
{
    0x0ff,
    0x0ff,
    0x0ff,
    0x0ff,
    0x07f,
    0x01f,
    0x07f,
    0x07f,
    0x0f3,
    0x01f,
    0x07f,
    0x01f,
    0x03f,
    0x0ff,
    0x03f,
    0x0ff,

    0x0ff,
    0x0ff,
    0x0ff,
    0x0ff,
    0x0ff,
    0x0ff,
    0x0ff,
    0x0ff,
    0x0ff,
    0x0ff,
    0x0ff,
    0x0ff,
    0x0ff,
    0x0ff,
    0x0ff,
    0x0ff,
};

void arn_RegisterSelectCRTC(core_crocods_t *core, u8 RegisterIndex)
{
    core->CRTC_Reg = (unsigned char)RegisterIndex;
}

/*------------------------------------------------------------------------------------------------------*/
int CRTC_GetSelectedRegister(core_crocods_t *core)
{
    return core->CRTC_Reg;
}

void CRTC_SetVsyncOutput(core_crocods_t *core, BOOL bState)
{
    // if state has changed, and state is now active

    // Computer_UpdateVsync

    BOOL bVsync = FALSE;
    u8 crtcType = 0;

    /* vsync can be driven by PPI or by crtc */
    if ((crtcType == 0) && ((core->Mask[1] ^ 0xff) == 0x0ff)) {  // core->Output
        /* PPI is driving vsync */
        /* assume that it takes control always */
        bVsync = ((core->RegsPPI[1] & 1) != 0); // TODO
    } else {
        /* CRTC is driving vsync */
        bVsync = bState;
    }

    GateArray_UpdateVsync(core, bVsync);
}

void arn_ResetCRTC(core_crocods_t *core)
{
    int i;
    
    core->screenIsOptimized = 1;

    /* set light pen registers - this is what my CPC type 0 reports! */
    core->RegsCRTC[16] = 0x014;
    core->RegsCRTC[17] = 0x07c;

    /* vsync counter not active */
    CRTC_ClearFlag(CRTC_VSCNT_FLAG);
    /* not in hsync */
    CRTC_ClearFlag(CRTC_HS_FLAG);
    /* not in a vsync */
    CRTC_ClearFlag(CRTC_VS_FLAG);
    /* not reached end of line */
    CRTC_ClearFlag(CRTC_HTOT_FLAG);
    /* not reached end of frame */
    CRTC_ClearFlag(CRTC_VTOT_FLAG);

    /*		core->GA_State.GA_Flags &=~GA_HSYNC_FLAG;
     * core->GA_State.GA_Flags &=~GA_VSYNC_FLAG;
     */
    /* not reached last raster in char */
    CRTC_ClearFlag(CRTC_MR_FLAG);
    /* not in vertical adjust */
    CRTC_ClearFlag(CRTC_VADJ_FLAG);
    /* do not display graphics */
    CRTC_ClearFlag(CRTC_VDISP_FLAG);
    CRTC_ClearFlag(CRTC_HDISP_FLAG);
    CRTC_ClearFlag(CRTC_VADJWANTED_FLAG);
    CRTC_ClearFlag(CRTC_R8DT_FLAG);

    /* reset all registers */
    for (i = 0; i < 16; i++) {
        /* select register */
        RegisterSelectCRTC(core, i);

        /* write data */
        WriteCRTC(core, 0);
    }

    /* reset CRTC internal registers */

    /* reset horizontal count */
    core->HCount = 0;
    /* reset line counter (vertical count) */
    core->LineCounter = 0;
    /* reset raster count */
    core->RasterCounter = 0;
    /* reset MA */
    core->MA = 0;
    core->MAStore = 0;
    core->Frame = 0;

    core->CursorOutput = 0;
    core->CursorBlinkCount = 0;

    CRTC_DoLineChecks(core);
}

/*------------------------------------------------------------------------------------------------------*/
unsigned char CRTC_GetRegisterData(core_crocods_t *core, int RegisterIndex)
{
    return (unsigned char)core->RegsCRTC[RegisterIndex & 0x01f];
}

void CRTC_SetDispEnable(core_crocods_t *core, BOOL DispEnable)
{
    GateArray_DoDispEnable(core, DispEnable);
}

void CRTC_SetHsyncOutput(core_crocods_t *core, BOOL bState)
{
    GateArray_UpdateHsync(core, bState);
}

void CRTC_DoDispEnable(core_crocods_t *core)
{
    /* disp enable is based on the output of HDISP, VDISP and R8 delay */
    /* confirmed for type 3 */
    if ((core->CRTC_Flags & (CRTC_HDISP_FLAG | CRTC_VDISP_FLAG | CRTC_R8DT_FLAG)) == (CRTC_HDISP_FLAG | CRTC_VDISP_FLAG)) {
        CRTC_SetDispEnable(core, TRUE);
    } else {
        CRTC_SetDispEnable(core, FALSE);
    }
}

void CRTC_InitVsync(core_crocods_t *core)
{
    core->LinesAfterVsyncStart = 0;

    if (!(core->CRTC_Flags & CRTC_VSCNT_FLAG))
    {
        core->VerticalSyncCount = 0;

        core->VerticalSyncWidth = CRTC_GetVerticalSyncWidth(core);

        CRTC_SetFlag(CRTC_VSCNT_FLAG);
        CRTC_SetVsyncOutput(core, TRUE);
    }
}

void CRTC_RefreshHStartAndHEnd(core_crocods_t *core)
{
//    core->XStart = max( ( 50 - core->RegsCRTC[2] ) << 1, 0);
//    core->XEnd = min(core->XStart + ( core->RegsCRTC[1] << 1 ), 96);

    /* if Reg 8 is used, start and end positions are delayed by amount
     * programmed. HStart can also be additionally delayed by ASIC. */

    /* set start and end positions of lines */
    core->HEnd = (unsigned char)(core->RegsCRTC[1] + core->HDelayReg8);
    core->HStart = core->HDelayReg8;

    /* set HStart and HEnd to same, because Reg1 is set to 0 */
    if (core->RegsCRTC[1] == 0)
        core->HStart = core->HEnd = 0;

    /* update rendering function */
    CRTC_DoDispEnable(core);
}

void CRTC_DoReg1(core_crocods_t *core)
{
    CRTC_RefreshHStartAndHEnd(core);
}

void CRTC_DoReg8(core_crocods_t *core)
{
    int Delay;

    /* on type 3 changing r8 rapidly shows nothing */

    /* number of characters delay */
    Delay = (core->RegsCRTC[8] >> 4) & 0x03;
    CRTC_ClearFlag(CRTC_R8DT_FLAG);

    if (Delay == 3) {
        /* Disable display of graphics */
        CRTC_SetFlag(CRTC_R8DT_FLAG);
        Delay = 0;
    }

    core->HDelayReg8 = (unsigned char)Delay;

    CRTC_RefreshHStartAndHEnd(core);
} // CRTC_DoReg8

/********************************/
/* CRTC type 0 - HD6845S/UM6845 */
/********************************/

/* vadj is 1 char line? mr checked against vertical adjust counter? */
/* allows repeat of rest; to confirm; we should be able to change value when R9 = 4 at a different pos */

void CRTC_UpdateState(core_crocods_t *core, int RegIndex)
{
    /* re-programming vsync position doesn't cut vsync */
    /* re-programming length doesn't seem to cut vsync */

    switch (RegIndex) {
        case 1:
            CRTC_DoReg1(core);
            break;

        case 3:
            break;

        case 5: // vertical total adjust
//            CRTC.vt_adjust = val & 0x1f;

        case 6: {
            /* confirmed: immediate on type 0 */
            if (core->LineCounter == core->RegsCRTC[6]) {
                CRTC_ClearFlag(CRTC_VDISP_FLAG);
            }

            if ((core->LineCounter == 0) && (core->RasterCounter == 0)) {
                if (core->RegsCRTC[6] != 0) {
                    CRTC_SetFlag(CRTC_VDISP_FLAG);
                }
            }

            CRTC_DoDispEnable(core);
        }
        break;

        case 7: {
            /* confirmed: Register can be written at any time and takes immediate effect; not sure if 0 or HCC=R0 */
            if ((core->LineCounter == core->RegsCRTC[7]) && (core->HCount != 0)) {
                CRTC_InitVsync(core);
            }
        }
        break;

        case 8:
            CRTC_DoReg8(core);
            break;

        case 9: {
            if (core->CRTC_Flags & CRTC_VADJ_FLAG) {
                if (core->VertAdjustCount == core->RegsCRTC[9]) {
                    CRTC_SetFlag(CRTC_MR_FLAG);
                } else {
                    CRTC_ClearFlag(CRTC_MR_FLAG);
                }
            } else {
                // confirm r8
                if (core->RasterCounter == core->RegsCRTC[9]) {
                    CRTC_SetFlag(CRTC_MR_FLAG);
                } else {
                    CRTC_ClearFlag(CRTC_MR_FLAG);
                }
            }
        }
        break;

        case 14:
        case 15:
            core->CursorMA = (core->RegsCRTC[14] << 8) | core->RegsCRTC[15];
            break;

        default:
            break;
    } // switch
} // CRTC_UpdateState

/*---------------------------------------------------------------------------*/

void arn_WriteCRTC(core_crocods_t *core, u8 val)
{
    int CRTC_RegIndex = core->CRTC_Reg & 0x1f;

    /* store registers using current CRTC information - masking out appropiate bits etc for this CRTC*/
    core->RegsCRTC[CRTC_RegIndex] = (unsigned char)(val & HD6845S_WriteMaskTable[CRTC_RegIndex]);

    CRTC_UpdateState(core, CRTC_RegIndex);

    core->XStart = max( (50 - core->RegsCRTC[2]) << 1, 0);
    core->XEnd = min(core->XStart + (core->RegsCRTC[1] << 1), 96);
}

/*---------------------------------------------------------------------------*/

void CRTC_DoHDisp(core_crocods_t *core)
{
    CRTC_ClearFlag(CRTC_HDISP_FLAG);  // crt1 only ???
    CRTC_DoDispEnable(core);// crt1 only ???

    /* confirmed: if rcc=r9 at HDISP time then store MA for reload. It is possible to change R9 around R1 time only
     * and get the graphics to repeat but doesn't cause problems for RCC */
    /* confirmed: gerald's tests seem to indicate that MAStore is not updated when vdisp is not active. i.e. in lower border */

    if ((core->CRTC_Flags & CRTC_MR_FLAG) && (core->CRTC_Flags & CRTC_VDISP_FLAG))
        /* remember it for next line */
        core->MAStore = core->MA;
}

void CRTC_RestartFrame(core_crocods_t *core)
{
    core->LinesAfterFrameStart = 0;

    core->MAStore = GET_MA;
    core->MA = core->MAStore;

    core->RasterCounter = 0;
    core->LineCounter = 0;

    CRTC_SetFlag(CRTC_VDISP_FLAG);

    CRTC_DoDispEnable(core);

    /* on type 0, the first line is always visible */

    /* if type 0 is a HD6845S */
    CRTC_SetFlag(CRTC_VDISP_FLAG);

    /* incremented when? */
    core->CursorBlinkCount++;
    if (core->RegsCRTC[10] & (1 << 6)) {
        /* blink */
        if (core->RegsCRTC[11] & (1 << 5)) {
            /* 32 field period */
            /* should we just test bit 5? */
            if (core->CursorBlinkCount == 32) {
                core->CursorBlinkCount = 0;
                core->CursorBlinkOutput ^= 1;
            }
        } else {
            /* 16 field period */
            /* should we just test bit 4? */
            if (core->CursorBlinkCount == 16) {
                core->CursorBlinkCount = 0;
                core->CursorBlinkOutput ^= 1;
            }
        }
    } else {
        if (core->RegsCRTC[10] & (1 << 5)) {
            /* no blink, no output */
            core->CursorBlinkOutput = 0;
        } else {
            /* no blink */
            core->CursorBlinkOutput = 1;
        }
    }
} // CRTC_RestartFrame

void    CRTC_MaxRasterMatch(core_crocods_t *core)
{
    if (core->CRTC_Flags & CRTC_INTERLACE_ACTIVE) {
        if (core->RegsCRTC[8] & (1 << 1)) {
            if (core->RasterCounter == (core->RegsCRTC[9] >> 1)) {
                CRTC_SetFlag(CRTC_MR_FLAG);
            } else {
                CRTC_ClearFlag(CRTC_MR_FLAG);
            }
        }
    } else {
        if (core->CRTC_Flags & CRTC_VADJ_FLAG) {
            if (core->VertAdjustCount == core->RegsCRTC[9]) {
                CRTC_SetFlag(CRTC_MR_FLAG);
            }
        } else {
            if (core->RasterCounter == core->RegsCRTC[9]) {
                CRTC_SetFlag(CRTC_MR_FLAG);
            }
        }
    }

    if (core->CRTC_Flags & CRTC_MR_FLAG) {
        if (core->LineCounter == core->RegsCRTC[4]) {
            CRTC_SetFlag(CRTC_VTOT_FLAG);
        }
    }
} // CRTC_MaxRasterMatch

/* appears that on crtc type 0 and type 3, Vertical Sync width can be reprogrammed
 * while it is active. The Vertical Sync Counter is 4-bit. Comparison for both appears to be equal! */
static void CRTC_DoVerticalSyncCounter(core_crocods_t *core)
{
    /* are we counting vertical syncs? */
    if (core->CRTC_Flags & CRTC_VSCNT_FLAG) {
        /* update vertical sync counter */
        core->VerticalSyncCount++;

        /* if vertical sync count = vertical sync width then stop vertical sync */
        /* if vertical sync width = 0, the counter will wrap after incrementing from 15 causing
         * a vertical sync width of 16*/
        if (core->VerticalSyncCount == core->VerticalSyncWidth) {
            /* count done */
            core->VerticalSyncCount = 0;

            CRTC_ClearFlag(CRTC_VSCNT_FLAG);
        }
    }
}

void CRTC_DoLineChecks(core_crocods_t *core)
{
    /* confirmed: immediate on type 0 */
    if (core->LineCounter == core->RegsCRTC[6]) {
        CRTC_ClearFlag(CRTC_VDISP_FLAG);
        CRTC_DoDispEnable(core);
    }

    /* check Vertical sync position */
    if (core->LineCounter == core->RegsCRTC[7]) {
        CRTC_InitVsync(core);
    }
}

void CRTC_InterlaceControl_SetupStandardVsync(core_crocods_t *core)
{
    /* set VSYNC immediatly */
    CRTC_SetFlag(CRTC_VS_FLAG);

    /* keep VSYNC set at HTOT/2 */
    core->CRTC_HalfHtotFlags = CRTC_VS_FLAG;

    CRTC_SetVsyncOutput(core, TRUE);
}

void CRTC_InterlaceControl_FinishStandardVsync(core_crocods_t *core)
{
    /* clear vsync */
    CRTC_ClearFlag(CRTC_VS_FLAG);

    /* no VSYNC on next HTOT/2 */
    core->CRTC_HalfHtotFlags = 0;
    CRTC_SetVsyncOutput(core, FALSE);
}

void CRTC_InterlaceControl_VsyncStart(core_crocods_t *core)
{
    CRTC_InterlaceControl_SetupStandardVsync(core);
}

void CRTC_InterlaceControl_VsyncEnd(core_crocods_t *core)
{
    CRTC_InterlaceControl_FinishStandardVsync(core);
}

/* executed each NOP cycle performed by the Z80 */
void arn_CRTC_DoCycles(core_crocods_t *core, u32 Cycles)
{
    int i;

    for (i = Cycles - 1; i >= 0; i--) {
        core->CharsAfterHsyncStart++;
        /* increment horizontal count */
        core->HCount = (unsigned char)((core->HCount + 1) & 0x0ff);
        core->MA = (core->MA + 1) & 0x03fff;

        if (core->CRTC_Flags & CRTC_HTOT_FLAG) {
            unsigned long PreviousFlags = core->CRTC_Flags;
            CRTC_ClearFlag(CRTC_HTOT_FLAG);

            /* zero count */
            core->HCount = 0;
            core->LinesAfterFrameStart++;
            core->LinesAfterVsyncStart++;

            arn_CRTC_DoLine(core);

            if (((PreviousFlags ^ core->CRTC_Flags) & CRTC_VSCNT_FLAG) != 0) {
                /* vsync counter bit has changed state */
                if (core->CRTC_Flags & CRTC_VSCNT_FLAG) {
                    /* change from vsync counter inactive to active */
                    CRTC_InterlaceControl_VsyncStart(core);
                } else {
                    /* change from counter active to inactive */
                    CRTC_InterlaceControl_VsyncEnd(core);
                }
            }

            core->CRTC_FlagsAtLastHtot = core->CRTC_Flags;
        }

        /* does horizontal equal Htot? */
        if (core->HCount == core->RegsCRTC[0]) {
            CRTC_SetFlag(CRTC_HTOT_FLAG);
        }

        if (core->HCount == (core->RegsCRTC[0] >> 1)) {
            unsigned long Flags;

            /* get flags */
            Flags = core->CRTC_Flags;
            /* clear VSYNC flag */
            Flags &= ~CRTC_VS_FLAG;
            /* set/clear VSYNC flag */
            Flags |= core->CRTC_HalfHtotFlags;
            /* store new flags */
            core->CRTC_Flags = Flags;
        }

        /* Horizontal Sync Width Counter */
        /* are we counting horizontal syncs? */
        if (core->CRTC_Flags & CRTC_HS_FLAG) {
            core->HorizontalSyncCount++;
            /* if horizontal sync count = Horizontal Sync Width then
             * stop horizontal sync */
            if (core->HorizontalSyncCount == core->HorizontalSyncWidth) {
                core->HorizontalSyncCount = 0;

                /* stop horizontal sync counter */
                CRTC_ClearFlag(CRTC_HS_FLAG);

                /* call functions that would happen on a HSYNC */
                CRTC_SetHsyncOutput(core, FALSE);
            }
        }

        /* does current horizontal count equal position to start horizontal sync? */
        if (core->HCount == core->RegsCRTC[2]) {
            core->CharsAfterHsyncStart = 0;
            core->HorizontalSyncWidth = CRTC_GetHorizontalSyncWidth(core);

            /* if horizontal sync = 0, in the HD6845S no horizontal
             * sync is generated. The input to the flip-flop is 1 from
             * both Horizontal Sync Position and HorizontalSyncWidth, and
             * the HSYNC is not even started */
            if (core->HorizontalSyncWidth != 0) {
                /* are we already in a HSYNC? */
                if (!(core->CRTC_Flags & CRTC_HS_FLAG)) {
                    /* no.. */

                    /* enable horizontal sync counter */
                    CRTC_SetFlag(CRTC_HS_FLAG);

                    CRTC_SetHsyncOutput(core, TRUE);

                    /* initialise counter */
                    core->HorizontalSyncCount = 0;
                }
            }
        }

        if (core->HCount == core->HStart) {   // confirmed: on type 3, border is turned off at HStart - enable horizontal display
            CRTC_SetFlag(CRTC_HDISP_FLAG);
            CRTC_DoDispEnable(core);
        }

        if (core->HCount == core->HEnd) {     // confirmed: on type 3, border is turned on at HEnd.
            CRTC_ClearFlag(CRTC_HDISP_FLAG);
            CRTC_DoDispEnable(core);
        }

        if (core->HCount == core->RegsCRTC[1]) {    // confirmed: on type 3, hdisp is triggered from R1 because I don't see the screen distort which would happen if it's ad HEnd
            CRTC_DoHDisp(core);
        }

        Graphics_Update(core);
    }
}

/* executed for each complete line done by the CRTC */
void arn_CRTC_DoLine(core_crocods_t *core)
{
    /* to be confirmed; ma works during vadjust */
    /* increment raster counter */
    core->RasterCounter = (unsigned char)((core->RasterCounter + 1) & 0x01f);

    CRTC_DoVerticalSyncCounter(core);

    /* are we in vertical adjust ? */
    if (core->CRTC_Flags & CRTC_VADJ_FLAG) {
        core->VertAdjustCount = (unsigned char)((core->VertAdjustCount + 1) & 0x01f);

        /* vertical adjust matches counter? */
        if (core->VertAdjustCount == core->RegsCRTC[5]) {
            CRTC_ClearFlag(CRTC_VADJ_FLAG);

            CRTC_RestartFrame(core);
        }
    }

    if (core->CRTC_Flags & CRTC_MR_FLAG) {
        CRTC_ClearFlag(CRTC_MR_FLAG);

        core->RasterCounter = 0;

        /* this will trigger once at vtot */
        if (core->CRTC_Flags & CRTC_VTOT_FLAG) {
            CRTC_ClearFlag(CRTC_VTOT_FLAG);

            /* toggle frame; here or after vadj? */
            core->Frame ^= 0x01;

            /* is it active? i.e. VertAdjust!=0 */
            if (core->RegsCRTC[5] != 0) {
                /* yes */
                core->VertAdjustCount = 0;
                CRTC_SetFlag(CRTC_VADJ_FLAG);

                /* confirmed: on type 0, line counter will increment when entering vertical adjust, but not count furthur.
                 * i.e. if R5!=0 and R7=VTOT then vertical sync will trigger */
                /* increment once going into vertical adjust */
                core->LineCounter = (unsigned char)((core->LineCounter + 1) & 0x07f);
            } else {
                /* restart frame */

                CRTC_RestartFrame(core);
            }
        } else {
            /* confirmed: on type 0, line counter will increment when entering vertical adjust, but not count furthur.
             * i.e. if R5!=0 and R7=VTOT then vertical sync will trigger */
            /* do not increment during vertical adjust */
            if (!(core->CRTC_Flags & CRTC_VADJ_FLAG)) {
                core->LineCounter = (unsigned char)((core->LineCounter + 1) & 0x07f);
            }
        }
    }

    /* transfer store value */

    core->MA = core->MAStore;

    if ((core->RegsCRTC[8] & 1) != 0)
        CRTC_SetFlag(CRTC_INTERLACE_ACTIVE);
    else
        CRTC_ClearFlag(CRTC_INTERLACE_ACTIVE);

    CRTC_MaxRasterMatch(core);

    /* do last to capture line counter increment in R5 and frame restart */
    CRTC_DoLineChecks(core);
}

int CRTC_GetRAOutput(core_crocods_t *core)
{
    if ((core->CRTC_Flags & CRTC_INTERLACE_ACTIVE) != 0)
        return (core->RasterCounter << 1) | core->Frame;
    if (core->CRTC_Flags & CRTC_VADJ_FLAG)
        return core->VertAdjustCount;
    return core->RasterCounter;
}

int CRTC_GetHorizontalSyncWidth(core_crocods_t *core)
{
    /* confirmed: a programmed hsync of 0 generates no hsync */
//    return core->RegsCRTC[3] & 0x0f;

    int HorizontalSyncWidth =  core->RegsCRTC[3] & 0x0f;
    if (HorizontalSyncWidth != 0)
        return HorizontalSyncWidth;
    return 16;
}

int CRTC_GetVerticalSyncWidth(core_crocods_t *core)
{
    /* confirmed: a programmed vsync width of 0, results in an actual width of 16 */
    /* 16 can happen when counter overflows */
    int VerticalSyncWidth = (core->RegsCRTC[3] >> 4) & 0x0f;
    if (VerticalSyncWidth != 0)
        return VerticalSyncWidth;
    return 16;
}

/*---------------------------------------------------------------------------*/

u8 arn_ReadCRTC(core_crocods_t *core)
{
    int CRTC_RegIndex = (core->CRTC_Reg & 0x01f);

    /* unreadable registers return 0 */
    return (core->RegsCRTC[CRTC_RegIndex] & HD6845S_ReadMaskTable[CRTC_RegIndex]);
}

/*---------------------------------------------------------------------------*/

// Gate array

void GateArray_DoDispEnable(core_crocods_t *core, BOOL bState)
{
    if (bState)
        core->BlankingOutput &= ~DISPTMG_ACTIVE;
    else
        core->BlankingOutput |= DISPTMG_ACTIVE;
}

/* chances are the crtc hsync could happen in the middle */
void arn_GateArray_Cycle(core_crocods_t *core)
{
    /* Gate-array outputs black for the duration of the hsync -> Horizontal blanking.
     * This black overrides border and graphics. The HSYNC to the monitor is 2us later and lasts for 4us (or shorter if HSYNC is shorter). */

    /* Gate-array outputs black for 26 hsyncs for vblank -> Vertical blanking.
    * It is triggered by the start of vsync.
    * 2 HSYNC later sync to monitor is triggered and lasts a max of 4 hsync */

    if (core->BlankingOutput & HBLANK_ACTIVE) {
        /* CHECK: Mode changed or not if hsync is too short? */
        switch (core->nHBlankCycle) {
            case 0:
                break;
            case 1:
                if (core->CRTCSyncInputs & HSYNC_INPUT)
                    Monitor_DoHsyncStart(core);
		break;
            case 5:
                /* end monitor hsync but continue blanking */
                Monitor_DoHsyncEnd(core);
		break;
        } // switch
        core->nHBlankCycle++;
    }
} // GateArray_Cycle

void GateArray_UpdateHsync(core_crocods_t *core, BOOL bState)       // Arnold only
{
    if (bState) {
        core->CRTCSyncInputs |= HSYNC_INPUT;
        core->BlankingOutput |= HBLANK_ACTIVE;

        core->nHBlankCycle = 0;
    } else {
        core->CRTCSyncInputs &= ~HSYNC_INPUT;
        core->BlankingOutput &= ~HBLANK_ACTIVE;

        Monitor_DoHsyncEnd(core);

        /* increment interrupt line count */
        core->CntHSync++;

        /* if line == 52 then interrupt should be triggered */
        if (core->CntHSync == 52) {
            /* clear counter. */
            core->CntHSync = 0;

            SetIRQZ80(core, 1);
        }

        /* CHECK: Vblank triggered off hsync end or start? */
        if (core->BlankingOutput & VBLANK_ACTIVE) {
            /* CHECK: VSYNC to monitor switched off at next HSYNC? */

            core->nVBlankCycle++;
            if (core->nVBlankCycle == 2) {
                /* has interrupt line counter overflowed? */
                if (core->CntHSync >= 32) {
                    /* following might not be required, because it is probably */
                    /* set by the code above */

                    SetIRQZ80(core, 1);
                }

                /* reset interrupt line count */
                core->CntHSync = 0;

                /* TODO: Check */
                if (core->BlankingOutput & VSYNC_INPUT) {
                    Monitor_DoVsyncStart(core);
                }
            } else if (core->nVBlankCycle == 6) {
                Monitor_DoVsyncEnd(core);
            } else if (core->nVBlankCycle == 26) {
                core->BlankingOutput &= ~VBLANK_ACTIVE;
            }
        }
    }
} // GateArray_UpdateHsync

void GateArray_UpdateVsync(core_crocods_t *core, BOOL bState)
{
    BOOL bCurrentState = ((core->CRTCSyncInputs & VSYNC_INPUT) != 0);

    if (bState == bCurrentState) return;

    if (bState) {
        core->CRTCSyncInputs |= VSYNC_INPUT;
        core->BlankingOutput |= VBLANK_ACTIVE;
        core->nVBlankCycle = 0;
    } else {
        core->CRTCSyncInputs &= ~VSYNC_INPUT;
        /* CHECK, immediate or at hsync */
        Monitor_DoVsyncEnd(core);
    }
}

void arn_ResetVGA(core_crocods_t *core)
{
    core->TabCoul[16] = 20; // SOFT 158 and Arnold 5A docs says that only border is black
    core->UpdateInk = 1; // Update ink if nec

    core->CRTCSyncInputs = 0;
    core->BlankingOutput = 0;
    core->nHBlankCycle = 0;
    core->nVBlankCycle = 0;
}

// Render

// Bordure: 32 a gauche, 32 a droite
// Border: 36 en haut, 36 en bas

// Draw word (used by the new Arnold CRTC core)

void TraceWord8B512(core_crocods_t *core, int x0, int y, int Adr)
{
    // if (y>yMax) yMax=y;
    // if (y<yMin) yMin=y;

//    y -= core->y0;

    if (core->resize != 4) {  // auto
        y -= (max( (35 - core->RegsCRTC[7]) << 3, 0) + 8);
    }

    if ((y < 0) || (y >= TAILLE_Y_LOW)) {
        return;
    }
    if (x0 < 0) {
        return;
    }

    if ((!core->hack_tabcoul) && (core->UpdateInk == 1)) { // It's would be beter to put before each lines
        CalcPoints(core);
    }

    core->crtc_updated = 1;

    u16 *p;

    p = (u16 *)core->MemBitmap;
    p += (y * core->MemBitmap_width);

    if (core->lastMode != 2) {
        if (core->resize == 1) {  // auto
            if (x0 < core->XStart / 2) {
                return;
            } else {
                x0 = x0 - core->XStart / 2;
            }
        } else if (core->resize == 2) { // 320
            if (x0 < 8 / 2) {
                return;
            } else {
                x0 = x0 - 8 / 2;
            }
        }
        p += x0 * 8;

        if ((core->BlankingOutput & DISPTMG_ACTIVE)) {
            int x;

            for (x = 0; x < 8; x++) {
                *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                p++;
            }
        } else {
            u16 *tab = &(core->TabPoints[ core->lastMode ][ core->MemCPC[ Adr] ][0]);
            memcpy(p, tab, 4 * sizeof(u16));

            tab = &(core->TabPoints[ core->lastMode ][ core->MemCPC[ Adr + 1] ][0]);
            memcpy(p + 4, tab, 4 * sizeof(u16));
        }
    } else {
        p += (y * core->MemBitmap_width);
        p += x0 * 16;

        if ((core->BlankingOutput & DISPTMG_ACTIVE)) {
            int x;

            for (x = 0; x < 16; x++) {
                *p = core->BG_PALETTE[core->TabCoul[ 16 ]];
                p++;
            }
        } else {
            int x;

            for (x = 0; x < 2; x++) {
                u8 car = core->MemCPC[ Adr ];
                int i;

                for (i = 0; i < 8; i++) {
                    *(p + 7 - i) = core->BG_PALETTE[core->TabCoul[car & 1]];
                    car = (car >> 1);
                }
                p += 8;
                Adr++;
            }
        }
    }
} /* TraceWord8B512 */

u16 arn_cpu_doFrame(core_crocods_t *core)
{
    long TimeOut = 0;
    int byCycle = 0;
    long tz80 = 0;

    // (FREQUENCY_MHZ 4.0 * FRAME_PERIOD_MS 20.0 * 1000 / 4)
    while (byCycle  < 19968) {
        tz80 -= getTicks(); // TODO("replace this function")

        u16 cycle = ExecInstZ80(core);
        int i;

        for (i = 0; i < cycle; i++) {
            if (CRTC_DoCycles != NULL) {
                CRTC_DoCycles(core, 1);
            }
            if (GateArray_Cycle != NULL) {
                GateArray_Cycle(core);
            }
        }

	// Fais tourner le CPU tant CptInstr < CYCLELIGNE
        byCycle += cycle;
        tz80 += getTicks();

        TimeOut += core->RegsCRTC[ 0 ] + 1;

        cap32_endofline(core);
    }

    // On 50Hz monitor frame (64 ms per line * 312 lines)
    byCycle -= 19968; 

    return TimeOut;
}
