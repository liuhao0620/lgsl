#include "lrmq.h"
#include "lrmq_basic.cpp"

namespace glory
{
	// Declare a Exchange
	// return exchange, err_msg when fail
	static int LrmqExchangeDeclare(lua_State * L)
	{
		LuaUserdata channel(NULL, "lrmq_channel");
		const char * exchange_name = NULL;
		const char * type = NULL;
		CheckLuaParam(L, channel, exchange_name, type);
		LrmqChannelData * channel_data = (LrmqChannelData *)channel._userdata;

		amqp_exchange_declare_ok_t * result = amqp_exchange_declare(
			channel_data->_connection_state,
			channel_data->_channel_id,
			amqp_cstring_bytes(exchange_name),
			amqp_cstring_bytes(type),
			0 /* passive */,
			0 /* durable */,
			0 /* auto_delete */,
			0 /* internal */,
			amqp_empty_table
		);
		if (result == NULL)
		{
			return LuaResult(L, LUA_NIL, "ExchangeDeclare Error! amqp_exchange_declare fail");
		}

		LrmqExchangeData * exchange_data = new LrmqExchangeData;
		exchange_data->_connection_state = channel_data->_connection_state;
		exchange_data->_channel_id = channel_data->_channel_id;
		exchange_data->_exchange.assign(exchange_name);
		LuaUserdata exchange(exchange_data, "lrmq_exchange");
		return LuaResult(L, exchange, LUA_NIL);
	}

	// Delete Exchange
	// return err_msg when fail
	static int LrmqExchangeDelete(lua_State * L)
	{
		LuaUserdata exchange(NULL, "lrmq_exchange");
		CheckLuaParam(L, exchange);
		LrmqExchangeData * exchange_data = (LrmqExchangeData *)exchange._userdata;

		amqp_exchange_delete_ok_t * result = amqp_exchange_delete(
			exchange_data->_connection_state,
			exchange_data->_channel_id,
			amqp_cstring_bytes(exchange_data->_exchange.c_str()),
			0 /* if_unused */
		);
		delete exchange_data;

		if (result == NULL)
		{
			return LuaResult(L, "ExchangeDelete Error! amqp_exchange_delete fail");
		}
		return LuaResult(L, LUA_NIL);
	}

	static int LrmqExchangeToString(lua_State * L)
	{
		LuaUserdata exchange(NULL, "lrmq_exchange");
		CheckLuaParam(L, exchange);
		LrmqExchangeData * exchange_data = (LrmqExchangeData *)exchange._userdata;

		return LuaResult(L, LuaString<40>("%s :%p", "lrmq_exchange", exchange_data));
	}

	// Bind exchange_src to exchange_dst
	// return err_msg when fail
	static int LrmqExchangeBind(lua_State * L)
	{
		LuaUserdata exchange_dst(NULL, "lrmq_exchange");
		LuaUserdata exchange_src(NULL, "lrmq_exchange");
		const char * routing_key = NULL;
		CheckLuaParam(L, exchange_dst, exchange_src, routing_key);
		LrmqExchangeData * exchange_dst_data = (LrmqExchangeData *)exchange_dst._userdata;
		LrmqExchangeData * exchange_src_data = (LrmqExchangeData *)exchange_src._userdata;
		if (exchange_dst_data->_connection_state != exchange_src_data->_connection_state
			|| exchange_dst_data->_channel_id != exchange_src_data->_channel_id)
		{
			return LuaResult(L, "ExchangeBind Error! connection or channel is not equel");
		}

		amqp_exchange_bind_ok_t * result = amqp_exchange_bind(
			exchange_dst_data->_connection_state,
			exchange_dst_data->_channel_id,
			amqp_cstring_bytes(exchange_dst_data->_exchange.c_str()),
			amqp_cstring_bytes(exchange_src_data->_exchange.c_str()),
			amqp_cstring_bytes(routing_key),
			amqp_empty_table
		);
		if (result == NULL)
		{
			return LuaResult(L, "ExchangeBind Error! amqp_exchange_bind fail");
		}
		return LuaResult(L, LUA_NIL);
	}

	// UnBind exchange_src from exchange_dst
	// return err_msg when fail
	static int LrmqExchangeUnbind(lua_State * L)
	{
		LuaUserdata exchange_dst(NULL, "lrmq_exchange");
		LuaUserdata exchange_src(NULL, "lrmq_exchange");
		const char * routing_key = NULL;
		CheckLuaParam(L, exchange_dst, exchange_src, routing_key);
		LrmqExchangeData * exchange_dst_data = (LrmqExchangeData *)exchange_dst._userdata;
		LrmqExchangeData * exchange_src_data = (LrmqExchangeData *)exchange_src._userdata;
		if (exchange_dst_data->_connection_state != exchange_src_data->_connection_state
			|| exchange_dst_data->_channel_id != exchange_src_data->_channel_id)
		{
			return LuaResult(L, "ExchangeUnbind Error! connection or channel is not equel");
		}

		amqp_exchange_unbind_ok_t * result = amqp_exchange_unbind(
			exchange_dst_data->_connection_state,
			exchange_dst_data->_channel_id,
			amqp_cstring_bytes(exchange_dst_data->_exchange.c_str()),
			amqp_cstring_bytes(exchange_src_data->_exchange.c_str()),
			amqp_cstring_bytes(routing_key),
			amqp_empty_table
		);
		if (result == NULL)
		{
			return LuaResult(L, "ExchangeUnbind Error! amqp_exchange_unbind fail");
		}
		return LuaResult(L, LUA_NIL);
	}

	static void LrmqInitExchange(lua_State * L)
	{
		LrmqInitConsumer(L);
		static const luaL_Reg lrmq_exchange_functions[] =
		{
			{ "delete", LrmqExchangeDelete },
			{ "bind", LrmqExchangeBind },
			{ "unbind", LrmqExchangeUnbind },
			{ "publish", LrmqBasicPublish },
			{ NULL, NULL }
		};

		luaL_newmetatable(L, "lrmq_exchange");
		lua_pushcfunction(L, LrmqExchangeToString);
		lua_setfield(L, -2, "__tostring");
		lua_newtable(L);
		luaL_setfuncs(L, lrmq_exchange_functions, 0);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);
	}
}
