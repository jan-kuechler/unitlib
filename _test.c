#include <stdio.h>

#include "unitlib.h"

int main(void)
{
	printf("Testing %s\n", UL_FULL_NAME);

	if (!ul_init()) {
		printf("init failed");
	}
	ul_debugging(true);

	if (!ul_parse_rule("   N=  kg^2 m s^-2  ")) {
		printf("Error: %s\n", ul_error());
	}

	unit_t unit;
	if (!ul_parse("N^2 m^-1", &unit)) {
		printf("Error: %s\n", ul_error());
	}

	printf("N^2 m^-1 = ");
	if (!ul_print(&unit, UL_FMT_PLAIN, NULL)) {
		printf("Error: %s\n", ul_error());
	}
	printf("\n");

	char buffer[1024];
	if (!ul_snprint(buffer, 1024, &unit, UL_FMT_PLAIN, NULL)) {
		printf("Error: %s\n", ul_error());
	}
	printf("snprint => '%s'\n", buffer);

	ul_print( &unit, UL_FMT_LATEX_FRAC, NULL);
	printf("\n");

	{
		unit_t unit;
		ul_parse("s^-1", &unit);
		printf("LaTeX: ");
		ul_print(&unit, UL_FMT_LATEX_FRAC, NULL);
		printf("\n");

	}

	ul_quit();

	return 0;
}
