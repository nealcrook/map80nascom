/* utilities to handle .nas type files
  ??  calls ihex to handle the ihex format
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <stdbool.h>
#include "nasutils.h"
#include "simz80.h"
#include "map80nascom.h"
#include "utilities.h"


// load a Nascom NAS format file into memory.
// It can be any filename.
// The extention is normally .nas, but can be .nal
// A file with a .rom extention will be loaded into it's own ROM space.
//   That makes it Read only and locked in the memory space - like the EPROMS on the Nascom 2 board.
//   The limits are it must start on a 2k boundary in memory and must not be more than 8k long.
// 
// The file format should be an address then 8 bytes and an optional checksum all as ASCII 0-9 a-f
// Currently it does not like where the last line is not 8 bytes - will ignore it
// returns 0 if all okay anything else is a failure
// will load into ram space but if .nal file it will create a rom space 
// and copy it from ram space to there.

int loadNASformat(const char *filetoload)
{
    int retval=0;
    int firstaddress=-1;
    int lastaddress=-1;
    char fileext[12]="";
    int count1=0;
    int count2=0;
    int namelen=0;
    
    namelen=strlen(filetoload);
    
    if (verbose) printf("Loading %s\n", filetoload);

    // find the . in the file name
    for (count1=namelen;count1>0;count1--){
        if (filetoload[count1]=='.') {
            break;
        }
    }
    
    count1++; // allow for the .
    // check if we have any extention 
    if (count1<1){
        if (verbose) printf("\tWarning - No extention on file %s\n", filetoload);
        count1=namelen;
    }
    // check extention name not too long
    if ((namelen-count1)>10){
        if (verbose) printf("\tWarning - extention on file %s too long, max 10\n", filetoload);
        count1=namelen;
    }
    
    // copy it over converting it to lower case 
    for (count2=0;count1<namelen;count1++ , count2++){
        fileext[count2]=mytolower(filetoload[count1]);
    }
    fileext[count2]=0; // mark end
    if (verbose) printf("\tfile ext %s\n",fileext);
    
    // split it out to make it easier to read the code :)
    retval=loadNASformatinternal(filetoload,&firstaddress,&lastaddress );
    
    // printf("loaded %s from %4.4X to %4.4X \n",filetoload,firstaddress,lastaddress);
    // allocate ROM type ( .nal file ) their own space when loading.
    // Cannot just lock the area in the first 64k - as we can move that into 32k lower or uppers :)
    // and it would appear in none locked areas.
    if (retval==0){
        if (verbose) printf("\tLoaded %s into memory at address 0x%4.4X\n",filetoload,firstaddress);
        // check if it was a .nal type file
        if ( strcmp( fileext,"rom") == 0 ){
            // allocate some extra memory for it 
            int memoryused = lastaddress-firstaddress + 1;
            if (verbose) printf("\tmemory used 0x%4.4X\n",memoryused);
            // check that fist address is on a 2k boundary to calculate rampages to set
            // also if > 0x1000
            // if not could upset the nascom stuff if it tries to be rom
            if (firstaddress < 0x1000){
                if (verbose) printf("\tcannot activate as rom as below address 0x1000\n");
            }
            else if ((firstaddress % 2048) > 0 ) {
                if (verbose) printf("\tcannot activate as rom as not starting on a 2k boundary\n");
            }
            else if (memoryused > (8*1024)  ) {
                if (verbose) printf("\tcannot activate as rom as larger than 8k\n");
            }
            else {
                // the memory must be in 2k chunks
                int remainder =(memoryused % 2048);
                if (remainder > 0 ) {
                    memoryused=((memoryused/2048)+1)*2048;   // add extra 2k 
                }
                if (verbose) printf("\tmemory used now 0x%4.4X actual 0x%4.4X\n",memoryused,(unsigned int)(memoryused  * sizeof(BYTE)));
                // allocate memory for the rom
                BYTE * newmemory = malloc(memoryused  * sizeof(BYTE)); 
                if (newmemory==NULL){
                    // whoops that failed 
                    printf("\tWarning - Unable to allocate 0x%4.4X ROM space for %s\n",(unsigned int)(memoryused  * sizeof(BYTE)),filetoload);
                }
                else{
                    // memory allocated 
                    // copy the info from the virtual memory to the rom space
                    // used the slow method as the ram may not be contiguous ?
                    int copycount=0;
                    for (copycount=0;copycount<memoryused;copycount++){
                        newmemory[copycount]=RAM(firstaddress+copycount);
                        RAM(firstaddress+copycount)=0x76;   // reset it to HALT 
                    }
                    copycount--;    // backup 1
                    // printf("Last mem copied add %4.4X count %4.4X value %2.2X \n",firstaddress+copycount,copycount,newmemory[copycount]);
                    // now point at it from rampagetable
                    // which entry do we need to use
                    int rampageentry = ((firstaddress)>>RAMPAGESHIFTBITS) & RAMPAGETABLESIZEMASK;
                    // printf("ram page table entry %2.2X\n",rampageentry);
                    
                    for (int memoryusedcopy=memoryused ;memoryusedcopy>0 ;memoryusedcopy-=(RAMPAGESIZE*1024), rampageentry++ )  {
                        if (rampageentry<RAMPAGETABLESIZE){
                            
                            rampagetable[rampageentry]=newmemory;
                            ramromtable[rampageentry]=1;    // say it is rom
                            ramlocktable[rampageentry]=1;   // and active nas ram disable 
                            //printf("rampage table %2.2x set to point to %p\n",rampageentry,newmemory);
                        }
                        else{
                            // should not happen but - - - -
                            printf("\tError - rampageentry %d past top of RAMPAGETABLESIZE %d\n",rampageentry,RAMPAGETABLESIZE);
                            break;
                        }
                        newmemory+=(RAMPAGESIZE*1024);
                    }
                    if (verbose) printf("\tLoaded into ROM at address 0x%4.4X for 0x%2.2X bytes\n",firstaddress,memoryused);
                    
                }
                
            }
        }        
        
    }
    else {
        if (verbose) printf("\tWarning - problem loading %s\n",filetoload);
    }
    
    return retval;
}

// this processes the file an returns first and last address used 
// technically the lowest and highest 

int loadNASformatinternal(const char *filetoload, int *firstaddressused, int *lastaddressused )
{
    int address;
    unsigned int bytecsum;
    unsigned int bytes[8];   // 0 to 7 bytes to receive data
    int count = 0;
    int ch;
    int numberRead;
    unsigned int checksum;
    int lineNumber=0;

    int retval=0;   // defaults to all okay
    int firstaddress = 100000;
    int lastaddress = -1;

    FILE *f = fopen(filetoload, "r");

    if (!f) {
        perror(filetoload);
        return 1;
    }


    // loop through until end of file
    for (; !feof(f) ;) {
        numberRead = fscanf(f, "%x %x %x %x %x %x %x %x %x %x",
                   &address, &bytes[0], &bytes[1], &bytes[2], &bytes[3], &bytes[4], &bytes[5], &bytes[6], &bytes[7], &bytecsum );
        lineNumber++;
        if (numberRead == 10){
            // 10 entries - address and 8 bytes and checksum byte
            // this bit of code checks the checksum 
            // first add in address
            numberRead=9; // fix for next bit
            // calculate checksum
            checksum = (((address & 0xFF) + ((address & 0xFF00)>>8)) & 0xFF);
            //printf("checksum after address %4.4X is %2.2X \n",address,checksum);
            for (int byteNumber=0;byteNumber<8;byteNumber++){
                checksum = (( checksum + (bytes[byteNumber] & 0xFF)) & 0xFF) ;
            }
            if (checksum != bytecsum){
                if (verbose) printf("\t%s - checksum failure on line %d, calculated checksum"
                               " 0x%1X does not match one from file 0x%1X \n",
                               filetoload, lineNumber,checksum,bytecsum);
                retval=1; // signify error - but process rest of file
            }
            //printf("checksum %4.4X from file %2.2X \n",checksum,bytecsum);

        }
        // now load the bytes into memory 
        // TODO - what to do if not actually 8 bytes
        // at present it ignores them
        if (numberRead == 9){
            if (firstaddress > address ){
                firstaddress=address;
            }
            // 9 entries - address and 8 bytes
            for (int byteNumber=0;byteNumber<8;byteNumber++){
                RAM(address)=bytes[byteNumber];
                if (lastaddress<address){
                    lastaddress=address;
                }
                address++;
                count ++;
            }
            
        }
/*
        else {
            if (numberRead == )
            printf("%s line %d unable to identify 8 bytes of data to load\n",filetoload,lineNumber);
            retval=1; // signify error - but process rest of file
        }
*/
        // skip to end of line in the file
        do
            ch = fgetc(f);
        while (ch != -1 && ch != '\n');
        // end of file exit
        if (ch == -1){
            break;
        }
    }

    fclose(f);
    if (firstaddress>0xFFFF){
        if (verbose) printf("\tWarning: No data loaded\n");
        retval=1;
    }
    if (verbose){
        if (retval){
            printf("\tError: Problems during load\n");
        } else {
            printf("\tSuccessfully loaded %d bytes\n", count);
        }
    }
    //printf("returning %d \n",retval);
    // return the addresses used 
    *firstaddressused = firstaddress;
    *lastaddressused = lastaddress;
    return retval;
}

