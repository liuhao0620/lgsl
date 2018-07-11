local llog = require("lspdlog")

local loggers = {}

function LogInit(config)
    for _, v in pairs(config) do
        local logger = llog.create_logger(v)
        if logger == nil then
            error("logger init failed", 0)
            return false
        end
        if v.level ~= nil then
            logger:set_level(v.level)
        end
        if v.pattern ~= nil then
            logger:set_pattern(v.pattern)
        end
        if v.flush_on ~= nil then
            logger:flush_on(v.flush_on)
        end
        loggers[v.name] = logger
    end
    return true
end

function Log(lv, fmt, ...)
    for _, v in pairs(loggers) do
        v:log(lv, string.format(fmt, ...))
    end 
end

function LogTrace(fmt, ...)
    Log("trace", fmt, ...)
end

function LogDebug(fmt, ...)
    Log("debug", fmt, ...)
end

function LogInfo(fmt, ...)
    Log("info", fmt, ...)
end

function LogWarn(fmt, ...)
    Log("warn", fmt, ...)
end

function LogErr(fmt, ...)
    Log("err", fmt, ...)
end

function LogCritical(fmt, ...)
    Log("critical", fmt, ...)
end

function LogFlush(logger_name)
    if logger_name ~= nil then
        loggers[logger_name]:flush()
    else
        for _, v in pairs(loggers) do
            v:flush()
        end 
    end
end

function LogClose(logger_name)
    if logger_name ~= nil then
        loggers[logger_name]:set_level("off")
    else 
        for _, v in pairs(loggers) do
            v:set_level("off")
        end 
    end
end
