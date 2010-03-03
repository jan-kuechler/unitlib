#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "intern.h"
#include "unitlib.h"

struct status
{
	bool (*put_char)(char c, void *info);
	void *info;

	const unit_t *unit;
	ul_format_t  format;
	void         *extra;
};

typedef bool (*printer_t)(struct status*,int,int,bool*);

struct f_info
{
	FILE *out;
};

static bool f_putc(char c, void *i)
{
	struct f_info *info = i;
	return fputc(c, info->out) == c;
}

struct sn_info
{
	char *buffer;
	int idx;
	int size;
};

static bool sn_putc(char c, void *i)
{
	struct sn_info *info = i;
	if (info->idx >= info->size) {
		return false;
	}
	info->buffer[info->idx++] = c;
	return true;
}

struct cnt_info
{
	size_t count;
};

static bool cnt_putc(char c, void *i)
{
	(void)c;
	struct cnt_info *info = i;
	info->count++;
	return true;
}

#define _putc(s,c) (s)->put_char((c),(s)->info)

#define CHECK(x) do { if (!(x)) return false; } while (0)

static bool _puts(struct status *s, const char *str)
{
	while (*str) {
		char c = *str++;
		if (!_putc(s, c))
			return false;
	}
	return true;
}

static bool _putd(struct status *stat, int n)
{
	char buffer[1024];
	snprintf(buffer, 1024, "%d", n);
	return _puts(stat, buffer);
}

static bool _putn(struct status *stat, ul_number n)
{
	char buffer[1024];
	snprintf(buffer, 1024, N_FMT, n);
	return _puts(stat, buffer);
}

static void getnexp(ul_number n, ul_number *mantissa, int *exp)
{
	bool neg = false;
	if (n < 0) {
		neg = true;
		n = -n;
	}
	if ((ncmp(n, 10.0) == -1) && (ncmp(n, 1.0) == 1)) {
		*mantissa = neg ? -n : n;
		*exp = 0;
		return;
	}
	else if (ncmp(n, 10.0) > -1) {
		int e = 0;
		do {
			e++;
			n /= 10;
		} while (ncmp(n, 10.0) == 1);
		*exp = e;
		*mantissa = neg ? -n : n;
	}
	else if (ncmp(n, 1.0) < 1) {
		int e = 0;
		while (ncmp(n, 1.0) == -1) {
			e--;
			n *= 10;
		}
		*exp = e;
		*mantissa = neg ? -n : n;
	}
}

// global for testing purpose, it's not declared in the header
void _ul_getnexp(ul_number n, ul_number *m, int *e)
{
	getnexp(n, m, e);
}

// Begin - plain
static bool print_one_plain(struct status* stat, int unit, int exp)
{
	if (exp == 0)
		return true;

	CHECK(_putc(stat, ' '));
	CHECK(_puts(stat, _ul_symbols[unit]));
	// and the exponent
	if (exp != 1) {
		CHECK(_putc(stat, '^'));
		CHECK(_putd(stat, exp));
	}
	return true;
}

static bool p_plain(struct status *stat)
{
	CHECK(_putn(stat, stat->unit->factor));

	for (int i=0; i < NUM_BASE_UNITS; ++i) {
		CHECK(print_one_plain(stat,i,stat->unit->exps[i]));
	}

	return true;
}
// End - plain

// Begin - LaTeX
static bool print_one_latex(struct status *stat, int unit, int exp, bool *first)
{
	if (exp == 0)
		return true;
	if (stat->extra) {
		bool pos = *(bool*)stat->extra;
		if (exp > 0 && !pos)
			return true;
		if (exp < 0) {
			if (pos)
				return true;
			exp = -exp;
		}
	}
	if (!first || !*first)
		CHECK(_putc(stat, ' '));

	CHECK(_puts(stat, "\\text{"));
	if (!first || !*first)
		CHECK(_putc(stat, ' '));

	CHECK(_puts(stat, _ul_symbols[unit]));
	CHECK(_puts(stat, "}"));
	if (exp != 1) {
		CHECK(_putc(stat, '^'));
		CHECK(_putc(stat, '{'));
		CHECK(_putd(stat, exp));
		CHECK(_putc(stat, '}'));
	}
	if (first)
		*first = false;
	return true;
}

