extern "C"
{
#include <amqp_time.h>
}
#include "lrmq.h"

namespace glory
{
	// Publish message to Queue
	// return error_msg when fail
	static int LrmqBasicPublish(lua_State * L)
	{
		LuaUserdata exchange(NULL, "lrmq_exchange");
		const char * routing_key = NULL;
		std::string chunk;
		CheckLuaParam(L, exchange, routing_key, chunk);
		LrmqExchangeData * exchange_data = (LrmqExchangeData *)exchange._userdata;

		amqp_bytes_t msg;
		msg.bytes = (void *)chunk.c_str();
		msg.len = chunk.size();

		amqp_basic_properties_t properties;
		properties._flags = 0;
		properties._flags |= AMQP_BASIC_DELIVERY_MODE_FLAG;
		properties.delivery_mode = AMQP_DELIVERY_NONPERSISTENT;

		int retval = amqp_basic_publish(
			exchange_data->_connection_state,
			exchange_data->_channel_id,
			amqp_cstring_bytes(exchange_data->_exchange.c_str()),
			amqp_cstring_bytes(routing_key),
			/* mandatory=*/1,
			/* immediate=*/0,
			&properties, 
			msg
		);

		if (retval != 0)
		{
			return LuaResult(L, LuaString<100>("BasicPublish Error! amqp_basic_publish fail[%d]", retval));
		}
		return LuaResult(L, LUA_NIL);
	}

	// Basic Consume
	// add a consumer to queue
	// return error_msg when fail
	static int LrmqBasicConsume(lua_State * L)
	{
		LuaUserdata queue(NULL, "lrmq_queue");
		const char * consumer_tag = NULL;
		CheckLuaParam(L, queue, consumer_tag);
		LrmqQueueData * queue_data = (LrmqQueueData *)queue._userdata;

		amqp_basic_consume_ok_t *result = amqp_basic_consume(
			queue_data->_connection_state,
			queue_data->_channel_id,
			amqp_cstring_bytes(queue_data->_queue.c_str()), 
			amqp_cstring_bytes(consumer_tag),
			0 /* no_local */,
			1 /* no_ack */,
			0 /* exclusive */, 
			amqp_empty_table
		);
		if (result == NULL)
		{
			return LuaResult(L, LUA_NIL, "BasicConsume Error! amqp_basic_consume fail");
		}
		LrmqConsumerData * consumer_data = new LrmqConsumerData;
		consumer_data->_connection_state = queue_data->_connection_state;
		consumer_data->_channel_id = queue_data->_channel_id;
		consumer_data->_consumer_tag.assign(consumer_tag);
		LuaUserdata consumer(consumer_data, "lrmq_consumer");
		return LuaResult(L, consumer, LUA_NIL);
	}

	// Basic Cancel
	// return error_msg when fail
	static int LrmqBasicCancel(lua_State * L)
	{
		LuaUserdata consumer(NULL, "lrmq_consumer");
		CheckLuaParam(L, consumer);
		LrmqConsumerData * consumer_data = (LrmqConsumerData *)consumer._userdata;

		amqp_basic_cancel_ok_t *result = amqp_basic_cancel(
			consumer_data->_connection_state, 
			consumer_data->_channel_id, 
			amqp_cstring_bytes(consumer_data->_consumer_tag.c_str())
		);
		delete consumer_data;

		if (result == NULL)
		{
			return LuaResult(L, "BasicCancel Error! amqp_basic_cancel fail");
		}
		return LuaResult(L, LUA_NIL);
	}

	static int LrmqConsumerToString(lua_State * L)
	{
		LuaUserdata consumer(NULL, "lrmq_consumer");
		CheckLuaParam(L, consumer);
		LrmqConsumerData * consumer_data = (LrmqConsumerData *)consumer._userdata;

		return LuaResult(L, LuaString<40>("%s :%p", "lrmq_consumer", consumer_data));
	}

