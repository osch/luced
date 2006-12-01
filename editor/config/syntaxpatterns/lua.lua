-------------------------------------------------------------------------------------
--
--   LucED - The Lucid Editor
--
--   Copyright (C) 2005-2006 Oliver Schmidt, osch at luced dot de
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


syntaxPatterns = {

	root = {
        	style = "default",
                childPatterns = {"comment1", "string1", "string2", "string3", "keyword"},
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
                pattern     = [[\+|\-|\*|\/|\%|\^|\#|\=\=|\~\=|\<\=|\>\=|\<|\>|\=|\(|\)|\{|\}|]]..
                              [[\[|\]|\;|\:|\,|\.|\.\.|\.\.\.|\blocal\b|\btrue\b|\bfalse\b|\band\b|]]..
                              [[\bbreak\b|\bdo\b|\belse\b|\belseif\b|\bend\b|\bfalse\b|\bfor\b|]]..
                              [[\bfunction\b|\bif\b|\bin\b|\blocal\b|\bnil\b|\bnot\b|\bor\b|\brepeat\b|]]..
                              [[\breturn\b|\bthen\b|\btrue\b|\buntil\b|\bwhile]],
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

        string3 = {
        	style = "string",
                beginPattern     = [[(?P<stringBegin3>\[\[)]],
                endPattern       =  "(?P<stringEnd3>\\]\\])",
                maxBeginExtend   = 2,
                maxEndExtend     = 2,
                beginSubstyles   = {stringBegin3 = "keyword"},
                endSubstyles     = {stringEnd3   = "keyword"},
        },
 
        stringescape = {
        	style = "boldstring",
                pattern          = [[\\\d{1,3}|\\.|\\\n]],
                maxExtend        = 2,
        },
        
}