static bool p_latex_frac(struct status *stat)
{
	bool first = true;
	bool positive = true;
	stat->extra = &positive;

	ul_number m; int e;
	getnexp(stat->unit->factor, &m, &e);

	CHECK(_puts(stat, "\\frac{"));

	// upper side of the fraction
	if (e >= 0) {
		// print factors |x| > 1
		CHECK(_putn(stat, m));
		if (e > 0) {
			CHECK(_puts(stat, " \\cdot 10^{"));
			CHECK(_putd(stat, e));
			CHECK(_putc(stat, '}'));
		}
		first = false;
	}
	for (int i=0; i < NUM_BASE_UNITS; ++i) {
		CHECK(print_one_latex(stat,i,stat->unit->exps[i], &first));
	}
	if (first) { // nothing up there...
		CHECK(_putc(stat, '1'));
	}
	CHECK(_puts(stat, "}{"));

	// lower side
	first = true;
	positive = false;
	if (e < 0) {
		// print factors |x| < 1
		e = -e;
		CHECK(_putn(stat, m));
		if (e > 0) {
			CHECK(_puts(stat, " \\cdot 10^{"));
			CHECK(_putd(stat, e));
			CHECK(_putc(stat, '}'));
		}
		first = false;
	}
	for (int i=0; i < NUM_BASE_UNITS; ++i) {
		CHECK(print_one_latex(stat,i,stat->unit->exps[i], &first));
	}
	if (first) {
		CHECK(_putc(stat, '1'));
	}
	CHECK(_putc(stat, '}'));
	return true;
}

static bool p_latex_inline(struct status *stat)
{
	CHECK(_putc(stat, '$'));

	ul_number m; int e;
	getnexp(stat->unit->factor, &m, &e);
	CHECK(_putn(stat, m));
	if (e != 0) {
		CHECK(_puts(stat, " \\cdot 10^{"));
		CHECK(_putd(stat, e));
		CHECK(_putc(stat, '}'));
	}

	for (int i=0; i < NUM_BASE_UNITS; ++i) {
		int exp = stat->unit->exps[i];
		if (exp != 0) {
			CHECK(print_one_latex(stat, i, exp, NULL));
		}
	}
	CHECK(_putc(stat, '$'));
	return true;
}
// End - LaTeX

static bool _print(struct status *stat)
{
	switch (stat->format) {
	case UL_FMT_PLAIN:
		return p_plain(stat);

	case UL_FMT_LATEX_FRAC:
		return p_latex_frac(stat);

	case UL_FMT_LATEX_INLINE:
		return p_latex_inline(stat);

	default:
		ERROR("Unknown format: %d", stat->format);
		return false;
	}
}

UL_API bool ul_fprint(FILE *f, const unit_t *unit, ul_format_t format)
{
	struct f_info info = {
		.out = f,
	};

	struct status status = {
		.put_char = f_putc,
		.info = &info,
		.unit = unit,
		.format = format,
		.extra  = NULL,
	};

	return _print(&status);
}

UL_API bool ul_snprint(char *buffer, size_t buflen, const unit_t *unit, ul_format_t format)
{
	struct sn_info info = {
		.buffer = buffer,
		.size   = buflen,
		.idx    = 0,
	};

	struct status status = {
		.put_char = sn_putc,
		.info = &info,
		.unit = unit,
		.format = format,
		.extra  = NULL,
	};

	memset(buffer, 0, buflen);

	return _print(&status);
}

UL_API size_t ul_length(const unit_t *unit, ul_format_t format)
{
	struct cnt_info info = {0};

	struct status status = {
		.put_char = cnt_putc,
		.info = &info,
		.unit = unit,
		.format = format,
		.extra  = NULL,
	};

	_print(&status);
	return info.count;
}
