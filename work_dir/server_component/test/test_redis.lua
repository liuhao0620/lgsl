package.path = package.path..";server_container/lua/?.lua"
local lredis = require("lredis")
require("functions")

local rc, err = lredis.connect("127.0.0.1", 6379, 5000000)
print(rc, err)
print(rc:set_timeout(5000000))
print(rc:command("auth 101921"))
print(rc:command("PING"))
print(rc:command("get 1"))
print(rc:command("del test_list"))
for i = 1,10 do
    print(rc:command("lpush test_list element_"..i))
end
local val, err = rc:command("lrange test_list 0 -1")
dump(val, "test_list")
print(err)
print(rc:command("del test_list"))

rc:close()
