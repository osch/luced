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

return -- default actionName key binding
{
    -----------------------------------------
    -- default lua edit actions
    --
--[[
    {
        actionName = "this.smartNewline",
        keys       = { "Return" },
    },
--]]
    {
        actionName = "this.formatParagraph",
        keys       = { "Ctrl+J" },
    },
    {
        actionName = "this.toUpper",
        keys       = { "Ctrl+6" },
    },
    {
        actionName = "this.toLower",
        keys       = { "Ctrl+Shift+6" },
    },
    -----------------------------------------
    -- default shell script actions
    --
    {
        actionName = "this.cvsEdit",
        keys       = { "Alt+S,E" },
    },
    {
        actionName = "this.cvsUnedit",
        keys       = { "Alt+S,U" },
    },
    {
        actionName = "this.cvsCommit",
        keys       = { "Alt+S,C" },
    },
    {
        actionName = "this.cvsDiff",
        keys       = { "Alt+S,D" },
    },
    {
        actionName = "this.cvsLog",
        keys       = { "Alt+S,L" },
    },

    -----------------------------------------
    -- default shell filter actions
    --
    {
        actionName = "this.formatXml",
        keys       = { "Alt+F,X" },
    },
    {
        actionName = "this.expandTabs4",
        keys       = { "Alt+F,T,4" },
    },
    {
        actionName = "this.expandTabs8",
        keys       = { "Alt+F,T,8" },
    },
    -----------------------------------------
    -- default keys     for builtin actions
    --
    {
        actionName = "builtin.cursorLeft",
        keys       = { "Left", "KP_Left" },
    },
    {
        actionName = "builtin.cursorRight",
        keys       = { "Right", "KP_Right" },
    },
    {
        actionName = "builtin.cursorBeginOfLine",
        keys       = { "Alt+Left", "Alt+KP_Left", "Home", "Begin", "KP_Home" },
    },
    {
        actionName = "builtin.cursorEndOfLine",
        keys       = { "Alt+Right", "Alt+KP_Right", "End", "KP_End" },
    },
    {
        actionName = "builtin.scrollLeft",
        keys       = { "Left", "Ctrl+Alt+Left", "Ctrl+Alt+KP_Left" },
    },
    {
        actionName = "builtin.scrollRight",
        keys       = { "Right", "Ctrl+Alt+Right", "Ctrl+Alt+KP_Right" },
    },
    {
        actionName = "builtin.cursorBeginOfText",
        keys       = { "Home", "Ctrl+Home", "Ctrl+Begin", "Ctrl+KP_Home" },
    },
    {
        actionName = "builtin.cursorEndOfText",
        keys       = { "End", "Ctrl+End", "Ctrl+KP_End" },
    },
    {
        actionName = "builtin.backSpace",
        keys       = { "BackSpace" },
    },
    {
        actionName = "builtin.deleteKey",
        keys       = { "Delete", "KP_Delete" },
    },
    {
        actionName = "builtin.copyToClipboard",
        keys       = { "Ctrl+C" },
    },
    {
        actionName = "builtin.cutToClipboard",
        keys       = { "Ctrl+X" },
    },
    {
        actionName = "builtin.pasteFromClipboardForward",
        keys       = { "Ctrl+V" },
    },
    {
        actionName = "builtin.pasteFromClipboardBackward",
        keys       = { "Ctrl+Shift+V" },
    },
    {
        actionName = "builtin.selectAll",
        keys       = { "Ctrl+A" },
    },
    {
        actionName = "builtin.selectionCursorLeft",
        keys       = { "Shift+Left", "Shift+KP_Left" },
    },
    {
        actionName = "builtin.selectionCursorRight",
        keys       = { "Shift+Right", "Shift+KP_Right" },
    },
    {
        actionName = "builtin.cursorWordLeft",
        keys       = { "Ctrl+Left" },
    },
    {
        actionName = "builtin.cursorWordRight",
        keys       = { "Ctrl+Right" },
    },
    {
        actionName = "builtin.selectionCursorWordLeft",
        keys       = { "Ctrl+Shift+Left" },
    },
    {
        actionName = "builtin.selectionCursorWordRight",
        keys       = { "Ctrl+Shift+Right" },
    },
    {
        actionName = "builtin.selectionCursorBeginOfLine",
        keys       = { "Shift+Home", "Shift+Begin", "Shift+KP_Home" },
    },
    {
        actionName = "builtin.selectionCursorEndOfLine",
        keys       = { "Shift+End" },
    },
    {
        actionName = "builtin.undo",
        keys       = { "Ctrl+Z" },
    },
    {
        actionName = "builtin.redo",
        keys       = { "Ctrl+Shift+Z" },
    },
    {
        actionName = "builtin.mainViewUndo",
        keys       = { "Alt+Z" },
    },
    {
        actionName = "builtin.mainViewRedo",
        keys       = { "Alt+Shift+Z" },
    },
    {
        actionName = "builtin.selectWordForward",
        keys       = { "Ctrl+space" },
    },
    {
        actionName = "builtin.selectWordBackward",
        keys       = { "Ctrl+Shift+space" },
    },
    {
        actionName = "builtin.spaceBackward",
        keys       = { "Insert", "KP_Insert" },
    },
    {
        actionName = "builtin.gotoMatchingBracket",
        keys       = { "Ctrl+M" },
    },
    {
        actionName = "builtin.cursorUp",
        keys       = { "Up", "KP_Up" },
    },
    {
        actionName = "builtin.cursorDown",
        keys       = { "Down", "KP_Down" },
    },
    {
        actionName = "builtin.cursorPageDown",
        keys       = { "Control+Down", "Page_Down", "KP_Next" },
    },
    {
        actionName = "builtin.cursorPageUp",
        keys       = { "Control+Up", "Page_Up", "KP_Prior" },
    },
    {
        actionName = "builtin.scrollPageDown",
        keys       = { "Control+Down", "Page_Down", "KP_Next" },
    },
    {
        actionName = "builtin.scrollPageUp",
        keys       = { "Control+Up", "Page_Up", "KP_Prior" },
    },
    {
        actionName = "builtin.scrollCursorUp",
        keys       = { "Alt+Up", "Alt+KP_Up" },
    },
    {
        actionName = "builtin.scrollCursorDown",
        keys       = { "Alt+Down", "Alt+KP_Down" },
    },
    {
        actionName = "builtin.scrollUp",
        keys       = { "Up", "Alt+Up", "Ctrl+Alt+Up", "Ctrl+Alt+KP_Up" },
    },
    {
        actionName = "builtin.scrollDown",
        keys       = { "Down", "Alt+Down", "Ctrl+Alt+Down", "Ctrl+Alt+KP_Down" },
    },
    {
        actionName = "builtin.insertNewLineAutoIndent",
        keys       = { "Return", "KP_Enter" },
    },
    {
        actionName = "builtin.appendNewLineAutoIndent",
        keys       = { "Alt+Return", "Alt+KP_Enter" },
    },
    {
        actionName = "builtin.newLineFixedColumnIndentForward",
        keys       = { "Ctrl+Return", "Ctrl+KP_Enter" },
    },
    {
        actionName = "builtin.newLineFixedColumnIndentBackward",
        keys       = { "Ctrl+Shift+Return", "Ctrl+Shift+KP_Enter" },
    },
    {
        actionName = "builtin.selectionCursorDown",
        keys       = { "Shift+Down", "Shift+KP_Down" },
    },
    {
        actionName = "builtin.selectionCursorUp",
        keys       = { "Shift+Up", "Shift+KP_Up" },
    },
    {
        actionName = "builtin.selectionLineCursorDown",
        keys       = { "Ctrl+Shift+Down", "Ctrl+Shift+KP_Down" },
    },
    {
        actionName = "builtin.selectionLineCursorUp",
        keys       = { "Ctrl+Shift+Up", "Ctrl+Shift+KP_Up" },
    },
    {
        actionName = "builtin.selectionCursorPageDown",
        keys       = { "Shift+Page_Down", "Shift+KP_Next" },
    },
    {
        actionName = "builtin.selectionCursorPageUp",
        keys       = { "Shift+Page_Up", "Shift+KP_Prior" },
    },
    {
        actionName = "builtin.shiftBlockLeft",
        keys       = { "Ctrl+9" },
    },
    {
        actionName = "builtin.shiftBlockRight",
        keys       = { "Ctrl+0" },
    },
    {
        actionName = "builtin.findNextLuaStructureElement",
        keys       = { "Alt+M" },
    },
    {
        actionName = "builtin.findPrevLuaStructureElement",
        keys       = { "Alt+Shift+M" },
    },
    {
        actionName = "default.openCorrespondingFile",
        keys       = { "Alt+H" },
    },
    {
        actionName = "builtin.historyBackward",
        keys       = { "Ctrl+Up", "Ctrl+KP_Up" },
    },
    {
        actionName = "builtin.historyForward",
        keys       = { "Ctrl+Down", "Ctrl+KP_Down" },
    },
    {
        actionName = "builtin.closeMessageBox",
        keys       = { "Escape", "Q" },
    },
    {
        actionName = "builtin.closePanel",
        keys       = { "Escape" },
    },
    {
        actionName = "builtin.focusNext",
        keys       = { "Tab" },
    },
    {
        actionName = "builtin.focusPrevious",
        keys       = { "Shift+Tab" },
    },
    {
        actionName = "builtin.focusRight",
        keys       = { "Right", "KP_Right" },
    },
    {
        actionName = "builtin.focusLeft",
        keys       = { "Left", "KP_Left" },
    },
    {
        actionName = "builtin.focusUp",
        keys       = { "Up", "KP_Up" },
    },
    {
        actionName = "builtin.focusDown",
        keys       = { "Down", "KP_Down" },
    },
    {
        actionName = "builtin.pressDefaultButton",
        keys       = { "Return", "KP_Enter" },
    },
    {
        actionName = "builtin.tabForward",
        keys       = { "Tab" },
    },
    {
        actionName = "builtin.pressFocusedButton",
        keys       = { "space" },
    },
    {
        actionName = "builtin.invokeGotoLinePanel",
        keys       = { "Ctrl+L", "Alt+X,Y,L" },
    },
    {
        actionName = "builtin.invokeExecutePanel",
        keys       = { "Alt+S,S" },
    },
    {
        actionName = "builtin.invokeFindPanelForward",
        keys       = { "Ctrl+F" },
    },
    {
        actionName = "builtin.invokeFindPanelBackward",
        keys       = { "Ctrl+Shift+F" },
    },
    {
        actionName = "builtin.invokeReplacePanelForward",
        keys       = { "Ctrl+R" },
    },
    {
        actionName = "builtin.invokeReplacePanelBackward",
        keys       = { "Ctrl+Shift+R" },
    },
    {
        actionName = "builtin.findSelectionForward",
        keys       = { "Ctrl+H" },
    },
    {
        actionName = "builtin.findSelectionBackward",
        keys       = { "Ctrl+Shift+H" },
    },
    {
        actionName = "builtin.replaceAgainForward",
        keys       = { "Ctrl+T" },
    },
    {
        actionName = "builtin.replaceAgainBackward",
        keys       = { "Ctrl+Shift+T" },
    },
    {
        actionName = "builtin.findAgainForward",
        keys       = { "Ctrl+G" },
    },
    {
        actionName = "builtin.findAgainBackward",
        keys       = { "Ctrl+Shift+G" },
    },
    {
        actionName = "builtin.requestProgramTermination",
        keys       = { "Ctrl+Q" },
    },
    {
        actionName = "builtin.handleSaveKey",
        keys       = { "Ctrl+S" },
    },
    {
        actionName = "builtin.handleSaveAsKey",
        keys       = { "Ctrl+Shift+S" },
    },
    {
        actionName = "builtin.requestCloseWindow",
        keys       = { "Ctrl+W" },
    },
    {
        actionName = "builtin.createCloneWindow",
        keys       = { "Alt+C" },
    },
    {
        actionName = "builtin.createEmptyWindow",
        keys       = { "Ctrl+N" },
    },
    {
        actionName = "builtin.executeLuaScript",
        keys       = { "Alt+L" },
    },
    {
        actionName = "builtin.resetLuaModules",
        keys       = { "Alt+K,M" },
    },
}

