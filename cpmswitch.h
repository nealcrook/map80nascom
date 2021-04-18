/*
 header file for cpmswitch.c
 
 */
 
extern int cpmswitchstate;   // set to 1 if cpm mode set
                         // vfc boot rom in at 0 to start
                        // Nascom ROM and VWRAM not enabled
                       //
                       
 extern void setcpmswitch(int state); // set the mode for cpm switch 
 // 0 - NASSYS
 // 1 - cpm - mapvfc link 4 is 
  