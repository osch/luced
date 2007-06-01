/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2007 Oliver Schmidt, oliver at luced dot de
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

#ifndef REPLACEPANEL_H
#define REPLACEPANEL_H

#include "String.hpp"

#include "DialogPanel.hpp"
#include "Button.hpp"
#include "CheckBox.hpp"
#include "TextEditorWidget.hpp"
#include "SingleLineEditField.hpp"
#include "MessageBox.hpp"
#include "Callback.hpp"
#include "Regex.hpp"
#include "types.hpp"
#include "ReplaceUtil.hpp"
#include "EditFieldGroup.hpp"
#include "FindPanel.hpp"

namespace LucED {



class ReplacePanel : public  DialogPanel,
                     private FindPanelAccess
{
public:
    typedef OwningPtr<ReplacePanel> Ptr;

    static Ptr create(GuiWidget* parent, TextEditorWidget* editorWidget, FindPanel* findPanel,
                      Callback1<MessageBoxParameter> messageBoxInvoker,
                      Callback1<DialogPanel*>        panelInvoker)
    {
        return Ptr(new ReplacePanel(parent, editorWidget, findPanel, messageBoxInvoker, panelInvoker));
    }
    
    void replaceAgainForward();
    void replaceAgainBackward();
    
    virtual void treatFocusIn();
    virtual void treatFocusOut();
    
    void setDefaultDirection(Direction::Type direction) {
        ASSERT(direction == Direction::UP || direction == Direction::DOWN);
        defaultDirection = direction;
        findPrevButton->setAsDefaultButton(direction != Direction::DOWN);
        findNextButton->setAsDefaultButton(direction == Direction::DOWN);
        replacePrevButton->setAsDefaultButton(false);
        replaceNextButton->setAsDefaultButton(false);
    }
    
    virtual ProcessingResult processKeyboardEvent(const XEvent *event);
    
    virtual void show();
    virtual void hide();
    
private:
    ReplacePanel(GuiWidget* parent, TextEditorWidget* editorWidget, FindPanel* findPanel,
                 Callback1<MessageBoxParameter> messageBoxInvoker,
                 Callback1<DialogPanel*>        panelInvoker);

    void executeFind(bool isWrapping, const Callback0& handleContinueSearchButton);

    void handleButtonPressed(Button* button);

    void handleContinueAtBeginningButton();
    void handleContinueAtEndButton();
    
    void handleContinueSelectionFindAtBeginningButton();
    void handleContinueSelectionFindAtEndButton();
    
    void internalFindNext(bool wrapping);
    
    void handleModifiedEditField(bool modifiedFlag);
    
    void findAgainForward();
    void findAgainBackward();

    WeakPtr<TextEditorWidget> e;

    SingleLineEditField::Ptr findEditField;
    SingleLineEditField::Ptr replaceEditField;
    SingleLineEditField::Ptr lastFocusedEditField;
    
    Button::Ptr findNextButton;
    Button::Ptr findPrevButton;
    Button::Ptr replaceNextButton;
    Button::Ptr replacePrevButton;
    Button::Ptr cancelButton;
    Button::Ptr replaceSelectionButton;
    Button::Ptr replaceWindowButton;
    CheckBox::Ptr ignoreCaseCheckBox;
    CheckBox::Ptr wholeWordCheckBox;
    CheckBox::Ptr regularExprCheckBox;
    Callback1<MessageBoxParameter> messageBoxInvoker;
    Callback1<DialogPanel*>        panelInvoker;
    Regex regex;
    Direction::Type defaultDirection;
    int historyIndex;
    String selectionSearchString;
    bool selectSearchRegexFlag;
    EditFieldGroup::Ptr editFieldGroup;
    ReplaceUtil         replaceUtil;
    String rememberedSelection;
};

} // namespace LucED

#endif // REPLACEPANEL_H
