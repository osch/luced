return 
{
        root = {
                style = "textcomment",
                childPatterns = {  "embeddedJava", "scriptTag", "xmlTag",
                                },
        },
        
        embeddedJava = {
                style = "regex",
                beginPattern     = [[(?P<embedBegin>\<\%) ]],
                endPattern       = [[(?P<embedEnd>\%\>) ]],
                beginSubstyles   = { embedBegin = "regex" },
                endSubstyles     = { embedEnd   = "regex" },
                maxBeginExtend   = 3,
                maxEndExtend     = 2,

        },
        
        scriptTag = {
                style = "default",
                beginPattern     = [[ (?P<scriptBeginOpen>\<)(?P<scriptBeginName>script)(?P<scriptBeginArgs>.*)(?P<scriptBeginClose>\>) ]],
                endPattern       = [[ (?P<scriptEndOpen>\<\/)(?P<scriptEndName>script)(?P<scriptEndClose>\>) ]],
                beginSubstyles   = { scriptBeginOpen="keyword", scriptBeginName="textKey", scriptBeginArgs="textKey1", scriptBeginClose="keyword" },
                endSubstyles     = { scriptEndOpen="keyword", scriptEndName="textKey", scriptEndClose="keyword" },
                maxBeginExtend   = 10,
                maxEndExtend     = 2,
                childPatterns    = { "embeddedJava" },
        },
        
        xmlTag = {
                style = "textKey1",
                beginPattern     = [[ (?P<tagBegin>\<\/?)(?P<tagName>\w*) ]],
                endPattern       = [[ (?P<tagEnd>\>) ]],
                beginSubstyles   = { tagBegin = "keyword", tagName = "textKey" },
                endSubstyles     = { tagEnd   = "keyword" },
                maxBeginExtend   = 3,
                maxEndExtend     = 2,
                childPatterns    = { "string", "embeddedJava" }
        },
        
        string = {
        	style = "textKey1",
                beginPattern     = [["]],
                endPattern       = [["|\n]],
                maxBeginExtend   = 2,
                maxEndExtend     = 2,
                childPatterns    = { "stringescape", "embeddedJava" },
        },
        
        stringescape = {
                style = "textKey1",
                pattern          = [[(?:\\u[\dA-Faf]{4}|\\[0-7]{1,3}|\\[btnfr'"\\])]],
                maxExtend        = 100
        },

}
