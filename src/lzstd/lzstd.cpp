extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <zstd.h>
#include <string>
#include <assert.h>

#define DST_CAPACITY_MAX (1024)

namespace glory
{
	enum MetatableName
	{
		MN_ZSTD_CCTX = 0,
		MN_ZSTD_DCTX,
		MN_ZSTD_CDICT,
		MN_ZSTD_DDICT,
	};

	static const char * const kMetatableNames[] = 
	{
		"zstd_cctx", 
		"zstd_dctx",
		"zstd_cdict",
		"zstd_ddict",
	};

	template <class TableType, int metatable_name_index>
	class LuaToString
	{
	public:
		static int Func(lua_State * L)
		{
			//static_assert(metatable_name_index < sizeof(kMetatableNames) / sizeof(const char *));
			TableType * user_data = (TableType *)LzstdCheckUserdata(L, 1, kMetatableNames[metatable_name_index]);
			lua_pushfstring(L, "%s: %p", kMetatableNames[metatable_name_index], user_data);
			return 1;
		}
	};

	template <class TableType, int metatable_name_index>
	void LzstdNewMetatable(lua_State * L, int(*gc_func)(lua_State *),
		int(*tostring_func)(lua_State*), const luaL_Reg * funcs)
	{
		assert(metatable_name_index < sizeof(kMetatableNames) / sizeof(const char *));
		if (tostring_func == NULL)
		{
			tostring_func = LuaToString<TableType, metatable_name_index>::Func;
		}
		luaL_newmetatable(L, kMetatableNames[metatable_name_index]);
		lua_pushcfunction(L, tostring_func);
		lua_setfield(L, -2, "__tostring");
		if (gc_func != NULL)
		{
			lua_pushcfunction(L, gc_func);
			lua_setfield(L, -2, "__gc");
		}
		if (funcs != NULL)
		{
			lua_newtable(L);
			luaL_setfuncs(L, funcs, 0);
			lua_setfield(L, -2, "__index");
		}
		lua_pop(L, 1);
	}

	void LzstdNewUserdata(lua_State * L, void * data, const char * metatable_name)
	{
		*(void **)lua_newuserdata(L, sizeof(void **)) = data;
		if (metatable_name != NULL)
		{
			luaL_getmetatable(L, metatable_name);
			lua_setmetatable(L, -2);
		}
	}

	void * LzstdCheckUserdata(lua_State * L, int index, const char * metatable_name)
	{
		void * data = NULL;
		if (metatable_name != NULL)
		{
			data = *((void **)luaL_checkudata(L, index, metatable_name));
			luaL_argcheck(L, data != NULL, index, (std::string("Expected ") + metatable_name).c_str());
		}
		else
		{
			data = *(void **)lua_touserdata(L, index);
		}
		return data;
	}

	static int LzstdCompress(lua_State * L)
	{
		static char dst[DST_CAPACITY_MAX];
		size_t len = 0;
		const char * buf = luaL_checklstring(L, 1, &len);
		int compression_level = ZSTD_CLEVEL_DEFAULT;
		if (!lua_isnoneornil(L, 2))
		{
			compression_level = (int)luaL_checkinteger(L, 2);
		}
		size_t res = ZSTD_compress(dst, DST_CAPACITY_MAX, buf, len, compression_level);
		if (ZSTD_isError(res))
		{
			lua_pushnil(L);
			return 1;
		}
		lua_pushlstring(L, dst, res);
		return 1;
	}

	static int LzstdDecompress(lua_State * L)
	{
		static char dst[DST_CAPACITY_MAX];
		size_t len = 0;
		const char * buf = luaL_checklstring(L, 1, &len);
		size_t res = ZSTD_decompress(dst, DST_CAPACITY_MAX, buf, len);
		if (ZSTD_isError(res))
		{
			lua_pushnil(L);
			return 1;
		}
		lua_pushlstring(L, dst, res);
		return 1;
	}

	static int LzstdGetFrameContentSize(lua_State * L)
	{
		size_t len = 0;
		const char * buf = luaL_checklstring(L, 1, &len);
		unsigned long long res = ZSTD_getFrameContentSize(buf, len);
		lua_pushinteger(L, res);
		return 1;
	}

