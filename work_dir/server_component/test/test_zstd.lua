package.path = package.path..";server_container/lua/?.lua"

local luv = require("luv")
local lzstd = require("lzstd")

local str1 = [[{
    {
        name = "test_logger",
        level = "trace",
        flush_on = "err",
        sinks = {
            {
                type = "stdout",
            },
            {
                type = "basic",
                file_name = "server_component/test/test_logger.log",
                truncate = true,
            },
        },
    },
    {
        name = "test_logger2",
        level = "trace",
        flush_on = "err",
        sinks = {
            {
                type = "stdout",
            },
            {
                type = "basic",
                file_name = "server_component/test/test_logger2.log",
                truncate = true,
            },
        },
    },
}]]

local compress_level = 3

function test1()
    print("---------test1----------")
    -- test simple
    local temp = lzstd.compress(str1, compress_level)
    local src_size = lzstd.get_frame_content_size(temp)
    local str2 = lzstd.decompress(temp)
    print("temp", #temp--[[, temp]])
    print("src_size", src_size)
    print("str1", #str1--[[, str1]])
    print("str2", #str2--[[, str2]])
end

function test2()
    print("---------test2----------")
    local temp
    local str2

    local compress_begin = luv.hrtime() / 1000
    for i = 1, 10000 do
        temp = lzstd.compress(str1, compress_level)
    end
    local compress_end = luv.hrtime() / 1000

    local decompress_begin = luv.hrtime() / 1000
    for i = 1, 10000 do
        local str2 = lzstd.decompress(temp)
    end
    local decompress_end = luv.hrtime() / 1000

    print("compress 10000", compress_begin, compress_end, compress_end-compress_begin)
    print("decompress 10000", decompress_begin, decompress_end, decompress_end-decompress_begin)
end

function test3()
    print("---------test3----------")
    -- test simple with cctx/dctx
    local cctx = lzstd.create_cctx()
    local dctx = lzstd.create_dctx()
    local temp = cctx:compress(str1, compress_level)
    local str2 = dctx:decompress(temp)
    print("temp", #temp--[[, temp]])
    print("str1", #str1--[[, str1]])
    print("str2", #str2--[[, str2]])
    print("cctx", cctx, "dctx", dctx)
end

function test4()
    print("---------test4----------")
    local cctx = lzstd.create_cctx()
    local dctx = lzstd.create_dctx()
    local temp
    local str2
    
    local compress_begin = luv.hrtime() / 1000
    for i = 1, 10000 do
        temp = cctx:compress(str1, compress_level)
    end
    local compress_end = luv.hrtime() / 1000
    
    local decompress_begin = luv.hrtime() / 1000
    for i = 1, 10000 do
        local str2 = dctx:decompress(temp)
    end
    local decompress_end = luv.hrtime() / 1000

    print("cctx compress 10000", compress_begin, compress_end, compress_end-compress_begin)
    print("dctx decompress 10000", decompress_begin, decompress_end, decompress_end-decompress_begin)
end

function test5()
    print("---------test5----------")
    local dict_fd = luv.fs_open('server_component/test/a.dict', 'r', tonumber('644', 8))
    local dict_stat = luv.fs_fstat(dict_fd)
    local dict_chunk = luv.fs_read(dict_fd, dict_stat.size, 0)
    luv.fs_close(dict_fd)

    local cctx = lzstd.create_cctx()
    local dctx = lzstd.create_dctx()
    local cdict = lzstd.create_cdict(dict_chunk, compress_level)
    local ddict = lzstd.create_ddict(dict_chunk)
    local temp = cctx:compress_using_cdict(str1, cdict)
    local str2 = dctx:decompress_using_ddict(temp, ddict)
    
    print("temp", #temp--[[, temp]])
    print("str1", #str1--[[, str1]])
    print("str2", #str2--[[, str2]])
    print("cctx", cctx, "dctx", dctx, "cdict", cdict, "ddict", ddict)
end

function test6()
    print("---------test6----------")
    local dict_fd = luv.fs_open('server_component/test/a.dict', 'r', tonumber('644', 8))
    local dict_stat = luv.fs_fstat(dict_fd)
    local dict_chunk = luv.fs_read(dict_fd, dict_stat.size, 0)
    luv.fs_close(dict_fd)

    local cctx = lzstd.create_cctx()
    local dctx = lzstd.create_dctx()
    local cdict = lzstd.create_cdict(dict_chunk, compress_level)
    local ddict = lzstd.create_ddict(dict_chunk)
    local temp
    local str2

    local compress_begin = luv.hrtime() / 1000
    for i = 1, 10000 do
        temp = cctx:compress_using_cdict(str1, cdict)
    end
    local compress_end = luv.hrtime() / 1000
    
    local decompress_begin = luv.hrtime() / 1000
    for i = 1, 10000 do
        str2 = dctx:decompress_using_ddict(temp, ddict)
    end
    local decompress_end = luv.hrtime() / 1000

    print("cctx compress_using_cdict 10000", compress_begin, compress_end, compress_end-compress_begin)
    print("dctx decompress_using_ddict 10000", decompress_begin, decompress_end, decompress_end-decompress_begin)
end

test1()
test2()
test3()
test4()
--test5()
--test6()
