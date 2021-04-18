/* code to handle serial input and output 

*/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "map80nascom.h"
#include "serial.h"

// global variables 
int tape_led = 0;       // set if tape led is on
int tape_led_force = 0; // set if tape led forced on

char * serial_input_filename=NULL;
int serial_input_available = 0;  // set to 1 if there is a file and not at end of file
                                       // used when checking the UART for input 
long int tape_in_pos = 0;       // set to tape position as we read characters 
char serial_input_error[100];   // somewhere to store the serial input error message
int  serial_input_errornumber=0;

char * serial_output_filename=NULL;
long int tape_out_pos = 0;      // set to tape length of output
char serial_output_error[100];   // somewhere to store the serial input error message

int serial_only1file=0;      // set to 1 if the input file and output file are the same file.

// local 
static FILE *serial_out;
static FILE *serial_in;



// open and read the next character from the serial file
int readserialin(void){
    
    char ch=0;
    int result=0;
    
    if (tape_led){
        if (serial_input_filename!=NULL){
            serial_in = fopen(serial_input_filename, "rb");
            if (serial_in){
                // printf("Serial file %s open\n",serial_input_filename);
                // position at current point 
                result=fseek(serial_in, tape_in_pos, SEEK_SET );
                if (result ==0 ) {
                    ch = fgetc(serial_in);
                    serial_input_available = !feof(serial_in);
                    tape_in_pos = ftell(serial_in);
                }
                else{
                    perror(serial_input_filename);
                    printf("seek failed\n");
                }

                fclose(serial_in);
            }
            else{
                serial_input_error[0]=0;
                //perror(serial_input_filename);
            }
        }
    }
    // check if we only have 1 file 
    if (serial_only1file){
        tape_out_pos=tape_in_pos;
    }


    // printf("Uart read %2.2X, serial ready %2.2X, tape led %2.2X, \n",ch,serial_input_available,tape_led);
    return ch;
}

// check if there is any serial data to process
int checkifanyserialinputdata(void){
    
    int returnval=0;
    if (serial_input_filename!=NULL){  // check if we have a file name set
        serial_in = fopen(serial_input_filename, "rb");
        if (serial_in){
            // position at end of file
            int result=fseek(serial_in, 0, SEEK_END );
            if (result ==0 ) {
                // is there any data there
                if (tape_in_pos < ftell(serial_in)){
                    returnval=1;
                }
                else{
                    returnval=0;
                }
                //printf("Uart checkserial eof  %2.2X\n",returnval);
            }
            else{
            }
            fclose(serial_in);
        }
        else{
            serial_input_error[0]=0;
            //perror(serial_input_filename);
        }
    }
    // set the global variable
    serial_input_available=returnval;
    //printf("Uart checkserial serial ready %2.2X, ret %2.2X, \n",serial_input_available,returnval);
    
    return returnval;
}

void setserialinputfile(char * filename){
    
    tape_in_pos=0;
    // changed to store filename and open when asked
    if (serial_input_filename!=NULL){
        // free memory already allocated
        free(serial_input_filename);
    }
    // copy in the file name to local storage
    int len = strlen( filename );
    serial_input_filename = malloc( len + 1 );
    strcpy( serial_input_filename, filename );
    // check that file exists 
    serial_in = fopen(serial_input_filename, "r");
    if (!serial_in){
        serial_input_errornumber=errno;
        serial_input_error[0]=0;
        //perror(serial_input_filename), exit(1);
    }
    else {
        serial_input_available = !feof(serial_in);
        fclose(serial_in); // will open it when needed
    }
    //printf("serial input %s -> %p\n", serial_input_filename, serial_in);
    // if any characters in the file say serial available
}

// reset the serial input file 
void resetserialinput(void){

    if (verbose){
        printf("Serial input reset to start\n");
    }
    
    tape_in_pos=0;
    // check if we only have 1 file 
    if (serial_only1file){
        tape_out_pos=tape_in_pos;
    }
    
}

// set the filename to be used for the serial output
void setserialoutfilename(char * filename){

    if (serial_output_filename!=NULL){
        // free memory already allocated
        free(serial_output_filename);
    }
    // copy in the file name to local storage
    int len = strlen( filename );
    serial_output_filename = malloc( len + 1 );
    strcpy( serial_output_filename, filename );
    // check that file exists 
    
    serial_out = fopen(serial_output_filename, "rb");
    if (!serial_out){
        // no file - it will be created when output to it
        //perror(serial_output_filename), exit(1);
        serial_output_error[0]=0;
    }
    else{
        // find out how big the file is
        int result=fseek(serial_out, 0, SEEK_END );
        if (result ==0 ) {
            tape_out_pos = ftell(serial_out);
        }
        fclose(serial_out); // will open it when needed
    }
    // printf("serial output %s -> %p\n", serial_output_filename, serial_out);

    // not working to give position - until something is written to it ?
    //tape_out_pos = ftell(serial_out);
}

// put a character into the serial out file
void writeserialout(unsigned charvalue){
    
//        fputc(value, serial_out);
//        tape_out_pos = ftell(serial_out);
//        fputc(value, serial_out);
//        tape_out_pos = ftell(serial_out);

//    printf("output to serial file %2.2X\n",charvalue);
    if (serial_output_filename!=NULL){
        serial_out = fopen(serial_output_filename, "a");
        if (serial_out){
            fputc(charvalue, serial_out);
            tape_out_pos = ftell(serial_out);
//            printf("output to serial file position %ld\n",tape_out_pos);
            fclose(serial_out);
        }
        else{
            perror(serial_input_filename);
        }
    }
    // check if we only have 1 file 
    if (serial_only1file){
        tape_in_pos=tape_out_pos;
    }

}


int getuartstatus(void){            
    int uart_status= UART_TBR_EMPTY | (checkifanyserialinputdata() & tape_led ? UART_DATA_READY : 0);
    // printf("Uart status check %2.2X, serial ready %2.2X, tape led %2.2X, \n",uart_status,serial_input_available,tape_led);
    return uart_status;
}