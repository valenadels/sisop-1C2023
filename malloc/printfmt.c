#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "printfmt.h"

// this size can be updated
// for larger outputs
#define MAX_SIZE 2048

static char buf[MAX_SIZE];

// prints formatted data to stdout
// without using `printf(3)` function
//
// it prevents indirect calls to `malloc(3)`
//
int
printfmt(char *format, ...)
{
	int r;
	va_list ap;

	memset(buf, 0, MAX_SIZE);

	va_start(ap, format);
	vsnprintf(buf, MAX_SIZE, format, ap);
	va_end(ap);

	r = write(STDOUT_FILENO, buf, MAX_SIZE);

	return r;
}