	// Consume Message
	// return message, error_msg when fail
	static int LrmqConsumeMessage(lua_State * L)
	{
		LuaUserdata connection(NULL, "lrmq_connection");
		long long timeout_usec = 0;
		CheckLuaParam(L, connection, timeout_usec);
		amqp_connection_state_t connection_state = (amqp_connection_state_t)connection._userdata;
		
		struct timeval timeout = { timeout_usec / 1000000, timeout_usec % 1000000 };
		amqp_envelope_t envelope;
		amqp_rpc_reply_t rpc_reply = amqp_consume_message(
			connection_state, 
			&envelope, 
			&timeout, 
			0
		);
		if (rpc_reply.reply_type != AMQP_RESPONSE_NORMAL)
		{
			return LuaResult(L, LUA_NIL, LuaString<100>("ConsumeMessage Error! amqp_consume_message error. rpc_reply.reply_type=%d", rpc_reply.reply_type));
		}
		std::string chunk((const char *)envelope.message.body.bytes, envelope.message.body.len);
		amqp_destroy_envelope(&envelope);
		return LuaResult(L, chunk, LUA_NIL);
	}

	// Get one message from queue
	// return msg, err_msg when fail
	static int LrmqBasicGet(lua_State * L)
	{
		LuaUserdata queue(NULL, "lrmq_queue");
		long long timeout_usec = 0;
		CheckLuaParam(L, queue, timeout_usec);
		LrmqQueueData * queue_data = (LrmqQueueData *)queue._userdata;

		struct timeval timeout = { timeout_usec / 1000000, timeout_usec % 1000000 };
		amqp_time_t deadline;
		int time_rc = amqp_time_from_now(&deadline, &timeout);
		if (time_rc != AMQP_STATUS_OK)
		{
			return LuaResult(L, LUA_NIL, LuaString<100>("BasicGet Error ! amqp_time_from_now error time_rc=%d", time_rc));
		}

		amqp_rpc_reply_t rpc_reply;
		do
		{
			rpc_reply = amqp_basic_get(
				queue_data->_connection_state, 
				queue_data->_channel_id,
				amqp_cstring_bytes(queue_data->_queue.c_str()), 
				1 /* no_ack */
			);
		} while (rpc_reply.reply_type == AMQP_RESPONSE_NORMAL &&
			rpc_reply.reply.id == AMQP_BASIC_GET_EMPTY_METHOD &&
			amqp_time_has_past(deadline) == AMQP_STATUS_OK);

		if (rpc_reply.reply_type != AMQP_RESPONSE_NORMAL
			|| rpc_reply.reply.id != AMQP_BASIC_GET_OK_METHOD)
		{
			return LuaResult(L, LUA_NIL, LuaString<100>("BasicGet Error ! amqp_basic_get error rpc_reply.reply_type=%d rpc_reply.reply.id=%d", rpc_reply.reply_type, rpc_reply.reply.id));
		}

		amqp_message_t message;
		rpc_reply = amqp_read_message(
			queue_data->_connection_state,
			queue_data->_channel_id, 
			&message, 
			0);
		if (rpc_reply.reply_type != AMQP_RESPONSE_NORMAL)
		{
			return LuaResult(L, LUA_NIL, LuaString<100>("BasicGet Error ! amqp_read_message error rpc_reply.reply_type=%d", rpc_reply.reply_type));
		}

		std::string msg((const char *)message.body.bytes, message.body.len);
		amqp_destroy_message(&message);
		return LuaResult(L, msg, LUA_NIL);
	}

	static void LrmqInitConsumer(lua_State * L)
	{
		static const luaL_Reg lrmq_queue_functions[] =
		{
			{ "cancel", LrmqBasicCancel },
			{ NULL, NULL }
		};

		luaL_newmetatable(L, "lrmq_consumer");
		lua_pushcfunction(L, LrmqConsumerToString);
		lua_setfield(L, -2, "__tostring");
		lua_newtable(L);
		luaL_setfuncs(L, lrmq_queue_functions, 0);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);
	}
}
