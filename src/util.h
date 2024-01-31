#ifndef util_h
#define util_h

#include <stdio.h>

#define len(a) (sizeof(a) / sizeof(a[0]))

long  GetFileSize(FILE *file);
char *ReadFile(FILE *file);

void Print(char *, ...);
void BeginIndent();
void EndIndent();

#endif
