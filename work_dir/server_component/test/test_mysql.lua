--local mysql = require("luasql.mysql")

--local env = mysql.mysql()
--local conn, err = env:connect("mysql", "root", "", "127.0.0.1", 3306)
--print(conn, err)

--local status, err = conn:execute([[CREATE DATABASE IF NOT EXISTS mysql_test]])
--print(status, err)
--local status, err = conn:execute([[USE mysql_test]])
--print(status, err)
--local status, err = conn:execute([[CREATE TABLE IF NOT EXISTS test (id INT, name TEXT)]])
--print(status, err)
--local status, err = conn:execute([[INSERT INTO test VALUES(1, "liuhao")]])
--print(status, err)
--local cursor, err = conn:execute([[SELECT * FROM test]])
--print(cursor, err)
--local row = cursor:fetch({}, "a")
--while row do
--    print("id : ", row.id, " name : ", row.name)
--    row = cursor:fetch(row, "a")
--end

--local status, err = conn:execute([[DROP DATABASE mysql_test]])
--print(status, err)

--cursor:close()
--conn:close()
--env:close()

package.path = package.path..";server_container/lua/?.lua"
require("mysql")

function CheckErr(err, s)
    if err then
        print(s, err)
    end
end

local mysql = MySql.new()
local err = mysql:Connect("mysql_test", "root", "", "127.0.0.1", 3306)
CheckErr(err, "Connect")
local err = mysql:CreateTable("test", {{"id", "INT"}, {"name", "TEXT"}}) -- 使用数组是为了保证顺序
CheckErr(err, "CreateTable")
local err = mysql:InsertData("test", {1, "liuhao"})
CheckErr(err, "InsertData")
local err = mysql:InsertData("test", {id=2, name="dufei"})
CheckErr(err, "InsertData")
local err = mysql:InsertData("test", {3, "liuhao"})
CheckErr(err, "InsertData")
local values, err = mysql:SelectData("test", {"id", "name"}, {either=true, {">=", "id", 1}, {"=", "name", "liuhao"}}, {"name ASC", "id DESC"})
CheckErr(err, "SelectData")
dump(values, "test")
local err = mysql:UpdateData("test", {name="huhuabing"}, {{"=", "id", 3}})
CheckErr(err, "UpdateData")
local values, err = mysql:SelectData("test", {"id", "name"})
CheckErr(err, "SelectData")
dump(values, "test")
local err = mysql:DeleteData("test", {{"=", "id", 3}})
CheckErr(err, "DeleteData")
local values, err = mysql:SelectData("test", {"id", "name"})
CheckErr(err, "SelectData")
dump(values, "test")
local err = mysql:DropTable("test")
CheckErr(err, "DropTable")
mysql:Close()
