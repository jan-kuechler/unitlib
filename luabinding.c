#include <lua.h>
#include <lauxlib.h>

#include "unitlib.h"
#include <string.h>
#include "intern.h"

#define UNUSED(var) do{(void)(var);}while(0)

#define UNIT "unit"
#define CLEANUP "__ul_cleanup"

static void error(lua_State *L)
{
	lua_pushstring(L, ul_error());
	lua_error(L);	//Note: lua_error does not return
}

static unit_t *create_unit(lua_State *L)
{
	unit_t *unit = lua_newuserdata(L, sizeof(*unit));

	luaL_getmetatable(L, UNIT);
	lua_setmetatable(L, -2);

	return unit;
}

static void mult_uu(lua_State *L, unit_t *left, unit_t *right, bool inv)
{
	unit_t *res = create_unit(L);
	if (!ul_copy(res, right))
		error(L);
	if (inv) {
		if (!ul_inverse(res))
		error(L);
	}
	if (!ul_combine(res, left))
		error(L);
}

static void mult_un(lua_State *L, unit_t *unit, ul_number n, bool inv)
{
	unit_t *res = create_unit(L);
	if (!ul_copy(res, unit))
		error(L);

	if (inv) {
		if (ncmp(n, 0.0) == 0) {
			luaL_error(L, "Cannot devide by 0");
		}
		n = 1 / n;
	}
	if (!ul_mult(res, n)) {
		error(L);
	}
}

static void mult_nu(lua_State *L, ul_number n, unit_t *unit, bool inv)
{
	unit_t *res = create_unit(L);
	if (!ul_copy(res, unit))
		error(L);

	if (inv) {
		if (!ul_inverse(res))
			error(L);
	}
	if (!ul_mult(res, n)) {
		error(L);
	}
}

int l_init(lua_State *L)
{
	if (!ul_init())
		error(L);
	return 0;
}

int l_quit(lua_State *L)
{
	UNUSED(L);
	ul_quit();
	return 0;
}

int l_parse(lua_State *L)
{
	const char *str = luaL_checkstring(L, 1);
	unit_t *u = create_unit(L);
	if (!ul_parse(str, u)) {
		error(L);
	}
	return 1;
}

int l_parse_rule(lua_State *L)
{
	const char *str = luaL_checkstring(L, 1);
	if (!ul_parse_rule(str)) {
		error(L);
	}
	return 0;
}

int lm_tostring(lua_State *L)
{
	unit_t *unit = luaL_checkudata(L, 1, UNIT);

	// TODO: Find something better than a static buffer
	static char buffer[4096];
	if (!ul_snprint(buffer, 4096, unit, UL_FMT_PLAIN, 0))
		error(L);

	lua_pushstring(L, buffer);
	return 1;
}

int lm_equal(lua_State *L)
{
	unit_t *a = luaL_checkudata(L, 1, UNIT);
	unit_t *b = luaL_checkudata(L, 2, UNIT);

	lua_pushboolean(L, ul_equal(a, b));
	return 1;
}

int lm_mul(lua_State *L)
{
	if (lua_isnumber(L, 1)) {
		ul_number n = luaL_checknumber(L, 1);
		unit_t *unit = luaL_checkudata(L, 2, UNIT);
		mult_nu(L, n, unit, false);
	}
	else if (lua_isnumber(L, 2)) {
		unit_t *unit = luaL_checkudata(L, 1, UNIT);
		ul_number n = luaL_checknumber(L, 2);
		mult_un(L, unit, n, false);

	}
	else {
		unit_t *left = luaL_checkudata(L, 1, UNIT);
		unit_t *right  = luaL_checkudata(L, 2, UNIT);
		mult_uu(L, left, right, false);
	}
	return 1;
}

int lm_div(lua_State *L)
{
	if (lua_isnumber(L, 1)) {
		ul_number n = luaL_checknumber(L, 1);
		unit_t *unit = luaL_checkudata(L, 2, UNIT);
		mult_nu(L, n, unit, true);
	}
	else if (lua_isnumber(L, 2)) {
		unit_t *unit = luaL_checkudata(L, 1, UNIT);
		ul_number n = luaL_checknumber(L, 2);
		mult_un(L, unit, n, true);

	}
	else {
		unit_t *left = luaL_checkudata(L, 1, UNIT);
		unit_t *right  = luaL_checkudata(L, 2, UNIT);
		mult_uu(L, left, right, true);
	}
	return 1;
}

int lm_len(lua_State *L)
{
	unit_t *unit = luaL_checkudata(L, 1, UNIT);
	lua_pushnumber(L, ul_factor(unit));
	return 1;
}

int m_gc(lua_State *L)
{
	UNUSED(L);
	printf("m_gc\n");
	return 0;
}

__declspec(dllexport) int luaopen_unitlib(lua_State *L)
{
	luaL_Reg lib[] = {
		{"init",  l_init},
		{"quit",  l_quit},
		{"parse", l_parse},
		{"parse_rule", l_parse_rule},
		{NULL, NULL},
	};

	luaL_Reg meta[] = {
		{"__tostring", lm_tostring},
		{"__eq", lm_equal},
		{"__mul", lm_mul},
		{"__div", lm_div},
		{"__len", lm_len},
		{NULL, NULL},
	};

	luaL_Reg cleanup[] = {
		{"__gc", m_gc},
		{NULL, NULL},
	};

	luaL_register(L, "unitlib", lib);

	luaL_newmetatable(L, CLEANUP);
	luaL_register(L, NULL, cleanup);
	lua_pop(L, 1);

	lua_pushstring(L, "__cleanup_proxy");
	lua_newuserdata(L, 1);
	luaL_getmetatable(L, CLEANUP);
	lua_setmetatable(L, -2);
	lua_settable(L, -3);

	luaL_newmetatable(L, UNIT);
	luaL_register(L, NULL, meta);
	lua_pop(L, 1);

	return 1;
}
