package.path = package.path..";server_container/lua/?.lua"
require("functions")
require("log")
local mysql = require("luasql.mysql")

MySql = class("MySql")

function MySql:ToString(v)
    if type(v) == "string" then
         return "'"..v.."'"
    end
    return tostring(v)
end

function MySql:ParseConds(conds)
    local sql_conds = ""
    if conds then
        sql_conds = "WHERE "
        for i = 1, #conds do
            if i ~= 1 then
                if conds.either then
                    sql_conds = sql_conds.." OR "
                else
                    sql_conds = sql_conds.." AND "
                end
            end
            local one_cond = conds[i]
            if one_cond[1] == nil or one_cond[2] == nil or one_cond[3] == nil then
                return "MySql:SelectData Param Error ! conds["..i.."] is error."
            end
            if one_cond[1] == "BETWEEN" then
                if one_cond[4] == nil then
                    return "MySql:SelectData Param Error ! conds["..i.."] is error."
                end
                sql_conds = sql_conds..one_cond[2].." BETWEEN "..self:ToString(one_cond[3]).." AND "..self:ToString(one_cond[4])
            else
                sql_conds = sql_conds..one_cond[2].." "..one_cond[1].." "..self:ToString(one_cond[3])
            end
        end
    end
    return sql_conds
end

function MySql:ctor()
end

function MySql:Connect(database, user, password, host, port)
    self._env = mysql.mysql()
    local err
    self._conn, err = self._env:connect("mysql", user, password, host, port)
    if err then
        LogErr("MySql:Connect Fail : connect : %s", err)
        return err
    end

    local status, err = self._conn:execute([[CREATE DATABASE IF NOT EXISTS ]]..database)
    if err then
        LogErr("MySql:Connect Fail : create database %s: %s", database, err)
        return err
    end
    local status, err = self._conn:execute([[USE ]]..database)
    if err then
        LogErr("MySql:Connect Fail : use database %s: %s", database, err)
        return err
    end
    LogTrace("MySql:Connect Success: database %s", database)
end

function MySql:Close()
    if self._conn then
        self._conn:close()
        self._conn = nil
    end
    if self._env then
        self._env:close()
        self._env = nil
    end
end

function MySql:CreateTable(table_name, fields)
    local sql_fields = ""
    for i = 1, #fields do
        if i ~= 1 then
            sql_fields = sql_fields..", "
        end
        sql_fields = sql_fields..fields[i][1].." "..fields[i][2]
        if fields[i][3] then
            sql_fields = sql_fields.." "..fields[i][3]
        end
    end
    local sql_create = string.format([[CREATE TABLE IF NOT EXISTS %s (%s)]], table_name, sql_fields)
    local status, err = self._conn:execute(sql_create)
    if err then
        LogErr("MySql:CreateTable Fail : create table %s : %s", table_name, err)
        return err
    end
    LogTrace("MySql:CreateTable Success: create table %s : %s", table_name, sql_fields)
end

function MySql:DropTable(table_name)
    local sql_drop = string.format([[DROP TABLE %s]], table_name)
    local status, err = self._conn:execute(sql_drop)
    if err then
        LogErr("MySql:DropTable Fail : drop table %s : %s", table_name, err)
        return err
    end
    LogTrace("MySql:DropTable Success: drop table %s", table_name)
end

function MySql:InsertData(table_name, values)
    local sql_fields = ""
    local sql_values = ""
    if #values == 0 then
        sql_fields = "("
        for k, v in pairs(values) do
            if sql_fields ~= "(" then
                sql_fields = sql_fields..", "
            end
            if sql_values ~= "" then
                sql_values = sql_values..", "
            end
            sql_fields = sql_fields..k
            sql_values = sql_values..self:ToString(v)
        end
        sql_fields = sql_fields..")"
    else
        for i = 1, #values do
            if i ~= 1 then
                sql_values = sql_values..", "
            end
            sql_values = sql_values..self:ToString(values[i])
        end
    end
    local sql_insert = string.format([[INSERT INTO %s%s VALUES(%s)]], table_name, sql_fields, sql_values)
    local status, err = self._conn:execute(sql_insert)
    if err then
        LogErr("MySql:InsertData Fail : insert into %s : %s", table_name, err)
        return err
    end
    LogTrace("MySql:InsertData Success: insert into %s", table_name)
end

function MySql:DeleteData(table_name, conds)
    local sql_conds = self:ParseConds(conds)
    local sql_delete = string.format([[DELETE FROM %s %s]], table_name, sql_conds)
    local status, err = self._conn:execute(sql_delete)
    if err then
        LogErr("MySql:DeleteData Fail : delete from %s : %s", table_name, err)
        return err
    end
    LogTrace("MySql:DeleteData Success: delete from %s", table_name)
end

function MySql:UpdateData(table_name, values, conds)
    local sql_values = ""
    for k, v in pairs(values) do
        if sql_values ~= "" then
            sql_values = sql_values..", "
        end
        sql_values = sql_values..k.." = "..self:ToString(v)
    end
    local sql_conds = self:ParseConds(conds)
    local sql_update = string.format([[UPDATE %s SET %s %s]], table_name, sql_values, sql_conds)
    local status, err = self._conn:execute(sql_update)
    if err then
        LogErr("MySql:UpdateData Fail : update %s : %s", table_name, err)
        return err
    end
    LogTrace("MySql:UpdateData Success: update %s", table_name)
end

function MySql:SelectData(table_name, fields, conds, orders)
    local sql_fields = ""
    for i = 1, #fields do
        if i ~= 1 then
            sql_fields = sql_fields..", "
        end
        sql_fields = sql_fields..fields[i]
    end
    local sql_conds = self:ParseConds(conds)
    local sql_orders = ""
    if orders then
        sql_orders = "ORDER BY "
        for i = 1, #orders do
            if i ~= 1 then
                sql_orders = sql_orders..", "
            end
            sql_orders = sql_orders..orders[i]
        end
    end
    local sql_select = string.format([[SELECT %s FROM %s %s %s]], sql_fields, table_name, sql_conds, sql_orders)
    local cursor, err = self._conn:execute(sql_select)
    if err then
        LogErr("MySql:SelectData Fail : select from %s : %s", table_name, err)
        return nil, err
    end
    local results = {}
    local row = cursor:fetch({}, "a")
    while row do
        local index = #results + 1
        results[index] = {}
        for k, v in pairs(row) do
            results[index][k] = v
        end
        row = cursor:fetch(row, "a")
    end
    cursor:close()
    LogTrace("MySql:SelectData Success: select from %s", table_name)
    return results, nil
end
