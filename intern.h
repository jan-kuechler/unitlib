#ifndef UL_INTERN_H
#define UL_INTERN_H

#include <float.h>
#include <math.h>
#include "unitlib.h"

extern const char *_ul_symbols[];
extern size_t _ul_symlens[];

extern bool _ul_debugging;
UL_LINKAGE void _ul_debug(const char *fmt, ...);
#define debug(fmt,...) \
	do { \
		if (_ul_debugging) _ul_debug("[%s] " fmt "\n",\
		                             __func__, ##__VA_ARGS__);\
	} while(0)


UL_LINKAGE void _ul_set_error(const char *func, int line, const char *fmt, ...);
#define ERROR(msg, ...) _ul_set_error(__func__, __LINE__, msg, ##__VA_ARGS__)

UL_LINKAGE const char *_ul_reduce(const unit_t *unit);

UL_LINKAGE bool _ul_init_parser(void);
UL_LINKAGE void _ul_free_rules(void);

#define EXPS_SIZE(unit) (sizeof((unit)->exps[0]) * NUM_BASE_UNITS)

static inline void init_unit(unit_t *unit)
{
	memset(unit->exps, 0, EXPS_SIZE(unit));
	unit->factor = 1.0;
}

static inline void copy_unit(const unit_t *restrict src, unit_t *restrict dst)
{
	memcpy(dst->exps, src->exps, EXPS_SIZE(dst));
	dst->factor = src->factor;
}

static inline void add_unit(unit_t *restrict to, const unit_t *restrict other, int times)
{
	for (int i=0; i < NUM_BASE_UNITS; ++i) {
		to->exps[i] += (times * other->exps[i]);
	}
	to->factor *= _pown(other->factor, times);
}

static inline int ncmp(ul_number a, ul_number b)
{
	if (_fabsn(a-b) < N_EPSILON)
		return 0;
	if (a < b)
		return -1;
	return 1;
}

#endif /*UL_INTERN_H*/