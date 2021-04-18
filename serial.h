/* header file for the serial processing

*/


extern int tape_led;    // set if tape led is on

extern int tape_led_force; // set if tape led forced on

extern char * serial_input_filename;

extern long int tape_in_pos; // current tape position - serial input
extern long int tape_out_pos; // current output tape   - serial output
extern int serial_input_available;

extern char * serial_output_filename;
extern long int tape_out_pos;      // set to tape length of output
//extern char serial_output_error;   // somewhere to store the serial input error message



extern int readserialin(void);
extern void setserialinputfile(char * filename);
extern void resetserialinput(void);
extern int checkifanyserialinputdata(void);
extern void setserialoutfilename(char * filename);
extern void writeserialout(unsigned charvalue);

extern int getuartstatus(void);

//end of file
