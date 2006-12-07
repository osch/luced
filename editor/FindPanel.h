/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch at luced dot de
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

#ifndef FINDPANEL_H
#define FINDPANEL_H

#include "DialogPanel.h"
#include "Button.h"
#include "CheckBox.h"
#include "TextEditorWidget.h"
#include "SingleLineEditField.h"
#include "MessageBox.h"
#include "Callback.h"
#include "Regex.h"
#include "types.h"

namespace LucED {

class FindPanel : public DialogPanel
{
public:
    typedef OwningPtr<FindPanel> Ptr;

    static Ptr create(GuiWidget* parent, TextEditorWidget* editorWidget, Callback1<MessageBoxParameter> messageBoxInvoker) {
        return Ptr(new FindPanel(parent, editorWidget, messageBoxInvoker));
    }
    
    virtual void treatFocusIn();
    
    void setDefaultDirection(Direction::Type direction) {
        ASSERT(direction == Direction::UP || direction == Direction::DOWN);
        defaultDirection = direction;
        findPrevButton->setAsDefaultButton(direction != Direction::DOWN);
        findNextButton->setAsDefaultButton(direction == Direction::DOWN);
    }
    
    void findAgainForward();
    void findAgainBackward();
    
private:
    FindPanel(GuiWidget* parent, TextEditorWidget* editorWidget, Callback1<MessageBoxParameter> messageBoxInvoker);

    void handleButtonPressed(Button* button);

    void handleContinueAtBeginningButton();
    void handleContinueAtEndButton();
    
    void internalFindNext(bool forward, int textPosition, bool wrapping);

    WeakPtr<TextEditorWidget> editorWidget;

    SingleLineEditField::Ptr editField;
    Button::Ptr findNextButton;
    Button::Ptr findPrevButton;
    Button::Ptr cancelButton;
    Button::Ptr goBackButton;
    CheckBox::Ptr ignoreCaseCheckBox;
    CheckBox::Ptr wholeWordCheckBox;
    CheckBox::Ptr regularExprCheckBox;
    Callback1<MessageBoxParameter> messageBoxInvoker;
    Regex regex;
    Direction::Type defaultDirection;
};

} // namespace LucED

#endif // FINDPANEL_H
