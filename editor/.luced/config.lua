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
          doNotUseX11XkbExtension = false, -- this should be set to false
          keyPressRepeatFirstMilliSecs = 200,
          keyPressRepeatNextMilliSecs  =  20,

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


local textStyles =
{
    {name="default",     font=normal_font, color="black"},
    {name="comment",     font=italic_font, color="grey20"},
    {name="preproc",     font=normal_font, color="RoyalBlue4"},
    {name="keyword",     font=bold_font,   color="black"},
    {name="type",        font=bold_font,   color="brown"},
    {name="string",      font=normal_font, color="darkGreen"},
    {name="string1",     font=normal_font, color="SeaGreen"},
    {name="boldstring",  font=bold_font,   color="darkGreen"},
    {name="regex",       font=bold_font,   color="#009944"},
    {name="textKey",     font=bold_font,   color="VioletRed4"},
    {name="textKey1",    font=normal_font, color="VioletRed4"},
    {name="command",     font=normal_font, color="darkgoldenrod4"},
    {name="boldblue",    font=bold_font,   color="RoyalBlue4"},
    {name="boldcommand", font=bold_font,   color="darkgoldenrod4"},
    {name="orange",      font=normal_font, color="DarkOrange"},
    {name="boldorange",  font=bold_font,   color="DarkOrange"},
    {name="textcomment", font=italic_font, color="SteelBlue4"},
    {name="error",       font=bold_font,   color="red"},
    {name="numericconst",font=normal_font, color="darkGreen"},
    {name="subroutine",  font=normal_font, color="brown"},
    {name="identifier1", font=normal_font, color="RoyalBlue4"},
    
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
      fileNameRegex   = [[^.*\.(?:cpp|hpp|cxx|hxx|c|h|cc|hh)(?:\.emlua)?$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
    
    {
      name            = "lua",
      fileNameRegex   =  [[^.*\.lua$|^.*\.lua\.emlua$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
    
    {
      name            = "rexx",
      fileNameRegex   =  [[.*\.rexx?|.*\.r]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
    
    {
      name            = "emlua",
      fileNameRegex   =  [[^(.*\.emlua|.*\.(jcl))$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },

    {
      name            = "makefile",
      fileNameRegex   =  [[^.*/(Makefile|.*\.h?mk|.*\.dep)$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 0,
    },

    {
      name            = "java",
      fileNameRegex   = [[^.*\.(?:java)$]],
      approximateUnknownHiliting = true,
      approximateUnknownHilitingReparseRange = 2000,
      hilitingBreakPointDistance = 50,
      hardTabWidth = 8,
      softTabWidth = 4,
    },
}


local actions = 
{
    {
        name  = "cvs edit",
        keys  = "Alt+S,E",
        type  = "shell",
        script = [[ set -e
                    cd "`dirname $FILE`"
                    fn="`basename $FILE`"
                    cvs update "$fn"
                    rev=`cat CVS/Entries | grep "/$fn/" | cut -d/ -f3` 
                    cvs editors "$fn" 
                    cvs edit -a none "$fn" ]],
    },
    {
        name  = "cvs unedit",
        keys  = "Alt+S,U",
        type  = "shell",
        script = [[ set -e
                    cd "`dirname $FILE`" 
                    fn="`basename $FILE`" 
                    rev="`cat CVS/Entries | grep /$fn/ | cut -d/ -f3`" 
                    (echo "yes"|cvs unedit "$fn" 1>/dev/null 2>&1) 
                    touch "$fn" ]],
    },
    {
        name  = "cvs commit",
        keys  = "Alt+S,C",
        type  = "shell",
        script = [[ set -e
                    cd `dirname $FILE` 
                    rxvt -e cvs commit `basename $FILE` ]],
    },
    {
        name  = "cvs diff",
        keys  = "Alt+S,D",
        type  = "shell",
        script = [[ set -e
                    cd `dirname $FILE` 
                    fn=`basename $FILE` 
                    tkdiff $fn ]],
    },
    {
        name  = "test1",
        keys  = "Alt+S,T",
        type  = "shell",
        script = [[ echo "<$FILE>"; ls|head -n3 ]],
    },
    {
        name  = "test1a",
        keys  = "Alt+S,R",
        type  = "shell",
        script = [[ echo "<$FILE>"
                    ls
                    echo -n "testeerituertiouertoiuretioreutoireutret ABC" ]],
    },
    {
        name  = "test2",
        keys  = "Alt+S,Z",
        type  = "shell",
        script = [[ echo "moin at `date`" > trt ]],
    },
}


local config =
{
    generalConfig  = generalConfig,
    textStyles     = textStyles,
    languageModes  = languageModes,
    actions        = actions,
}

local getOverrideFunction = loadfile("overrideConfig.lua")
if getOverrideFunction then
    config = getOverrideFunction()(config)
end

return config