	static int LzstdCreateCCtx(lua_State * L)
	{
		ZSTD_CCtx * cctx = ZSTD_createCCtx();
		LzstdNewUserdata(L, cctx, kMetatableNames[MN_ZSTD_CCTX]);
		return 1;
	}

	static int LzstdFreeCCtx(lua_State * L)
	{
		ZSTD_CCtx * cctx = (ZSTD_CCtx *)LzstdCheckUserdata(L, 1, kMetatableNames[MN_ZSTD_CCTX]);
		ZSTD_freeCCtx(cctx);
		return 0;
	}

	static int LzstdCompressCCtx(lua_State * L)
	{
		static char dst[DST_CAPACITY_MAX];
		ZSTD_CCtx * cctx = (ZSTD_CCtx *)LzstdCheckUserdata(L, 1, kMetatableNames[MN_ZSTD_CCTX]);
		size_t len = 0;
		const char * buf = luaL_checklstring(L, 2, &len);
		int compression_level = ZSTD_CLEVEL_DEFAULT;
		if (!lua_isnoneornil(L, 3))
		{
			compression_level = (int)luaL_checkinteger(L, 3);
		}
		size_t res = ZSTD_compressCCtx(cctx, dst, DST_CAPACITY_MAX, buf, len, compression_level);
		if (ZSTD_isError(res))
		{
			lua_pushnil(L);
			return 1;
		}
		lua_pushlstring(L, dst, res);
		return 1;
	}

	static int LzstdCreateDCtx(lua_State * L)
	{
		ZSTD_DCtx * dctx = ZSTD_createDCtx();
		LzstdNewUserdata(L, dctx, kMetatableNames[MN_ZSTD_DCTX]);
		return 1;
	}

	static int LzstdFreeDCtx(lua_State * L)
	{
		ZSTD_DCtx * dctx = (ZSTD_DCtx *)LzstdCheckUserdata(L, 1, kMetatableNames[MN_ZSTD_DCTX]);
		ZSTD_freeDCtx(dctx);
		return 0;
	}

	static int LzstdDecompressDCtx(lua_State * L)
	{
		static char dst[DST_CAPACITY_MAX];
		ZSTD_DCtx * dctx = (ZSTD_DCtx *)LzstdCheckUserdata(L, 1, kMetatableNames[MN_ZSTD_DCTX]);
		size_t len = 0;
		const char * buf = luaL_checklstring(L, 2, &len);
		size_t res = ZSTD_decompressDCtx(dctx, dst, DST_CAPACITY_MAX, buf, len);
		if (ZSTD_isError(res))
		{
			lua_pushnil(L);
			return 1;
		}
		lua_pushlstring(L, dst, res);
		return 1;
	}

	static int LzstdCreateCDict(lua_State * L)
	{
		size_t len = 0;
		const char * buf = luaL_checklstring(L, 1, &len);
		int compression_level = ZSTD_CLEVEL_DEFAULT;
		if (!lua_isnoneornil(L, 2))
		{
			compression_level = (int)luaL_checkinteger(L, 2);
		}
		
		ZSTD_CDict * cdict = ZSTD_createCDict(buf, len, compression_level);
		LzstdNewUserdata(L, cdict, kMetatableNames[MN_ZSTD_CDICT]);
		return 1;
	}

	static int LzstdFreeCDict(lua_State * L)
	{
		ZSTD_CDict * cdict = (ZSTD_CDict *)LzstdCheckUserdata(L, 1, kMetatableNames[MN_ZSTD_CDICT]);
		ZSTD_freeCDict(cdict);
		return 0;
	}

	static int LzstdCompressUsingCDict(lua_State * L)
	{
		static char dst[DST_CAPACITY_MAX];
		ZSTD_CCtx * cctx = (ZSTD_CCtx *)LzstdCheckUserdata(L, 1, kMetatableNames[MN_ZSTD_CCTX]);
		size_t len = 0;
		const char * buf = luaL_checklstring(L, 2, &len);
		ZSTD_CDict * cdict = (ZSTD_CDict *)LzstdCheckUserdata(L, 3, kMetatableNames[MN_ZSTD_CDICT]);
		size_t res = ZSTD_compress_usingCDict(cctx, dst, DST_CAPACITY_MAX, buf, len, cdict);
		if (ZSTD_isError(res))
		{
			lua_pushnil(L);
			return 1;
		}
		lua_pushlstring(L, dst, res);
		return 1;
	}

