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

globalConfig = {

          useKeyPressRepeater = true,
          keyPressRepeatFirstMilliSecs = 200,
          keyPressRepeatNextMilliSecs  =  21,

          scrollBarWidth = 12+2,
          scrollBarRepeatFirstMilliSecs = 300,
          scrollBarRepeatNextMilliSecs  =  10,
          
          doubleClickMilliSecs = 400,
          
          guiColor01 = "grey30",
          guiColor02 = "grey70",
          guiColor03 = "grey80",
          guiColor04 = "grey85",
          guiColor05 = "grey90",
          guiFont      = "-*-helvetica-medium-r-*-*-*-120-75-75-*-*-*-*",
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


local normal_font = "-*-courier-medium-r-*-*-*-120-75-75-*-*-*-*"
local italic_font = "-*-courier-medium-o-*-*-*-120-75-75-*-*-*-*"
local bold_font   = "-*-courier-bold-r-*-*-*-120-75-75-*-*-*-*"


textStyles = {

    {name="default",    font=normal_font, color="black"},
    {name="comment",    font=italic_font, color="grey20"},
    {name="preproc",    font=normal_font, color="RoyalBlue4"},
    {name="keyword",    font=bold_font,   color="black"},
    {name="type",       font=bold_font,   color="brown"},
    {name="string",     font=normal_font, color="darkGreen"},
    {name="boldstring", font=bold_font,   color="darkGreen"},
    {name="regex",      font=bold_font,   color="#009944"},
    {name="textKey",    font=bold_font,   color="VioletRed4"},
    {name="textKey1",   font=normal_font, color="VioletRed4"},
    {name="command",    font=normal_font, color="darkgoldenrod4"},
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
languageModes = {

    { 
      name            = "cplusplus",
      fileNameRegex   = [[^.*\.(?:cpp|hpp|cxx|hxx|c|h|cc|hh)(?:\.emlua)?$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
    
    {
      name            = "lua",
      fileNameRegex   =  [[.*\.lua]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
    
    {
      name            = "makefile",
      fileNameRegex   =  [[^.*/(Makefile|.*\.h?mk)$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 0,
    },
}
