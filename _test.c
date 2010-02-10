#include <stdio.h>

#include "unitlib.h"

int main(void)
{
	if (!ul_init()) {
		printf("init failed");
	}
	ul_debugging(true);

	if (!ul_parse_rule("   N  =  kg^2 m s^-2  ")) {
		printf("Error: %s\n", ul_error());
	}

	unit_t unit;
	if (!ul_parse("N^2 m^-1", &unit)) {
		printf("Error: %s\n", ul_error());
	}

	ul_quit();

	return 0;
}
