package.path = package.path..";server_container/lua/?.lua"
require("functions")
require("net_tool")
require("log")
local luv = require("luv")
local lkcp = require("lkcp")

Session = class("Session")

function Session:TcpSend(chunk)
    chunk = IntToString4(#chunk)..chunk
    self._tcp:write(chunk, function(err)
        -- sent
    end)
end

function Session:TcpRecv(chunk)
    self._tcp_recv_msg_buffer = self._tcp_recv_msg_buffer..chunk
    while true do
        local complte_buffer
        complte_buffer, chunk = CheckCompleteBuffer(chunk)
        if complte_buffer == nil then
            break
        end
        self._recv_callback(complte_buffer)
    end
end

function Session:UdpSend(channel, chunk)
    if self._udp == nil or self._udp_remote_addr == nil then
        return
    end
    chunk = self._udp_head..IntToString1(channel)..chunk
    self._udp:send(chunk, self._udp_remote_addr.ip, self._udp_remote_addr.port, function(err)
        -- sent
    end)
end

function Session:UdpRecv(chunk)
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
        return
    end
    if channel == Channel.KCP_DATA then
        self._kcp:input(data)
        self:Update(luv.now(), true)
    elseif channel == Channel.UDP_DATA then
        self._recv_callback(data)
    end
end

function Session:ctor(session_id, tcp, recv_callback)
    self._session_id = session_id
    self._tcp = tcp
    self._recv_callback = recv_callback

    self._kcp_conv = math.random(256 * 256 * 256 * 127)
    self._udp_head = IntToString4(self._session_id)..IntToString4(self._kcp_conv)
    self._kcp = lkcp.create(self._kcp_conv, function(chunk)
        self:UdpSend(Channel.KCP_DATA, chunk)
    end)
    self._kcp_time_to_update = 0
    self._udp = nil
    self._udp_remote_addr = nil
    self._tcp_recv_msg_buffer = ""
end

function Session:OnUdpHandShake(udp, remote_addr, chunk)
    self._udp = udp
    self._udp_remote_addr = remote_addr
    self:UdpRecv(chunk)
    self:UdpSend(Channel.KCP_DATA, "")
end

function Session:OnUdpHeartbeat()
    -- send back to client
    print("Session:OnUdpHeartbeat %d", self._session_id)
    self:UdpSend(Channel.UDP_DATA, "")
end

function Session:OnTcpConnect()
    -- send sid and key to client
    self:TcpSend(self._udp_head)
end

function Session:Send(channel, chunk)
    if channel == Channel.TCP_DATA then
        self:TcpSend(chunk)
    elseif channel == Channel.KCP_DATA then
        self._kcp:send(chunk)
        self:Update(luv.now(), true)
    elseif channel == Channel.UDP_DATA then
        self:UdpSend(Channel.UDP_DATA, chunk)
    end
end

function Session:Recv(channel, chunk)
    if channel == Channel.TCP_DATA then
        self:TcpRecv(chunk)
    elseif channel == Channel.UDP_DATA then
        self:UdpRecv(chunk)
    end
end

function Session:Update(current, force_update)
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
        self._recv_callback(chunk)
    end
end

function Session:GetTcp()
    return self._tcp
end

function Session:GetRemoteAddr()
    return self._udp_remote_addr
end
