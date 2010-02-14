#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intern.h"
#include "unitlib.h"

#define sizeofarray(ar) (sizeof((ar))/sizeof((ar)[0]))

static FILE *dbg_out = NULL;
bool _ul_debugging = false;
UL_LINKAGE void _ul_debug(const char *fmt, ...)
{
	assert(dbg_out);
	va_list ap;
	va_start(ap, fmt);
	vfprintf(dbg_out, fmt, ap);
	va_end(ap);
}

const char *_ul_symbols[] = {
	"m", "kg", "s", "A", "K", "mol", "C", "L",
};

// The last error message
static char errmsg[1024];
UL_LINKAGE void _ul_set_error(const char *func, int line, const char *fmt, ...)
{
	snprintf(errmsg, 1024, "[%s:%d] ", func, line);
	size_t len = strlen(errmsg);

	va_list ap;
	va_start(ap, fmt);
	vsnprintf(errmsg + len, 1024 - len, fmt, ap);
	va_end(ap);
}

static void add_unit(unit_t *to, const unit_t *other, int times)
{
	int i=0;
	for (; i < NUM_BASE_UNITS; ++i) {
		to->exps[i] += (times * other->exps[i]);
	}
}

UL_API bool ul_combine(unit_t *unit, const unit_t *with)
{
	add_unit(unit, with, 1);
	return true;
}

UL_API bool ul_inverse(unit_t *unit)
{
	int i=0;
	for (; i < NUM_BASE_UNITS; ++i) {
		unit->exps[i] = -unit->exps[i];
	}
	return true;
}

UL_API bool ul_sqrt(unit_t *unit)
{
	int i=0;
	for (; i < NUM_BASE_UNITS; ++i) {
		if ((unit->exps[i] % 2) != 0) {
			ERROR("Cannot take root of an odd exponent");
			return false;
		}
	}
	for (; i < NUM_BASE_UNITS; ++i) {
		unit->exps[i] /= 2;
	}
	return false;
}

UL_API void ul_debugging(bool flag)
{
	_ul_debugging = flag;
}

UL_API void ul_debugout(FILE *out)
{
	dbg_out = out ? out : stderr;
}

UL_API const char *ul_error(void)
{
	return errmsg;
}

UL_API bool ul_init(void)
{
	assert(sizeofarray(_ul_symbols) == NUM_BASE_UNITS);

	if (!dbg_out)
		ul_debugout(stderr);

	debug("Initializing base rules");
	_ul_init_rules();

	const char *rulePath = getenv("UL_RULES");
	if (rulePath) {
		debug("UL_RULES is set: %s", rulePath);
		if (!ul_load_rules(rulePath)) {
			ERROR("Failed to load rules: %s", errmsg);
			return false;
		}
	}

	return true;
}

UL_API void ul_quit(void)
{
	_ul_free_rules();
}

