#include "monitor.h"
#include "z80.h"

#include "platform.h"

#define MONITOR_HORIZONTAL_RATE 64
#define MONITOR_VERTICAL_RATE   312     /* change to cycles */
#define MONITOR_WIDTH_CHARS     52

#define MONITOR_HTRACE_COUNT    MONITOR_HORIZONTAL_RATE - MONITOR_WIDTH_CHARS
#define MONITOR_HEIGHT_LINES    MONITOR_VERTICAL_RATE - 24
#define MONITOR_VTRACE_COUNT    24 * MONITOR_HORIZONTAL_RATE

int CRTC_GetRAOutput(core_crocods_t *core);

void Monitor_Reset(core_crocods_t *core)
{
    core->MonitorVCount = 0;
    core->MonitorHCount = 0;
    core->MonitorSyncInputs = 0;
    core->MonitorHorizontalCount = 0;
    core->MonitorScanLineCount = 0;
    core->MonitorVTraceActive = FALSE;
    core->MonitorVTraceCount = 0;
    core->MonitorHTraceActive = FALSE;
    core->MonitorHTraceCount = 0;
}

static void RenderFunction(core_crocods_t *core)
{
    int x       = core->MonitorHCount - 1; // from 0 to 52
    int y       = core->MonitorScanLineCount;
    int LocalMA = core->MA << 1;
    int Adr     = ((LocalMA & 0x06000) << 1) | ((CRTC_GetRAOutput(core) & 0x07) << 11) | (LocalMA & 0x07ff);

    TraceWord8B512(core, x, y, Adr);
}

void Monitor_DoVsyncStart(core_crocods_t *core)
{
    /* indicate vsync input active */
    if ((core->MonitorSyncInputs & VSYNC_INPUT) == 0) {
        core->MonitorSyncInputs |= VSYNC_INPUT;

        /* hard sync */
        core->MonitorVCount = MONITOR_HEIGHT_LINES;
        core->MonitorVTraceActive = TRUE;
        core->MonitorVTraceCount = MONITOR_VTRACE_COUNT;
    }
}


/* ---------------------------------------------------------------------------------------------------------- */

/* called when vsync input ends */
void Monitor_DoVsyncEnd(core_crocods_t *core)
{
    /* indicate vsync input inactive */
    if ((core->MonitorSyncInputs & VSYNC_INPUT) != 0)
        core->MonitorSyncInputs &= ~VSYNC_INPUT;
}

/* ---------------------------------------------------------------------------------------------------------- */

/* called when hsync inputs starts */
void Monitor_DoHsyncStart(core_crocods_t *core)
{
    /* only start if not already started */
    if ((core->MonitorSyncInputs & HSYNC_INPUT) == 0) {
        /* indicate hsync is active */
        core->MonitorSyncInputs |= HSYNC_INPUT;

        /* hard sync */
        core->MonitorHCount = MONITOR_WIDTH_CHARS;
        core->MonitorHTraceActive = TRUE;
        core->MonitorHTraceCount = MONITOR_HTRACE_COUNT;
    }

    /* to sync, we need to move the htrace start backwards and forwards */

    /* htrace could be speeded up or slowed down which would then allow the input to sync */
    /* we could also start the sequence faster or slower.. */
}

/* ---------------------------------------------------------------------------------------------------------- */

/* called when hsync input ends */
void Monitor_DoHsyncEnd(core_crocods_t *core)
{
    /* only end if not already ended */
    if ((core->MonitorSyncInputs & HSYNC_INPUT) != 0) {
        /* indicate hsync is inactive */
        core->MonitorSyncInputs &= ~HSYNC_INPUT;
    }

}

static void Monitor_Cycle(core_crocods_t *core)
{
    /* horizontal trace and vertical trace operate at the same time */
    /* TODO: Change vertical trace to be cycles and not lines */
    /* monitor does go black if hsync goes missing */

    if (core->MonitorVTraceActive) {
        core->MonitorVTraceCount--;
        if (core->MonitorVTraceCount == 0) {
            core->MonitorVCount = 0;
            core->MonitorVTraceActive = FALSE;
            core->MonitorScanLineCount = -1;
        }
    }

    {
        if (core->MonitorHTraceActive) {
            core->MonitorHTraceCount--;
            if (core->MonitorHTraceCount == 0) {
                core->MonitorHTraceActive = FALSE;

                /* go to next line */
                if (core->MonitorVCount == MONITOR_HEIGHT_LINES) {
                    /* draw position */
                    core->MonitorVTraceActive = TRUE;
                    core->MonitorVTraceCount = MONITOR_VTRACE_COUNT;
                }

                core->MonitorVCount++;
                core->MonitorHCount = 0;
                core->MonitorScanLineCount++;
            }
        } else {
            core->MonitorHorizontalCount++;

            core->MonitorHCount++;
            if (core->MonitorHCount == MONITOR_WIDTH_CHARS) {
                core->MonitorHTraceActive = TRUE;
                core->MonitorHTraceCount = MONITOR_HTRACE_COUNT;
                core->MonitorHorizontalCount = 0;
            }

        }
    }

    /* when tracing the beam is blanked on a real television.
     * We park the draw position */
    if (core->MonitorVTraceActive) {
        core->MonitorScanLineCount = 311;
    }
    if (core->MonitorHTraceActive) {
        core->MonitorHorizontalCount = 63;
    }
}

void Graphics_Update(core_crocods_t *core)
{
    Monitor_Cycle(core);

    RenderFunction(core);

}
