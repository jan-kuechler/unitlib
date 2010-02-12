#ifndef UL_INTERN_H
#define UL_INTERN_H

#include "unitlib.h"

extern const char *_ul_symbols[];
extern size_t _ul_symlens[];

extern bool _ul_debugging;
UL_LINKAGE void _ul_debug(const char *fmt, ...);
#define debug(fmt,...) \
	do { \
		if (_ul_debugging) _ul_debug("[ul - %s] " fmt "\n",\
		                             __func__, ##__VA_ARGS__);\
	} while(0)


UL_LINKAGE void _ul_set_error(const char *func, int line, const char *fmt, ...);
#define ERROR(msg, ...) _ul_set_error(__func__, __LINE__, msg, ##__VA_ARGS__)

UL_LINKAGE void _ul_init_rules(void);
UL_LINKAGE void _ul_free_rules(void);

#endif /*UL_INTERN_H*/
