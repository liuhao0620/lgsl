package.path = package.path..";server_container/lua/?.lua"
require("functions")
local lzstd = require("lzstd")

local ZUtil = class("ZUtil")

function ZUtil:ctor(compress_level)
    self._cctx = lzstd.create_cctx()
    self._dctx = lzstd.create_dctx()
    self._compress_level = compress_level
end

function ZUtil:SetDict(dict_chunk)
    self._cdict = lzstd.create_cdict(dict_chunk, self._compress_level)
    self._ddict = lzstd.create_ddict(dict_chunk)
end

function ZUtil:Compress(chunk)
    if self._cdict then
        return self._cctx:compress_using_cdict(chunk, self._cdict)
    else
        return self._cctx:compress(chunk, self._compress_level)
    end
end

function ZUtil:Decompress(chunk)
    if self._ddict then
        return self._dctx:decompress_using_ddict(chunk, self._ddict)
    else
        return self._dctx:decompress(chunk)
    end
end

return ZUtil.new()
