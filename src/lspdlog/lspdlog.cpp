extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <spdlog.h>

namespace glory
{
	static void * LspdlogMalloc(size_t size)
	{
		return malloc(size);
	}

	static void LspdlogFree(void * block)
	{
		free(block);
	}

	static const char * const kLspdlogLogTypes[] =
	{
		"stdout", "basic", "daily", "rotating", NULL
	};

	enum LogType
	{
		LT_STDOUT = 0,
		LT_BASIC,
		LT_DAILY,
		LT_ROTATING,
	};

	static const char * const kLspdlogLogLevels[] =
	{
		"trace", "debug", "info", "warn", "err", "critical", "off", NULL
	};

	std::string LspdlogGetTableStringValue(lua_State * L, int index, const char * key, bool optional = false)
	{
		if (index < 0)
		{
			--index;
		}
		lua_pushstring(L, key);
		lua_gettable(L, index);
		std::string value = optional ? luaL_optstring(L , -1, ""): luaL_checkstring(L, -1);
		lua_pop(L, 1);
		return value;
	}

	int LspdlogGetTableOptionValue(lua_State * L, int index, const char * key, const char * def, const char * const * lst)
	{
		if (index < 0)
		{
			--index;
		}
		lua_pushstring(L, key);
		lua_gettable(L, index);
		const char * str_value = lua_tostring(L, -1);
		int value = luaL_checkoption(L, -1, def, lst);
		lua_pop(L, 1);
		return value;
	}

	int LspdlogGetTableIntergerValue(lua_State * L, int index, const char * key, int def = 0)
	{
		if (index < 0)
		{
			--index;
		}
		lua_pushstring(L, key);
		lua_gettable(L, index);
		int value = def;
		if (lua_isinteger(L, -1))
		{
			value = (int)lua_tointeger(L, -1);
		}
		lua_pop(L, 1);
		return value;
	}

	bool LspdlogGetTableBooleanValue(lua_State * L, int index, const char * key, bool def = false)
	{
		if (index < 0)
		{
			--index;
		}
		lua_pushstring(L, key);
		lua_gettable(L, index);
		bool value = def;
		if (lua_isboolean(L, -1))
		{
			value = lua_toboolean(L, -1);
		}
		lua_pop(L, 1);
		return value;
	}

	std::shared_ptr<spdlog::logger> LspdlogCreateLoggerDirectly(lua_State * L, std::string logger_name, LogType log_type)
	{
		std::shared_ptr<spdlog::logger> logger;
		switch (log_type)
		{
		case LT_STDOUT:
		{
			logger = spdlog::stdout_logger_st(logger_name);
		}
			break;
		case LT_BASIC:
		{
			logger = spdlog::basic_logger_st(logger_name, LspdlogGetTableStringValue(L, 1, "file_name"), LspdlogGetTableBooleanValue(L, 1, "truncate"));
		}
			break;
		case LT_DAILY:
		{
			logger = spdlog::daily_logger_st(logger_name, LspdlogGetTableStringValue(L, 1, "file_name"), 
				LspdlogGetTableIntergerValue(L, 1, "hour"), LspdlogGetTableIntergerValue(L, 1, "minute"));
		}
			break;
		case LT_ROTATING:
		{
			int max_file_size = LspdlogGetTableIntergerValue(L, 1, "max_file_size");
			assert(max_file_size > 0);
			int max_files = LspdlogGetTableIntergerValue(L, 1, "max_files");
			assert(max_files > 0);
			logger = spdlog::rotating_logger_st(logger_name, LspdlogGetTableStringValue(L, 1, "file_name"), max_file_size, max_files);
		}
			break;
		default:
			break;
		}
		return logger;
	}

	std::shared_ptr<spdlog::sinks::sink> LspdlogCreateSink(lua_State * L, LogType sink_type)
	{
		std::shared_ptr<spdlog::sinks::sink> sink;
		switch (sink_type)
		{
		case LT_STDOUT:
		{
			sink = std::make_shared<spdlog::sinks::stdout_sink_st>();
		}
			break;
		case LT_BASIC:
		{
			sink = std::make_shared<spdlog::sinks::simple_file_sink_st>(LspdlogGetTableStringValue(L, -1, "file_name"), 
				LspdlogGetTableBooleanValue(L, -1, "truncate"));
		}
			break;
		case LT_DAILY:
		{
			sink = std::make_shared<spdlog::sinks::daily_file_sink_st>(LspdlogGetTableStringValue(L, -1, "file_name"),
				LspdlogGetTableIntergerValue(L, -1, "hour"), LspdlogGetTableIntergerValue(L, -1, "minute"));
		}
			break;
		case LT_ROTATING:
		{
			int max_file_size = LspdlogGetTableIntergerValue(L, -1, "max_file_size");
			assert(max_file_size > 0);
			int max_files = LspdlogGetTableIntergerValue(L, -1, "max_files");
			assert(max_files > 0);
			sink = std::make_shared<spdlog::sinks::rotating_file_sink_st>(LspdlogGetTableStringValue(L, -1, "file_name"), 
				max_file_size, max_files);
		}
			break;
		default:
			break;
		}
		return sink;
	}

	static spdlog::logger * LspdlogCheckLogger(lua_State * L, int index)
	{
		spdlog::logger * logger = *(spdlog::logger **)luaL_checkudata(L, index, "spdlogger");
		luaL_argcheck(L, logger != NULL, index, "Expected spdlogger");
		return logger;
	}

