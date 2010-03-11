#include <stdio.h>

#include "unitlib.h"

#ifndef SMASH
int main(void)
{
	printf("Testing %s\n", UL_FULL_NAME);

	ul_debugging(true);
	ul_debugout("debug.log", false);
	if (!ul_init()) {
		printf("init failed");
	}

	if (!ul_parse_rule("   N= 1 kg m s^-2  ")) {
		printf("Error: %s\n", ul_error());
	}

	unit_t unit;
	if (!ul_parse("( 0.2 N^2 ) * 0.75 m^-1", &unit)) {
		printf("Error: %s\n", ul_error());
	}

	printf("0.2 N^2 * 0.75 m^-1 = ");
	if (!ul_fprint(stdout, &unit, UL_FMT_PLAIN, 0)) {
		printf("Error: %s\n", ul_error());
	}
	printf("\n");

	char buffer[1024];
	if (!ul_snprint(buffer, 1024, &unit, UL_FMT_PLAIN, 0)) {
		printf("Error: %s\n", ul_error());
	}
	printf("snprint => '%s'\n", buffer);


	{
		printf("\n----------\n");

		unit_t n;
		ul_parse("N", &n);
		printf("1 N = ");
		ul_print(&n, UL_FMT_PLAIN, UL_FOP_REDUCE);
		printf("\n----------\n\n");
	}

	ul_print( &unit, UL_FMT_LATEX_FRAC, 0);
	printf("\n");
	ul_print(&unit, UL_FMT_LATEX_INLINE, 0);
	printf("\n");

	{
		unit_t unit;
		ul_parse("s^-1", &unit);
		printf("LaTeX: ");
		ul_print(&unit, UL_FMT_LATEX_FRAC, 0);
		printf("\n");
	}

	{
		const char *u = "1 mg";
		unit_t unit;
		if (!ul_parse(u, &unit))
			printf("%s\n", ul_error());

		printf("%s = ", u);
		ul_print(&unit, UL_FMT_PLAIN, 0);
		printf("\n");
	}

	ul_quit();

	return 0;
}
#else

void smashit(const char *str, bool asRule)
{
	bool ok = true;
	if (asRule) {
		ok = ul_parse_rule(str);
	}
	else {
		unit_t u;
		ok = ul_parse(str, &u);
	}
	if (ok) {
		fprintf(stderr, "successfull?\n");
	}
	else {
		fprintf(stderr, "error: %s\n", ul_error());
	}
}

int main(void)
{
	ul_init();

	srand(time(NULL));

	{
		fprintf(stderr, "Smash 1: ");
		char test[] = "dil^aöjf lkfjda gäklj#ä#jadf klnhöklj213jl^^- kjäjre";
		smashit(test, false);

		fprintf(stderr, "Smash 2: ");
		smashit(test, true);
	}

	{
		size_t size = 4096;
		int *_test = malloc(size);
		int i=0;
		for (; i < size / sizeof(int); ++i) {
			_test[i] = rand();
		}
		char *test = (char*)_test;
		for (i=0; i < size; ++i) {
			if (!test[i])
				test[i] = 42;
		}

		fprintf(stderr, "%4d bytes garbage: ", size );
		smashit(test, false);
		fprintf(stderr, "as rule:            ");
		smashit(test, true);
	}

	{
		char test[] = "  \t\t\n\n\r kg =     ";

		printf("Evil: ");
		smashit(test, true);
	}

	{
		char test[] = "kg^2!";
		printf("test: ");
		ul_debugging(true);
		smashit(test, false);
	}

	return 0;
}

#endif
