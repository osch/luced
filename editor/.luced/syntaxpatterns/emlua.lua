

return
{

        root = {
                style = "default",

                childPatterns = {
                                 "emluaexpr", "emlualine",
                                 "emrexxexpr", "emrexxline"
                                },
        },
        
        emluaexpr = {
                style = "regex",
                beginPattern     = [[@\(]],
                endPattern       = [[\)|$]],
                maxBeginExtend   = 2,
                maxEndExtend     = 1,
                childPatterns    = { "emluaexpr2" },
        }, 
        emlualine = {
                style = "regex",
                beginPattern     = [[^@]],
                endPattern       = [[\n]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = { "emrexxexpr" },
        }, 
        emluaexpr2 = {
                style = "regex",
                beginPattern     = [[\(]],
                endPattern       = [[\)|$]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = { "emluaexpr2" },
        }, 
        ----------------------------------------------------
        emrexxexpr = {
                style = "boldblue",
                beginPattern     = [[\$\(]],
                endPattern       = [[\)|\$]],
                maxBeginExtend   = 2,
                maxEndExtend     = 1,
                childPatterns    = { "emluaexpr", "emrexxexpr2" },
        }, 
        emrexxline = {
                style = "boldblue",
                beginPattern     = [[^\$]],
                endPattern       = [[\n]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = { "emluaexpr" },
        }, 
        emrexxexpr2 = {
                style = "boldblue",
                beginPattern     = [[\(]],
                endPattern       = [[\)|\$]],
                maxBeginExtend   = 1,
                maxEndExtend     = 1,
                childPatterns    = { "emrexxexpr2" },
        }, 
}
