#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "intern.h"
#include "unitlib.h"

typedef struct rule
{
	const char *symbol;
	unit_t unit;
	bool   force;
	struct rule *next;
} rule_t;

// A list of all rules
static rule_t *rules = NULL;
// The base rules
static rule_t base_rules[NUM_BASE_UNITS];

#define dynamic_rules (base_rules[NUM_BASE_UNITS-1].next)

struct parser_state
{
	int sign;
};

enum {
	MAX_SYM_SIZE = 128,
	MAX_ITEM_SIZE = 1024,
};

// Returns the last rule in the list
static rule_t *last_rule(void)
{
	rule_t *cur = rules;
	while (cur) {
		if (!cur->next)
			return cur;
		cur = cur->next;
	}
	assert(false);
	return NULL;
}

static rule_t *get_rule(const char *sym)
{
	assert(sym);
	for (rule_t *cur = rules; cur; cur = cur->next) {
		if (strcmp(cur->symbol, sym) == 0)
			return cur;
	}
	return NULL;
}

static size_t skipspace(const char *text, size_t start)
{
	assert(text);
	size_t i = start;
	while (text[i] && isspace(text[i]))
		i++;
	return i;
}

static size_t nextspace(const char *text, size_t start)
{
	assert(text);
	size_t i = start;
	while (text[i] && !isspace(text[i]))
		i++;
	return i;
}

static bool try_parse_factor(const char *str, unit_t *unit,
                             struct parser_state *state)
{
	assert(str); assert(unit); assert(state);
	char *endptr;
	ul_number f = _strton(str, &endptr);
	if (endptr && *endptr) {
		debug("'%s' is not a factor", str);
		return false;
	}
	unit->factor *= f;
	return true;
}

static bool is_special(const char *str)
{
	assert(str);
	if (strlen(str) == 1) {
		switch (str[0]) {
		case '*':
			return true;
		case '/':
			return true;
		}
	}
	return false;
}

static bool handle_special(const char *str, struct parser_state *state)
{
	assert(str); assert(state);
	switch (str[0]) {
	case '*':
		// ignore
		return true;

	case '/':
		state->sign *= -1;
		return true;
	}
	ERROR("Internal error: is_special/handle_special missmatch for '%s'.", str);
	return false;
}

static bool parse_item(const char *str, unit_t *unit, struct parser_state *state)
{
	assert(str); assert(unit); assert(state);
	debug("Parse item: '%s'", str);

	// Split symbol and exponent
	char symbol[MAX_SYM_SIZE];
	int exp = 1;

	size_t symend = 0;
	while (str[symend] && str[symend] != '^')
		symend++;

	if (symend >= MAX_SYM_SIZE) {
		ERROR("Symbol to long");
		return false;
	}
	strncpy(symbol, str, symend);
	symbol[symend] = '\0';

	if (str[symend]) {
		// The '^' should not be the last value of the string
		if (!str[symend+1]) {
			ERROR("Missing exponent after '^' while parsing '%s'", str);
			return false;
		}

		// Parse the exponent
		char *endptr = NULL;
		exp = strtol(str+symend+1, &endptr, 10);

		// the whole exp string was valid only if *endptr is '\0'
		if (endptr && *endptr) {
			ERROR("Invalid exponent at char '%c' while parsing '%s'", *endptr, str);
			return false;
		}
	}
	debug("Exponent is %d", exp);

	// Find the matching rule
	rule_t *rule = get_rule(symbol);
	if (!rule) {
		ERROR("No matching rule found for '%s'", symbol);
		return false;
	}

	// And add the definitions
	add_unit(unit, &rule->unit, state->sign * exp);

	return true;
}

UL_API bool ul_parse(const char *str, unit_t *unit)
{
	if (!str || !unit) {
		ERROR("Invalid paramters");
		return false;
	}
	debug("Parse unit: '%s'", str);

	struct parser_state state;
	state.sign = 1;

	init_unit(unit);

	size_t len = strlen(str);
	size_t start = 0;
	do {
		char this_item[MAX_ITEM_SIZE ];

		// Skip leading whitespaces
		start = skipspace(str, start);
		// And find the next whitespace
		size_t end = nextspace(str, start);

		if (end == start) {// End of string
			break;
		}
		// sanity check
		if ((end - start) > MAX_ITEM_SIZE ) {
			ERROR("Item too long");
			return false;
		}

		// copy the item out of the string
		strncpy(this_item, str+start, end-start);
		this_item[end-start] = '\0';

		// and parse it
		if (is_special(this_item)) {
			if (!handle_special(this_item, &state))
				return false;
		}
		else if (try_parse_factor(this_item, unit, &state)) {
			// nothing todo
		}
		else {
			if (!parse_item(this_item, unit, &state))
				return false;
		}

		start = end + 1;
	} while (start < len);

	return true;
}

