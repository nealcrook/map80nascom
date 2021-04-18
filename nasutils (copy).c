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

// says verbose is defined elsewhere
//extern bool  verbose;


// load a .nas ( or .nal ) file
// it should be address then 8 bytes maybe plus checksum as ASCII
// does not like where the last line is not 8 bytes - will ignore it
// Loads NAS format file into memory area specified
// will fail if it tries to store outside that provided memory block
// used to load the monitor into the space allocated for it.
// returns 0 if all okay anything else is a failure

int loadNASformat(const char *filetoload, unsigned char *memory, int memorySize)
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
        if (verbose) printf("Error - No extention on file %s\n", filetoload);
        return 1;
    }
    // check extention name not too long
    if ((namelen-count1)>10){
        if (verbose) printf("Error - extention on file %s too long, max 10\n", filetoload);
        return 1;
    }
    // copy it over converting it to lower case 
    for (count2=0;count1<namelen;count1++ , count2++){
        fileext[count2]=mytolower(filetoload[count1]);
    }
    fileext[count2]=0; // mark end
    printf("file ext %s\n",fileext);
    
    retval=loadNASformatinternal(filetoload,memory,memorySize,&firstaddress,&lastaddress );
    
    // printf("loaded %s from %4.4X to %4.4X \n",filetoload,firstaddress,lastaddress);
    // TODO - workout how to allocate ROM type of space when loading maybe .nal files
    // do we use a separate 64k space for ROMS ?
    // Cannot just lock the area in the first 64k - as we can move that about :)
    // 
    if (retval==0){
        // check if it was a .nal type file
        if ( strcmp( fileext,"nal") == 0 ){
            // allocate some extra memory for it 
            int memoryused = lastaddress-firstaddress + 1;
            printf("memory used %4.4X\n",memoryused);
            // allocate memory for the rom
            BYTE * newmemory = malloc(memoryused  * sizeof(BYTE)); 
            if (newmemory==NULL){
                // whoops that failed 
                printf("Unable to allocate ROM space for %s\n",filetoload);
            }
            else{
                // memory allocated now point at it from rampagetable
                // which entry do we need to use
                int rampageentry = ((firstaddress)>>RAMPAGESHIFTBITS) & RAMPAGETABLESIZEMASK;
                printf("ram page table entry %2.2X\n",rampageentry);
                for ( ;memoryused>0 ;memoryused-=(RAMPAGESIZE*1024), rampageentry++ )  {
                    if (rampageentry<RAMPAGETABLESIZE){
                        
                        rampagetable[rampageentry]=newmemory;
                        ramromtable[rampageentry]=1;    // say it is rom
                        ramlocktable[rampageentry]=1;   // and active nas ram disable 
                        printf("rampoage table %2.2x set to point to %p\n",rampageentry,newmemory);
                    }
                    else{
                        // should not happen but - - - -
                        printf("Error - rampageentry %d past top of RAMPAGETABLESIZE %d\n",rampageentry,RAMPAGETABLESIZE);
                        break;
                    }
                    newmemory+=(RAMPAGESIZE*1024);
                }
                
                
                
            }
        }        
        
    }


    
    return retval;
}

// this processes the file an returns first and last address used 
// technically the lowest and highest 

int loadNASformatinternal(const char *filetoload, unsigned char *memory, int memorySize, int *firstaddressused, int *lastaddressused )
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
    int firstaddress = -1;
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
                printf("%s - checksum failure on line %d, calculated checksum"
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
            if (firstaddress < 0 ){
                firstaddress=address;
            }
            // 9 entries - address and 8 bytes
            for (int byteNumber=0;byteNumber<8;byteNumber++){
                if (address < memorySize){
                    memory[address] = bytes[byteNumber];
                    if (lastaddress<address){
                        lastaddress=address;
                    }
                    address++;
                    count ++;
                }
                else {
                    printf("%s Address 0x%1X on line %d passed end of memory size 0x%1X \n",
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
        if (firstaddress<0){
            printf("Warning: No data loaded\n");
        }
        else {
            if (retval){
                printf("Error: Problems during load\n");
            } else {
                printf("Successfully loaded %d bytes\n", count);
            }
        }
    }
    //printf("returning %d \n",retval);
    // return the addresses used 
    *firstaddressused = firstaddress;
    *lastaddressused = lastaddress;
    return retval;
}


// end of file
