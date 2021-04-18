/*  WD2797 floppy controller based on the MAP80 VFC card.

   data definition for the controller and the drives

    David Allday Janusry 2021

   data sheet http://www.datasheetcatalog.com/datasheets_pdf/W/D/2/7/WD2797.shtml
       and       https://www.digchip.com/datasheets/parts/datasheet/922/WD2797-pdf.php

*/

/*
 * MAP80 VFC card
 *
 * floppy controller read
 *
 * P 0xE0 read status
 * P 0xE1 read track register
 * P 0xE2 read Sector register
 * P 0xE3 read Data Register
 * P 0xE4 bit 0 - intrq
 *        bit 1 – inverted ready
 *        bit 2 – DRQ Port
 * P 0xE5 -  as 0xE4
 *
 * floppy controller write
 *
 * P 0xE0 command write
 * P 0xE1 track register
 * P 0xE2 Sector register
 * P 0xE3 Data Register
 * P 0xE4 Drive Port
 * P 0xE5 Drive Port - as 0xE4
 *
 *
 * Keyboard Read only - not used
 *
 * P 0xE6 - read keyboard
 * P 0xE7 - as 0xE6
 *
 * Alarm port read or write - not used
 *
 * P 0xE8 - set alarm pulse
 * P 0xE9 - as 0xE8
 *
 * Display 6845 chip
 *
 * P 0xEA - set the 6845 register Write only
 * P 0xEB - set/read the 6845 register Read or Write
 *
 * P 0xEC - write ONLY - Controls ROM and Video memory
 *
 *        bit 7 – )
 *        bit 6 – ) 4k boundary
 *        bit 5 – )
 *        bit 4 – )
 *        bit 3 – Character Generator EPROM1(0) or EPROM2(1)
 *        bit 2 – Upper(0)/inverse(1) video
 *        bit 1 – VSOFT Enable - depends on link 4
 *        bit 0 – Video Ram disable (0) or enable (1)
 *
 *    When VSOFT is enable RAMDISABLE is active for reads
 *    When Video Ram is enabled  RAMDISABLE is active for reads and writes
 *        In this version the RAMDISABLE is enabled for both read and write
 *
 *
*/


//check if we have defined all the entries else where
// not really required as no varibles defined.
#ifndef FLOPPY_DEFINED_H
#define FLOPPY_DEFINED_H

// number of drives we will have
#define NUMBEROFDRIVES 4
// maximum sector size
//#define FLOPPYMAXSECTORSIZE 1024
#define FLOPPYMAXSECTORSIZE 1024*8

// commands to the WD2797 controller
// the first 4 bits give the command
// type 1 commands
#define floppyCmdRestore             (0x00)
#define floppyCmdSeek                (0x10)
#define floppyCmdStep                (0x20)
#define floppyCmdStepTrackUpdate     (0x30)
#define floppyCmdStepin              (0x40)
#define floppyCmdStepinTrackUpdate   (0x50)
#define floppyCmdStepout             (0x60)
#define floppyCmdStepOutTrackUpdate  (0x70)
// type 2 commands
#define floppyCmdReadSector          (0x80)
#define floppyCmdReadSectorMulti     (0x90)
#define floppyCmdWriteSector         (0xA0)
#define floppyCmdWriteSectorMulti    (0xB0)
// type 3 commands
#define floppyCmdReadAddress         (0xC0)
#define floppyCmdReadTrack           (0xE0)
#define floppyCmdWriteTrack          (0xF0)
// type 4 command
#define floppyCmdForceInterupt       (0xD0)
// The rest of the bits have other meanings

// not sure if I'll bother with this lot unless I need to :)
// type 1 commands
// bit 3 0=unload 1=load head at beginning
#define floppyCmdLoadHead            (0x8)
// bit 2 0= No verify 1=verify on destination track
#define floppyCmdVerify              (0x4)
// bit 1 and 0 set motor rate


