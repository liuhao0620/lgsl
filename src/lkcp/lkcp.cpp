extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <ikcp.h>
}

namespace glory
{
	static void * LkcpMalloc(size_t size)
	{
		return malloc(size);
	}

	static void LkcpFree(void * block)
	{
		free(block);
	}

	struct LkcpHandle
	{
		lua_State * _L = NULL;
		int _output_callback = LUA_NOREF;
	};

	static int LkcpCheckUser(lua_State * L, int index, int old_user)
	{
		luaL_unref(L, LUA_REGISTRYINDEX, old_user);
		lua_pushvalue(L, index);
		return luaL_ref(L, LUA_REGISTRYINDEX);
	}

	static int LkcpCheckCallback(lua_State * L, int index, int old_callback)
	{
		if (!lua_isfunction(L, index))
		{
			luaL_argerror(L, index, "Expected a function");
		}
		luaL_unref(L, LUA_REGISTRYINDEX, old_callback);
		lua_pushvalue(L, index);
		return luaL_ref(L, LUA_REGISTRYINDEX);
	}

	static int Traceback(lua_State *L)
	{
		if (!lua_isstring(L, 1))  /* 'message' not a string? */
			return 1;  /* keep it intact */
		lua_pushglobaltable(L);
		lua_getfield(L, -1, "debug");
		lua_remove(L, -2);
		if (!lua_istable(L, -1)) {
			lua_pop(L, 1);
			return 1;
		}
		lua_getfield(L, -1, "traceback");
		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 2);
			return 1;
		}
		lua_pushvalue(L, 1);  /* pass error message */
		lua_pushinteger(L, 2);  /* skip this function and traceback */
		lua_call(L, 2, 1);  /* call debug.traceback */
		return 1;
	}

	static void LkcpCallCallback(lua_State * L, int callback, int argc)
	{
		if (callback == LUA_NOREF)
		{
			lua_pop(L, argc);
		}
		else
		{
			// Get the traceback function in case of error
			lua_pushcfunction(L, Traceback);
			int errfunc = lua_gettop(L);
			// And insert it before the args if there are any.
			if (argc != 0)
			{
				lua_insert(L, -1 - argc);
				errfunc -= argc;
			}
			// Get the callback
			lua_rawgeti(L, LUA_REGISTRYINDEX, callback);
			// And insert it before the args if there are any.
			if (argc)
			{
				lua_insert(L, -1 - argc);
			}

			int ret = lua_pcall(L, argc, 0, errfunc);
			switch (ret) {
			case 0:
				break;
			case LUA_ERRMEM:
				fprintf(stderr, "System Error: %s\n", lua_tostring(L, -1));
				exit(-1);
				break;
			case LUA_ERRRUN:
			case LUA_ERRSYNTAX:
			case LUA_ERRERR:
			default:
				fprintf(stderr, "Uncaught Error: %s\n", lua_tostring(L, -1));
				lua_pop(L, 1);
				break;
			}
			// Remove the traceback function
			lua_pop(L, 1);
		}
	}

	static ikcpcb * LkcpCheckKcp(lua_State * L, int index)
	{
		ikcpcb * kcp = *((ikcpcb **)luaL_checkudata(L, index, "ikcpcb"));
		luaL_argcheck(L, kcp != NULL, index, "Expected ikcpcb");
		return kcp;
	}

	static void LkcpPushUser(lua_State * L, int user)
	{
		if (user == LUA_NOREF)
		{
			lua_pushnil(L);
		}
		else
		{
			lua_rawgeti(L, LUA_REGISTRYINDEX, user);
		}
	}

	static int LkcpOutputCb(const char * buf, int len, struct IKCPCB *kcp, void *user)
	{
		LkcpHandle * handle = (LkcpHandle *)user;
		lua_State * L = handle->_L;
		lua_pushlstring(L, buf, len);
		LkcpCallCallback(L, handle->_output_callback, 1);
		return 0;
	}

	static int LkcpCreate(lua_State * L)
	{
		int conv = (int)luaL_checkinteger(L, 1);
		LkcpHandle * handle = (LkcpHandle *)LkcpMalloc(sizeof(LkcpHandle));
		handle->_L = L;
		handle->_output_callback = LkcpCheckCallback(L, 2, handle->_output_callback);
		ikcpcb* kcp = ikcp_create(conv, (void *)handle);
		if (kcp == NULL)
		{
			lua_pushnil(L);
			return 1;
		}
		(*(ikcpcb **)lua_newuserdata(L, sizeof(ikcpcb*))) = kcp;
		luaL_getmetatable(L, "ikcpcb");
		lua_setmetatable(L, -2);
		kcp->output = LkcpOutputCb;
		return 1;
	}

	static int LkcpRelease(lua_State * L)
	{
		ikcpcb * kcp = LkcpCheckKcp(L, 1);
		if (kcp->user != NULL)
		{
			LkcpHandle * handle = (LkcpHandle *)kcp->user;
			LkcpFree(handle);
		}
		ikcp_release(kcp);
		return 0;
	}

	static int LkcpNodelay(lua_State * L)
	{
		ikcpcb * kcp = LkcpCheckKcp(L, 1);
		int nodelay = (int)luaL_checkinteger(L, 2);
		int interval = (int)luaL_checkinteger(L, 3);
		int resend = (int)luaL_checkinteger(L, 4);
		int nc = (int)luaL_checkinteger(L, 5);

		int ret_code = ikcp_nodelay(kcp, nodelay, interval, resend, nc);
		lua_pushinteger(L, ret_code);
		return 1;
	}

	static int LkcpWndsize(lua_State * L)
	{
		ikcpcb * kcp = LkcpCheckKcp(L, 1);
		int sndwnd = (int)luaL_checkinteger(L, 2);
		int rcvwnd = (int)luaL_checkinteger(L, 3);
		int ret_code = ikcp_wndsize(kcp, sndwnd, rcvwnd);
		lua_pushinteger(L, ret_code);
		return 1;
	}

	static int LkcpSetMtu(lua_State * L)
	{
		ikcpcb * kcp = LkcpCheckKcp(L, 1);
		int mtu = (int)luaL_checkinteger(L, 2);
		int ret_code = ikcp_setmtu(kcp, mtu);
		lua_pushinteger(L, ret_code);
		return 1;
	}

	static int LkcpSetMinRto(lua_State * L)
	{
		ikcpcb * kcp = LkcpCheckKcp(L, 1);
		int minrto = (int)luaL_checkinteger(L, 2);
		kcp->rx_minrto = minrto;
		return 0;
	}

	static int LkcpUpdate(lua_State * L)
	{
		ikcpcb * kcp = LkcpCheckKcp(L, 1);
		unsigned int current = (unsigned int)luaL_checkinteger(L, 2);
		ikcp_update(kcp, current);
		return 0;
	}

	static int LkcpCheck(lua_State * L)
	{
		ikcpcb * kcp = LkcpCheckKcp(L, 1);
		unsigned int current = (unsigned int)luaL_checkinteger(L, 2);
		unsigned int invoke_time = ikcp_check(kcp, current);
		lua_pushinteger(L, invoke_time);
		return 1;
	}

