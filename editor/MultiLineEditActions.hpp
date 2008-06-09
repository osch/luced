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

#ifndef MULTI_LINE_EDIT_ACTIONS_HPP
#define MULTI_LINE_EDIT_ACTIONS_HPP

#include "RawPtr.hpp"
#include "OwningPtr.hpp"
#include "TextEditorWidget.hpp"
#include "ActionMethodBinding.hpp"

namespace LucED
{

class MultiLineEditActions : public ActionMethodBinding<MultiLineEditActions>
{
public:
    typedef OwningPtr<MultiLineEditActions> Ptr;
    
    static Ptr create(RawPtr<TextEditorWidget> editWidget) {
        return Ptr(new MultiLineEditActions(editWidget));
    }

    void cursorDown();
    void cursorUp();
    void cursorPageDown();
    void cursorPageUp();
    void scrollDown();
    void scrollUp();
    void scrollCursorDown();
    void scrollCursorUp();
    void scrollPageUp();
    void scrollPageDown();
    void insertNewLineAutoIndent();
    void appendNewLineAutoIndent();
    void newLineFixedColumnIndentForward();
    void newLineFixedColumnIndentBackward();
    void selectionCursorDown();
    void selectionCursorUp();
    void selectionLineCursorDown();
    void selectionLineCursorUp();
    void selectionCursorPageDown();
    void selectionCursorPageUp();
    void shiftBlockLeft();
    void shiftBlockRight();
    void findNextLuaStructureElement();
    void findPrevLuaStructureElement();
    void openCorrespondingFile();

private:
    MultiLineEditActions(RawPtr<TextEditorWidget> editWidget)
        : ActionMethodBinding<MultiLineEditActions>(this),
          e(editWidget)
    {}

    void newLineFixedColumnIndent(bool forward);
    void newLineAutoIndent(bool insert);

    RawPtr<TextEditorWidget> e;
};

} // namespace LucED

#endif // MULTI_LINE_EDIT_ACTIONS_HPP
