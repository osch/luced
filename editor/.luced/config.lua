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

local generalConfig =
{
          useOwnKeyPressRepeater = true,  -- try this on systems with slow keyboard repeat rate
          doNotUseX11XkbExtension = true, -- this must be set to true for newer x servers
                                          -- because of strange (broken?) DetectableAutorepeat  
                                          -- in x11XkbExtension
          keyPressRepeatFirstMilliSecs = 200,
          keyPressRepeatNextMilliSecs  =  18,

          scrollBarWidth = 12+2,
          scrollBarRepeatFirstMilliSecs = 300,
          scrollBarRepeatNextMilliSecs  =  10,
          
          doubleClickMilliSecs = 400,
          
          guiColor01 = "grey30",
          guiColor02 = "grey70",
          guiColor03 = "grey80",
          guiColor04 = "grey85",
          guiColor05 = "grey90",
          guiFont      = "-*-helvetica-medium-r-*-*-*-120-75-75-*-*-iso10646-1",
          guiFontColor = "black",
          primarySelectionColor = "grey",
          pseudoSelectionColor  = "rgb:f1/f1/f1",
          
          initialWindowWidth  = 100,    -- size in characters
          initialWindowHeight =  45,    -- size in characters
          
          x11SelectionChunkLength = 200000,
          buttonInnerSpacing = 2,
          guiSpacing = 2,
          editorPanelOnTop = false,
          
          keepRunningIfOwningClipboard = false,
          
          maxRegexAssertionLength = 3000,
}


local fonts =
{
    { name = "normal", x11FontId = "-*-courier-medium-r-*-*-*-120-75-75-*-*-iso10646-1" },
    { name = "italic", x11FontId = "-*-courier-medium-o-*-*-*-120-75-75-*-*-iso10646-1" },
    { name = "bold",   x11FontId = "-*-courier-bold-r-*-*-*-120-75-75-*-*-iso10646-1"   }
}



local textStyles =
{
    {name="default",     font="normal", color="black"},
    {name="comment",     font="italic", color="grey20"},
    {name="preproc",     font="normal", color="RoyalBlue4"},
    {name="keyword",     font="bold",   color="black"},
    {name="type",        font="bold",   color="brown"},
    {name="string",      font="normal", color="darkGreen"},
    {name="string1",     font="normal", color="SeaGreen"},
    {name="boldstring",  font="bold",   color="darkGreen"},
    {name="regex",       font="bold",   color="#009944"},
    {name="textKey",     font="bold",   color="VioletRed4"},
    {name="textKey1",    font="normal", color="VioletRed4"},
    {name="command",     font="normal", color="darkgoldenrod4"},
    {name="boldblue",    font="bold",   color="RoyalBlue4"},
    {name="boldcommand", font="bold",   color="darkgoldenrod4"},
    {name="orange",      font="normal", color="DarkOrange"},
    {name="boldorange",  font="bold",   color="DarkOrange"},
    {name="textcomment", font="italic", color="SteelBlue4"},
    {name="error",       font="bold",   color="red"},
    {name="numericconst",font="normal", color="darkGreen"},
    {name="subroutine",  font="normal", color="brown"},
    {name="identifier1", font="normal", color="RoyalBlue4"},
    
}

------
------
--
-- SyntaxHiliting definitions are stored in separate files under ./syntaxpatterns/*.lua
-- for each language mode.
--
-- approximateUnknownHiliting: approximate Hiliting for display if exact Hiliting is not known 
--                             (because it is still calculated in the background).
-- approximateUnknownHilitingReparseRange: start approximation at actualPosition minus reparseRange,
--                             and assume root pattern at this position. This implies that
--                             structures greater than reparseRange are not properly recognized
--                             in the approximation process.
--
local languageModes =
{
    { 
      name            = "cplusplus",
      syntaxName      = "default.cplusplus",
      fileNameRegex   = [[^.*\.(?:CPP|HPP|H|cpp|hpp|cxx|hxx|c|h|cc|hh)(?:\.emlua)?$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
    
    {
      name            = "jsp",
      syntaxName      = "default.jsp",
      fileNameRegex   =  [[^.*\.(jspf?|html?)$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
    
    {
      name            = "lua",
      syntaxName      = "default.lua",
      fileNameRegex   =  [[^.*\.lua$|^.*\.lua\.emlua$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
    
    {
      name            = "rexx",
      syntaxName      = "default.rexx",
      fileNameRegex   =  [[.*\.rexx?|.*\.r]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
    
    {
      name            = "emlua",
      syntaxName      = "default.emlua",
      fileNameRegex   =  [[^(.*\.emlua|.*\.(jcl))$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },

    {
      name            = "makefile",
      syntaxName      = "default.makefile",
      fileNameRegex   =  [[^.*/(Makefile|.*\.h?mk(.in)?|.*\.dep)$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 0,
    },

    {
      name            = "java",
      syntaxName      = "default.java",
      fileNameRegex   = [[^.*\.(?:java)$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
}


local actionKeyBindings =
{
    {
        actionName = "default.test2",
        keys       = { "Alt+S,T" },
    },
    {
        actionName = "default.test3",
        keys       = { "Alt+S,R" },
    },
    {
        actionName = "default.test1",
        keys       = { "Alt+S,Z" },
    },
    {
        referToPackage = "default"
    },
}


local config =
{
    generalConfig     = generalConfig,
    fonts             = fonts,
    textStyles        = textStyles,
    languageModes     = languageModes,
    actionKeyBindings = actionKeyBindings,
}


return config
