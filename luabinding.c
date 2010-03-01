#include <lua.h>
#include <lauxlib.h>

#include "unitlib.h"

#define EXPORT __declspec(dllexport)

#define UNUSED(var) do{(void)(var);}while(0)

int l_test(lua_State *L)
{
	UNUSED(L);
	printf("test\n");
	return 0;
}

int l_init(lua_State *L)
{
	bool ok = ul_init();
	lua_pushboolean(L, ok);
	return 1;
}

int l_quit(lua_State *L)
{
	UNUSED(L);
	ul_quit();
	return 0;
}

int l_error(lua_State *L)
{
	const char *msg = ul_error();
	lua_pushstring(L, msg);
	return 1;
}

EXPORT int luaopen_unitlib(lua_State *L)
{
	luaL_Reg reg[] = {
		{"init", l_init},
		{"quit", l_quit},
		{"error", l_error},
		{"test", l_test},
		{NULL, NULL},
	};

	luaL_register(L, "unitlib", reg);

	return 0;
}
