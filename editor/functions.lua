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
    s = string.gsub(s, [[\]], [[\\]])
    s = string.gsub(s, [["]], [[\"]])
    s = string.gsub(s, "\n",  [[\n]])
    s = string.gsub(s, "\r",  [[\r]])
    return '"'..s..'"'
end

return  {
            argList        = argList,
            filledArgList  = filledArgList,
            typedArgList   = typedArgList,
            classList      = classList,
            capitalize     = capitalize,
            upperize       = upperize,
            stringLiteral  = stringLiteral,
        }
