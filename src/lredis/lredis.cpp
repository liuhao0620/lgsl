extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <hiredis.h>
}
#include <lua_wrapper.h>
#ifdef WIN32
#include <WinSock2.h>
#else
#include <unistd.h>
#endif // WIN32


namespace glory
{
	// Connect redis server
	// return connection, err_msg when fail
	static int LredisConnect(lua_State * L)
	{
		const char * ip = NULL;
		int port = 0;
		long long timeout_val = 0;
		CheckLuaParam<true, 2>(L, ip, port, timeout_val);

		redisContext * redis_ctx = NULL;
		if (timeout_val == 0)
		{
			redis_ctx = redisConnect(ip, port);
		}
		else
		{
			struct timeval timeout = { timeout_val / 1000000, timeout_val % 1000000 };
			redis_ctx = redisConnectWithTimeout(ip, port, timeout);
		}
		if (redis_ctx == NULL)
		{
			return LuaResult(L, LUA_NIL, "Connection fail");
		}
		if (redis_ctx->err)
		{
			return LuaResult(L, LUA_NIL, LuaString<100>("Connection error : err[%d], errstr[%s]", redis_ctx->err, redis_ctx->errstr));
		}

		LuaUserdata context(redis_ctx, "redis_context");
		return LuaResult(L, context, LUA_NIL);
	}

	static int LredisContextClose(lua_State * L)
	{
		LuaUserdata context(NULL, "redis_context");
		CheckLuaParam(L, context);
		redisContext * redis_ctx = (redisContext *)context._userdata;

		redisFree(redis_ctx);

		return LuaResult(L);
	}

	// return str
	static int LredisContextToString(lua_State * L)
	{
		LuaUserdata context(NULL, "redis_context");
		CheckLuaParam(L, context);
		redisContext * redis_ctx = (redisContext *)context._userdata;

		return LuaResult(L, LuaString<40>("%s : %p", "redis_context", redis_ctx));
	}

	// set read/write timeout on a blocking socket
	// return err_msg when fail
	static int LredisSetTimeout(lua_State * L)
	{
		LuaUserdata context(NULL, "redis_context");
		long long timeout_val = 0;
		CheckLuaParam(L, context, timeout_val);
		redisContext * redis_ctx = (redisContext *)context._userdata;

		struct timeval timeout = { timeout_val / 1000000, timeout_val % 1000000 };
		int result = redisSetTimeout(redis_ctx, timeout);
		if (result == REDIS_ERR)
		{
			return LuaResult(L, "redisSetTimeout return error");
		}
		return LuaResult(L, LUA_NIL);
	}

	static std::string LredisParseReply(lua_State * L, redisReply * reply)
	{
		if (reply == NULL)
		{
			LuaResult(L, LUA_NIL);
			return "redisCommand return NULL";
		}

		switch (reply->type)
		{
		case REDIS_REPLY_STRING:
		{
			std::string result(reply->str, reply->len);
			LuaResult(L, result);
		}
		break;
		case REDIS_REPLY_ARRAY:
		{
			lua_newtable(L);
			for (int i = 0; i < reply->elements; ++i)
			{
				lua_pushinteger(L, i + 1);
				LredisParseReply(L, reply->element[i]);
				lua_settable(L, -3);
			}
		}
		break;
		case REDIS_REPLY_INTEGER:
		{
			LuaResult(L, reply->integer);
		}
		break;
		case REDIS_REPLY_NIL:
		{
			LuaResult(L, LUA_NIL);
		}
		break;
		case REDIS_REPLY_STATUS:
		{
			std::string result(reply->str, reply->len);
			LuaResult(L, result);
		}
		break;
		case REDIS_REPLY_ERROR:
		{
			LuaResult(L, LUA_NIL);
			return std::string(reply->str, reply->len);
		}
		break;
		default:
		{
			return "undefine reply type";
		}
			break;
		}
		return "";
	}

	// execute a redis command
	// return result, err_msg when fail
	static int LredisCommand(lua_State * L)
	{
		LuaUserdata context(NULL, "redis_context");
		const char * cmd_str = NULL;
		CheckLuaParam(L, context, cmd_str);
		redisContext * redis_ctx = (redisContext *)context._userdata;

		redisReply * reply = (redisReply *)redisCommand(redis_ctx, cmd_str);
		std::string err_msg = LredisParseReply(L, reply);
		freeReplyObject(reply);
		if (err_msg.empty())
		{
			return LuaResult(L, LUA_NIL) + 1;
		}
		return LuaResult(L, err_msg) + 1;
	}

	static void InitRedisContext(lua_State * L)
	{
		// redis_context functions
		static const luaL_Reg redis_context_functions[] =
		{
			{ "close", LredisContextClose },
			{ "set_timeout", LredisSetTimeout },
			{ "command", LredisCommand },
			{ NULL, NULL }
		};
		luaL_newmetatable(L, "redis_context");
		lua_pushcfunction(L, LredisContextToString);
		lua_setfield(L, -2, "__tostring");
		lua_newtable(L);
		luaL_setfuncs(L, redis_context_functions, 0);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);
	}
}

extern "C"
{
	LUALIB_API int luaopen_lredis(lua_State * L)
	{
#ifdef WIN32
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif // WIN32

		glory::InitRedisContext(L);
		// lredis functions
		static const luaL_Reg lredis_functions[] =
		{
			{ "connect", glory::LredisConnect },
			{ NULL, NULL }
		};

		luaL_newlib(L, lredis_functions);
		return 1;
	}
}
