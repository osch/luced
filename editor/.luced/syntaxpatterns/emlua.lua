

return
{

	root = {
        	style = "default",

                childPatterns = {
                                 "emluaexpr", "emlualine"
                                },
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
                childPatterns    = { "emluaexpr2" },
        }, 
}
