local lrmq = require("lrmq")
local luv = require("luv")

local TEST_CHANNEL_ID = 1
local TEST_EXCHANGE_NAME = "test_exchange"
local TEST_QUEUE_NAME = "test_queue"

local usec_time = luv.hrtime() / 1000
function PrintTime()
    local temp = usec_time
    usec_time = luv.hrtime() / 1000
    print(usec_time - temp)
end

function basic_publish_and_get(chunk)
    local connection = lrmq.connection_open("127.0.0.1", 5672, "test", 1, "liuhao", "101921")
    local channel, err = connection:channel_open(TEST_CHANNEL_ID)
    assert(err == nil)
    local exchange, err = channel:exchange_declare(TEST_EXCHANGE_NAME, "direct")
    assert(err == nil)
    local queue, err = channel:queue_declare(TEST_QUEUE_NAME)
    assert(err == nil)
    local err = queue:bind(exchange, "test")
    assert(err == nil)

    -- publish
    local err = exchange:publish("test", chunk)
    assert(err == nil)

    -- get
    local msg, err = queue:get(5000000)
    assert(err == nil)

    queue:unbind(exchange, "test")
    queue:delete()
    exchange:delete()
    channel:close()
    connection:close()

    print(msg, chunk)
end

function basic_publish_and_consume(chunk)
    local connection = lrmq.connection_open("127.0.0.1", 5672, "test", 1, "liuhao", "101921")
    local channel, err = connection:channel_open(TEST_CHANNEL_ID)
    assert(err == nil)
    local exchange, err = channel:exchange_declare(TEST_EXCHANGE_NAME, "direct")
    assert(err == nil)
    local queue, err = channel:queue_declare(TEST_QUEUE_NAME)
    assert(err == nil)
    local err = queue:bind(exchange, "test")
    assert(err == nil)

    -- publish
    local err = exchange:publish("test", chunk)
    assert(err == nil)

    -- consume
    local consumer, err = queue:consume("consumer1")
    assert(err == nil)
    local msg, err = connection:consume_message(5000000)
    assert(err == nil)
    assert(msg == chunk)

    consumer:cancel()
    queue:unbind(exchange, "test")
    queue:delete()
    exchange:delete()
    channel:close()
    connection:close()

    print(msg, chunk)
end

basic_publish_and_get("")
basic_publish_and_get("nihao123")
basic_publish_and_consume("")
basic_publish_and_consume("nihao456")
