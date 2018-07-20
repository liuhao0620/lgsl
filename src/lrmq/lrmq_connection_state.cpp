extern "C"
{
#include <amqp_tcp_socket.h>
}
#include "lrmq_channel.cpp"

namespace glory
{
	// Create a Connection
	// return connection, err_msg if fail
	static int LrmqConnectionOpen(lua_State * L)
	{
		const char * host = NULL;
		int port = 0;
		const char * vhost = NULL;
		int channel_max = 0;
		const char * username = NULL;
		const char * password = NULL;

		CheckLuaParam(L, host, port, vhost, channel_max, username, password);

		amqp_connection_state_t connection_state = amqp_new_connection();
		amqp_socket_t *socket = amqp_tcp_socket_new(connection_state);
		if (socket == NULL)
		{
			return LuaResult(L, LUA_NIL, "ConnectionCreate Error! amqp_tcp_socket_new fail");
		}

		int rc = amqp_socket_open(socket, host, port);
		if (rc != AMQP_STATUS_OK)
		{
			return LuaResult(L, LUA_NIL, "ConnectionCreate Error! amqp_socket_open fail");
		}

		amqp_rpc_reply_t rpc_reply = amqp_login(
			connection_state, vhost, channel_max, AMQP_DEFAULT_FRAME_SIZE,
			AMQP_DEFAULT_HEARTBEAT, AMQP_SASL_METHOD_PLAIN, username, password);
		if (rpc_reply.reply_type != AMQP_RESPONSE_NORMAL)
		{
			return LuaResult(L, LUA_NIL, "ConnectionCreate Error! amqp_login fail");
		}
		return LuaResult(L, LuaUserdata(connection_state, "lrmq_connection"), LUA_NIL);
	}

	// return err_msg when fail
	static int LrmqConnectionClose(lua_State * L)
	{
		LuaUserdata connection(NULL, "lrmq_connection");
		CheckLuaParam(L, connection);
		amqp_connection_state_t connection_state = (amqp_connection_state_t)connection._userdata;
		amqp_rpc_reply_t rpc_reply =
			amqp_connection_close(connection_state, AMQP_REPLY_SUCCESS);
		if (rpc_reply.reply_type != AMQP_RESPONSE_NORMAL)
		{
			return LuaResult(L, "ConnectionDestory Error! amqp_connection_close fail");
		}
		int rc = amqp_destroy_connection(connection_state);
		if (rc != AMQP_STATUS_OK)
		{
			return LuaResult(L, "ConnectionDestory Error! amqp_destroy_connection fail");
		}
		return LuaResult(L, LUA_NIL);
	}

	static int LrmqConnectionToString(lua_State * L)
	{
		LuaUserdata connection(NULL, "lrmq_connection");
		CheckLuaParam(L, connection);
		amqp_connection_state_t connection_state = (amqp_connection_state_t)connection._userdata;
		return LuaResult(L, LuaString<40>("%s: %p", "lrmq_connect_state", connection_state));
	}

	static void LrmqConnectionInit(lua_State * L)
	{
		LrmqInitChannel(L);
		static const luaL_Reg lrmq_connection_functions[] =
		{
			{ "close", LrmqConnectionClose },
			{ "channel_open",LrmqChannelOpen },
			{ "consume_message", LrmqConsumeMessage },
			{ NULL, NULL }
		};

		luaL_newmetatable(L, "lrmq_connection");
		lua_pushcfunction(L, LrmqConnectionToString);
		lua_setfield(L, -2, "__tostring");
		lua_newtable(L);
		luaL_setfuncs(L, lrmq_connection_functions, 0);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);
	}
}
