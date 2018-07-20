extern "C"
{
#include <amqp_time.h>
}
#include "lrmq.h"
#include "lrmq_queue.cpp"

namespace glory
{
	// Open a Channel
	// return channel, error_msg when fail
	static int LrmqChannelOpen(lua_State * L)
	{
		LuaUserdata connection(NULL, "lrmq_connection");
		int channel_id = 0;
		CheckLuaParam(L, connection, channel_id);
		amqp_connection_state_t connection_state = (amqp_connection_state_t)connection._userdata;

		amqp_channel_open_ok_t *res =
			amqp_channel_open(connection_state, channel_id);
		if (res == NULL)
		{
			return LuaResult(L, LUA_NIL, "ChannelOpen Error! amqp_channel_open fail");
		}
		LrmqChannelData * channel_data = new LrmqChannelData;
		channel_data->_connection_state = connection_state;
		channel_data->_channel_id = channel_id;
		LuaUserdata channel(channel_data, "lrmq_channel");
		return LuaResult(L, channel, LUA_NIL);
	}

	// Close Channel
	// return err_msg when fail
	static int LrmqChannelClose(lua_State * L)
	{
		LuaUserdata channel(NULL, "lrmq_channel");
		CheckLuaParam(L, channel);
		LrmqChannelData * channel_data = (LrmqChannelData *)channel._userdata;
		
		amqp_rpc_reply_t rpc_reply = amqp_channel_close(
			channel_data->_connection_state,
			channel_data->_channel_id,
			0
		);
		delete channel_data;

		if (rpc_reply.reply_type != AMQP_RESPONSE_NORMAL)
		{
			return LuaResult(L, "ChannelClose Error! amqp_channel_close fail");
		}
		
		return LuaResult(L, LUA_NIL);
	}

	static int LrmqChannelToString(lua_State * L)
	{
		LuaUserdata channel(NULL, "lrmq_channel");
		CheckLuaParam(L, channel);
		LrmqChannelData * channel_data = (LrmqChannelData *)channel._userdata;

		return LuaResult(L, LuaString<40>("%s :%p", "lrmq_channel", channel_data));
	}

	static void LrmqInitChannel(lua_State * L)
	{
		LrmqInitQueue(L);
		static const luaL_Reg lrmq_channel_functions[] =
		{
			{ "close", LrmqChannelClose },
			{ "exchange_declare", LrmqExchangeDeclare },
			{ "queue_declare", LrmqQueueDeclare },
			{ NULL, NULL }
		};

		luaL_newmetatable(L, "lrmq_channel");
		lua_pushcfunction(L, LrmqChannelToString);
		lua_setfield(L, -2, "__tostring");
		lua_newtable(L);
		luaL_setfuncs(L, lrmq_channel_functions, 0);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);
	}
}
