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

-- The following hiliting patterns were copied 
-- from
--                  Java highlighting patterns for NEdit 5.1. Version 1.5 
--                  Author/maintainer: Joachim Lous - jlous at users.sourceforge.net
--
-- and adapted to LucED.

return
{

	root = {
        	style = "command",

                childPatterns = { "javaCode",
                                
                                 "doccomment",     "comment",           "cpluscomment", "bracesParensSigns",
                                 "string", "singlequoted"
                                },
        },
        
        javaCode = {
                style = "default",
            
                beginPattern     = [[(?P<javaCodeBegin>\{)]],
                endPattern       = [[(?P<javaCodeEnd>\})]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                
                beginSubstyles   = { javaCodeBegin = "keyword" },
                endSubstyles     = {  javaCodeEnd  = "keyword" },

                childPatterns    = { "javaCode",
                                     "doccomment",     "comment",           "cpluscomment",     "string",            "singlequoted",
                                     "hexconst",       "longconst",         "decimalconst",     "include",           "classdef",
                                     "extends",        "implthrow",         "case",             "instanceof",        "newarray",
                                     "constructorDef", "keywords",          "keyliterals",      "functiondef",       "functioncall",   
                                     "cast",           "declaration",       "variable",         "bracesParensSigns", "error",
                                   },
        },
        
        doccomment = {
        	style = "textcomment",
                beginPattern     = [[/\*\*]],
                endPattern       = [[\*/]],
                maxBeginExtend   = 5,
                maxEndExtend     = 3,
                beginSubstyles   = {},
                endSubstyles     = {},
                childPatterns    = { "doccommenttag" },
        },

        doccommenttag = {
                style = "textKey1",
                pattern          = [[\@\w*]],
                maxExtend        = 200
        },

        comment = {
        	style = "comment",
                beginPattern     = [[/\*]],
                endPattern       = [[\*/]],
                maxBeginExtend   = 2,
                maxEndExtend     = 2,
                childPatterns    = { },
        },

        cpluscomment = {
        	style = "comment",
                beginPattern     = [[//]],
                endPattern       = [[\n]],
                maxBeginExtend   = 2,
                maxEndExtend     = 1,
                childPatterns    = {  },
        },
        
        string = {
        	style = "string",
                beginPattern     = [["]],
                endPattern       = [["|\n]],
                maxBeginExtend   = 2,
                maxEndExtend     = 2,
                childPatterns    = { "stringescape" },
        },
        
        stringescape = {
                style = "string1",
                pattern          = [[(?:\\u[\dA-Faf]{4}|\\[0-7]{1,3}|\\[btnfr'"\\])]],
                maxExtend        = 100
        },
        
        singlequoted = {
        	style = "string",
                beginPattern     = [[']],
                endPattern       = [['|\n]],
                maxBeginExtend   = 2,
                maxEndExtend     = 2,
                childPatterns    = { "stringescape" },
        },

        singlequotedescape = {
                style = "string1",
                pattern          = [[(?:\\u[\dA-Faf]{4}|\\[0-7]{1,3}|\\[btnfr'"\\])(?=')]],
                maxExtend        = 100
        },
        
        singlequotedchar = {
                style = "string",
                pattern          = [[.(?=')]],
                maxExtend        = 2
        },
        
        singlequotederror = {
                style = "error",
                pattern          = [[ [^'] ]],
                maxExtend        = 1
        },
        
        hexconst = {
                style = "numericconst",
                pattern          = [[\b(?i)0[X][\dA-F]+\b]],
                maxExtend        = 40,
        },
        longconst = {
                style = "numericconst",
                pattern          = [[\b(?i)[\d]+L\b]],
                maxExtend        = 40,
        },
        
        decimalconst = {
                style = "numericconst",
                pattern          = [[(?<!\w)(?i)\d+(?:\.\d*)?(?:E[+\-]?\d+)?[FD]?|\.\d+(?:E[+\-]?\d+)?[FD]?(?!\Y)]],
                maxExtend        = 40,
        },

        include = {
        	style = "preproc",
                beginPattern     = [[\b(?:import|package)\b]],
                endPattern       = [[;|\n]],
                maxBeginExtend   = 20,
                maxEndExtend     = 2,
                childPatterns    = {  },
        },
        
        classdef = {
                style = "keyword",
                pattern          = [[\b(?:class|interface)\b\s*\n?\s*(?P<classdefName>[a-zA-Z_]\w*)]],
                maxExtend        = 200,
                substyles        = { classdefName = "type" },
        },

        extends = {
        	style = "keyword",
                beginPattern     = [[\b(?:extends)\b]],
                endPattern       = [[(?=(?:\bimplements\b|[{;]))]],
                maxBeginExtend   = 20,
                maxEndExtend     = 2,
                childPatterns    = { "extendsargument", "extendscomma", "comment", "cpluscomment", "extendserror" },
        },
        
        extendsargument = {
                style = "type",
                pattern          = [[\b[a-zA-Z_][\w\.]*(?=\s*(?:/\*.*\*/)?(?://.*)?\n?\s*(?:[,;{]|\bimplements\b))]],
                maxExtend        = 200,
        },

        extendscomma = {
                style = "type",
                pattern          = [[,]],
                maxExtend        = 1,
        },

        extendserror = {
                style = "error",
                pattern          = [[(?!(?:\bimplements\b|[{;])).]],
                maxExtend        = 1,
        },
        
        implthrow = {
        	style = "keyword",
                beginPattern     = [[\b(?:implements|throws)\b]],
                endPattern       = [[(?=[{;])]],
                maxBeginExtend   = 40,
                maxEndExtend     = 5,
                childPatterns    = { "implthrowargument", "implthrowcomma", "comment", "cpluscomment", "implthrowerror" },
        },
        
        implthrowargument = {
                style = "type",
                pattern          = [[\b[a-zA-Z_][\w\.]*(?=\s*(?:/\*.*\*/)?(?://.*)?\n?\s*[,;{])]],
                maxExtend        = 200,
        },

        implthrowcomma = {
                style = "type",
                pattern          = [[,]],
                maxExtend        = 1,
        },

        implthrowerror = {
                style = "error",
                pattern          = [[ [^{;] ]],
                maxExtend        = 1,
        },
        
        case = {
        	style = "keyword",
                beginPattern     = [[\b(?:case)\b]],
                endPattern       = [[:|(?=[;\n])]],
                maxBeginExtend   = 10,
                maxEndExtend     = 3,
                childPatterns    = { "singlequoted", "hexconst", "longconst", "decimalconst" },
        },
        
        instanceof = {
                style = "keyword",
                pattern          = [[\binstanceof\b\s*\n?\s*(?P<instanceofClass>[a-zA-Z_][\w.]*)]],
                maxExtend        = 100,
                substyles        = { instanceofClass = "type" },
        
        },
        
        newarray = {
                style = "keyword",
                pattern          = [[new\s*[\n\s]\s*(?P<newArrayType>[a-zA-Z_][\w\.]*)\s*\n?\s*(?=\[)]],
                maxExtend        = 100,
                substyles        = { newArrayType = "type" },
        
        },
        
        constructorDef = {
                style = "subroutine",
                pattern          = [[\b(?P<constructorDefModifier>abstract|final|native|private|protected|public|static|synchronized)\s*[\n|\s]\s*[a-zA-Z_]\w*\s*\n?\s*(?=\()]],
                maxExtend        = 100,
                substyles        = { constructorDefModifier = "keyword" },
        },
        
        keywords = {
                style = "keyword",
                pattern          = [[\b(?: abstract| final     | native       | private  | protected | public
                                         | static  | transient | synchronized | volatile | catch     | do
                                         | else    | finally   | for          | if       | return    | switch
                                         | throw   | try       | while        | new      | super     | this
                                    )\b]],
                maxExtend        = 20,
        },
        
        keyliterals = {
                style = "numericconst",
                pattern          = [[\b(?: false | true | null
                                    )\b]],
                maxExtend        = 20,
        },
        
        functiondef = {
                style = "default",
                pattern          = [[\b(?P<functionDefType>[a-zA-Z_][\w\.]*)\b(?P<functionDefTypeBrackets>(?:\s*\[\s*\])*)\s*[\n|\s]\s*\b[a-zA-Z_]\w*\b\s*\n?\s*(?=\()]],
                maxExtend        = 100,
                substyles        = { functionDefType = "type", functionDefTypeBrackets = "keyword" },
        },
        
        functioncall = {
                style = "default",
                pattern          = [[\b[a-zA-Z_]\w*\b\s*\n?\s*(?=\()]],
                maxExtend        = 100,
                substyles        = { functionDefType = "type", functionDefTypeBrackets = "keyword" },
        },
        

        cast = {
                style = "keyword",
                pattern          = [[[^\w\s]\s*\n?\s*\(\s*(?P<castType>[a-zA-Z_][\w\.]*)\s*\)]],
                maxExtend        = 1,
                substyles        = { castType = "type" },
        },

        declaration = {
                style = "type",
                pattern          = [[\b[a-zA-Z_]\w*(?:\.\w+)*(?:(?:\<[^\>]*\>)|\b)(?P<declarationBrackets>(:?\s*\[\s*\]\s*)*)(?=\s*\n?\s*(?!instanceof)[a-zA-Z_]\w*)]],
                maxExtend        = 100,
                substyles        = { declarationBrackets = "keyword" },
        },
        
        variable = {
                style = "identifier1",
                pattern          = [[(?:\b|\$|\@)[a-zA-Z_]\w*\b]],
                maxExtend        = 100,
        },
        
        bracesParensSigns = {
                style = "keyword",
                pattern          = [[ [()[\]\-+*/%=,.;:<>!|&^?] ]],
                maxExtend        = 10,
        },
        
        error = {
                style = "error",
                pattern          = [[ [^}] ]],
                maxExtend        = 1,
        },
}
