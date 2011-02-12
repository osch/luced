local topChildPatterns = { "comment", "entity", "xmlTag"  }

local contentStyle        = "preproc"
local contentStyleEscaped = "boldblue"

return 
{
        root = {
                style = contentStyle,
                childPatterns = { "withpreambel", "withoutpreambel" },
        },
        
        withpreambel = {
                style = contentStyle,
                beginPattern     = [[ ^(?P<preambel1>\<\?xml)(?P<preambel>.*)(?P<preambel2>\?\>) ]],
                endPattern       = [[ (?!.|\n) ]],
                beginSubstyles   = {preambel1 = "keyword", preambel="textKey", preambel2="keyword"},
                maxBeginExtend   = 200,
                maxEndExtend     = 1,
                childPatterns = topChildPatterns,
        },
        
        withoutpreambel = {
                style = contentStyle,
                beginPattern     = [[ (?=.|\n) ]],
                endPattern       = [[ (?!.|\n) ]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns = topChildPatterns,
        },
        
        comment = {
                style = "comment",
                beginPattern     = [[ \<\!\-\- ]],
                endPattern       = [[ \-\-\> ]],
                maxBeginExtend   = 5,
                maxEndExtend     = 5,
                childPatterns = {},
        },
        entity = {
                style = contentStyleEscaped,
                beginPattern     = [[ \& ]],
                endPattern       = [[ \; | \n ]],
                maxBeginExtend   = 5,
                maxEndExtend     = 5,
                childPatterns = {},
        },
        
        xmlTag = {
                style = "textKey1",
                beginPattern     = [[ (?P<tagBegin>\<\/?)(?P<tagName>\w*) ]],
                endPattern       = [[ (?P<tagEnd>\>) ]],
                beginSubstyles   = { tagBegin = "keyword", tagName = "textKey" },
                endSubstyles     = { tagEnd   = "keyword" },
                maxBeginExtend   = 3,
                maxEndExtend     = 2,
                childPatterns    = { "operator", "string1", "string2" }
        },
        
        operator = {
            style = "keyword",
            pattern = [[ \= ]],
            maxExtend = 1
        },
        string1 = {
        	style = contentStyle,
                beginPattern     = [["]],
                endPattern       = [["|\n]],
                maxBeginExtend   = 2,
                maxEndExtend     = 2,
                childPatterns    = { "entity" },
        },
        string2 = {
        	style = contentStyle,
                beginPattern     = [[']],
                endPattern       = [['|\n]],
                maxBeginExtend   = 2,
                maxEndExtend     = 2,
                childPatterns    = { "entity" },
        },
        
}
