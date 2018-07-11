package.path = package.path..";server_container/lua/?.lua"
require("session")
local luv = require("luv")
local lkcp = require("lkcp")

Server = class("Server")

function Server:CloseSession(session_id)
    if self._sessions_by_sid[session_id] ~= nil then
        local session = self._sessions_by_sid[session_id]
        local client = session:GetTcp()
        client:read_stop()
        client:close()
        self._close_callback(self, session_id)
        local addr = session:GetRemoteAddr()
        self._sids_by_addr[addr.ip..addr.port] = nil
        self._sessions_by_sid[session_id] = nil
    end
end

function Server:NewSessionId()
    self._session_id_index = self._session_id_index + 1
    return self._session_id_index
end

function Server:TcpRecv(session_id, err, chunk)
    if err ~= nil then
        LogErr(string.format("Server:TcpRecv Error! session_id:%d err:%s", session_id, err))
        -- close session
        self:CloseSession(session_id)
        return
    end
    if chunk == nil then
        -- EOF
        -- close session
        self:CloseSession(session_id)
        return
    end
    if self._sessions_by_sid[session_id] ~= nil then
        self._sessions_by_sid[session_id]:Recv(Channel.TCP_DATA, chunk)
    end
end

function Server:UdpRecv(err, chunk, addr, flags)
    if #chunk < UDP_DATA_MIN_SIZE then
        return
    end
    local session_id = String4ToInt(chunk)
    if self._sessions_by_sid[session_id] == nil then
        -- session is not exist
        return
    end
    if self._sids_by_addr[addr.ip..addr.port] == nil then
        -- hand shake
        self._sids_by_addr[addr.ip..addr.port] = session_id
        self._sessions_by_sid[session_id]:OnUdpHandShake(self._udp, addr, chunk)
        self._connect_callback(self, session_id)
    else
        if #chunk == UDP_DATA_MIN_SIZE then
            self._sessions_by_sid[session_id]:OnUdpHeartbeat()
        else
            self._sessions_by_sid[session_id]:Recv(Channel.UDP_DATA, chunk)
        end
    end
end

function Server:TcpConnect()
    -- create session
    local client = luv.new_tcp()
    local session_id = self:NewSessionId()
    local session = Session.new(session_id, client, function(chunk)
        self._recv_callback(self, session_id, chunk)
    end)
    self._sessions_by_sid[session_id] = session
    
    -- start read
    self._tcp:accept(client)
    client:read_start(function(err, chunk)
        self:TcpRecv(session_id, err, chunk)
    end)

    -- 
    session:OnTcpConnect()
end

function Server:Update()
    local current = luv.now()
    for _, v in pairs(self._sessions_by_sid) do
        v:Update(current)
    end
end

function Server:ctor(connect_callback, recv_callback, close_callback)
    self._connect_callback = connect_callback
    self._recv_callback = recv_callback
    self._close_callback = close_callback
    self._session_id_index = 0
    self._sessions_by_sid = {}
    self._sids_by_addr = {}
end

function Server:Listen(ip, port, tcp_backlog)
    -- ensure server is closed
    self:Close()

    -- start tcp listen
    self._tcp = luv.new_tcp()
    self._tcp:bind(ip, port)
    self._tcp:listen(tcp_backlog, function()
        self:TcpConnect()
    end)

    -- start udp recv
    self._udp = luv.new_udp()
    self._udp:bind(ip, port)
    self._udp:recv_start(function(err, chunk, addr, flags)
        self:UdpRecv(err, chunk, addr, flags)
    end)

    -- start timer
    self._timer = luv.new_timer()
    self._timer:start(10, 10, function()
        self:Update()
    end)
end

function Server:Close()
    -- close all sessions
    for k, _ in pairs(self._sessions_by_sid) do
        self:CloseSession(k)
    end
    self._sessions_by_sid = {}

    -- stop timer
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

    -- stop tcp listen
    if self._tcp ~= nil then
        self._tcp:close()
        self._tcp = nil
    end
end

function Server:Send(session_id, channel, chunk)
    if self._sessions_by_sid[session_id] ~= nil then
        self._sessions_by_sid[session_id]:Send(channel, chunk)
    end
end

function Server:Run()
    luv.run()

    -- Close all then handles, including stdout
    luv.walk(luv.close)
    luv.run()
end
