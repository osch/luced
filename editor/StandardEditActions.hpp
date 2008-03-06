/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
//
//   This program is free software; you can redistribute it and/or modify it
//   under the terms of the GNU General Public License Version 2 as published
//   by the Free Software Foundation in June 1991.
//
//   This program is distributed in the hope that it will be useful, but WITHOUT
//   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//   more details.
//
//   You should have received a copy of the GNU General Public License along with 
//   this program; if not, write to the Free Software Foundation, Inc., 
//   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
/////////////////////////////////////////////////////////////////////////////////////

#ifndef STANDARD_EDIT_ACTIONS_HPP
#define STANDARD_EDIT_ACTIONS_HPP

#include "TextEditorWidget.hpp"
#include "HeapObject.hpp"
#include "ValidPtr.hpp"

namespace LucED
{

class StandardEditActions : public HeapObject
{
public:
    typedef OwningPtr<StandardEditActions> Ptr;
    
    static Ptr createSingleLineActions(ValidPtr<TextEditorWidget> editWidget) {
        Ptr rslt(new StandardEditActions(editWidget));
        rslt->registerSingleLineEditActionsToEditWidget();
        return rslt;
    }
    static Ptr createMultiLineActions(ValidPtr<TextEditorWidget> editWidget) {
        Ptr rslt(new StandardEditActions(editWidget));
        rslt->registerMultiLineEditActionsToEditWidget();
        return rslt;
    }
    
    void cursorLeft();
    void cursorRight();
    void cursorDown();
    void cursorUp();
    void cursorPageDown();
    void cursorPageUp();
    void cursorBeginOfLine();
    void cursorEndOfLine();
    void scrollDown();
    void scrollUp();
    void scrollCursorDown();
    void scrollCursorUp();
    void scrollLeft();
    void scrollRight();
    void scrollPageUp();
    void scrollPageDown();
    void scrollPageLeft();
    void scrollPageRight();
    void cursorBeginOfText();
    void cursorEndOfText();
    void insertNewLineAutoIndent();
    void appendNewLineAutoIndent();
    void newLineFixedColumnIndentForward();
    void newLineFixedColumnIndentBackward();
    void backSpace();
    void deleteKey();
    void copyToClipboard();
    void cutToClipboard();
    void pasteFromClipboardForward();
    void pasteFromClipboardBackward();
    void selectAll();
    void selectionCursorLeft();
    void selectionCursorRight();
    void selectionCursorDown();
    void selectionCursorUp();
    void selectionLineCursorDown();
    void selectionLineCursorUp();
    void cursorWordLeft();
    void cursorWordRight();
    void selectionCursorWordLeft();
    void selectionCursorWordRight();
    void selectionCursorBeginOfLine();
    void selectionCursorEndOfLine();
    void selectionCursorPageDown();
    void selectionCursorPageUp();
    void undo();
    void redo();
    void selectWordForward();
    void selectWordBackward();
    void spaceBackward();
    void tabForward();
    void gotoMatchingBracket();
    void shiftBlockLeft();
    void shiftBlockRight();
    void findNextLuaStructureElement();
    void findPrevLuaStructureElement();
    void openCorrespondingFile();
    
private:
    StandardEditActions(ValidPtr<TextEditorWidget> editWidget);
    
    void registerSingleLineEditActionsToEditWidget();
    void registerMultiLineEditActionsToEditWidget();

    void newLineFixedColumnIndent(bool forward);
    void newLineAutoIndent(bool insert);

    ValidPtr<TextEditorWidget> e;
};


} // namespace LucED


#endif // STANDARD_EDIT_ACTIONS_HPP
