#include <lua.h>
#include <lauxlib.h>

#include "unitlib.h"

#define EXPORT __declspec(dllexport)

int l_test(lua_State *L)
{
	printf("test\n");
	return 0;
}

EXPORT int luaopen_unitlib(lua_State *L)
{
	luaL_Reg reg[] = {
		{"test", l_test},
		{NULL, NULL},
	};

	luaL_register(L, "unitlib", reg);

	return 0;
}
