package.path = package.path..";server_container/lua/?.lua"

require("log")
local luv = require("luv")

LogInit({
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
})

Log("trace", "test %s", "trace")
LogTrace("test %s", "trace")
LogDebug("test debug %d", 1)
LogInfo("test info %c", 83)
LogWarn("test warn")
LogErr("test err %s", "err_msg")
LogCritical("test critical")
LogClose("test_logger")
Log("trace", "test %s", "trace")
LogTrace("test %s", "trace")
LogDebug("test debug %d", 1)
LogInfo("test info %c", 83)
LogWarn("test warn")
LogErr("test err %s", "err_msg")
LogCritical("test critical")
