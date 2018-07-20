#include "lrmq.h"
#include "lrmq_exchange.cpp"

namespace glory
{
	// Declare a Queue
	// return queue, error_msg when fail
	static int LrmqQueueDeclare(lua_State * L)
	{
		LuaUserdata channel(NULL, "lrmq_channel");
		const char * queue_name = NULL;
		CheckLuaParam(L, channel, queue_name);
		LrmqChannelData * channel_data = (LrmqChannelData *)channel._userdata;

		amqp_queue_declare_ok_t *res = amqp_queue_declare(
			channel_data->_connection_state, 
			channel_data->_channel_id, 
			amqp_cstring_bytes(queue_name),
			/*passive*/ 0,
			/*durable*/ 0,
			/*exclusive*/ 0,
			/*auto_delete*/ 0, amqp_empty_table);
		if (res == NULL)
		{
			return LuaResult(L, LUA_NIL, "QueueDeclare Error! amqp_queue_declare fail");
		}

		LrmqQueueData * queue_data = new LrmqQueueData;
		queue_data->_connection_state = channel_data->_connection_state;
		queue_data->_channel_id = channel_data->_channel_id;
		queue_data->_queue.assign(queue_name);
		LuaUserdata queue(queue_data, "lrmq_queue");
		return LuaResult(L, queue, LUA_NIL);
	}

	// Delete a Queue
	// return err_msg if fail
	static int LrmqQueueDelete(lua_State * L)
	{
		LuaUserdata queue(NULL, "lrmq_queue");
		CheckLuaParam(L, queue);
		LrmqQueueData * queue_data = (LrmqQueueData *)queue._userdata;

		amqp_queue_delete_ok_t * result = amqp_queue_delete(
			queue_data->_connection_state, 
			queue_data->_channel_id,
			amqp_cstring_bytes(queue_data->_queue.c_str()),
			0 /* if_unused */,
			0 /* if_empty */
		);
		delete queue_data;

		if (result == NULL)
		{
			return LuaResult(L, "QueueDelete Error! amqp_queue_delete fail");
		}
		return LuaResult(L, LUA_NIL);
	}

	static int LrmqQueueToString(lua_State * L)
	{
		LuaUserdata queue(NULL, "lrmq_queue");
		CheckLuaParam(L, queue);
		LrmqQueueData * queue_data = (LrmqQueueData *)queue._userdata;

		return LuaResult(L, LuaString<40>("%s :%p", "lrmq_queue", queue_data));
	}

	// Purge Queue
	// return err_msg if fail
	static int LrmqQueuePurge(lua_State * L)
	{
		LuaUserdata queue(NULL, "lrmq_queue");
		CheckLuaParam(L, queue);
		LrmqQueueData * queue_data = (LrmqQueueData *)queue._userdata;

		amqp_queue_purge_ok_t * result = amqp_queue_purge(
			queue_data->_connection_state,
			queue_data->_channel_id,
			amqp_cstring_bytes(queue_data->_queue.c_str())
		);

		if (result == NULL)
		{
			return LuaResult(L, "QueuePurge Error! amqp_queue_purge fail");
		}
		return LuaResult(L, LUA_NIL);
	}

	// Bind queue to exchange
	// return err_msg when fail
	static int LrmqQueueBind(lua_State * L)
	{
		LuaUserdata queue(NULL, "lrmq_queue");
		LuaUserdata exchange(NULL, "lrmq_exchange");
		const char * routing_key = NULL;
		CheckLuaParam(L, queue, exchange, routing_key);
		LrmqQueueData * queue_data = (LrmqQueueData *)queue._userdata;
		LrmqExchangeData * exchange_data = (LrmqExchangeData *)exchange._userdata;

		if (queue_data->_connection_state != exchange_data->_connection_state
			|| queue_data->_channel_id != exchange_data->_channel_id)
		{
			return LuaResult(L, "QueueBind Error! connection or channel is not equel");
		}
		amqp_queue_bind_ok_t * result = amqp_queue_bind(
			queue_data->_connection_state,
			queue_data->_channel_id,
			amqp_cstring_bytes(queue_data->_queue.c_str()),
			amqp_cstring_bytes(exchange_data->_exchange.c_str()),
			amqp_cstring_bytes(routing_key),
			amqp_empty_table
		);
		if (result == NULL)
		{
			return LuaResult(L, "QueueBind Error! amqp_queue_bind fail");
		}
		return LuaResult(L, LUA_NIL);
	}

	// Unbind queue from exchange
	// return err_msg when fail
	static int LrmqQueueUnbind(lua_State * L)
	{
		LuaUserdata queue(NULL, "lrmq_queue");
		LuaUserdata exchange(NULL, "lrmq_exchange");
		const char * routing_key = NULL;
		CheckLuaParam(L, queue, exchange, routing_key);
		LrmqQueueData * queue_data = (LrmqQueueData *)queue._userdata;
		LrmqExchangeData * exchange_data = (LrmqExchangeData *)exchange._userdata;

		if (queue_data->_connection_state != exchange_data->_connection_state
			|| queue_data->_channel_id != exchange_data->_channel_id)
		{
			return LuaResult(L, "QueueUnbind Error! connection or channel is not equel");
		}
		amqp_queue_unbind_ok_t * result = amqp_queue_unbind(
			queue_data->_connection_state,
			queue_data->_channel_id,
			amqp_cstring_bytes(queue_data->_queue.c_str()),
			amqp_cstring_bytes(exchange_data->_exchange.c_str()),
			amqp_cstring_bytes(routing_key),
			amqp_empty_table
		);
		if (result == NULL)
		{
			return LuaResult(L, "QueueUnbind Error! amqp_queue_bind fail");
		}
		return LuaResult(L, LUA_NIL);
	}

	static void LrmqInitQueue(lua_State * L)
	{
		LrmqInitExchange(L);
		static const luaL_Reg lrmq_queue_functions[] =
		{
			{ "delete", LrmqQueueDelete },
			{ "purge", LrmqQueuePurge },
			{ "bind", LrmqQueueBind },
			{ "unbind", LrmqQueueUnbind },
			{ "consume", LrmqBasicConsume },
			{ "get", LrmqBasicGet },
			{ NULL, NULL }
		};

		luaL_newmetatable(L, "lrmq_queue");
		lua_pushcfunction(L, LrmqQueueToString);
		lua_setfield(L, -2, "__tostring");
		lua_newtable(L);
		luaL_setfuncs(L, lrmq_queue_functions, 0);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);
	}
}