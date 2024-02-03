#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

static int indent;

long
GetFileSize(FILE *file)
{
	long pos, size;

	pos = ftell(file);
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, pos, SEEK_SET);

	return size;
}

char *
ReadFile(FILE *file)
{
	char *buf;
	long  size;

	size      = GetFileSize(file);
	buf       = malloc(size + 1);
	buf[size] = '\0';
	fread(buf, 1, size, file);
	fclose(file);

	return buf;
}

void
BeginIndent()
{
	indent++;
}

void
EndIndent()
{
	indent--;
}

void
Print(char *string, ...)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "%*s%s\n", indent * 2, "", string);

	va_list args;
	va_start(args, string);
	vprintf(buf, args);
	va_end(args);
}
