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

enum result
{
	R_ERROR = 0,
	R_FAIL,
	R_OK,
};

struct printer;

typedef enum result (*print_all_f)(struct printer* p, struct status *stat);
typedef bool (*print_fac_f)(struct status *stat, ul_number factor, bool *first);
typedef bool (*print_sym_f)(struct status *stat, const char *sym, int exp, bool *first);

struct printer
{
	print_sym_f sym;
	print_fac_f fac;
	print_all_f normal;
	print_all_f reduce;
	const char  *prefix;
	const char  *postfix;
};

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

#define CHECK_R(x) do { if (!(x)) return R_ERROR; } while (0)

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

static enum result p_lfrac(struct printer *p, struct status *stat)
{
	if (p->prefix)
		CHECK_R(_puts(stat, p->prefix));

	bool first = true;

	CHECK_R(_puts(stat, "\\frac{"));
	if (_fabsn(stat->unit->factor) >= 1)
		CHECK_R(p->fac(stat, stat->unit->factor, &first));

	for (int i=0; i < NUM_BASE_UNITS; ++i) {
		if (stat->unit->exps[i] > 0)
			CHECK_R(p->sym(stat, _ul_symbols[i], stat->unit->exps[i], &first));
	}
	if (first)
		CHECK_R(p->fac(stat, 1.0, &first));

	CHECK_R(_puts(stat, "}{"));
	first = true;
	if (_fabsn(stat->unit->factor) < 1)
		CHECK_R(p->fac(stat, stat->unit->factor, &first));
	for (int i=0; i < NUM_BASE_UNITS; ++i) {
		if (stat->unit->exps[i] < 0)
			CHECK_R(p->sym(stat, _ul_symbols[i], -stat->unit->exps[i], &first));
	}
	if (first)
		CHECK_R(p->fac(stat, 1.0, &first));

	CHECK_R(_putc(stat, '}'));

	if (p->postfix)
		CHECK_R(_puts(stat, p->postfix));

	return R_OK;
}

static bool p_plain_fac(struct status *stat, ul_number fac, bool *first)
{
	if (!*first)
		CHECK(_putc(stat, ' '));
	CHECK(_putn(stat, fac));
	*first = false;
	return true;
}

static bool p_plain_sym(struct status *stat, const char *sym, int exp, bool *first)
{
	if (!exp)
		return true;

	if (!*first)
		CHECK(_putc(stat, ' '));

	CHECK(_puts(stat, sym));
	if (exp != 1) {
		CHECK(_putc(stat, '^'));
		CHECK(_putd(stat, exp));
	}
	*first = false;
	return true;
}

static bool p_latex_fac(struct status *stat, ul_number fac, bool *first)
{
	ul_number m; int e;
	getnexp(fac, &m, &e);

	if (!*first)
		CHECK(_putc(stat, ' '));
	CHECK(_putn(stat, m));
	if (e != 0) {
		CHECK(_puts(stat, " \\cdot 10^{"));
		CHECK(_putd(stat, e));
		CHECK(_putc(stat, '}'));
	}
	*first = false;
	return true;
}

static bool p_latex_sym(struct status *stat, const char *sym, int exp, bool *first)
{
	if (!*first)
		CHECK(_putc(stat, ' '));

	CHECK(_puts(stat, "\\text{"));
	if (!*first)
		CHECK(_putc(stat, ' '));

	CHECK(_puts(stat, sym));
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

static enum result def_normal(struct printer *p, struct status *stat)
{
	if (p->prefix)
		CHECK_R(_puts(stat, p->prefix));

	bool first = true;

	CHECK_R(p->fac(stat, stat->unit->factor, &first));

	for (int i=0; i < NUM_BASE_UNITS; ++i) {
		if (stat->unit->exps[i] != 0)
			CHECK_R(p->sym(stat, _ul_symbols[i], stat->unit->exps[i], &first));
	}

	if (p->postfix)
		CHECK_R(_puts(stat, p->postfix));

	return R_OK;
}

static enum result def_reduce(struct printer *p, struct status *stat)
{
	const char *sym = _ul_reduce(stat->unit);
	if (!sym)
		return R_FAIL;

	if (p->prefix)
		CHECK_R(_puts(stat, p->prefix));

	bool first = true;
	CHECK_R(p->fac(stat, stat->unit->factor, &first));
	CHECK_R(p->sym(stat, sym, 1, &first));

	if (p->postfix)
		CHECK_R(_puts(stat, p->postfix));

	return R_OK;
}

static struct printer printer[UL_NUM_FORMATS] = {
	[UL_FMT_PLAIN] = {
		.sym = p_plain_sym,
		.fac = p_plain_fac,
		.normal = def_normal,
		.reduce = def_reduce,
		.prefix = NULL,
		.postfix = NULL,
	},
	[UL_FMT_LATEX_INLINE] = {
		.sym = p_latex_sym,
		.fac = p_latex_fac,
		.normal = def_normal,
		.reduce = def_reduce,
		.prefix = "$",
		.postfix = "$",
	},
	[UL_FMT_LATEX_FRAC] = {
		.sym = p_latex_sym,
		.fac = p_latex_fac,
		.normal = p_lfrac,
		.reduce = def_reduce,
		.prefix = "$",
		.postfix = "$",
	},
};

static bool _print(struct status *stat, int opts)
{
	if (stat->format >= UL_NUM_FORMATS) {
		ERROR("Invalid format: %d\n", stat->format);
		return false;
	}

	struct printer *p = &printer[stat->format];

	print_all_f f = p->normal;
	if (opts & UL_FOP_REDUCE) {
		f = p->reduce;
	}

	enum result res = f(p, stat);
	if (res == R_FAIL)
		res = p->normal(p, stat);
	return res == R_OK;
}

UL_API bool ul_fprint(FILE *f, const unit_t *unit, ul_format_t format, int fops)
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

	return _print(&status, fops);
}

UL_API bool ul_snprint(char *buffer, size_t buflen, const unit_t *unit, ul_format_t format, int fops)
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

	return _print(&status, fops);
}

UL_API size_t ul_length(const unit_t *unit, ul_format_t format, int fops)
{
	struct cnt_info info = {0};

	struct status status = {
		.put_char = cnt_putc,
		.info = &info,
		.unit = unit,
		.format = format,
		.extra  = NULL,
	};

	_print(&status, fops);
	return info.count;
}
