/**
 * unitlib.h - Main header for the unitlib
 */
#ifndef UNITLIB_H
#define UNITLIB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#include "unitlib-config.h"

#define UL_NAME      "unitlib"
#define UL_VERSION   "0.5"
#define UL_FULL_NAME UL_NAME "-" UL_VERSION

#ifdef __cplusplus
#define UL_LINKAGE extern "C"
#else
#define UL_LINKAGE
#endif
#define UL_API UL_LINKAGE

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

typedef enum ul_format
{
	UL_FMT_PLAIN = 0,
	UL_FMT_LATEX_FRAC,
	UL_FMT_LATEX_INLINE,
	UL_NUM_FORMATS,
} ul_format_t;

typedef enum ul_cmpres
{
	UL_DIFFERENT   = 0x00,
	UL_SAME_UNIT   = 0x01,
	UL_SAME_FACTOR = 0x02,
	UL_EQUAL       = 0x03,
	UL_ERROR       = 0xFF,
} ul_cmpres_t;

enum ul_fmtop
{
	UL_FOP_REDUCE = 0x01,
};

typedef struct unit
{
	int exps[NUM_BASE_UNITS];
	ul_number factor;
} unit_t;

/**
 * Initializes the unitlib. Has to be called before any
 * other ul_* function (excl. the ul_debug* functions).
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
UL_API void ul_debugout(const char *path, bool append);

/**
 * Returns the full name of unitlib, including the version
 * @return String in the form "unitlib-x.yz"
 */
UL_API const char *ul_get_name(void);

/**
 * Returns the version of unitlib
 * @return String in the form "x.yz"
 */
UL_API const char *ul_get_version(void);

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
 * Returns the factor of a unit
 * @param unit The unit
 * @return The factor
 */
static inline ul_number ul_factor(const unit_t *unit)
{
	if (!unit)
		return 0.0;
	return unit->factor;
}

/**
 * Compares two units
 * @param a A unit
 * @param b Another unit
 * @return Compare result
 */
UL_API ul_cmpres_t ul_cmp(const unit_t *a, const unit_t *b);

/**
 * Compares two units
 * @param a A unit
 * @param b Another unit
 * @return true if both units are equal
 */
static inline bool ul_equal(const unit_t *a, const unit_t *b)
{
	return ul_cmp(a, b) == UL_EQUAL;
}

/**
 * Copies a unit into another
 * @param dst Destination unit
 * @param src Source unit
 * @return success
 */
UL_API bool ul_copy(unit_t *restrict dst, const unit_t *restrict src);

/**
 * Multiplies a unit to a unit.
 * @param unit One factor and destination of the operation
 * @param with The other unit
 * @return success
 */
UL_API bool ul_combine(unit_t *restrict unit, const unit_t *restrict with);

/**
 * Multiplies a unit with a factor
 * @param unit The unit
 * @param factor The factor
 * @return success
 */
UL_API bool ul_mult(unit_t *unit, ul_number factor);

/**
 * Builds the inverse of a unit
 * @param unit The unit
 * @return success
 */
UL_API bool ul_inverse(unit_t *unit);

/**
 * Takes the square root of the unit
 * @param unit The unit
 * @return success
 */
UL_API bool ul_sqrt(unit_t *unit);

/**
 * Checks whether a unit is reduceable to a composed unit
 * @param unit The unit
 * @return True if the unit is reduceable, false if not or if an error occured
 */
UL_API bool ul_reduceable(const unit_t *unit);

/**
 * Prints the unit to a file according to the format
 * @param file   The file
 * @param unit   The unit
 * @param format The format
 * @param fops   A bitmap containing UL_FOP_* flags
 * @return success
 */
UL_API bool ul_fprint(FILE *f, const unit_t *unit, ul_format_t format, int fops);

/**
 * Prints the unit to stdout according to the format
 * @param unit   The unit
 * @param format The format
 * @param fops   A bitmap containing UL_FOP_* flags
 * @return success
 */
static inline bool ul_print(const unit_t *unit, ul_format_t format, int fops)
{
	return ul_fprint(stdout, unit, format, fops);
}

/**
 * Prints the unit to a buffer according to the format
 * @param buffer The buffer
 * @param buflen Length of the buffer
 * @param unit   The unit
 * @param format The format
 * @param fops   A bitmap containing UL_FOP_* flags
 * @return success
 */
UL_API bool ul_snprint(char *buffer, size_t buflen, const unit_t *unit, ul_format_t format, int fops);

/**
 * Returns the length of the formated unit
 * @param unit   The unit
 * @param format Format option
 * @param fops   A bitmap containing UL_FOP_* flags
 * @return Length of the formated string
 */
UL_API size_t ul_length(const unit_t *unit, ul_format_t format, int fops);

#endif /*UNITLIB_H*/
