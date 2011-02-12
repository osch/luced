return
{
    { 
      name            = "cplusplus",
      syntaxName      = "this.cplusplus",
      fileNameRegex   = [[^.*\.(?:CPP|HPP|H|cpp|hpp|cxx|hxx|c|h|cc|hh)(?:\.emlua)?$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
    
    {
      name            = "jsp",
      syntaxName      = "this.jsp",
      fileNameRegex   =  [[^.*\.(jspf?|html?)$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
    
    {
      name            = "xml",
      syntaxName      = "this.xml",
      fileNameRegex   =  [[^.*\.(xml|xsd)$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
    
    {
      name            = "lua",
      syntaxName      = "this.lua",
      fileNameRegex   =  [[^.*\.lua$|^.*\.lua\.emlua$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
    
    {
      name            = "rexx",
      syntaxName      = "this.rexx",
      fileNameRegex   =  [[.*\.rexx?|.*\.r]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
    
    {
      name            = "emlua",
      syntaxName      = "this.emlua",
      fileNameRegex   =  [[^(.*\.emlua|.*\.(jcl))$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },

    {
      name            = "makefile",
      syntaxName      = "this.makefile",
      fileNameRegex   =  [[^.*/(Makefile|.*\.h?mk(.in)?|.*\.dep)$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 0,
    },

    {
      name            = "java",
      syntaxName      = "this.java",
      fileNameRegex   = [[^.*\.(?:java)$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
}