// loads a nas file into a specific part of memory 
// used to load the nassys3 and map80vfc rom file

int loadNASformatspecial(const char *filetoload, unsigned char *memory, int memorySize)
{
    int address;
    unsigned int bytecsum;
    unsigned int bytes[8];   // 0 to 7 bytes to receive data
    int count = 0;
    int ch;
    int numberRead;
    unsigned int checksum;
    int lineNumber=0;

    int retval=0;   // defaults to all okay

    if (verbose) printf("Loading %s\n", filetoload);

    FILE *f = fopen(filetoload, "r");

    if (!f) {
        perror(filetoload);
        return 1;
    }

    // loop through until end of file
    for (; !feof(f) ;) {
        numberRead = fscanf(f, "%x %x %x %x %x %x %x %x %x %x",
                   &address, &bytes[0], &bytes[1], &bytes[2], &bytes[3], &bytes[4], &bytes[5], &bytes[6], &bytes[7], &bytecsum );
        lineNumber++;
        if (numberRead == 10){
            // 10 entries - address and 8 bytes and checksum byte
            // this bit of code checks the checksum 
            // first add in address
            numberRead=9; // fix for next bit
            // calculate checksum
            checksum = (((address & 0xFF) + ((address & 0xFF00)>>8)) & 0xFF);
            //printf("checksum after address %4.4X is %2.2X \n",address,checksum);
            for (int byteNumber=0;byteNumber<8;byteNumber++){
                checksum = (( checksum + (bytes[byteNumber] & 0xFF)) & 0xFF) ;
            }
            if (checksum != bytecsum){
                if (verbose) printf("\t%s - checksum failure on line %d, calculated checksum"
                               " 0x%1X does not match one from file 0x%1X \n",
                               filetoload, lineNumber,checksum,bytecsum);
                retval=1; // signify error - but process rest of file
            }
            //printf("checksum %4.4X from file %2.2X \n",checksum,bytecsum);

        }
        // now load the bytes into memory 
        // TODO - what to do if not actually 8 bytes
        // at present it ignores them
        if (numberRead == 9){
            // 9 entries - address and 8 bytes
            for (int byteNumber=0;byteNumber<8;byteNumber++){
                if (address < memorySize){
                    memory[address] = bytes[byteNumber];
                    address++;
                    count ++;
                }
                else {
                    if (verbose) printf("\t%s Address 0x%1X on line %d passed end of memory size 0x%1X \n",
                                   filetoload,address,lineNumber,memorySize);
                    retval=1; // signify error - but process rest of file
                    break;
                }
            }
        }
/*
        else {
            if (numberRead == )
            printf("%s line %d unable to identify 8 bytes of data to load\n",filetoload,lineNumber);
            retval=1; // signify error - but process rest of file
        }
*/
        // skip to end of line in the file
        do
            ch = fgetc(f);
        while (ch != -1 && ch != '\n');
        // end of file exit
        if (ch == -1){
            break;
        }
    }

    fclose(f);
    if (verbose){
        if (count<1){
            printf("\tWarning: No data loaded\n");
        }
        else {
            if (retval){
                printf("\tError: Problems during load\n");
            } else {
                printf("\tSuccessfully loaded %d bytes into it's own memory\n", count);
            }
        }
    }

    return retval;

}



// end of file
