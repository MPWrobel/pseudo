#include <stdlib.h>

#include "util.h"

long
filesize(FILE *file)
{
	long pos, size;

	pos = ftell(file);
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, pos, SEEK_SET);

	return size;
}

char *
readfile(FILE *file)
{
	char *buf;
	long  size;

	size = filesize(file);
	buf = malloc(size + 1);
	buf[size] = '\0';
	fread(buf, 1, size, file);
	fclose(file);

	return buf;
}
