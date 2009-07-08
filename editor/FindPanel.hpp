/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#ifndef FIND_PANEL_HPP
#define FIND_PANEL_HPP

#include "String.hpp"

#include "DialogPanel.hpp"
#include "Button.hpp"
#include "CheckBox.hpp"
#include "TextEditorWidget.hpp"
#include "SingleLineEditField.hpp"
#include "MessageBox.hpp"
#include "Callback.hpp"
#include "BasicRegex.hpp"
#include "types.hpp"
#include "KeyMapping.hpp"
#include "SearchInteraction.hpp"
#include "PasteDataCollector.hpp"
#include "ActionMethodBinding.hpp"

namespace LucED
{

class FindPanel : public DialogPanel
{
public:
    typedef DialogPanel            BaseClass;
    typedef OwningPtr<FindPanel>   Ptr;

    static Ptr create(RawPtr<TextEditorWidget> editorWidget, Callback<const MessageBoxParameter&>::Ptr messageBoxInvoker,
                                                             Callback<>::Ptr                           panelInvoker,
                                                             Callback<>::Ptr                           panelCloser)
    {
        return Ptr(new FindPanel(editorWidget, messageBoxInvoker, panelInvoker, panelCloser));
    }
    
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

    virtual void show();
    
private:
    class EditFieldActions : public ActionMethodBinding<EditFieldActions>
    {
    public:
        typedef OwningPtr<EditFieldActions> Ptr;

        static Ptr create(RawPtr<FindPanel> thisFindPanel) {
            return Ptr(new EditFieldActions(thisFindPanel));
        }
        void historyBackward() {
            thisFindPanel->executeHistoryBackwardAction();
        }
        void historyForward() {
            thisFindPanel->executeHistoryForwardAction();
        }
    private:
        EditFieldActions(RawPtr<FindPanel> thisFindPanel)
            : ActionMethodBinding<EditFieldActions>(this),
              thisFindPanel(thisFindPanel)
        {}
        RawPtr<FindPanel> thisFindPanel;
    };
    friend class ActionMethodBinding<EditFieldActions>;
    friend class FindPanelAccess;
    friend class PasteDataCollector<FindPanel>;
    
    FindPanel(RawPtr<TextEditorWidget> editorWidget, Callback<const MessageBoxParameter&>::Ptr messageBoxInvoker,
                                                     Callback<>::Ptr                           panelInvoker,
                                                     Callback<>::Ptr                           panelCloser);
    void executeHistoryBackwardAction();
    void executeHistoryForwardAction();

    void invalidateOutdatedInteraction();
    
    void handleException();
    
    void handleButtonPressed(Button* button, Button::ActivationVariant variant);
    void handleCheckBoxPressed(CheckBox* checkBox);

    void internalFindAgain(bool forwardFlag);
    void internalFindSelection(bool forwardFlag);
    
    
    void handleModifiedEditField(bool modifiedFlag);

    void requestCloseFromInteraction(SearchInteraction* interaction);

    SearchParameter getSearchParameterFromGuiControls() const {
        return SearchParameter().setIgnoreCaseFlag (!caseSensitiveCheckBox->isChecked())
                                .setRegexFlag      (regularExprCheckBox->isChecked())
                                .setWholeWordFlag  (wholeWordCheckBox->isChecked())
                                .setFindString     (editField->getTextData()->getAsString());
    }

    RawPtr<TextEditorWidget> e;

    SingleLineEditField::Ptr editField;
    Button::Ptr findNextButton;
    Button::Ptr findPrevButton;
    Button::Ptr cancelButton;
    Button::Ptr goBackButton;
    CheckBox::Ptr caseSensitiveCheckBox;
    CheckBox::Ptr wholeWordCheckBox;
    CheckBox::Ptr regularExprCheckBox;
    Callback<const MessageBoxParameter&>::Ptr messageBoxInvoker;
    Callback<>::Ptr                           panelInvoker;
    BasicRegex regex;
    Direction::Type defaultDirection;
    int historyIndex;

    TextData::Ptr editFieldTextData;

    SearchInteraction::Ptr         currentInteraction;
    SearchInteraction::Callbacks   interactionCallbacks;
};

class FindPanelAccess
{
protected:
    static TextData::Ptr getFindEditFieldTextData(FindPanel* p) {
        return p->editFieldTextData;
    }
};

} // namespace LucED

#endif // FIND_PANEL_HPP
