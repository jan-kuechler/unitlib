#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "intern.h"
#include "unitlib.h"

struct status
{
	bool (*putc)(char c, void *info);
	void *info;

	const unit_t *unit;
	ul_format_t  format;
	void *fmtp;
};

struct f_info
{
	int magic;
	FILE *out;
};

static bool f_putc(char c, void *i)
{
	struct f_info *info = i;
	assert(info->magic == 0x42);
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

#define _putc(s,c) (s)->putc((c),(s)->info)

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

static bool _putn(struct status *stat, int n)
{
	char buffer[1024];
	snprintf(buffer, 1024, "%d", n);
	return _puts(stat, buffer);
}


static bool p_plain(struct status *stat)
{
	int i=0;
	bool first = true;
	for (; i < NUM_BASE_UNITS; ++i) {
		int exp = stat->unit->exps[i];
		if (exp != 0) {
			// leading ' '
			if (!first)
				CHECK(_putc(stat, ' '));
			// the unit symbol
			CHECK(_puts(stat, _ul_symbols[i]));
			// and the exponent
			if (exp != 1) {
				CHECK(_putc(stat, '^'));
				CHECK(_putn(stat, exp));
			}
			first = false;
		}
	}
	return true;
}

static bool _latex_unit(struct status *stat, int unit, int exp)
{
	CHECK(_puts(stat, "\\text{"));
	CHECK(_puts(stat, _ul_symbols[unit]));
	CHECK(_puts(stat, "}"));
	if (exp != 1) {
		CHECK(_putc(stat, '^'));
		CHECK(_putc(stat, '{'));
		CHECK(_putn(stat, exp));
		CHECK(_putc(stat, '}'));
	}
	return true;
}

static bool p_latex_frac(struct status *stat)
{
	int i=0;

	bool first = true;

	CHECK(_puts(stat, "\\frac{"));
	for (i=0; i < NUM_BASE_UNITS; ++i) {
		int exp = stat->unit->exps[i];
		if (exp > 0) {
			if (!first)
				CHECK(_puts(stat, "\\cdot"));
			CHECK(_latex_unit(stat, i, exp));
			first = false;
		}
	}
	if (first) {
		// nothing up there...
		CHECK(_putc(stat, '1'));
	}

	CHECK(_puts(stat, "}{"));
	first = true;
	for (i=0; i < NUM_BASE_UNITS; ++i) {
		int exp = stat->unit->exps[i];
		if (exp < 0) {
			if (!first)
				CHECK(_puts(stat, "\\cdot"));
			CHECK(_latex_unit(stat, i, -exp));
			first = false;
		}
	}
	if (first) {
		CHECK(_putc(stat, '1'));
	}
	CHECK(_putc(stat, '}'));
	return true;
}

static bool _print(struct status *stat)
{
	switch (stat->format) {
	case UL_FMT_PLAIN:
		return p_plain(stat);

	case UL_FMT_LATEX_FRAC:
		return p_latex_frac(stat);

	default:
		ERROR("Unknown format: %d", stat->format);
		return false;
	}
}

bool ul_fprint(FILE *f,  const unit_t *unit, ul_format_t format, void *fmtp)
{
	struct f_info info = {
		.out = f,
		.magic = 0x42,
	};

	struct status status = {
		.putc = f_putc,
		.info = &info,
		.unit = unit,
		.format = format,
		.fmtp   = fmtp,
	};

	return _print(&status);
}

bool ul_snprint(char *buffer, size_t buflen, const unit_t *unit, ul_format_t format, void *fmtp)
{
	struct sn_info info = {
		.buffer = buffer,
		.size   = buflen,
		.idx    = 0,
	};

	struct status status = {
		.putc = sn_putc,
		.info = &info,
		.unit = unit,
		.format = format,
		.fmtp   = fmtp,
	};

	memset(buffer, 0, buflen);

	return _print(&status);
}
