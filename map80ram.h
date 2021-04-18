/*
 * definitions for MAP80RAM
 */


void map80RamInitialise();
void map80Ram(unsigned char value);

// external variables
// rams areas for map80 card
extern  int ramromtable[];

extern int ramlocktable[];

extern BYTE *ramdefaultpagetable[RAMPAGETABLESIZE];

extern BYTE NascomMonVWram[];
extern BYTE dummyram[];

extern BYTE vfcrom[];
extern BYTE vfcdisplayram[];
extern BYTE statusdisplayram[];

extern int rampagedebug;    // set to 1 to display rampage details

// end of file

