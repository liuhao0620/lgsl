package.path = package.path..";server_container/lua/?.lua"
require("client")

LogInit({
    {
        name = "test_client_logger",
        type = "stdout",
        level = "trace",
        flush_on = "err",
    }
})

local count = 0
local test_client = Client.new(function(client)
    LogTrace("test client connect")
    client:Send(Channel.TCP_DATA, "test_tcp_data")
    client:Send(Channel.KCP_DATA, "test_kcp_data")
    client:Send(Channel.UDP_DATA, "test_udp_data")
end, function(client, chunk)
    LogTrace("test client recv %s", chunk)
    count = count + 1
    if count >= 9 then
        client:Close()
    end
end, function(client,session_id)
    LogTrace("test client close")
end)

test_client:Connect("127.0.0.1", 1019, "127.0.0.1", 1020)
test_client:Run()
