/*
 * routine to switch from NASSYS mode to CPM mode 
 * 
 */

#include "simz80.h"
#include "map80ram.h"


int cpmswitchstate=0;          // set to 1 if cpm mode set
                              // vfc boot rom in at 0 to start
                             // Nascom ROM and VWRAM not enabled
                            //

void setcpmswitch(int state){ // set the mode for cpm switch 
    // 0 - NASSYS
    // 1 - cpm - map80vfc link 4 is set to bring in the rom at start
    
    if (state==0){
         // fix DA - set locked memory pages
        // now fix for monitor and Nascom 2 working ram
        // but not if in cpm mode (-b option)
        // in effect setting the RAMDISABLE flag
        // location for 0 to 1 linked to addresses in special area
        // this is for the monitor and video/working ram
        // assumes 2k pages so allow for monitor
        ramlocktable[0] = 1;
        // assumes 2k pages so allow for video and work ram
        ramlocktable[1] = 1;
        // lock the nascom rom area/
        // using loadNASformat which uses the memory directly so not an issue
        // simz80 uses putbyte to store stuff and ramromtable stops it.
        // assumes 2k pages so allow for monitor
        ramromtable[0] = 1;
        // point those ram locations in rampagetable to Nascom MonVWram 4k area of ram for NASCOM etc.,
        for (int c=0; c< 2 ; ++c) {
            rampagetable[c]=NascomMonVWram+(c<<RAMPAGESHIFTBITS);
            // printf(" rampagetable %d address %p \n",c,rampagetable[c]);
        }
    }
    // tell the rest if the code we are in cpm mode
    cpmswitchstate=state;
    
}
    