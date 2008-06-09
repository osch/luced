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
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Left", "KP_Left" }
    },
    { name = "cursorRight",                       description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Right", "KP_Right" }
    },
    
    
    { name = "cursorBeginOfLine",                 description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Alt+Left", "Alt+KP_Left", 
                                                                  "Home", "Begin" }
    },
    { name = "cursorEndOfLine",                   description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Alt+Right", "Alt+KP_Right",
                                                                  "End" }
    },
    
    { name = "scrollLeft",                        description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+Alt+Left", "Ctrl+Alt+KP_Left" }
    },
    { name = "scrollRight",                       description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+Alt+Right", "Ctrl+Alt+KP_Right" }
    },
    
    
    { name = "cursorBeginOfText",                 description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+Home", "Ctrl+Begin" }
    },
    { name = "cursorEndOfText",                   description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+End" }
    },
    
    
    
    { name = "backSpace",                         description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "BackSpace" }
    },
    { name = "deleteKey",                         description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Delete" }
    },
    
    
    
    { name = "copyToClipboard",                   description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+C" }
    },
    { name = "cutToClipboard",                    description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+X" }
    },
    { name = "pasteFromClipboardForward",         description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+V" }
    },
    { name = "pasteFromClipboardBackward",        description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+Shift+V" }
    },
    { name = "selectAll",                         description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+A" }
    },
    
    
    
    { name = "selectionCursorLeft",               description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Shift+Left", "Shift+KP_Left" }
    },
    { name = "selectionCursorRight",              description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Shift+Right", "Shift+KP_Right" }
    },
    
    
    { name = "cursorWordLeft",                    description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+Left" }
    },
    { name = "cursorWordRight",                   description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+Right" }
    },
    
    
    { name = "selectionCursorWordLeft",           description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+Shift+Left" }
    },
    { name = "selectionCursorWordRight",          description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+Shift+Right" }
    },
    
    
    
    { name = "selectionCursorBeginOfLine",        description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Shift+Home", "Shift+Begin" }
    },
    { name = "selectionCursorEndOfLine",          description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Shift+End" }
    },
    
    
    
    { name = "undo",                              description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+Z" }
    },
    { name = "redo",                              description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+Shift+Z" }
    },
    
    
    
    { name = "selectWordForward",                 description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+space" }
    },
    { name = "selectWordBackward",                description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+Shift+space" }
    },
    
    
    { name = "spaceBackward",                     description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Insert" }
    },
    { name = "gotoMatchingBracket",               description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Ctrl+M" }
    },
    
    
    { name = "cursorUp",                          description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Up", "KP_Up" }
    },
    { name = "cursorDown",                        description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Down", "KP_Down" }
    },
        
    { name = "cursorPageDown",                    description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Page_Down" }
    },
    { name = "cursorPageUp",                      description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Page_Up" }
    },
    
    
    
    { name = "scrollCursorUp",                    description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Alt+Up", "Alt+KP_Up" }
    },
    { name = "scrollCursorDown",                  description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Alt+Down", "Alt+KP_Down" }
    },
    
    
    
        
    { name = "scrollUp",                          description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Ctrl+Alt+Up", "Ctrl+Alt+KP_Up" }
    },
    { name = "scrollDown",                        description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Ctrl+Alt+Down", "Ctrl+Alt+KP_Down" }
    },
    
    
    
    
    { name = "insertNewLineAutoIndent",           description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Return", "KP_Enter" }
    },
    
    
    { name = "appendNewLineAutoIndent",           description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Alt+Return", "Alt+KP_Enter" }
    },
    
    
    
    { name = "newLineFixedColumnIndentForward",   description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Ctrl+Return", "Ctrl+KP_Enter" }
    },
    
    
    
    { name = "newLineFixedColumnIndentBackward",  description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Ctrl+Shift+Return", "Ctrl+Shift+KP_Enter" }
    },
    
    
    { name = "selectionCursorDown",               description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Shift+Down", "Shift+KP_Down" }
    },
    { name = "selectionCursorUp",                 description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Shift+Up", "Shift+KP_Up" }
    },
    
    
    { name = "selectionLineCursorDown",           description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Ctrl+Shift+Down", "Ctrl+Shift+KP_Down" }
    },
    { name = "selectionLineCursorUp",             description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Ctrl+Shift+Up", "Ctrl+Shift+KP_Up" }
    },
    
    { name = "selectionCursorPageDown",           description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Shift+Page_Down" }
    },
    { name = "selectionCursorPageUp",             description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Shift+Page_Up" }
    },
    
    
    
    { name = "shiftBlockLeft",                    description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Ctrl+9" }
    },
    { name = "shiftBlockRight",                   description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Ctrl+0" }
    },
    
    
    
    { name = "findNextLuaStructureElement",       description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Alt+M" }
    },
    { name = "findPrevLuaStructureElement",       description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Alt+Shift+M" }
    },
    
    
    
    { name = "openCorrespondingFile",             description = "",
                                                  classes = { "MultiLineEditActions" },
                                                  defaultKeys = { "Alt+H" }
    },
    

    { name = "historyBackward",                   description = "", 
                                                  classes     = {    "FindPanel::EditFieldActions",
                                                                  "ReplacePanel::EditFieldActions" },
                                                  defaultKeys = { "Up", "KP_Up" }
    },
    { name = "historyForward",                    description = "", 
                                                  classes     = {    "FindPanel::EditFieldActions",
                                                                  "ReplacePanel::EditFieldActions" },
                                                  defaultKeys = { "Down", "KP_Down" }
    },
    
    { name = "closeMessageBox",                   description = "", 
                                                  classes     = { "MessageBox::Actions" },
                                                  defaultKeys = { "Escape" }
    },
    { name = "closePanel",                        description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Escape" }
    },

    { name = "focusNext",                         description = "", 
                                                  classes     = { "DialogPanel::Actions" },
                                                  defaultKeys = { "Tab", "Right", "KP_Right" }
    },
    { name = "focusPrevious",                     description = "", 
                                                  classes     = { "DialogPanel::Actions" },
                                                  defaultKeys = { "Shift+Tab", "Left", "KP_Left" }
    },
    { name = "pressDefaultButton",                description = "", 
                                                  classes     = { "DialogPanel::Actions" },
                                                  defaultKeys = { "Return", "KP_Enter" }
    },
    
    { name = "tabForward",                        description = "",
                                                  classes = { "SingleLineEditActions" },
                                                  defaultKeys = { "Tab" }
    },
    
    { name = "pressFocusedButton",                description = "", 
                                                  classes     = {   "Button::Actions",
                                                                  "CheckBox::Actions" },
                                                  defaultKeys = { "space" }
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
                                                  classes     = { "EditorTopWinActions", 
                                                                  "SearchInteraction::MessageBoxActions" },
                                                  defaultKeys = { "Ctrl+H" }
    },
        
    { name = "findSelectionBackward",             description = "", 
                                                  classes     = { "EditorTopWinActions",
                                                                  "SearchInteraction::MessageBoxActions" },
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
                                                  classes     = { "EditorTopWinActions",
                                                                  "SearchInteraction::MessageBoxActions" },
                                                  defaultKeys = { "Ctrl+G" }
    },
        
    { name = "findAgainBackward",                 description = "", 
                                                  classes     = { "EditorTopWinActions",
                                                                  "SearchInteraction::MessageBoxActions" },
                                                  defaultKeys = { "Ctrl+Shift+G" }
    },
        
    { name = "requestProgramTermination",         description = "", 
                                                  classes     = { "EditorTopWinActions", },
                                                  defaultKeys = { "Ctrl+Q" }
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
