package.path = package.path..";server_container/lua/?.lua"
require("functions")
local luv = require("luv")

local FsUtil = class("FsUtil")

function FsUtil:ctor()
end

function FsUtil:ReadFile(file_name, callback)
    if callback then
        luv.fs_open(file_name, 'r', tonumber('644', 8), function (err, fd)
            if err then
                callback(err, nil)
                return
            end
            luv.fs_fstat(fd, function (err, stat)
                if err then
                    callback(err, nil)
                    return
                end
                luv.fs_read(fd, stat.size, 0, function(err, chunk)
                    if err then
                        callback(err, nil)
                        return
                    end
                    luv.fs_close(fd. function(err)
                        if err then
                            callback(err, nil)
                            return
                        end
                        callback(nil, chunk)
                    end)
                end)
            end)
        end)
    else
        local fd = luv.fs_open(file_name, 'r', tonumber('644', 8))
        if fd == nil then
            return string.format("fs_open %s fail!!!", file_name), nil
        end
        local stat = luv.fs_fstat(fd)
        if stat == nil then
            return string.format("fs_fstat %s(%d) fail!!!", file_name, fd), nil
        end
        local chunk = luv.fs_read(fd, stat.size, 0)
        if chunk == nil then
            return string.format("fs_read %s(%d) %d fail!!!", file_name, fd, stat.size), nil
        end
        luv.fs_close(dict_fd)
        return nil, chunk
    end
end

return FsUtil.new()
