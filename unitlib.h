/**
 * unitlib.h - Main header for the unitlib
 */
#ifndef UNITLIB_H
#define UNITLIB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

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
	UL_FMT_PLAIN,
	UL_FMT_LATEX_FRAC,
	UL_FMT_LATEX_INLINE,
} ul_format_t;

typedef struct unit
{
	int exps[NUM_BASE_UNITS];
} unit_t;

/**
 * Initializes the unitlib. Has to be called before any
 * other ul_* function.
 * @return success
 */
bool ul_init(void);

/**
 * Deinitializes the unitlib and frees all. This function
 * has to be called at the end of the program.
 * internals resources.
 */
void ul_quit(void);

/**
 * Enables or disables debugging messages
 * @param flag Enable = true
 */
void ul_debugging(bool flag);

/**
 * Returns the last error message
 * @return The last error message
 */
const char *ul_error(void);

/**
 * Parses a rule and adds it to the rule list
 * @param rule The rule to parse
 * @return success
 */
bool ul_parse_rule(const char *rule);

/**
 * Parses the unit definition from str to unit
 * @param str  The unit definition
 * @param unit The parsed unit will be stored here
 * @return success
 */
bool ul_parse(const char *str, unit_t *unit);

/**
 * Prints the unit to a file according to the format
 * @param file   The file
 * @param unit   The unit
 * @param format The format
 * @param fmtp   Additional format parameters
 * @return success
 */
bool ul_fprint(FILE *f, const unit_t *unit, ul_format_t format, void *fmtp);

static inline bool ul_print(const unit_t *unit, ul_format_t format, void *fmtp)
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
bool ul_snprintf(char *buffer, size_t buflen, const unit_t *unit,
                 ul_format_t format, void *fmtp);

#endif /*UNITLIB_H*/