static bool add_rule(const char *symbol, const unit_t *unit, bool force)
{
	assert(symbol);	assert(unit);
	rule_t *rule = malloc(sizeof(*rule));
	if (!rule) {
		ERROR("Failed to allocate memory");
		return false;
	}
	rule->next = NULL;

	rule->symbol = symbol;
	rule->force  = force;

	copy_unit(unit, &rule->unit);

	rule_t *last = last_rule();
	last->next = rule;

	return true;
}

static bool rm_rule(rule_t *rule)
{
	assert(rule);
	if (rule->force) {
		ERROR("Cannot remove forced rule");
		return false;
	}

	rule_t *cur = dynamic_rules; // base rules cannot be removed
	rule_t *prev = &base_rules[NUM_BASE_UNITS-1];

	while (cur && cur != rule) {
		prev = cur;
		cur = cur->next;
	}

	if (cur != rule) {
		ERROR("Rule not found.");
		return false;
	}

	prev->next = rule->next;
	return true;
}

static bool valid_symbol(const char *sym)
{
	assert(sym);
	while (*sym) {
		if (!isalpha(*sym))
			return false;
		sym++;
	}
	return true;
}

static char *get_symbol(const char *rule, size_t splitpos, bool *force)
{
	assert(rule); assert(force);
	size_t skip   = skipspace(rule, 0);
	size_t symend = nextspace(rule, skip);
	if (symend > splitpos)
		symend = splitpos;

	if (skipspace(rule,symend) != splitpos) {
		// rule was something like "a b = kg"
		ERROR("Invalid symbol, whitespaces are not allowed.");
		return NULL;
	}

	if ((symend-skip) > MAX_SYM_SIZE) {
		ERROR("Symbol to long");
		return NULL;
	}
	if ((symend-skip) == 0) {
		ERROR("Empty symbols are not allowed.");
		return NULL;
	}

	if (rule[skip] == '!') {
		debug("Forced rule.");
		*force = true;
		skip++;
	}
	else {
		*force = false;
	}

	debug("Allocate %d bytes", symend-skip + 1);
	char *symbol = malloc(symend-skip + 1);
	if (!symbol) {
		ERROR("Failed to allocate memory");
		return NULL;
	}

	strncpy(symbol, rule + skip, symend-skip);
	symbol[symend-skip] = '\0';
	debug("Symbol is '%s'", symbol);

	return symbol;
}

// parses a string like "symbol = def"
UL_API bool ul_parse_rule(const char *rule)
{
	if (!rule) {
		ERROR("Invalid parameter");
		return false;
	}

	// split symbol and definition
	size_t len = strlen(rule);
	size_t splitpos = 0;

	debug("Parsing rule '%s'", rule);

	for (size_t i=0; i < len; ++i) {
		if (rule[i] == '=') {
			debug("Split at %d", i);
			splitpos = i;
			break;
		}
	}
	if (!splitpos) {
		ERROR("Missing '=' in rule definition '%s'", rule);
		return false;
	}

	// Get the symbol
	bool force = false;
	char *symbol = get_symbol(rule, splitpos, &force);
	if (!symbol)
		return false;

	if (!valid_symbol(symbol)) {
		ERROR("Symbol '%s' is invalid.", symbol);
		free(symbol);
		return false;
	}

	rule_t *old_rule = NULL;
	if ((old_rule = get_rule(symbol)) != NULL) {
		if (old_rule->force || !force) {
			ERROR("You may not redefine '%s'", symbol);
			free(symbol);
			return false;
		}
		// remove the old rule, so it cannot be used in the definition
		// of the new one, so something like "!R = R" is not possible
		if (force) {
			if (!rm_rule(old_rule)) {
				free(symbol);
				return false;
			}
		}
	}

	rule = rule + splitpos + 1; // ommiting the '='
	debug("Rest definition is '%s'", rule);

	unit_t unit;
	if (!ul_parse(rule, &unit)) {
		free(symbol);
		return false;
	}

	return add_rule(symbol, &unit, force);
}

UL_API bool ul_load_rules(const char *path)
{
	FILE *f = fopen(path, "r");
	if (!f) {
		ERROR("Failed to open file '%s'", path);
		return false;
	}

	bool ok = true;
	char line[1024];
	while (fgets(line, 1024, f)) {
		size_t skip = skipspace(line, 0);
		if (!line[skip] || line[skip] == '#')
			continue; // empty line or comment
		ok = ul_parse_rule(line);
		if (!ok)
			break;
	}
	fclose(f);
	return ok;
}

UL_LINKAGE void _ul_init_rules(void)
{
	for (int i=0; i < NUM_BASE_UNITS; ++i) {
		base_rules[i].symbol = _ul_symbols[i];

		init_unit(&base_rules[i].unit);
		base_rules[i].force = true;
		base_rules[i].unit.exps[i] = 1;

		base_rules[i].next = &base_rules[i+1];
	}
	dynamic_rules = NULL;
	rules = base_rules;
}

UL_LINKAGE void _ul_free_rules(void)
{
	debug("Freeing rule list");
	rule_t *cur = dynamic_rules;
	while (cur) {
		rule_t *next = cur->next;
		free((char*)cur->symbol);
		free(cur);
		cur = next;
	}
}