	//	input a config table from lua to create logger
	//	example1:
	//	{
	//		name = "example_logger",
	//		type = "stdout",
	//		level = "trace",
	//		pattern = "%v",
	//	}
	//	exaple2:
	//	{
	//		name = "example_logger",
	//		sinks = 
	//		{
	//			{
	//				type = "stdout",
	//				level = "trace",
	//				pattern = "%v",
	//			},
	//			{
	//				type = "basic",
	//				file_name = "test.log",
	//				level = "trace",
	//				pattern = "%v",
	//				flush_on = "err",
	//				truncate = false,
	//			},
	//		},
	//	}
	static int LspdlogCreateLogger(lua_State * L)
	{
		if (!lua_istable(L, 1))
		{
			lua_pushnil(L);
			return 1;
		}
		std::shared_ptr<spdlog::logger> logger;
		std::vector<std::shared_ptr<spdlog::sinks::sink> > sinks;
		// get logger_name from config
		auto logger_name = LspdlogGetTableStringValue(L, 1, "name");

		// get logger_type
		lua_pushstring(L, "type");
		lua_gettable(L, 1);
		if (!lua_isnil(L, -1))
		{
			// create logger directly
			int log_type = luaL_checkoption(L, -1, NULL, kLspdlogLogTypes);
			lua_pop(L, 1);
			logger = LspdlogCreateLoggerDirectly(L, logger_name, LogType(log_type));
		}
		else
		{
			// create logger by sinks
			lua_pushstring(L, "sinks");
			lua_gettable(L, 1);
			if (!lua_istable(L, -1))
			{
				lua_pushnil(L);
				return 1;
			}
			
			// ergodic all sinks
			lua_pushnil(L);
			while (lua_next(L, -2) != 0)
			{
				if (lua_istable(L, -1))
				{
					// get sink_type
					int sink_type = LspdlogGetTableOptionValue(L, -1, "type", NULL, kLspdlogLogTypes);
					auto sink = LspdlogCreateSink(L, (LogType)sink_type);
					if (sink.get() != NULL)
					{
						sinks.push_back(sink);
					}
				}
				// pop value
				lua_pop(L, 1);
			}
			// pop the last key
			lua_pop(L, 1);

			// pop sinks table
			lua_pop(L, 1);

			if (sinks.empty())
			{
				lua_pushnil(L);
				return 1;
			}
			logger = spdlog::create(logger_name, sinks.begin(), sinks.end());
		}

		if (logger.get() == NULL)
		{
			lua_pushnil(L);
			return 1;
		}

		(*(spdlog::logger **)lua_newuserdata(L, sizeof(void **))) = logger.get();
		luaL_getmetatable(L, "spdlogger");
		lua_setmetatable(L, -2);
		return 1;
	}

	static int LspdlogSetLevel(lua_State * L)
	{
		auto logger = LspdlogCheckLogger(L, 1);
		int log_level = luaL_checkoption(L, 2, NULL, kLspdlogLogLevels);
		logger->set_level((spdlog::level::level_enum)log_level);
		return 0;
	}

	static int LspdlogSetPattern(lua_State * L)
	{
		auto logger = LspdlogCheckLogger(L, 1);
		const char * pattern = luaL_checkstring(L, 2);
		logger->set_pattern(pattern);
		return 0;
	}

	static int LspdlogFlushOn(lua_State * L)
	{
		auto logger = LspdlogCheckLogger(L, 1);
		int log_level = luaL_checkoption(L, 2, NULL, kLspdlogLogLevels);
		logger->flush_on((spdlog::level::level_enum)log_level);
		return 0;
	}

	static int LspdlogLog(lua_State * L)
	{
		auto logger = LspdlogCheckLogger(L, 1);
		int log_level = luaL_checkoption(L, 2, NULL, kLspdlogLogLevels);
		const char * log_str = luaL_checkstring(L, 3);
		logger->log((spdlog::level::level_enum)log_level, log_str);
		return 0;
	}

	static int LspdlogFlush(lua_State * L)
	{
		auto logger = LspdlogCheckLogger(L, 1);
		logger->flush();
		return 0;
	}

	static int LspdlogToString(lua_State * L)
	{
		auto logger = LspdlogCheckLogger(L, 1);
		lua_pushfstring(L, "spdlogger: %p", logger);
		return 1;
	}

	static int LspdlogDestroy(lua_State * L)
	{
		auto logger = LspdlogCheckLogger(L, 1);
		spdlog::drop(logger->name());
		return 0;
	}
}

extern "C"
{
	LUALIB_API int luaopen_lspdlog(lua_State * L)
	{
		// lspdlog functions
		static const luaL_Reg lspdlog_functions[] =
		{
			{"create_logger", glory::LspdlogCreateLogger},
			{NULL, NULL}
		};

		static const luaL_Reg spdlogger_functions[] = 
		{
			{"set_level", glory::LspdlogSetLevel},
			{"set_pattern", glory::LspdlogSetPattern},
			{"flush_on", glory::LspdlogFlushOn},
			{"log", glory::LspdlogLog},
			{"flush", glory::LspdlogFlush},
			{NULL, NULL}
		};

		// create a metatable named spdlogger
		luaL_newmetatable(L, "spdlogger");
		lua_pushcfunction(L, glory::LspdlogToString);
		lua_setfield(L, -2, "__tostring");
		lua_pushcfunction(L, glory::LspdlogDestroy);
		lua_setfield(L, -2, "__gc");
		lua_newtable(L);
		luaL_setfuncs(L, spdlogger_functions, 0);
		lua_setfield(L, -2, "__index");
		lua_pop(L, 1);

		// registe the lua lib named lspdlog
		luaL_newlib(L, lspdlog_functions);
		return 1;
	}
}
