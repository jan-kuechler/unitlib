/**
 * unitlib.h - Main header for the unitlib
 */
#ifndef UNITLIB_H
#define UNITLIB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define UL_NAME      "unitlib"
#define UL_VERSION   "0.2b2"
#define UL_FULL_NAME UL_NAME "-" UL_VERSION

#ifdef __cplusplus
#define UL_LINKAGE extern "C"
#else
#define UL_LINKAGE
#endif

#if defined(UL_EXPORT_DLL)
#define UL_API UL_LINKAGE __declspec(dllexport)
#elif defined(UL_IMPORT_DLL)
#define UL_API UL_LINKAGE __declspec(dllimport)
#else
#define UL_API UL_LINKAGE
#endif

typedef enum base_unit
{
	U_METER,
	U_KILOGRAM,
	U_SECOND,
	U_AMPERE,
	U_KELVIN,
	U_MOL,
	U_CANDELA,
	U_LEMMING, /* Man kann alles in Lemminge umrechnen! */
	NUM_BASE_UNITS,
} base_unit_t;

#define U_ANY -1

typedef enum ul_format
{
	UL_FMT_PLAIN,
	UL_FMT_LATEX_FRAC,
	UL_FMT_LATEX_INLINE,
} ul_format_t;

typedef struct ul_format_ops
{
	bool sort;
	int  order[NUM_BASE_UNITS];
} ul_fmtops_t;

typedef struct unit
{
	int exps[NUM_BASE_UNITS];
} unit_t;

/**
 * Initializes the unitlib. Has to be called before any
 * other ul_* function.
 * @return success
 */
UL_API bool ul_init(void);

/**
 * Deinitializes the unitlib and frees all. This function
 * has to be called at the end of the program.
 * internals resources.
 */
UL_API void ul_quit(void);

/**
 * Enables or disables debugging messages
 * @param flag Enable = true
 */
UL_API void ul_debugging(bool flag);

/**
 * Sets the debug output stream
 * @param out The outstream
 */
UL_API void ul_debugout(const char *path);

/**
 * Returns the last error message
 * @return The last error message
 */
UL_API const char *ul_error(void);

/**
 * Parses a rule and adds it to the rule list
 * @param rule The rule to parse
 * @return success
 */
UL_API bool ul_parse_rule(const char *rule);

/**
 * Loads a rule file
 * @param path Path to the file
 * @return success
 */
UL_API bool ul_load_rules(const char *path);

/**
 * Parses the unit definition from str to unit
 * @param str  The unit definition
 * @param unit The parsed unit will be stored here
 * @return success
 */
UL_API bool ul_parse(const char *str, unit_t *unit);

/**
 * Multiplies a unit to a unit.
 * @param unit One factor and destination of the operation
 * @param with The other unit
 * @return success
 */
UL_API bool ul_combine(unit_t *unit, const unit_t *with);

/**
 * Builds the inverse of a unit
 * @param unit The unit
 * @return success                                                                                                                    * @
 */
UL_API bool ul_inverse(unit_t *unit);

/**
 * Takes the square root of the unit
 * @param unit The unit
 * @return success
 */
UL_API bool ul_sqrt(unit_t *unit);

/**
 * Prints the unit to a file according to the format
 * @param file   The file
 * @param unit   The unit
 * @param format The format
 * @param fmtp   Additional format parameters
 * @return success
 */
UL_API bool ul_fprint(FILE *f, const unit_t *unit, ul_format_t format,
                      ul_fmtops_t *fmtp);

/**
 * Prints the unit to stdout according to the format
 * @param unit   The unit
 * @param format The format
 * @param fmtp   Additional format parameters
 * @return success
 */
static inline bool ul_print(const unit_t *unit, ul_format_t format,
                            ul_fmtops_t *fmtp)
{
	return ul_fprint(stdout, unit, format, fmtp);
}

/**
 * Prints the unit to a buffer according to the format
 * @param buffer The buffer
 * @param buflen Length of the buffer
 * @param unit   The unit
 * @param format The format
 * @param fmtp   Additional format parameters
 * @return success
 */
UL_API bool ul_snprint(char *buffer, size_t buflen, const unit_t *unit,
                       ul_format_t format, ul_fmtops_t *fmtp);

#endif /*UNITLIB_H*/
