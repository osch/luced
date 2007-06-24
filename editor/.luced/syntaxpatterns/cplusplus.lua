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


return
{

	root = {
        	style = "default",

                childPatterns = {
                                 "emlualine", "emluaexpr",
                                 "string",    "char",         "comment1",         "comment2",     "preprop",      "template",     
                                 "sizeof",
                                 "newcast",   "oldcast",      "namespace",        "class",        "keywords",     "typekeywords", 
                                 "decl",      "operators"  
                                },
        },
        
        keywords = {
        	style = "keyword",
                pattern          = [[
                                       \b(?> true   |false    |this    |static    |inline   |typedef  |if       |using     |class    |else
                                            |do     |for      |return  |try       |catch    |public   |private  |protected |virtual  |while
                                            |switch |case     |break   |continue  |template |typename |struct   |explicit  |mutable  |throw
                                            |new    |operator |sizeof  |extern    |friend   |delete
                                       )\b
                                   ]],
                maxExtend        = 15,
        }, 
        emlualine = {
        	style = "regex",
                beginPattern     = [[^@]],
                endPattern       = [[\n]],
                maxBeginExtend   = 1,
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
        }, 
        operators = {
        	style = "keyword",
                pattern          = [=[ [\-<>()/!:,;+*{}\\\[\]&%|] ]=],
                maxExtend        = 15,
        }, 
        typekeywords = {
        	style = "type",
                pattern          = [[\b(?:const|unsigned|signed)\b]],
                maxExtend        = 15,
        }, 
        namespace = {
        	style = "default",
                beginPattern     = [[(?P<namespaceBegin>\bnamespace\b)]],
                endPattern       = [[(?P<namespaceEnd>[;{}():])]],
                maxBeginExtend   = 15,
                maxEndExtend     = 1,
                beginSubstyles   = {namespaceBegin = "keyword"},
                endSubstyles     = {namespaceEnd   = "keyword"},
                childPatterns    = {},
        },
        newcast = {
        	style = "type",
                beginPattern     = [[(?P<newcastBegin>\b(?>(?>reinterpret_cast|const_cast|static_cast|dynamic_cast)\s*<))]],
                endPattern       = [[(?P<newcastEnd>[;{}()>])]],
                maxBeginExtend   = 15,
                maxEndExtend     = 1,
                beginSubstyles   = {newcastBegin = "keyword"},
                endSubstyles     = {newcastEnd   = "keyword"},
                childPatterns    = {"template2", "templateKeywords", "keywords", "decl"},
        },
        oldcast = {
        	style = "type",
                pattern          = [[(?P<oldcastBegin>\()(?> 
                                                             [^()]*[*&][*&\s]* 
                                                           | (?>(?>int|char|unsigned|signed|const|long|short|double|float|void|\s*)*)
                                                          )(?P<oldcastEnd>\))]],
                maxExtend        = 80,
                substyles        = { oldcastBegin = "keyword", oldcastEnd = "keyword" },
        },
        template = {
        	style = "type",
                beginPattern     = [[(?P<templateBegin>\b(?>template\s*<))]],
                endPattern       = [[(?P<templateEnd>[;{}()>])]],
                maxBeginExtend   = 15,
                maxEndExtend     = 1,
                beginSubstyles   = {templateBegin = "keyword"},
                endSubstyles     = {templateEnd   = "keyword"},
                childPatterns    = {"comment1", "comment2", "emlualine", "emluaexpr", "template2", "templateKeywords", "keywords", "decl"},
        },
        template2 = {
        	style = "type",
                beginPattern     = [[(?P<templateBegin><)]],
                endPattern       = [[(?P<templateEnd>[>]|(?=[{}()]))]],
                maxBeginExtend   = 15,
                maxEndExtend     = 1,
                childPatterns    = {"template2"},
        },
        templateKeywords = {
        	style = "keyword",
                pattern          = [=[ [,] ]=],
                maxExtend        = 1,
        }, 
        class = {
        	style = "type",
                beginPattern     = [[(?P<classBegin>\bclass|struct|enum\b)]],
                endPattern       = [[(?P<classEnd>[;{}()])]],
                maxBeginExtend   = 15,
                maxEndExtend     = 1,
                beginSubstyles   = {classBegin = "keyword"},
                endSubstyles     = {classEnd   = "keyword"},
                childPatterns    = { "emlualine", "emluaexpr", "comment1", "comment2", "keywords", "classoperators"},
        },
        classoperators = {
        	style = "keyword",
                pattern          = [=[ [():] ]=],
                maxExtend        = 15,
        }, 
        comment1 = {
        	style = "comment",
                beginPattern     = [[//]],
                endPattern       = [[\n]],
                maxBeginExtend   = 2,
                maxEndExtend     = 1,
                childPatterns    = { "emlualine", "emluaexpr" },
        },
        
        comment2 = {
        	style = "comment",
                beginPattern     = [[/\*]],
                endPattern       = [[\*/]],
                maxBeginExtend   = 2,
                maxEndExtend     = 2,
                childPatterns    = { "emlualine", "emluaexpr" },
        },
        
        preprop = {
        	style = "preproc",
                beginPattern     = [[^[\t\ ]*\#]],
                endPattern       = [[\n]],
                maxBeginExtend   = 200,
                maxEndExtend     = 1,
                childPatterns    = {"emlualine", "emluaexpr", "comment1inpreprop", "comment2", "prepropescape"},
        },
        
        comment1inpreprop = {
        	style = "comment",
                beginPattern     = [[//]],
                endPattern       = [[(?=\n)]],
                maxBeginExtend   = 2,
                maxEndExtend     = 1,
                childPatterns    = { "emlualine", "emluaexpr" },
        },
        
        prepropescape = {
        	style = "preproc",
                pattern          = [[\\.|\\\n]],
                maxExtend        = 2,
        },

        operator = {
        	style = "keyword",
                pattern          = [=[ [+*\[\]<>\-\?:!&] ]=],
                maxExtend        = 4,
        }, 
        
        decl = {
        	style = "default",
                beginPattern     =    [[
                                        (?P<declType>(?>\b(?>const|unsigned|struct)\b(?>\s+))*
                                                     (?>(?>\:\:)?\b[A-Za-z_](?:\w|\s*::(?>\s*(?>(?>typename|struct|signed|unsigned|template|const)\s*)*))*)
                                                     (?>(?> \s | \b(?>const|unsigned|struct)\b )*)
                                                     (?P<templparm><((?>[^<\->;{}]*)|(?P>templparm))*>(?>\s*::\s*\w*(?P>templparm)?)?)?
                                                     
                                                     (?>(?>\s*)(?<=\s)[&*]+(?=\S)
                                                       |(?>\s*)(?<=\s)[&*]+(?=\s*\n)
                                                       |(?>\s*)(?<=\S)[&*]+(?=\s)
                                                       |(?>\s*))
                                                     (?>\s*)
                                        )
                                        (?! [~%*&?<>;(),\[\]!=\-+/|.:}] )
                                      ]],
                                   
                endPattern       = [[(?=[@\[;,{()}]|\b(?:for|while|do)\b)|(?=^[\t\ ]*\#|[=)])]],
                maxBeginExtend   = 200,
                maxEndExtend     = 5,
                childPatterns    = { "comment1", "comment2", "typekeywords", "operatorInDecl"},
                beginSubstyles   = { declType = "type" }, --emluaInType = "regex" },
                endSubstyles     = {},
        },
        
        operatorInDecl = {
        	style = "keyword",
                pattern          = [[ [:] | (?>operator(?>\s*)(?>[<>+\-*=]|->|==|!=|<<|>>|\[\])) ]],
                maxExtend        = 20,
        }, 

        char = {
        	style = "string",
                beginPattern     = [[(?P<charBegin>')]],
                endPattern       = [[(?P<charEnd>')|\n]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = {"stringescape"},
                beginSubstyles   = {charBegin = "boldstring"},
                endSubstyles     = {charEnd   = "boldstring"},
        },

        string = {
        	style = "string",
                beginPattern     = [[(?P<stringBegin>")]],
                endPattern       = [[(?P<stringEnd>")|\n]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = { "emlualine", "emluaexpr", "stringescape"},
                beginSubstyles   = {stringBegin = "boldstring"},
                endSubstyles     = {stringEnd   = "boldstring"},
        },

        stringescape = {
        	style = "boldstring",
                pattern          = [[\\\d{1,3}|\\x[a-fA-F0-9]{1,2}|\\.|\\\n]],
                maxExtend        = 2,
        },
        
        sizeof = {
        	style = "type",
                beginPattern     = [[(?P<sizeofBegin>\bsizeof\s*\()]],
                endPattern       = [=[(?P<sizeofEnd>[);])]=],
                maxBeginExtend   = 100,
                maxEndExtend     = 1,
                childPatterns    = { "emlualine", "emluaexpr"},
                beginSubstyles   = {sizeofBegin = "keyword"},
                endSubstyles     = {sizeofEnd   = "keyword"},
        },
}
