package.path = package.path..";../deps/lua-resty-redis/lib/?.lua"

local redis = require("resty.redis")

local rr = redis.new()
rr:set_timeout(1000)
print(rr)

local ok, err = rr:connect("127.0.0.1", 6379)
print(ok, err)
local ok, err = rr:set_keepalive(1000, 100)
print(ok, err)

rr:auth("101921")
rr:set("name", "liuhao")
local name = rr:get("name")
print(name)