#define MAX_RECV_SIZE (65535)
	static int LkcpRecv(lua_State * L)
	{
		ikcpcb * kcp = LkcpCheckKcp(L, 1);
		char buffer[MAX_RECV_SIZE];
		int recv_size = ikcp_recv(kcp, buffer, MAX_RECV_SIZE);
		if (recv_size == 0)
		{
			lua_pushstring(L, "");
		}
		else if (recv_size > 0)
		{
			lua_pushlstring(L, buffer, recv_size);
		}
		else
		{
			lua_pushnil(L);
		}
		return 1;
	}
#undef MAX_RECV_SIZE

	static int LkcpInput(lua_State * L)
	{
		ikcpcb * kcp = LkcpCheckKcp(L, 1);
		size_t size;
		const char * data = luaL_checklstring(L, 2, &size);
		int ret_code = ikcp_input(kcp, data, (long)size);
		lua_pushinteger(L, ret_code);
		return 1;
	}

	static int LkcpSend(lua_State * L)
	{
		ikcpcb * kcp = LkcpCheckKcp(L, 1);
		size_t len;
		const char * buffer = luaL_checklstring(L, 2, &len);
		int ret_code = ikcp_send(kcp, buffer, (int)len);
		lua_pushinteger(L, ret_code);
		return 1;
	}

	static int LkcpFlush(lua_State * L)
	{
		ikcpcb * kcp = LkcpCheckKcp(L, 1);
		ikcp_flush(kcp);
		return 0;
	}

	static int LkcpWaitSnd(lua_State * L)
	{
		ikcpcb * kcp = LkcpCheckKcp(L, 1);
		int wait_send_num = ikcp_waitsnd(kcp);
		lua_pushinteger(L, wait_send_num);
		return 1;
	}

	static int LkcpGetConv(lua_State * L)
	{
		ikcpcb * kcp = LkcpCheckKcp(L, 1);
		size_t len;
		const char * buf = luaL_checklstring(L, 2, &len);
		IUINT32 conv = ikcp_getconv(buf);
		lua_pushinteger(L, conv);
		return 1;
	}

	static int LkcpToString(lua_State * L)
	{
		ikcpcb * kcp = LkcpCheckKcp(L, 1);
		lua_pushfstring(L, "ikcpcb: %p", kcp);
		return 1;
	}
}

extern "C"
{
	LUALIB_API int luaopen_lkcp(lua_State * L)
	{
		// lkcp functions
		static const luaL_Reg lkcp_functions[] = 
		{
			// create and release
			{ "create", glory::LkcpCreate },
			{ NULL, NULL }
		};

		// 
		static const luaL_Reg lkcp_methods[] = 
		{
			// set optionas
			{ "nodelay", glory::LkcpNodelay },
			{ "wndsize", glory::LkcpWndsize },
			{ "setmtu", glory::LkcpSetMtu },
			{ "setminrto", glory::LkcpSetMinRto },

			// 
			{ "update", glory::LkcpUpdate },
			{ "check", glory::LkcpCheck },
			{ "recv", glory::LkcpRecv },
			{ "input", glory::LkcpInput },
			{ "send", glory::LkcpSend },
			{ "flush", glory::LkcpFlush },
			{ "waitsnd", glory::LkcpWaitSnd },
			{ "getconv", glory::LkcpGetConv },
			{ NULL, NULL }
		};
		
		// set kcp allocator
		ikcp_allocator(glory::LkcpMalloc, glory::LkcpFree);

		// create a metatable named ikcpcb
		luaL_newmetatable(L, "ikcpcb");
		lua_pushcfunction(L, glory::LkcpToString);
		lua_setfield(L, -2, "__tostring");
		lua_pushcfunction(L, glory::LkcpRelease);
		lua_setfield(L, -2, "__gc");
		lua_newtable(L);
		luaL_setfuncs(L, lkcp_methods, 0);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);

		// registe the lua lib named lkcp
		luaL_newlib(L, lkcp_functions);
		return 1;
	}
}
