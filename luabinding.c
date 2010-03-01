#include <lua.h>
#include <lauxlib.h>

#include "unitlib.h"

#define EXPORT __declspec(dllexport)
#define UNUSED(var) do{(void)(var);}while(0)

#define UNIT "unit"

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

int l_test(lua_State *L)
{
	UNUSED(L);
	ul_parse(NULL, NULL);
	error(L);
	return 0;
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
int lm_tostring(lua_State *L)
{
	unit_t *unit = luaL_checkudata(L, 1, UNIT);

	// TODO: Find something better than a static buffer
	static char buffer[4096];
	if (!ul_snprint(buffer, 4096, unit, UL_FMT_PLAIN, NULL))
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

EXPORT int luaopen_unitlib(lua_State *L)
{
	luaL_Reg lib[] = {
		{"init",  l_init},
		{"quit",  l_quit},
		{"parse", l_parse},
		{"test",  l_test},
		{NULL, NULL},
	};

	luaL_Reg meta[] = {
		{"__tostring", lm_tostring},
		{"__eq", lm_equal},
		{NULL, NULL},
	};

	luaL_register(L, "unitlib", lib);

	luaL_newmetatable(L, UNIT);
	luaL_register(L, NULL, meta);
	lua_pop(L, 1);

	return 1;
}
