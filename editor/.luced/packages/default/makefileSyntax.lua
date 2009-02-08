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

return 
{
        root = {
                style = "default",
                childPatterns = { "emlualine", "comment", "assignment", "define", 
                                  "keywords",  "command", 
                                  "specialDependencyLine", "dependencyLine", 
                                  "varSubst1", "varSubst2", "varSubst3",
                                  "emluaexpr", 
                                },
        },

        emlualine = {
                style = "regex",
                beginPattern     = [[^@(?!\()]],
                endPattern       = [[\n]],
                maxBeginExtend   = 2,
                maxEndExtend     = 1,
        }, 

        emluaexpr = {
                style = "regex",
                beginPattern     = [[@\(]],
                endPattern       = [[\)|$]],
                maxBeginExtend   = 2,
                maxEndExtend     = 1,
                childPatterns    = { "emluaexpr2" },
        }, 
        emluaexpr2 = {
                style = "regex",
                beginPattern     = [[\(]],
                endPattern       = [[\)|$]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = { "emluaexpr2" },
        }, 
        comment = {
                style = "comment",
                beginPattern     = [=[ (?>\s*) [#] ]=],
                endPattern       = [[ $ ]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = { "backslashInComment", "continuation", "emluaexpr" },
        },
 
        backslashInComment = {
                style = "comment",
                pattern = [[ \\ \\ ]],
                maxExtend = 2,
        },
 
        command = {
                style = "command",
                beginPattern     = [[^ [\t>] ]],
                endPattern       = [[ $ ]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = {"backslashInCommand", "continuation", "varSubst1", "varSubst2", "varSubst3"},
        }, 
 
        backslashInCommand = {
                style = "command",
                pattern = [[ \\ \\ ]],
                maxExtend = 2,
        },

        define = {
                style = "command",
                beginPattern     = [[^(?P<defineBegin>[ ]*define\b)(?P<defineIdent>[^\n]*$) ]],
                endPattern       = [[\n(?P<defineEnd>[ ]*endef\b)]],
                maxBeginExtend   = 100,
                maxEndExtend     = 100,
                beginSubstyles   = { defineBegin = "keyword", defineIdent="preproc" },
                endSubstyles     = { defineEnd = "keyword" },
                childPatterns    = {"backslashInNormal", "continuation", "varSubst1", "varSubst2", "varSubst3"},
        },
 
        backslashInNormal = {
                style = "default",
                pattern = [[ \\ \\ ]],
                maxExtend = 2,
        },
 
        assignment = {
                style = "preproc",
                beginPattern     = [[^(?>[ ]*\w+\s*)[:+?]?= ]],
                endPattern       = [[\n]],
                maxBeginExtend   = 100,
                maxEndExtend     = 1,
                childPatterns    = {"backslashInAssignment", "continuation", "emluaexpr", "comment" },
        },
 
        backslashInAssignment = {
                style = "preproc",
                pattern = [[ \\ \\ ]],
                maxExtend = 2,
        },
 
        continuation = {
                style = "keyword",
                beginPattern  = [[ \\ \n ]],
                endPattern    = [[    (?=.|\n) ]],
                maxBeginExtend   = 2,
                maxEndExtend     = 2,
                childPatterns    = {"emluaexpr", "emlualine"},
        },
 

        keywords = {
                style = "keyword",
                pattern      = [[ \b(?> include | ifeq | ifdef | ifndef| else | ifneq | define | endef | endif )\b ]],
                maxExtend    = 20,
        },
 
        specialDependencyLine = {
            style = "textKey1",
            beginPattern     = [[^(?>[ ]*(?:\.PHONY|\.INTERMEDIATE)\s*)[:](?!=) ]],
            endPattern       = [[ \n ]],
            maxBeginExtend   = 100,
            maxEndExtend     = 1,
            childPatterns    = {"varSubst1", "varSubst2", "varSubst3", "backslashInDependency", "continuation"},
        },
 

        dependencyLine = {
            style = "textKey1",
            beginPattern     = [[^(?P<target>(?>[ ]*(?>(?:\\\\)*\\\n|[^\n:])+))[:](?!=) ]],
            endPattern       = [[ \n ]],
            maxBeginExtend   = 100,
            maxEndExtend     = 1,
            beginSubstyles   = { target = "textKey" },
            childPatterns    = {"errorinDependency", "varSubst1", "varSubst2", "varSubst3", "backslashInDependency", "continuation",
                                "emluaexpr"},
        },
 
        errorinDependency = {
            style = "error",
            beginPattern     = [[^([ ]*\>|\t) ]],
            endPattern       = [[ \n ]],
            maxBeginExtend   = 5,
            maxEndExtend     = 1,
            childPatterns    = {"backslashInError", "continuation"},
        },
 
        backslashInError = {
                style = "error",
                pattern = [[ \\ \\ ]],
                maxExtend = 2,
        },

        backslashInDependency = {
                style = "textKey1",
                pattern = [[ \\ \\ ]],
                maxExtend = 2,
        },

        varSubst1 = {
            style = "preproc",
            beginPattern     = [[ \$\( ]],
            endPattern       = [[ \) | (?=\n) ]],
            maxBeginExtend   = 2,
            maxEndExtend     = 1,
            childPatterns    = {"emluaexpr", "varSubst1", "varSubst2", "varSubst4", "varSubst5", "backslashInAssignment", "continuation"},
        },

        varSubst2 = {
            style = "preproc",
            beginPattern     = [[ \$\{ ]],
            endPattern       = [[ \} | (?=\n) ]],
            maxBeginExtend   = 2,
            maxEndExtend     = 1,
            childPatterns    = {"emluaexpr", "varSubst1", "varSubst2", "varSubst4", "varSubst5", "backslashInAssignment", "continuation"},
        },

        varSubst3 = {
            style = "preproc",
            pattern     = [[ \$. ]],
            maxExtend   = 2,
        },

        varSubst4 = {
            style = "preproc",
            beginPattern     = [[ \( ]],
            endPattern       = [[ \) | (?=\n) ]],
            maxBeginExtend   = 1,
            maxEndExtend     = 1,
            childPatterns    = {"varSubst1", "varSubst2", "varSubst4", "varSubst5", "backslashInAssignment", "continuation"},
        },

        varSubst5 = {
            style = "preproc",
            beginPattern     = [[ \{ ]],
            endPattern       = [[ \} | (?=\n) ]],
            maxBeginExtend   = 1,
            maxEndExtend     = 1,
            childPatterns    = {"varSubst1", "varSubst2", "varSubst4", "varSubst5", "backslashInAssignment", "continuation"},
        },

}