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
                childPatterns = {"comment1", "comment2", "preprop", "for", "if", "operator", "return", 
                                 "curlelse", "case", "catch", "decl", "standalonesemicolon", "string", "normal"},
        },
        
        comment1 = {
        	style = "comment",
                beginPattern     = [[//]],
                endPattern       = [[\n]],
                maxBeginExtend   = 2,
                maxEndExtend     = 1,
                childPatterns    = {},
        },
        
        comment2 = {
        	style = "comment",
                beginPattern     = [[/\*]],
                endPattern       = [[\*/]],
                maxBeginExtend   = 2,
                maxEndExtend     = 2,
                childPatterns    = {"comment2"},
        },
        
        preprop = {
        	style = "preproc",
                beginPattern     = [[^[\t\ ]*\#]],
                endPattern       = [[\n]],
                maxBeginExtend   = 200,
                maxEndExtend     = 1,
                childPatterns    = {"comment1inpreprop", "comment2", "prepropescape"},
        },
        
        comment1inpreprop = {
        	style = "comment",
                beginPattern     = [[//]],
                endPattern       = [[(?=\n)]],
                maxBeginExtend   = 2,
                maxEndExtend     = 1,
                childPatterns    = {},
        },
        
        prepropescape = {
        	style = "preproc",
                pattern          = [[\\.|\\\n]],
                maxExtend        = 2,
        },

        ["if"] = {
        	style = "default",
                beginPattern     = [[\b(?P<ifBegin1>(?:if|while|switch)\b(?>\s*)\()]],
                endPattern       = [[(?P<ifEnd1>\)|\{|\})]],
                maxBeginExtend   = 7,
                maxEndExtend     = 1,
                childPatterns    = {"paren", "comment1", "comment2", "operator"},
                beginSubstyles   = {ifBegin1 = "keyword"},
                endSubstyles     = {ifEnd1   = "keyword"},
        },
        
        paren = {
        	style = "default",
                beginPattern     = [[(?P<parenBegin1>\()]],
                endPattern       = [[(?P<parenEnd1>(?:\)|\{|\}))]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = {"paren", "comment1", "comment2", "operator", "string"},
                beginSubstyles   = {parenBegin1 = "keyword"},
                endSubstyles     = {parenEnd1  = "keyword"},
        },
        
        operator = {
        	style = "keyword",
                pattern          = [[\+|\*|\[|\]|\<|\>|\-|\!|\=]],
                maxExtend        = 4,
        }, 
        
        ["return"] = {
        	style = "default",
                beginPattern     = [[(?P<returnKW>return|throw)]],
                endPattern       = [[(?P<returnEnd>;)]],
                maxBeginExtend   = 10,
                maxEndExtend     = 0,
                childPatterns    = {"paren"},
                beginSubstyles   = {returnKW  = "keyword"},
                endSubstyles     = {returnEnd = "keyword"},
        },
        
        curlelse = {
        	style = "keyword",
                pattern          = [[\{|\}|\b(?:if|else|do|break|typedef|static|inline|try|using|namespace)\b]],
                maxExtend        = 10,
        },
        
        catch = {
        	style = "default",
                beginPattern     = [[(?P<catchBegin>\bcatch)]],
                endPattern       = [[(?=^[\t\ ]*\#|=|\{|\})]],
                maxBeginExtend   = 7,
                maxEndExtend     = 20,
                childPatterns    = {"comment1", "comment2", "catchparen"},
                beginSubstyles   = {catchBegin = "keyword"},
                endSubstyles     = {},
        },
        
        catchparen = {
        	style = "default",
                beginPattern     = [[(?P<catchParenBegin>\()]],
                endPattern       = [[(?P<catchParenEnd>\))]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = {"comment1", "comment2", "catchparen", "declinparen"},
                beginSubstyles   = {catchParenBegin = "keyword"},
                endSubstyles     = {catchParenEnd   = "keyword"},
        },
        
        case = {
        	style = "default",
                beginPattern     = [[(?P<caseBegin>\bcase)\s]],
                endPattern       = [[(?P<caseEnd>\:|\;|\{|\})]],
                maxBeginExtend   = 10,
                maxEndExtend     = 0,
                childPatterns    = {"paren", "comment1", "comment2", "operator"},
                beginSubstyles   = {caseBegin = "keyword"},
                endSubstyles     = {caseEnd   = "keyword"},
        },
        
        decl = {
        	style = "default",
                beginPattern     = [[(?P<declType>\b(?:(?:const|unsigned|struct)(?>\s+))*\b(?>(?:\w|::)+)\b(?>\s*)(?!;|,))]]..
                                   [[(?!\(|\[|\=|\-|\+|\||\.|\:)]],
                endPattern       = [[(?P<declEnd>;|\{|\})|(?=^[\t\ ]*\#|=)]],
                maxBeginExtend   = 40,
                maxEndExtend     = 5,
                childPatterns    = {"comment1", "comment2", "declparen"},
                beginSubstyles   = {declType = "type"},
                endSubstyles     = {declEnd  = "keyword"},
        },
        
        declinparen = {
        	style = "default",
                beginPattern     = [[(?P<declType>\b(?:(?:const|unsigned|struct)(?>\s+))*\b(?>(?:\w|::)+)\b(?>\s*))]]..
                                   [[(?!\(|\[|\=|\-|\+|\||\.|\:)]],
                endPattern       = [[(?P<declEnd>;|,|\{|\})|(?=^[\t\ ]*\#)|(?=\))]],
                maxBeginExtend   = 40,
                maxEndExtend     = 5,
                childPatterns    = {"comment1", "comment2", "declparenparen"},
                beginSubstyles   = {declType = "type"},
                endSubstyles     = {declEnd  = "keyword"},
        },

        declparen = {
        	style = "default",
                beginPattern     = [[(?P<declparenbegin>\()]],
                endPattern       = [[(?P<declparenend>\))|(?=^[\t\ ]*\#)|(?=\{|\}|;)]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = {"comment1", "comment2", "declinparen", "declparen"},
                beginSubstyles   = { declparenbegin = "keyword"},
                endSubstyles     = { declparenend = "keyword"},
        },
        
        declparenparen = {
        	style = "default",
                beginPattern     = [[\(]],
                endPattern       = [[\)|(?=^[\t\ ]*\#)|(?=\{|\}|;)]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = {"comment1", "comment2", "declparenparen"},
                beginSubstyles   = {},
                endSubstyles     = {},
        },
        
        standalonesemicolon = {
        	style            = "keyword",
                pattern          = [[;]],
                maxExtend        = 1,
        },

        normal = {
        	style = "default",
        
                beginPattern     = [[\S]],
                endPattern       = [[(?P<normalEnd>;|\{|\})|(?=^[\t\ ]*\#)]],
                maxBeginExtend   = 1,
                maxEndExtend     = 5,
                childPatterns    = {"paren", "operator", "comment1", "comment2", "string"},
                endSubstyles     = {normalEnd = "keyword"},
        },
        
        ["for"] = {
        	style = "default",
                beginPattern     = [[\b(?P<forBegin1>for\b(?>\s*)\()]],
                endPattern       = [[(?P<forEnd1>\)|\{|\})]],
                maxBeginExtend   = 7,
                maxEndExtend     = 1,
                childPatterns    = {"paren", "comment1", "comment2", "operator", "forOperator"},
                beginSubstyles   = {forBegin1 = "keyword"},
                endSubstyles     = {forEnd1   = "keyword"},
        },
        
        forOperator = {
        	style            = "keyword",
                pattern          = [[\+|\*|\[|\]|\<|\>|\-|\!|\=|\;]],
                maxExtend        = 4,
        },
        
        string = {
        	style = "string",
                beginPattern     = [[(?P<stringBegin>")]],
                endPattern       = [[(?P<stringEnd>")|\n]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = {"stringescape"},
                beginSubstyles   = {stringBegin = "boldstring"},
                endSubstyles     = {stringEnd   = "boldstring"},
        },

        stringescape = {
        	style = "boldstring",
                pattern          = [[\\\d{1,3}|\\x[a-fA-F0-9]{1,2}|\\.|\\\n]],
                maxExtend        = 2,
        },
}
