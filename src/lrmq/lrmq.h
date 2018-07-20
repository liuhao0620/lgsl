#ifndef __GLORY_LRMQ_LRMQ_H__
#define __GLORY_LRMQ_LRMQ_H__
extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <amqp.h>
}
#include "lua_wrapper.h"

namespace glory
{
	struct LrmqConsumerData
	{
		amqp_connection_state_t _connection_state = NULL;
		int _channel_id = 0;
		std::string _consumer_tag;
	};

	struct LrmqExchangeData
	{
		amqp_connection_state_t _connection_state = NULL;
		int _channel_id = 0;
		std::string _exchange;
	};

	struct LrmqQueueData
	{
		amqp_connection_state_t _connection_state = NULL;
		int _channel_id = 0;
		std::string _queue;
	};

	struct LrmqChannelData
	{
		amqp_connection_state_t _connection_state = NULL;
		int _channel_id = 0;
	};

	void LrmqCheckTimeVal(lua_State * L, int index, timeval & time);
}

#endif // !__GLORY_LRMQ_LRMQ_H__
