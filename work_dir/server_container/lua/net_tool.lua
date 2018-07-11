Channel =
{
    TCP_DATA = 1,
    KCP_DATA = 2,
    UDP_DATA = 3,
}

--[[
    0xaabbccdd          -- session_id
    0xeeffgghh          -- key
    0xii                -- channel
    0x...               -- data
]]
-- head = session_id + key
UDP_DATA_HEAD_OFFSET = 1
UDP_DATA_HEAD_SIZE = 8
-- min data = session_id + key + channel
UDP_DATA_MIN_SIZE = 9

UDP_DATA_SESSION_ID_OFFSET = 1
UDP_DATA_SESSION_ID_SIZE = 4
UDP_DATA_KEY_OFFSET = 5
UDP_DATA_KEY_SIZE = 4
UDP_DATA_CHANNEL_OFFSET = 9
UDP_DATA_CHANNEL_SIZE = 1
UDP_DATA_DATA_OFFSET = 10

function IntToString4(x)
    return string.format("%c%c%c%c", math.fmod(math.floor(x), 256), math.fmod(math.floor(x/256), 256), math.fmod(math.floor(x/256/256), 256), math.floor(x/256/256/256))
end

function IntToString2(x)
    return string.format("%c%c", math.fmod(math.floor(x), 256), math.fmod(math.floor(x/256), 256))
end

function IntToString1(x)
    return string.format("%c", math.fmod(math.floor(x), 256))
end

function String4ToInt(str)
    if #str < 4 then
        return nil
    end
    return string.byte(str, 1) + string.byte(str, 2) * 256 + string.byte(str, 3) * 256 * 256 + string.byte(str, 4) * 256 * 256 * 256
end

function String2ToInt(str)
    if #str < 2 then
        return nil
    end
    return string.byte(str, 1) + string.byte(str, 2) * 256
end

function String1ToInt(str)
    if #str < 1 then
        return nil
    end
    return string.byte(str, 1)
end

--[[
    0xaabbccdd      -- len
    0x...           -- data
]]
function CheckCompleteBuffer(chunk)
    if #chunk < 4 then
        return nil, chunk
    end
    local len = String4ToInt(chunk)
    if #chunk < 4 + len then
        return nil, chunk
    end
    return string.sub(chunk, 5, 4 + len), string.sub(chunk, len + 5)
end
