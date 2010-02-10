#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intern.h"
#include "unitlib.h"

#define sizeofarray(ar) (sizeof((ar))/sizeof((ar)[0]))

bool _ul_debugging = false;
void _ul_debug(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

const char *_ul_symbols[] = {
	"m", "kg", "s", "A", "K", "mol", "C", "L",
};

// The last error message
static char errmsg[1024];
void _ul_set_error(const char *func, int line, const char *fmt, ...)
{
	snprintf(errmsg, 1024, "[%s:%d] ", func, line);
	size_t len = strlen(errmsg);

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(errmsg + len, 1024 - len, fmt, ap);
	va_end(ap);
}

void ul_debugging(bool flag)
{
	_ul_debugging = flag;
}

const char *ul_error(void)
{
	return errmsg;
}

bool ul_init(void)
{
	assert(sizeofarray(_ul_symbols) == NUM_BASE_UNITS);

	debug("Initializing base rules");
	_ul_init_rules();

	return true;
}

void ul_quit(void)
{
	_ul_free_rules();
}

