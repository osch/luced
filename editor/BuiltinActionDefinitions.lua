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

return
{
    { name = "cursorLeft",                        description = "", 
                                                  classes     = {} 
    },
    { name = "cursorRight",                       description = "",
                                                  classes     = {} 
    },
    { name = "cursorUp",                          description = "", 
                                                  classes     = {} 
    },
    { name = "cursorDown",                        description = "", 
                                                  classes     = {}  
    },
    
    { name = "historyBack",                       description = "", 
                                                  classes     = {} 
    },
    { name = "historyForward",                    description = "", 
                                                  classes     = {} 
    },
    
    { name = "focusNext",                         description = "", 
                                                  classes     = {} 
    },
    { name = "focusPrevious",                     description = "", 
                                                  classes     = {} 
    },
    
    { name = "insertTabulator",                   description = "", 
                                                  classes     = {} 
    },
    
    { name = "invokeGotoLinePanel",               description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+L", "Alt+X,Y,L" }
    },                                                                       ------- Test
    
    { name = "invokeFindPanelForward",            description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+F" }
    },
    
    { name = "invokeFindPanelBackward",           description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+Shift+F" }
    },
    
    { name = "invokeReplacePanelForward",         description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+R" }
    },
    
    { name = "invokeReplacePanelBackward",        description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+Shift+R" }
    },
    
    { name = "findSelectionForward",              description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+H" }
    },
        
    { name = "findSelectionBackward",             description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+Shift+H" }
    },
        
    { name = "replaceAgainForward",               description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+T" }
    },
        
    { name = "replaceAgainBackward",              description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+Shift+T" }
    },
        
    { name = "findAgainForward",                  description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+G" }
    },
        
    { name = "findAgainBackward",                 description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+Shift+G" }
    },
        
    { name = "requestProgramTermination",         description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+Q" }
    },
        
    { name = "handleEscapeKey",                   description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Escape" }
    },
        
    { name = "handleSaveKey",                     description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+S" }
    },
        
    { name = "handleSaveAsKey",                   description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+Shift+S" }
    },
        
    { name = "requestCloseWindow",                description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+W" }
    },
        
    { name = "createCloneWindow",                 description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Alt+C" }
    },
        
    { name = "createEmptyWindow",                 description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+N" }
    },
        
    { name = "executeLuaScript",                  description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Alt+L" }
    },
        
        
        
}
