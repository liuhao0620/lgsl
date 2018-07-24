#ifndef __GLORY_LUA_WRAPPER_LUA_WRAPPER_H__
#define __GLORY_LUA_WRAPPER_LUA_WRAPPER_H__
extern "C"
{
#include <lua.h>
}
#include <string>

#ifndef CHECK_LUA_INTEGER
#define CHECK_LUA_INTEGER(integer_type) \
	static void CheckLuaParam(lua_State * L, int index, integer_type & val, bool can_nil) \
	{ \
		if (can_nil && lua_isnoneornil(L, index)) \
		{ \
			return; \
		} \
		val = (integer_type)luaL_checkinteger(L, index); \
	}
#endif // !CHECK_LUA_INTEGER

#ifndef CHECK_LUA_NUMBER
#define CHECK_LUA_NUMBER(number_type) \
	static void CheckLuaParam(lua_State * L, int index, number_type & val, bool can_nil) \
	{ \
		if (can_nil && lua_isnoneornil(L, index)) \
		{ \
			return; \
		} \
		val = (number_type)luaL_checknumber(L, index); \
	}
#endif // !CHECK_LUA_NUMBER

#ifndef PUSH_LUA_INTEGER
#define PUSH_LUA_INTEGER(integer_type) \
	static void PushLuaResult(lua_State * L, integer_type result) \
	{ \
		lua_pushinteger(L, (lua_Integer)result); \
	}
#endif // !PUSH_LUA_INTEGER

#ifndef PUSH_LUA_NUMBER
#define PUSH_LUA_NUMBER(number_type) \
	static void PushLuaResult(lua_State * L, number_type result) \
	{ \
		lua_pushnumber(L, (lua_Number)result); \
	}
#endif // !PUSH_LUA_NUMBER

namespace glory
{
	static struct LuaNil
	{

	} LUA_NIL;

	struct LuaUserdata
	{
		void * _userdata = NULL;
		std::string _tname;

		LuaUserdata() {}
		LuaUserdata(void * userdata) : _userdata(userdata) {}
		LuaUserdata(void * userdata, const char * tname) : _userdata(userdata), _tname(tname) {}
	};

	//struct LuaLstring
	//{
	//	const char * _buf = NULL;
	//	size_t _len = 0;

	//	LuaLstring() {}
	//	LuaLstring(const char * buf, size_t len) : _buf(buf), _len(len) {}
	//};

	// check lua params
	static void CheckLuaParam(lua_State * L, int index, LuaNil & val, bool can_nil)
	{

	}

	static void CheckLuaParam(lua_State * L, int index, LuaUserdata & val, bool can_nil)
	{
		if (can_nil && lua_isnoneornil(L, index))
		{
			return;
		}
		if (val._tname.empty())
		{
			val._userdata = *(void **)lua_touserdata(L, index);
		}
		else
		{
			val._userdata = *((void **)luaL_checkudata(L, index, val._tname.c_str()));
			luaL_argcheck(L, val._userdata != NULL, index, (std::string("Expected ") + val._tname).c_str());
		}
	}

	//static void CheckLuaParam(lua_State * L, int index, LuaLstring & val)
	//{
	//	val._buf = luaL_checklstring(L, index, &val._len);
	//}

	static void CheckLuaParam(lua_State * L, int index, std::string & val, bool can_nil)
	{
		if (can_nil && lua_isnoneornil(L, index))
		{
			return;
		}
		size_t len = 0;
		const char * buf = luaL_checklstring(L, index, &len);
		val.assign(buf, len);
	}

	static void CheckLuaParam(lua_State * L, int index, const char * & val, bool can_nil)
	{
		if (can_nil && lua_isnoneornil(L, index))
		{
			return;
		}
		val = luaL_checkstring(L, index);
	}

	static void CheckLuaParam(lua_State * L, int index, bool & val, bool can_nil)
	{
		if (can_nil && lua_isnoneornil(L, index))
		{
			return;
		}
		luaL_argcheck(L, lua_isboolean(L, index), index, "Need a Boolean");
		val = lua_toboolean(L, index) != 0;
	}

