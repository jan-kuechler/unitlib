#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intern.h"
#include "unitlib.h"

#define static_assert(e) extern char (*STATIC_ASSERT(void))[sizeof(char[1 - 2*!(e)])]
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
	"m", "kg", "s", "A", "K", "mol", "Cd", "L",
};
static_assert(sizeofarray(_ul_symbols) == NUM_BASE_UNITS);

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

UL_API ul_cmpres_t ul_cmp(const unit_t *a, const unit_t *b)
{
	if (!a || !b) {
		ERROR("Invalid parameters");
		return UL_ERROR;
	}

	int res = UL_SAME_UNIT;
	for (int i=0; i < NUM_BASE_UNITS; ++i) {
		if (a->exps[i] != b->exps[i]) {
			res = 0;
			break;
		}
	}
	if (ncmp(a->factor, b->factor) == 0) {
		res |= UL_SAME_FACTOR;
	}
	return res;
}

UL_API bool ul_combine(unit_t *restrict unit, const unit_t *restrict with)
{
	if (!unit || !with) {
		ERROR("Invalid parameter");
		return false;
	}
	add_unit(unit, with, 1);
	return true;
}

UL_API bool ul_mult(unit_t *unit, ul_number factor)
{
	if (!unit) {
		ERROR("Invalid parameter");
		return false;
	}
	unit->factor *= factor;
	return true;
}

UL_API bool ul_copy(unit_t *restrict dst, const unit_t *restrict src)
{
	if (!dst || !src) {
		ERROR("Invalid parameter");
		return false;
	}
	copy_unit(src, dst);
	return true;
}

UL_API bool ul_inverse(unit_t *unit)
{
	if (!unit) {
		ERROR("Invalid parameter");
		return false;
	}
	if (ncmp(unit->factor, 0.0) == 0) {
		ERROR("Cannot inverse 0.0");
		return false;
	}

	for (int i=0; i < NUM_BASE_UNITS; ++i) {
		unit->exps[i] = -unit->exps[i];
	}

	unit->factor = 1/unit->factor;
	return true;
}

UL_API bool ul_sqrt(unit_t *unit)
{
	if (!unit) {
		ERROR("Invalid parameter");
		return false;
	}

	for (int i=0; i < NUM_BASE_UNITS; ++i) {
		if ((unit->exps[i] % 2) != 0) {
			ERROR("Cannot take root of an odd exponent");
			return false;
		}
	}
	for (int i=0; i < NUM_BASE_UNITS; ++i) {
		unit->exps[i] /= 2;
	}
	unit->factor = _sqrtn(unit->factor);
	return true;
}

UL_API bool ul_reduceable(const unit_t *unit)
{
	if (!unit) {
		ERROR("Invalid parameter");
		return false;
	}
	return _ul_reduce(unit) != NULL;
}

UL_API void ul_debugging(bool flag)
{
	_ul_debugging = flag;
}

UL_API void ul_debugout(const char *path, bool append)
{
	if (dbg_out && dbg_out != stderr) {
		debug("New debug file: %s", path ? path : "stderr");
		fclose(dbg_out);
	}
	if (!path) {
		dbg_out = stderr;
	}
	else {
		dbg_out = fopen(path,  append ? "a" : "w");
		if (!dbg_out) {
			dbg_out = stderr;
			debug("Failed to open '%s' as debugout, using stderr.", path);
		}
		setvbuf(dbg_out, NULL, _IONBF, 0);
	}
	fprintf(dbg_out, "** unitlib - debug log **\n");
}

UL_API const char *ul_error(void)
{
	return errmsg;
}

UL_API const char *ul_get_name(void)
{
	return UL_FULL_NAME;
}

UL_API const char *ul_get_version(void)
{
	return UL_VERSION;
}

UL_API bool ul_init(void)
{
	if(!dbg_out)
		dbg_out = stderr;

	debug("Initializing unitlib....");

	if (!_ul_init_parser()) {
		return false;
	}

	debug("Init done!");
	return true;
}

UL_API void ul_quit(void)
{
	_ul_free_rules();
	if (dbg_out && dbg_out != stderr)
		fclose(dbg_out);
}
