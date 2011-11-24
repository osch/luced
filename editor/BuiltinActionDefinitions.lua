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
    },
    { name = "cursorRight",                       description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    
    
    { name = "cursorBeginOfLine",                 description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    { name = "cursorEndOfLine",                   description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    
    { name = "scrollLeft",                        description = "",
                                                  classes = { "SingleLineDisplayActions" },
    },
    { name = "scrollRight",                       description = "",
                                                  classes = { "SingleLineDisplayActions" },
    },
    
    
    { name = "cursorBeginOfText",                 description = "",
                                                  classes = { "SingleLineDisplayActions" },
    },
    { name = "cursorEndOfText",                   description = "",
                                                  classes = { "SingleLineDisplayActions" },
    },
    
    
    
    { name = "backSpace",                         description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    { name = "deleteKey",                         description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    
    
    
    { name = "copyToClipboard",                   description = "",
                                                  classes = { "SingleLineDisplayActions" },
    },
    { name = "cutToClipboard",                    description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    { name = "pasteFromClipboardForward",         description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    { name = "pasteFromClipboardBackward",        description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    { name = "selectAll",                         description = "",
                                                  classes = { "SingleLineDisplayActions" },
    },
    
    
    
    { name = "selectionCursorLeft",               description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    { name = "selectionCursorRight",              description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    
    
    { name = "cursorWordLeft",                    description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    { name = "cursorWordRight",                   description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    
    
    { name = "selectionCursorWordLeft",           description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    { name = "selectionCursorWordRight",          description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    
    
    
    { name = "selectionCursorBeginOfLine",        description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    { name = "selectionCursorEndOfLine",          description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    
    
    
    { name = "undo",                              description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    { name = "redo",                              description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    
    
    
    { name = "selectWordForward",                 description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    { name = "selectWordBackward",                description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    
    
    { name = "spaceBackward",                     description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    { name = "gotoMatchingBracket",               description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    
    
    { name = "cursorUp",                          description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    { name = "cursorDown",                        description = "",
                                                  classes = { "MultiLineEditActions" },
    },
        
    { name = "cursorPageDown",                    description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    { name = "cursorPageUp",                      description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    
    { name = "scrollPageDown",                    description = "",
                                                  classes = { "MultiLineDisplayActions" },
    },
    { name = "scrollPageUp",                      description = "",
                                                  classes = { "MultiLineDisplayActions" },
    },
    
    
    
    { name = "scrollCursorUp",                    description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    { name = "scrollCursorDown",                  description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    
        
    { name = "scrollUp",                          description = "",
                                                  classes = { "MultiLineDisplayActions" },
    },
    { name = "scrollDown",                        description = "",
                                                  classes = { "MultiLineDisplayActions" },
    },
    
    
    
    
    { name = "insertNewLineAutoIndent",           description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    
    
    { name = "appendNewLineAutoIndent",           description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    
    
    
    { name = "newLineFixedColumnIndentForward",   description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    
    
    
    { name = "newLineFixedColumnIndentBackward",  description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    
    
    { name = "selectionCursorDown",               description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    { name = "selectionCursorUp",                 description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    
    
    { name = "selectionLineCursorDown",           description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    { name = "selectionLineCursorUp",             description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    
    { name = "selectionCursorPageDown",           description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    { name = "selectionCursorPageUp",             description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    
    
    
    { name = "shiftBlockLeft",                    description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    { name = "shiftBlockRight",                   description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    
    
    
    { name = "findNextLuaStructureElement",       description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    { name = "findPrevLuaStructureElement",       description = "",
                                                  classes = { "MultiLineEditActions" },
    },
    
    
    
    { name = "historyBackward",                   description = "", 
                                                  classes     = {    "FindPanel::EditFieldActions",
                                                                  "ReplacePanel::EditFieldActions" },
    },
    { name = "historyForward",                    description = "", 
                                                  classes     = {    "FindPanel::EditFieldActions",
                                                                  "ReplacePanel::EditFieldActions" },
    },
    
    { name = "closeMessageBox",                   description = "", 
                                                  classes     = { "MessageBox::Actions",
                                                                  "CommandOutputBox::Actions" },
    },

    { name = "closePanel",                        description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },

    { name = "focusNext",                         description = "", 
                                                  classes     = { "DialogPanel::Actions" },
    },
    { name = "focusPrevious",                     description = "", 
                                                  classes     = { "DialogPanel::Actions" },
    },
    { name = "focusRight",                        description = "", 
                                                  classes     = { "DialogPanel::Actions" },
    },
    { name = "focusLeft",                         description = "", 
                                                  classes     = { "DialogPanel::Actions" },
    },
    { name = "focusUp",                           description = "", 
                                                  classes     = { "DialogPanel::Actions" },
    },
    { name = "focusDown",                         description = "", 
                                                  classes     = { "DialogPanel::Actions" },
    },
    { name = "pressDefaultButton",                description = "", 
                                                  classes     = { "DialogPanel::Actions" },
    },
    
    { name = "tabForward",                        description = "",
                                                  classes = { "SingleLineEditActions" },
    },
    
    { name = "pressFocusedButton",                description = "", 
                                                  classes     = {   "Button::Actions",
                                                                  "CheckBox::Actions" },
    },
    
    { name = "invokeGotoLinePanel",               description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },                                                                       ------- Test
    
    { name = "invokeExecutePanel",                description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },                                                                       ------- Test
    
    { name = "invokeFindPanelForward",            description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },
    
    { name = "invokeFindPanelBackward",           description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },
    
    { name = "invokeReplacePanelForward",         description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },
    
    { name = "invokeReplacePanelBackward",        description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },
    
    { name = "findSelectionForward",              description = "", 
                                                  classes     = { "EditorTopWinActions", 
                                                                  "SearchInteraction::MessageBoxActions" },
    },
        
    { name = "findSelectionBackward",             description = "", 
                                                  classes     = { "EditorTopWinActions",
                                                                  "SearchInteraction::MessageBoxActions" },
    },
        
    { name = "replaceAgainForward",               description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },
        
    { name = "replaceAgainBackward",              description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },
        
    { name = "findAgainForward",                  description = "", 
                                                  classes     = { "EditorTopWinActions",
                                                                  "SearchInteraction::MessageBoxActions" },
    },
        
    { name = "findAgainBackward",                 description = "", 
                                                  classes     = { "EditorTopWinActions",
                                                                  "SearchInteraction::MessageBoxActions" },
    },
        
    { name = "requestProgramTermination",         description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },
        
    { name = "handleSaveKey",                     description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },
        
    { name = "handleSaveAsKey",                   description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },
        
    { name = "requestCloseWindow",                description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },
        
    { name = "createCloneWindow",                 description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },
        
    { name = "createEmptyWindow",                 description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },
        
    { name = "executeLuaScript",                  description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },
        
    { name = "resetLuaModules" ,                  description = "", 
                                                  classes     = { "EditorTopWinActions", },
    },
        
        
        
}