	CHECK_LUA_INTEGER(char)
	CHECK_LUA_INTEGER(unsigned char)
	CHECK_LUA_INTEGER(short)
	CHECK_LUA_INTEGER(unsigned short)
	CHECK_LUA_INTEGER(int)
	CHECK_LUA_INTEGER(unsigned int)
	CHECK_LUA_INTEGER(long long)
	CHECK_LUA_INTEGER(unsigned long long)
	CHECK_LUA_NUMBER(float)
	CHECK_LUA_NUMBER(double)

	template <bool CAN_NIL, int MINPN, int N, typename T>
	void CheckLuaParam(lua_State * L, T&& param)
	{
		CheckLuaParam(L, N, param, (CAN_NIL && (MINPN <= N)));
	}
	template <bool CAN_NIL, int MINPN, int N, class T, typename ...PTS>
	void CheckLuaParam(lua_State * L, T&& param, PTS&& ...params)
	{
		CheckLuaParam(L, N, param, (CAN_NIL && (MINPN <= N)));
		CheckLuaParam<CAN_NIL, MINPN, N + 1>(L, std::forward<PTS>(params)...);
	}
	template <bool CAN_NIL, int MINPN/*min param num*/, class T, typename ...PTS>
	void CheckLuaParam(lua_State * L, T&& param, PTS&& ...params)
	{
		CheckLuaParam<CAN_NIL, MINPN, 1>(L, param, std::forward<PTS>(params)...);
	}
	template <class T, typename ...PTS>
	void CheckLuaParam(lua_State * L, T&& param, PTS&& ...params)
	{
		CheckLuaParam<false, 0, 1>(L, param, std::forward<PTS>(params)...);
	}

	// return lua results
	static void PushLuaResult(lua_State * L, const LuaNil & result)
	{
		lua_pushnil(L);
	}

	static void PushLuaResult(lua_State * L, const LuaUserdata & result)
	{
		*(void **)lua_newuserdata(L, sizeof(void **)) = result._userdata;
		if (!result._tname.empty())
		{
			luaL_getmetatable(L, result._tname.c_str());
			lua_setmetatable(L, -2);
		}
	}

	//static void PushLuaResult(lua_State * L, const LuaLstring & result)
	//{
	//	lua_pushlstring(L, result._buf, result._len);
	//}

	static void PushLuaResult(lua_State * L, const std::string & result)
	{
		lua_pushlstring(L, result.c_str(), result.size());
	}

	static void PushLuaResult(lua_State * L, const char * result)
	{
		lua_pushstring(L, result);
	}
	
	static void PushLuaResult(lua_State * L, bool result)
	{
		lua_pushboolean(L, result);
	}

	PUSH_LUA_INTEGER(char)
	PUSH_LUA_INTEGER(unsigned char)
	PUSH_LUA_INTEGER(short)
	PUSH_LUA_INTEGER(unsigned short)
	PUSH_LUA_INTEGER(int)
	PUSH_LUA_INTEGER(unsigned int)
	PUSH_LUA_INTEGER(long long)
	PUSH_LUA_INTEGER(unsigned long long)
	PUSH_LUA_NUMBER(float)
	PUSH_LUA_NUMBER(double)

	template <typename T>
	int LuaResult(lua_State * L, const T& result)
	{
		PushLuaResult(L, result);
		return 1;
	}

	template<typename T, typename ...RTS>
	int LuaResult(lua_State * L, const T& result, const RTS& ...results)
	{
		LuaResult(L, result);
		return LuaResult(L, results...) + 1;
	}

	static int LuaResult(lua_State * L)
	{
		return 0;
	}

	// format string
	template <int MaxN, typename ...ARGS>
	static std::string LuaString(const char * fmt, ARGS ...args)
	{
		char temp[MaxN];
		memset(temp, 0, MaxN);
		snprintf(temp, MaxN - 1, fmt, args...);
		return std::string(temp);
	}
}
#endif // !__GLORY_LUA_WRAPPER_LUA_WRAPPER_H__
