#ifndef util_h
#define util_h

#include <stdio.h>

#define len(a) (sizeof(a) / sizeof(a[0]))

long filesize(FILE *file);
char * readfile(FILE *file);

#endif
