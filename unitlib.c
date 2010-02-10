#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intern.h"
#include "unitlib.h"

#define sizeofarray(ar) (sizeof((ar))/sizeof((ar)[0]))

static bool debugging = false;
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

 const size_t _ul_symlens[] = {
	1, 2, 1, 1, 1, 3, 1, 1
};

#define dynamic_rules (base_rules[NUM_BASE_UNITS-1].next)

// The last error message
static char _errmsg[1024];
void _ul_set_error(const char *func, int line, const char *fmt, ...)
{
	snprintf(errmsg, 1024, "[%s:%d] ", func, line);
	size_t len = strlen(errmsg);

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(errmsg + len, 1024 - len, fmt, ap);
	va_end(ap);
}

//------ Formating functions ------
bool plain_fmt(const unit_t *unit, char *buffer, size_t buflen)
{
	ERROR("Not implemented");
	return false;
}

bool latex_frac_fmt(const unit_t *unit, char *buffer, size_t buflen)
{
	ERROR("Not implemented");
	return false;
}

bool latex_inline_fmt(const unit_t *unit, char *buffer, size_t buflen)
{
	ERROR("Not implemented");
	return false;
}

bool ul_format(const unit_t *unit, char *buffer, size_t buflen,
               ul_format_t format, void *fmtp)
{
	switch (format) {
	case UL_FMT_PLAIN:
		return plain_fmt(unit, buffer, buflen);

	case UL_FMT_LATEX_FRAC:
		return latex_frac_fmt(unit, buffer, buflen);

	case UL_FMT_LATEX_INLINE:
		return latex_inline_fmt(unit, buffer, buflen);

	default:
		ERROR("Unknown format: %d", format);
		return false;
	}
}

char *ul_alloc_string(const unit_t *unit, ul_format_t format)
{
	ERROR("Not implemented");
	return NULL;
}


//------ General unitlib functions ------

void ul_debugging(bool flag)
{
	debugging = flag;
}

const char *ul_error(void)
{
	return errmsg;
}

bool ul_init(void)
{
	int i=0;

	assert(sizeofarray(_ul_symbols) == NUM_BASE_UNITS);
	assert(sizeofarray(_ul_symblens) == NUM_BASE_UNITS);

	debug("Initializing base rules");

	_ul_init_rules();

	initialized = true;

	return true;
}

void ul_quit(void)
{
	assert(initialized);

	_ul_free_rules();
}

