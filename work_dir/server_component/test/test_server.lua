package.path = package.path..";server_container/lua/?.lua"

require("server")

LogInit({
    {
        name = "test_server_logger",
        type = "stdout",
        level = "trace",
        flush_on = "err",
    }
})

local count = 0

local test_server = Server.new(function(server, session_id)
    LogTrace("test server %d connect", session_id)
end, function(server, session_id, chunk)
    LogTrace("test server %d recv %s", session_id, chunk)
    server:Send(session_id, Channel.TCP_DATA, "test_tcp_data")
    server:Send(session_id, Channel.KCP_DATA, "test_kcp_data")
    server:Send(session_id, Channel.UDP_DATA, "test_udp_data")
    count = count + 1
    if count >= 3 then
        server:Close()
    end
end, function(server,session_id)
    LogTrace("test server %d close", session_id)
end)

test_server:Listen("127.0.0.1", 1019, 100)
LogTrace("test server Listen success")

test_server:Run()
