 /*
    some utilities to help with stuff
*/


// check if the current character is a "white space"
extern int iswhitespace(int characterbyte);
// convert the current character to lower case
extern int mytolower(int characterbyte);

// copy a string to another but if too long add ... to end 
extern void copystringtobuffer(char * tostring, char * fromstring, int maxsize);

// hex string to int 
extern unsigned int hextoint(char **value);

// end of file


