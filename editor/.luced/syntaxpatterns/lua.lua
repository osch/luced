-------------------------------------------------------------------------------------
--
--   LucED - The Lucid Editor
--
--   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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



local patterns = 
{
	root = {
        	style = "default",
                childPatterns = {"string1", "string2"},
        },
        
        comment1 = {
        	style = "comment",
                beginPattern     = [[--]],
                endPattern       = [[\n]],
                maxBeginExtend   = 2,
                maxEndExtend     = 1,
                childPatterns    = {},
        },
        
        keyword = {
        	style = "keyword",
                pattern     = [[
                                \+|\-|\*|\/|\%|\^|\#|\=\=|\~\=|\<\=|\>\=|\<|\>|\=|\(|\)|\{|\}|
                                \[|\]|\;|\:|\,|\.|\.\.|\.\.\.

                                |\b(?> local   |true  |false  |and    |false  |for    |
                                       break   |do    |else   |elseif |end    |or     |
                                       function|if    |in     |local  |nil    |not    |
                                       return  |then  |true   |until  |while  |repeat
                                 )\b
                              ]],
                maxExtend   = 10,
        },

        string1 = {
        	style = "string",
                beginPattern     = [[(?P<stringBegin1>")]],
                endPattern       = [[(?P<stringEnd1>")|\n]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = {"stringescape"},
                beginSubstyles   = {stringBegin1 = "keyword"},
                endSubstyles     = {stringEnd1   = "keyword"},
        },

        string2 = {
        	style = "string",
                beginPattern     = [[(?P<stringBegin2>\')]],
                endPattern       =  "(?P<stringEnd2>\')|\n",
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = {"stringescape"},
                beginSubstyles   = {stringBegin2 = "keyword"},
                endSubstyles     = {stringEnd2   = "keyword"},
        },

        stringescape = {
        	style = "boldstring",
                pattern          = [[\\\d{1,3}|\\.|\\\n]],
                maxExtend        = 2,
        },
        
}

local append = table.insert
local format = string.format
local rep    = string.rep

local maxNumberOfEqualSigns = 20
local rootChildPatterns     = patterns.root.childPatterns

for i = 0, maxNumberOfEqualSigns do
    local patternName = format("bracketString%d", i)
    local equalSigns  = rep("=", i)
    append(rootChildPatterns, patternName)
    patterns[patternName] = {
                                style = "string",
                                beginPattern     = format([[(?P<%sBegin>\[%s\[)]], patternName, equalSigns),
                                endPattern       = format([[(?P<%sEnd>\]%s\])]],   patternName, equalSigns),
                                maxBeginExtend   = 2 + i,
                                maxEndExtend     = 2 + i,
                                beginSubstyles   = { [format("%sBegin", patternName)] = "keyword" },
                                endSubstyles     = { [format("%sEnd",   patternName)] = "keyword"},
                            }
end

append(rootChildPatterns, "comment1")
append(rootChildPatterns, "keyword")

return patterns
