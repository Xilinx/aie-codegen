#include <stdio.h>
#include <stdarg.h>
#include "xaie_log.h"

void XAie_Log(FILE *Fd, const char *prefix, const char *func, u32 line,
	const char *Format, ...)
{
	va_list ArgPtr;
	va_start(ArgPtr, Format);
	if (fprintf(Fd, "%s %s():%u: ", prefix, func, line) == -1) {
		va_end(ArgPtr);
		return;
	}
	if (vfprintf(Fd, Format, ArgPtr) == -1) {
		va_end(ArgPtr);
		return;
	}
	va_end(ArgPtr);
}
