-------------------------------------------------------------------------------------
--
--   LucED - The Lucid Editor
--
--   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
--
--   This program is free software; you can redistribute it and/or modify it
--   under the terms of the GNU General Public License Version 2 as published
--   by the Free Software Foundation in June 1991.
--
--   This program is distributed in the hope that it will be useful, but WITHOUT
--   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
--   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
--   more details.
--
--   You should have received a copy of the GNU General Public License along with 
--   this program; if not, write to the Free Software Foundation, Inc., 
--   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
--
-------------------------------------------------------------------------------------


--------------------------------------------------------------------------------
-- lua functions for template code generation.
--------------------------------------------------------------------------------

local append = table.insert
local concat = table.concat
local gsub   = string.gsub
local sort   = table.sort

local function buildList(itemCount, buildItemFunc)
    local t = {}
    for i = 1, itemCount do
       append(t, buildItemFunc(i))
       if i < itemCount then
         append(t, ", ")
       end
    end
    return concat(t)
end

local function buildFilledList(itemCount, maxCount, buildItemFunc, filler)
    local t = {}
    for i = 1, itemCount do
       append(t, buildItemFunc(i))
       if i < maxCount then
         append(t, ", ")
       end
    end
    for i = itemCount + 1, maxCount do
       append(t, filler)
       if i < maxCount then
         append(t, ", ")
       end
    end
    return concat(t)
end

local function argList(pref, number)
    return buildList(number, function(i) return pref..i end)
end

local function filledArgList(pref, filler, number, maxNumber)
    return buildFilledList(number, maxNumber, function(i) return pref..i end, filler)
end

local function typedArgList(typePref, argPref, number)
    return buildList(number, function(i) return typePref..i.." "..argPref..i end)
end

local function classList(pref, number)
    return buildList(number, function(i) return "class "..pref..i end)
end

local function capitalize(s)
    return string.upper(string.sub(s, 1, 1)) .. string.sub(s, 2)
end

local function upperize(s)
    local rslt = {}
    local lastUpper = ""
    for lowers, digits1, upper, digits2, pos in string.gmatch(s, "(%l*)(%d*)(%u*)(%d*)()") do
        if #lastUpper + #lowers > 0 then
            append(rslt, lastUpper..string.upper(lowers))
        end
        if #digits1 > 0 then
            append(rslt, digits1)
        end
        if #upper > 1 then
            if pos < #s then
                append(rslt, string.sub(upper, 1, -2))
                lastUpper =  string.sub(upper, -1, -1)
            else
                append(rslt, upper)
                lastUpper = ""
            end
        else
            lastUpper = upper
        end
        if #digits2 > 0 then
            append(rslt, digits2)
        end
    end
    return concat(rslt, "_")
end

local function stringLiteral(s)
    s = gsub(s, [[\]], [[\\]])
    s = gsub(s, [["]], [[\"]])
    s = gsub(s, "\n",  [[\n]])
    s = gsub(s, "\r",  [[\r]])
    return '"'..s..'"'
end

local function serializeValueTo(c, rslt)
    local t = type(c)
    if t == "number" or t == "boolean" or t == "nil" then
        rslt[#rslt + 1] = tostring(c)
    elseif t == "string" then
        rslt[#rslt + 1] = '"'
        rslt[#rslt + 1] = gsub(gsub(c, "[\"\\]", "\\%0"),
                                       "\n",     "\\n")
        rslt[#rslt + 1] = '"'
    else
        error("Serialization not possible for type "..t)
    end
end

local serializingTable = {}

local function serialize(c)
    local rslt = {}
    if type(c) == "table" then
        if serializingTable[c] then
            error("object serialization impossible")
        end
        serializingTable[c] = true
        local n = #c
        local keyList = {}
        for k, _ in pairs(c) do
            if type(k) ~= "number" or k < 1 or n < k then
                keyList[#keyList + 1] = k
            end
        end
        sort(keyList)
        rslt[#rslt + 1] = '{'
        for i = 1, n do
            local v = c[i]
            rslt[#rslt + 1] = serialize(v)
            if #keyList > 0 or i < n then
                rslt[#rslt + 1] = ','
            end
        end
        for i , k in ipairs(keyList) do
            local v = c[k]
            rslt[#rslt + 1] = '['   serializeValueTo(k, rslt)
            rslt[#rslt + 1] = ']='
            rslt[#rslt + 1] = serialize(v)
            if i < #keyList then
                rslt[#rslt + 1] = ','
            end
        end
        rslt[#rslt + 1] = '}'
        serializingTable[c] = nil
    else
        serializeValueTo(c, rslt)
    end
    return concat(rslt)
end

return  {
            argList        = argList,
            filledArgList  = filledArgList,
            typedArgList   = typedArgList,
            classList      = classList,
            capitalize     = capitalize,
            upperize       = upperize,
            stringLiteral  = stringLiteral,
            serialize      = serialize,
        }
