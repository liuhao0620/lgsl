package.path = package.path..";server_container/lua/?.lua"
require("functions")
require("net_tool")
require("log")
local luv = require("luv")
local lkcp = require("lkcp")

Client = class("Client")

function Client:CloseSession()
    self:Close()
    self._close_callback(self)
end

function Client:UdpSend(channel, chunk)
    chunk = self._udp_head..IntToString1(channel)..chunk
    self._udp:send(chunk, self._server_ip, self._server_port, function(err)
        -- sent
    end)
end

function Client:SendHeartbeat()
    self:UdpSend(Channel.UDP_DATA, "")
end

function Client:OnUdpHeartbeat()
    -- nothing to do
    print("Client:OnUdpHeartbeat")
end

function Client:UdpRecv(err, chunk, addr, flags)
    if #chunk < UDP_DATA_MIN_SIZE then
        return
    end
    local head = string.sub(chunk, UDP_DATA_HEAD_OFFSET, UDP_DATA_HEAD_OFFSET + UDP_DATA_HEAD_SIZE - 1)
    if head ~= self._udp_head then
        return
    end
    local channel = String1ToInt(string.sub(chunk, UDP_DATA_CHANNEL_OFFSET, UDP_DATA_CHANNEL_OFFSET + UDP_DATA_CHANNEL_SIZE - 1))
    local data = string.sub(chunk, UDP_DATA_DATA_OFFSET)
    if #data == 0 then
        self:OnUdpHeartbeat()
        return
    end
    if channel == Channel.KCP_DATA then
        self._kcp:input(data)
        self:Update(luv.now(), true)
    elseif channel == Channel.UDP_DATA then
        self._recv_callback(self, data)
    end
end

function Client:OnRecvUdpHeadFromTcp(udp_head)
    self._udp_head = udp_head
    self._kcp_conv = String4ToInt(string.sub(udp_head, UDP_DATA_KEY_OFFSET - UDP_DATA_HEAD_OFFSET + 1, UDP_DATA_KEY_OFFSET - UDP_DATA_HEAD_OFFSET + UDP_DATA_KEY_SIZE))
    self._kcp = lkcp.create(self._kcp_conv, function(chunk)
        self:UdpSend(Channel.KCP_DATA, chunk)
    end)

    -- send handshake to server
    self:UdpSend(Channel.KCP_DATA, "")
    self._connect_callback(self)

    -- start heartbeat timer
    if self._heartbeat_timer == nil then
        self._heartbeat_timer = luv.new_timer()
        self._heartbeat_timer:start(30 * 1000, 30 * 1000, function()
            self:SendHeartbeat()
        end)
    end
end

function Client:TcpSend(chunk)
    chunk = IntToString4(#chunk)..chunk
    self._tcp:write(chunk, function(err)
        -- sent
    end)
end

function Client:TcpRecv(err, chunk)
    if err ~= nil then
        LogErr(string.format("Server:TcpRecv Error! err:%s", session_id, err))
        -- close session
        -- self:CloseSession()
        return
    end
    if chunk == nil then
        -- EOF
        -- close session
        self:CloseSession()
        return
    end
    
    self._tcp_recv_msg_buffer = self._tcp_recv_msg_buffer..chunk
    while true do
        local complte_buffer
        complte_buffer, chunk = CheckCompleteBuffer(chunk)
        if complte_buffer == nil then
            break
        end
        if self._kcp == nil then
            if #complte_buffer ~= UDP_DATA_HEAD_SIZE then
                break
            end
            self:OnRecvUdpHeadFromTcp(complte_buffer)
        else
            self._recv_callback(self, complte_buffer)
        end
    end
end

function Client:TcpConnect()
    self._tcp:read_start(function(err, chunk)
        self:TcpRecv(err, chunk)
    end)
end

function Client:Update(force_update)
    if self._kcp == nil then
        return
    end

    local current = luv.now()
    if force_update or current >= self._kcp_time_to_update then
        self._kcp:update(current)
        self._kcp_time_to_update = self._kcp:check(current)
    end

    if force_update then
        self._kcp:flush()
    end

    while true do
        chunk = self._kcp:recv()
        if chunk == nil or chunk == "" then
            break
        end
        self._recv_callback(self, chunk)
    end
end

function Client:ctor(connect_callback, recv_callback, close_callback)
    self._connect_callback = connect_callback
    self._recv_callback = recv_callback
    self._close_callback = close_callback
    self._tcp_recv_msg_buffer = ""
    self._kcp_time_to_update = 0
end

function Client:Connect(server_ip, server_port, self_ip, self_port)
    self._server_ip = server_ip
    self._server_port = server_port

    -- ensure client is closed
    self:Close()

    -- tcp connect
    self._tcp = luv.new_tcp()
    self._tcp:bind(self_ip, self_port)
    self._tcp:connect(server_ip, server_port, function()
        self:TcpConnect()
    end)

    -- start udp recv
    self._udp = luv.new_udp()
    self._udp:bind(self_ip, self_port)
    self._udp:recv_start(function(err, chunk, addr, flags)
        self:UdpRecv(err, chunk, addr, flags)
    end)

    -- start timer
    self._timer = luv.new_timer()
    self._timer:start(10, 10, function()
        self:Update()
    end)
end

function Client:Close()
    -- close heartbeat timer
    if self._heartbeat_timer ~= nil then
        self._heartbeat_timer:stop()
        self._heartbeat_timer:close()
        self._heartbeat_timer = nil
    end

    -- close timer
    if self._timer ~= nil then
        self._timer:stop()
        self._timer:close()
        self._timer = nil
    end

    -- stop udp recv
    if self._udp ~= nil then
        self._udp:recv_stop()
        self._udp:close()
        self._udp = nil
    end

    -- stop tcp
    if self._tcp ~= nil then
        self._tcp:read_stop()
        self._tcp:close()
        self._tcp = nil
    end
end

function Client:Send(channel, chunk)
    if channel == Channel.TCP_DATA then
        self:TcpSend(chunk)
    elseif channel == Channel.KCP_DATA then
        self._kcp:send(chunk)
        self:Update(luv.now(), true)
    elseif channel == Channel.UDP_DATA then
        self:UdpSend(Channel.UDP_DATA, chunk)
    end
end

function Client:Run()
    luv.run()

    -- Close all then handles, including stdout
    luv.walk(luv.close)
    luv.run()
end
