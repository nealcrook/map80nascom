/* definitions for the nasutils
  ensure it is only does the declarations once

  Contains the load a .NAS file

  David Allday January 2021

  */

#ifndef NASUTILS_H
#define NASUTILS_H 1

int loadNASformat(const char *file);
int loadNASformatspecial(const char *file,  unsigned char *memory,  int memorySize);
int loadNASformatinternal(const char *file,  int *firstaddressused, int *lastaddressused );

#endif
