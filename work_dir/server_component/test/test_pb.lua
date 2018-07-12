package.path = package.path..";server_container/lua/?.lua"

require("functions")
local lpb = require("pb")
local protoc = require("protoc")

protoc:load([[
    syntax="proto3"
    package PB;
    enum Sex
    {
        Male = 1;
        Female = 2;
        Unkown = 3;
    }

    message Friend
    {
        string name = 1;
        int64 phone = 2;
    }

    message Person
    {
        string id = 1;
        string name = 2;
        int32 age = 3;
        Sex sex = 4;
        int64 phone = 5;
        repeated Friend friends = 6;
    }
]])

local person = {
    id = "xxxxxxxxxxxxxxxxxx",
    name = "xxxxxx",
    age = 24,
    sex = "Male",
    phone = 13245678910,
    friends = {
        {name = "arenold", phone = 12345678910},
        {name = "bernard", phone = 10987654321}
    }
}

local bytes = assert(lpb.encode("PB.Person", person))        
dump(lpb.tohex(bytes))
local decode_person = assert(lpb.decode("PB.Person", bytes))
dump(decode_person, "person")


