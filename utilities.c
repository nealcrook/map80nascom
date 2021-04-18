 /*
    some utilities to help with stuff
*/

#include "utilities.h"
#include <string.h>

// check if the current character is a "white space"
int iswhitespace(int characterbyte)
{
    return (characterbyte == ' ') || (characterbyte == '\t') || (characterbyte == '\n') || (characterbyte == '\r')
        || (characterbyte == '\f') || (characterbyte == '\v');
}

// convert the current character to lower case
int mytolower(int characterbyte)
{
    if ((characterbyte >= 'A') && (characterbyte <= 'Z'))
        characterbyte += 'a' - 'A';
    return characterbyte;
}

// copy over string limiting it to max size
// if > maxsize then reduce by 3 bytes and add ... to end 
void copystringtobuffer(char * tostring, char * fromstring, int maxsize){

    int maxstring=maxsize-3;
    if ((strlen(fromstring) > maxsize)){
        strncpy(tostring,fromstring,maxstring);
        tostring[maxstring]='.';
        tostring[maxstring+1]='.';
        tostring[maxstring+2]='.';
        tostring[maxstring+3]=0;
    }
    else {
        strcpy(tostring,fromstring);
    }

}

// convert part of a string from hex to interger
// maynot be the fastest but it works :)
// need to pass the address of the string pointer
unsigned int hextoint(char **value)
{

  struct CHexMap
  {
    char chr;
    int value;
  };
  const int HexMapL = 22;
  struct CHexMap HexMap[22] =
  {
    {'0', 0}, {'1', 1},
    {'2', 2}, {'3', 3},
    {'4', 4}, {'5', 5},
    {'6', 6}, {'7', 7},
    {'8', 8}, {'9', 9},
    {'A', 10}, {'B', 11},
    {'C', 12}, {'D', 13},
    {'E', 14}, {'F', 15},
    {'a', 10}, {'b', 11},
    {'c', 12}, {'d', 13},
    {'e', 14}, {'f', 15}
  };

  char *s = *value;

  unsigned int result = 0;

//   if (*s == '0' && *(s + 1) == 'X') s += 2;

  while (*s != '\0')
  {
    int found = 0;
    for (int i = 0; i < HexMapL; i++)
    {
      if (*s == HexMap[i].chr)
      {
        result <<= 4;
        result |= HexMap[i].value;
        found = 1;
        break;
      }
    }
    if (!found) break;
    s++;
  }
  // return the next character position
  *value=s;
  return result;
}

