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

#ifndef FINDPANEL_H
#define FINDPANEL_H

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
#include "FindUtil.hpp"
#include "PasteDataReceiver.hpp"

namespace LucED {



class FindPanel : public DialogPanel,
                  public PasteDataReceiver
{
public:
    typedef OwningPtr<FindPanel> Ptr;

    static Ptr create(GuiWidget* parent, TextEditorWidget* editorWidget, Callback<MessageBoxParameter>::Ptr messageBoxInvoker,
                                                                         Callback<DialogPanel*>::Ptr        panelInvoker)
    {
        return Ptr(new FindPanel(parent, editorWidget, messageBoxInvoker, panelInvoker));
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

    void findSelectionForward();
    void findSelectionBackward();

    virtual ProcessingResult processKeyboardEvent(const XEvent *event);
    virtual ProcessingResult processEvent(const XEvent *event);
    
    virtual void show();
    
private:
    friend class FindPanelAccess;
    
    FindPanel(GuiWidget* parent, TextEditorWidget* editorWidget, Callback<MessageBoxParameter>::Ptr messageBoxInvoker,
                                                                 Callback<DialogPanel*>::Ptr        panelInvoker);

    void executeFind(bool isWrapping, Callback<>::Ptr handleContinueSearchButton);

    void handleButtonPressed(Button* button);
    void handleButtonDefaultKey(Button* button);

    void handleContinueAtBeginningButton();
    void handleContinueAtEndButton();
    
    void handleContinueSelectionFindAtBeginningButton();
    void handleContinueSelectionFindAtEndButton();
    
    void internalFindNext(bool wrapping);
    
    void handleModifiedEditField(bool modifiedFlag);

    virtual void notifyAboutBeginOfPastingData();
    virtual void notifyAboutReceivedPasteData(const byte* data, long length);
    virtual void notifyAboutEndOfPastingData();

    WeakPtr<TextEditorWidget> e;

    SingleLineEditField::Ptr editField;
    Button::Ptr findNextButton;
    Button::Ptr findPrevButton;
    Button::Ptr cancelButton;
    Button::Ptr goBackButton;
    CheckBox::Ptr caseSensitiveCheckBox;
    CheckBox::Ptr wholeWordCheckBox;
    CheckBox::Ptr regularExprCheckBox;
    Callback<MessageBoxParameter>::Ptr messageBoxInvoker;
    Callback<DialogPanel*>::Ptr        panelInvoker;
    Regex regex;
    Direction::Type defaultDirection;
    int historyIndex;
    String selectionSearchString;
    bool   selectionSearchForwardFlag;
    bool selectSearchRegexFlag;
    TextData::Ptr editFieldTextData;
    FindUtil      findUtil;
};

class FindPanelAccess
{
protected:
    static TextData::Ptr getFindEditFieldTextData(FindPanel* p) {
        return p->editFieldTextData;
    }
};

} // namespace LucED

#endif // FINDPANEL_H
