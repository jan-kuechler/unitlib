#include <stdlib.h>
#include <string.h>
#include "unitlib.h"
#include "intern.h"

#define BEGIN_TEST(name) \
	{ printf("------ [ %-20s ]\n", name); const char * _test_name = name; int _fails=0;

#define END_TEST \
	if (_fails) printf(":%d error%s\n", _fails, _fails > 1 ? "s" : ""); \
	else printf(":Ok\n"); \
	printf("-------------------------------\n");}

#define CHECK(expr) \
	do { if (!(expr)) { printf("%s failed.\n", #expr); _fails++; }} while (0)

#define INFO(fmt, ...) \
	do { printf("* " fmt "\n", ##__VA_ARGS__); } while (0)

int main(void)
{
	printf("Unit tests for " UL_FULL_NAME "\n");

	ul_debugging(false);
	if (!ul_init()) {
		printf("ul_init failed: %s", ul_error());
		return 1;
	}

	BEGIN_TEST("Parser I")
		unit_t u;
		CHECK(ul_parse("m", &u));
		CHECK(u.exps[U_METER] == 1);

		int i=0;
		for (; i < NUM_BASE_UNITS; ++i) {
			if (i != U_METER) {
				CHECK(u.exps[i] == 0);
			}
		}

		CHECK(ncmp(u.factor, 1.0) == 0);
	END_TEST

	BEGIN_TEST("Parser II")
		unit_t u;

		CHECK(ul_parse("	\n kg^2 * m  ", &u));
		CHECK(u.exps[U_KILOGRAM] == 2);
		CHECK(u.exps[U_METER] == 1);
		CHECK(u.exps[U_SECOND] == 0);
		CHECK(ncmp(u.factor, 1.0) == 0);

		CHECK(ul_parse("2 Cd 7 s^-1", &u));
		CHECK(u.exps[U_CANDELA] == 1);
		CHECK(u.exps[U_SECOND] == -1);
		CHECK(ncmp(u.factor, 14.0) == 0);

		CHECK(ul_parse("", &u));
		int i=0;
		for (; i < NUM_BASE_UNITS; ++i) {
			CHECK(u.exps[i] == 0);
		}
		CHECK(ncmp(u.factor, 1.0) == 0);
	END_TEST

	BEGIN_TEST("Parser II")
		unit_t u;

		const char *strings[] = {
			"5*kg^2",    // need whitespace
			"5 ** kg^2", // double *
			"5! * kg^2", // !
			"5 * kg^2!", // !
			NULL
		};

		int i = 0;
		while (strings[i]) {
			CHECK(ul_parse(strings[i], &u) == false);
			i++;
		}
	END_TEST

	BEGIN_TEST("Parser IV")
		const char *strings[] = {
			"",          // empty rule
			" =",        // empty symbol
			"16 = 16",   // invalid rule
			" a b = s ", // invalid symbol
			" c == kg",  // double =
			"d = e",     // unknown 'e'
			" = kg",     // empty symbol
			NULL,
		};

		int i=0;
		while (strings[i]) {
			CHECK(ul_parse_rule(strings[i]) == false);
			//INFO("%s", ul_error());
			i++;
		}

	END_TEST

	BEGIN_TEST("Parser V")
		// Empty rules are allowed
		CHECK(ul_parse_rule("EmptySymbol = "));

		unit_t u;
		CHECK(ul_parse("EmptySymbol", &u));

		int i=0;
		for (; i < NUM_BASE_UNITS; ++i) {
			CHECK(u.exps[i] == 0);
		}
		CHECK(ncmp(u.factor, 1.0) == 0);

	END_TEST

	BEGIN_TEST("Parser VI")
		unit_t u;

		CHECK(ul_parse(NULL, NULL) == false);
		CHECK(ul_parse(NULL, &u)   == false);
		CHECK(ul_parse("kg", NULL) == false);

		CHECK(ul_parse_rule(NULL) == false);
		CHECK(ul_parse_rule("")   == false);
	END_TEST

	BEGIN_TEST("Parser VII")
		unit_t kg, s;

		CHECK(ul_parse("kg", &kg));
		CHECK(ul_parse("s", &s));

		unit_t u;

		CHECK(ul_parse_rule("!ForcedRule = kg"));
		CHECK(ul_parse("ForcedRule", &u));
		CHECK(ul_equal(&kg, &u));

		CHECK(ul_parse_rule("NewRule = kg"));
		CHECK(ul_parse("NewRule", &u));
		CHECK(ul_equal(&kg, &u));

		CHECK(ul_parse_rule("!NewRule = s"));
		CHECK(ul_parse("NewRule", &u));
		CHECK(ul_equal(&s, &u));

		CHECK(ul_parse_rule("!NewRule = m") == false);
		CHECK(ul_parse_rule("!kg = kg") == false);

		CHECK(ul_parse_rule(" Recurse = m"));
		CHECK(ul_parse_rule("!Recurse = Recurse") == false);

	END_TEST

}
