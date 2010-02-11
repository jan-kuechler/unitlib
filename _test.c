#include <stdio.h>

#include "unitlib.h"

int main(void)
{
	printf("Testing %s\n", UL_FULL_NAME);

	if (!ul_init()) {
		printf("init failed");
	}
	ul_debugging(false);

	if (!ul_parse_rule("   N=  kg m s^-2  ")) {
		printf("Error: %s\n", ul_error());
	}

	unit_t unit;
	if (!ul_parse("N^2 m^-1", &unit)) {
		printf("Error: %s\n", ul_error());
	}


	ul_fmtops_t fmtop;
	fmtop.sort = true;
	fmtop.order[0] = U_LEMMING;
	fmtop.order[1] = U_KILOGRAM;
	fmtop.order[2] = U_METER;
	fmtop.order[3] = U_ANY;


	printf("N^2 m^-1 = ");
	if (!ul_print(&unit, UL_FMT_PLAIN, &fmtop)) {
		printf("Error: %s\n", ul_error());
	}
	printf("\n");

	char buffer[1024];
	if (!ul_snprint(buffer, 1024, &unit, UL_FMT_PLAIN, NULL)) {
		printf("Error: %s\n", ul_error());
	}
	printf("snprint => '%s'\n", buffer);


	{
		printf("\n----------\n");

		fmtop.order[2] = U_ANY;
		unit_t n;
		ul_parse("N", &n);
		printf("1 N = 1 ");
		ul_print(&n, UL_FMT_PLAIN, &fmtop);
		printf("\n----------\n\n");
	}

	ul_print( &unit, UL_FMT_LATEX_FRAC, &fmtop);
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