// type 2 commands
//  on single side controllers bit 3 Side compare 0 or 1
// bit 3 L = length of sector based on 2 bits in the ID field ?
#define floppyCmdLength            (0x08)
// bit 2 50ms delay
//  on single sided controllers - bit 1 side compare 1 = enable 0 = disable
// bit 1 select side - 0 or 1
#define floppyCmdSelectSide        (0x02)
// bit 0 Data address mark ( write only ) 0=FB(DAM) 1=F8(deleted DAM)

// type 3 commands
// bit 2 50ms delay
// bit 1 select side - 0 or 1
// see above
// #define floppyCmdSelectSide          (0x02)

// type 4 command
// bits 0-3 set interrupt condition flag
// bit 3 = 1 immediate interupt requires a reset
// bit 2 = 1 index pulse
// bit 1 = 1 Ready to Not Ready Transaction
// bit 0 = 1 Not Ready to Ready Transaction
// all 0's = terminate with no interupt
//

// the values to set the status register to during command operations
#define FLOPPYSTATUSNOTREADY       (0x80)
#define FLOPPYSTATUSWRITEPROTECT   (0x40)	// either for Type1 or write command
#define FLOPPYSTATUSHEADLOADED     (0x20)       // type 1 only
#define FLOPPYSTATUSREADTYPE       (0x20)       // for read read sector command only
#define FLOPPYSTATUSWRITEFAULT     (0x20)       // for write commands only
#define FLOPPYSTATUSSEEKERROR      (0x10)       // for type 1 commands
#define FLOPPYSTATUSRECORDNOTFOUND (0x10)       // for type 2 and 3 commands
#define FLOPPYSTATUSCRCERROR       (0x08)       // CRC error some where in the data
#define FLOPPYSTATUSTRACKZERO      (0x04)       // at Track 0 only for type 1 commands
#define FLOPPYSTATUSLOSTDATA       (0x04)       // lost data for type 2 and 3
#define FLOPPYSTATUSINDEXFOUND     (0x02)       // Index marker found for type 1 commands only
#define FLOPPYSTATUSDATAREQUEST    (0x02)       // Data Request - should have same value as floppyDataRequest ( 0 or 1 )
#define FLOPPYSTATUSBUSY           (0x01)       // Command under execution

// number of calls to the status register to delay asking for or providing bytes
// adds small delay (value -1 ) to the write/read interchange 
// value of 1 means no delay - don't set below 1
#define floppyDelayByteRequestBy (1)
// if set > 1 then triggers busy status for x-1 calls to get status
// seems some programs expect a small delay 
// value of 1 means no delay - don't set below 1 
// CPM boot process needs it to be set to 2 or above else it won't boot.
#define floppyDelayResetBusy (2)


// define a structure to hold the details,of each of the drives
typedef struct {
                                    // these define the format of the image to be processed
    unsigned int numberOfHeads;		// number of heads / sides for the floppy
    unsigned int numberOfTracks;	// number of tracks
    unsigned int numberOfSectors;       // number of Sectors
    unsigned int firstSectorNumber;     // the first sector number of a track
    unsigned int interleaved;           // if floppy image has interleaved tracks
    unsigned int sidesswapped;         // if side 0 and 1 are swapped in the image
    unsigned int reverseside0;         // side 0 tracks are in reverse order in the image
    unsigned int reverseside1;         // side 1 tracks are in reverse order in the image
    unsigned int writeProtect;         // write protection if 1
    unsigned int sizeOfSector;        // the size of each sector

    int track;                       // current track for this drive - signed so I can check for -1 in step
                                     // sectors are not needed as logical not head movement
    char * fileNamePointer;			// pointer to the name of the file supporting the disc
    // TODO - when floppy changed - release name space
} FLOPPYDRIVEINFO;


// defined the externall visible functions - note extern not strictly necessary for functions.
extern void displayfloppydetails();
extern void resetalldrives();
extern int floppyMountDisk(unsigned int drive,char * filename); // call to mount a floppy disk image

// global variables to allow for debug etc.,
extern int vfcfloppydebug;
extern int vfcfloppydisplaysectors;

void outPortFloppy(unsigned int port, unsigned int value);
int inPortFloppy(unsigned int port);


#endif

// end of file
