#include  "platform.h"
#include  "ppi.h"
#include  "crtc.h"
#include  "upd.h"
#include  "vga.h"
#include  "gestport.h"

u8 ReadPort(core_crocods_t *core, u16 port)
{
    if (port == 0xfefe) {
        return 0xaa;        // CrocoDS Rompack
    }

    if (!(port & 0x0480) ) {
        return(ReadUPD(core, port) );
    }

    if (!(port & 0x04000) ) {
        u8 index = (port & 0x0300) >> 8;

        switch (index) {
            case 2:
                return 0x0ff;   // No Status Register CRTC
                break;

            case 3:
                return ReadCRTC(core);
                break;

            default:
                printf("index: %d\n", index);
                break;
        }
    }

    if (!(port & 0x0800) ) {
        u16 index = (port & 0x0300) >> 8;
        if (index == 3) {
            printf("index 3\n");
            //            Data = PPI_ReadControl();
            return(ReadPPI(core, port) );
        } else {
            //            Data = PPI_ReadPort(Index);
            return(ReadPPI(core, port) );
        }
    }

    // Kempston Mouse http://www.cpcwiki.eu/index.php/Kempston_Mouse - test4.dsk in Arnold "acid" test and with Carrier.DSK

    if (port == 0xFBEE) {   // I/O port for X position is decoded as xxxx x0x1 xxx0 xxx0
        return core->kempstonMouseX;
    }
    if (port == 0xFBEF) {   // I/O port for Y position is decoded as xxxx x0x1 xxx0 xxx1
        return core->kempstonMouseY;
    }
    if (port == 0xFAEF) {   // I/O port for Buttons is decoded as    xxxx x0x0 xxx0 xxxx
        u8 button = core->kempstonMouseButton;
        core->kempstonMouseButton = 0;
        return button;
    }

    printf("Read P%04X\n", port);

    return(0xFF);
}

void WritePort(core_crocods_t *core, u16 port, u8 val)
{
//    port=0; // A effacer

// http://cpcwiki.eu/index.php/Default_I/O_Port_Summary
// http://cpcwiki.eu/index.php/I/O_Port_Summary
// http://www.cpcwiki.eu/index.php/M4_Board <-- for the M4

    if ( (port & 0xC000) == 0x04000) {  // %01xxxxxx xxxxxxxx
        WriteVGA(core, port, val);
        return;
    }

    if (!(port & 0x4000) ) {
        // strcat(chaine, " CRC");

        u8 index;

        index = (port >> 8) & 0x03;

        switch (index) {
            case 0:     // 0xBC00
                RegisterSelectCRTC(core, val);
                break;

            case 1:     // 0xBD00
                WriteCRTC(core, val);
                break;

            default:
                break;
        }

        return;
    }

    if (!(port & 0x2000) ) {            // ROM
        WriteROM(core, val);
        return;
    }

    if (!(port & 0x1000) ) {            // PRN
        core->printer_port = val ^ 0x80; // invert bit 7
        if (!(core->printer_port & 0x80)) { // only grab data bytes; ignore the strobe signal
            printf("%c", core->printer_port); // Test printer
        }

        return;
    }

    if (!(port & 0x0800) ) {            // PPI
//
//        unsigned int            Index;
//
//        Index = (port & 0x0300) >> 8;
//
//        if (Index == 3)
//        {
//            WritePPIControl(core, );
//        }

        WritePPI(core, port, val);
        return;
    }

    if (!(port & 0x0480) ) {
        WriteUPD(core, port, val);
        return;
    }

    if (port == 0xF8FF) {     // Peripheral Soft Reset -> Do nothing
        return;
    }

    printf("Write P%04X %d\n", port, val);
}
