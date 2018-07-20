#include <string>
#include "lrmq_connection_state.cpp"

namespace glory
{

	void * LrmqMalloc(size_t size)
	{
		return malloc(size);
	}

	void LrmqFree(void * block)
	{
		free(block);
	}

	void LrmqCheckTimeVal(lua_State * L, int index, timeval & time)
	{
		time.tv_sec = 0;
		time.tv_usec = 0;
		if (lua_isnoneornil(L, index))
		{
			return;
		}
		luaL_argcheck(L, lua_istable(L, index), index, "Expected timeval Table!!!");
		lua_pushstring(L, "sec");
		lua_gettable(L, 1);
		if (!lua_isnoneornil(L, -1))
		{
			time.tv_sec = (long)luaL_checkinteger(L, -1);
		}
		lua_pop(L, 1);
		lua_pushstring(L, "usec");
		lua_gettable(L, 1);
		if (!lua_isnoneornil(L, -1))
		{
			time.tv_usec = (long)luaL_checkinteger(L, -1);
		}
		lua_pop(L, 1);
	}
}

extern "C"
{
    LUALIB_API int luaopen_lrmq(lua_State * L)
    {
		glory::LrmqConnectionInit(L);

		static const luaL_Reg lrmq_functions[] =
		{
			{ "connection_open", glory::LrmqConnectionOpen },
			{ NULL, NULL }
		};
		luaL_newlib(L, lrmq_functions);
		return 1;
    }
}
