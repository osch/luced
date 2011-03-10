-------------------------------------------------------------------------------------
--
--   LucED - The Lucid Editor
--
--   Copyright (C) 2005-2011 Oliver Schmidt, oliver at luced dot de
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

return
{
	root = {
        	style = "default",
        	childPatterns = { "comment", "controls", "builtin", "string1", "string2", 
        	                  "subst", "cmdsubst", "varassign" }
        },
        
        comment = {
        	style = "comment",
                pattern     = [=[\#(?:.*)\n]=],
                maxExtend   = 4000,
        },
        
        
        controls = {
        	style = "keyword",
                pattern     = [[
                                \+|\-|\*|\%|\^|\#|\=\=|\~\=|\<\=|\>\=|\<|\>|\=|\(|\)|\{|\}|
                                \[|\]|\;|\&\&|\|\||\|

                                |\b(?: if      |then  |else   |elif   |fi     |function
                                      |exit    |case  |esac   |for    |in     |do
                                      |done    |while
                                 )\b
                              ]],
                maxExtend   = 10,
        },
        
        builtin = {
        	style = "textKey1",
                pattern     = [[
                                \b(?: export   |alias |cd     |local  
                                 )\b
                              ]],
                maxExtend   = 10,
        },
        
        cmdsubst = {
        	style = "textKey1",
                beginPattern     = [[`]],
                endPattern       = [[`]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = {"cmdsubstscape", "subst"},
        },

        subst = {
        	style = "preproc",
                pattern          = [[\$(?:[-*@#?$!0-9]|[a-zA-Z_][0-9a-zA-Z_]*)]],
                maxExtend        = 200,
        },
        varassign = {
        	style = "preproc",
                pattern          = [[(?:[a-zA-Z_][0-9a-zA-Z_]*)(?=\=)]],
                maxExtend        = 200,
        },

        string1 = {
        	style = "string",
                beginPattern     = [[(?P<stringBegin1>")]],
                endPattern       = [[(?P<stringEnd1>")]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = {"stringescape", "subst"},
                beginSubstyles   = {stringBegin1 = "keyword"},
                endSubstyles     = {stringEnd1   = "keyword"},
        },

        string2 = {
        	style = "string",
                beginPattern     = [[(?P<stringBegin2>\')]],
                endPattern       = [[(?P<stringEnd2>\')]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = {},
                beginSubstyles   = {stringBegin2 = "keyword"},
                endSubstyles     = {stringEnd2   = "keyword"},
        },

        stringescape = {
        	style = "boldstring",
                pattern          = [[\\\d{1,3}|\\.|\\\n]],
                maxExtend        = 2,
        },
        
        cmdsubstscape = {
        	style = "textKey",
                pattern          = [[\\\d{1,3}|\\.|\\\n]],
                maxExtend        = 2,
        },
        
}