	static int LzstdCreateDDict(lua_State * L)
	{
		size_t len = 0;
		const char * buf = luaL_checklstring(L, 1, &len);

		ZSTD_DDict * ddict = ZSTD_createDDict(buf, len);
		LzstdNewUserdata(L, ddict, kMetatableNames[MN_ZSTD_DDICT]);
		return 1;
	}

	static int LzstdFreeDDict(lua_State * L)
	{
		ZSTD_DDict * ddict = (ZSTD_DDict *)LzstdCheckUserdata(L, 1, kMetatableNames[MN_ZSTD_DDICT]);
		ZSTD_freeDDict(ddict);
		return 0;
	}

	static int LzstdDecompressUsingDDict(lua_State * L)
	{
		static char dst[DST_CAPACITY_MAX];
		ZSTD_DCtx * dctx = (ZSTD_DCtx *)LzstdCheckUserdata(L, 1, kMetatableNames[MN_ZSTD_DCTX]);
		size_t len = 0;
		const char * buf = luaL_checklstring(L, 2, &len);
		ZSTD_DDict * ddict = (ZSTD_DDict *)LzstdCheckUserdata(L, 3, kMetatableNames[MN_ZSTD_DDICT]);
		size_t res = ZSTD_decompress_usingDDict(dctx, dst, DST_CAPACITY_MAX, buf, len, ddict);
		if (ZSTD_isError(res))
		{
			lua_pushnil(L);
			return 1;
		}
		lua_pushlstring(L, dst, res);
		return 1;
	}
}

extern "C"
{
    LUALIB_API int luaopen_lzstd(lua_State * L)
    {
		// lzstd functions
		static const luaL_Reg lzstd_functions[] =
		{
			{ "compress", glory::LzstdCompress },
			{ "decompress", glory::LzstdDecompress },
			{ "get_frame_content_size", glory::LzstdGetFrameContentSize },
			{ "create_cctx", glory::LzstdCreateCCtx },
			{ "compress_with_cctx", glory::LzstdCompressCCtx },
			{ "create_dctx", glory::LzstdCreateDCtx },
			{ "decompress_with_dctx", glory::LzstdDecompressDCtx },
			{ "create_cdict", glory::LzstdCreateCDict },
			{ "compress_using_cdict", glory::LzstdCompressUsingCDict },
			{ "create_ddict", glory::LzstdCreateDDict },
			{ "decompress_using_ddict", glory::LzstdDecompressUsingDDict },
			{ NULL, NULL }
		};

		static const luaL_Reg lzstd_cctx_functions[] =
		{
			{ "compress", glory::LzstdCompressCCtx },
			{ "compress_using_cdict", glory::LzstdCompressUsingCDict },
			{ NULL, NULL }
		};

		static const luaL_Reg lzstd_dctx_functions[] =
		{
			{ "decompress", glory::LzstdDecompressDCtx },
			{ "decompress_using_ddict", glory::LzstdDecompressUsingDDict },
			{ NULL, NULL }
		};

		glory::LzstdNewMetatable<ZSTD_CCtx, glory::MN_ZSTD_CCTX>(L, glory::LzstdFreeCCtx, NULL, lzstd_cctx_functions);
		glory::LzstdNewMetatable<ZSTD_DCtx, glory::MN_ZSTD_DCTX>(L, glory::LzstdFreeDCtx, NULL, lzstd_dctx_functions);
		glory::LzstdNewMetatable<ZSTD_CDict, glory::MN_ZSTD_CDICT>(L, glory::LzstdFreeCDict, NULL, NULL);
		glory::LzstdNewMetatable<ZSTD_DDict, glory::MN_ZSTD_DDICT>(L, glory::LzstdFreeDDict, NULL, NULL);

		luaL_newlib(L, lzstd_functions);
        return 1;
    }
}

