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
#include "PasteDataReceiver.hpp"
#include "KeyMapping.hpp"
#include "SearchInteraction.hpp"
#include "PasteDataCollector.hpp"
#include "PanelInvoker.hpp"
#include "ActionMethodBinding.hpp"

namespace LucED
{

class FindPanel : public DialogPanel
{
public:
    typedef OwningPtr<FindPanel> Ptr;

    static Ptr create(GuiWidget* parent, RawPtr<TextEditorWidget> editorWidget, Callback<const MessageBoxParameter&>::Ptr messageBoxInvoker,
                                                                                PanelInvoker::Ptr                         panelInvoker)
    {
        return Ptr(new FindPanel(parent, editorWidget, messageBoxInvoker, panelInvoker));
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

    virtual ProcessingResult processEvent(const XEvent* event);
    
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
    
    FindPanel(GuiWidget* parent, RawPtr<TextEditorWidget> editorWidget, Callback<const MessageBoxParameter&>::Ptr messageBoxInvoker,
                                                                        PanelInvoker::Ptr                         panelInvoker);
    void executeHistoryBackwardAction();
    void executeHistoryForwardAction();

    void invalidateOutdatedInteraction();
    
    void handleException();
    
    void handleButtonPressed(Button* button, Button::ActivationVariant variant);
    void handleCheckBoxPressed(CheckBox* checkBox);

    void internalFindAgain(bool forwardFlag);
    void internalFindSelection(bool forwardFlag);
    
    
    void handleModifiedEditField(bool modifiedFlag);

    void notifyAboutCollectedPasteData(String collectedSelectionData);
    
    void requestCurrentSelectionForInteraction(SearchInteraction* interaction, Callback<String>::Ptr selectionRequestedCallback);
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
    PanelInvoker::Ptr                         panelInvoker;
    BasicRegex regex;
    Direction::Type defaultDirection;
    int historyIndex;

    TextData::Ptr editFieldTextData;

    PasteDataReceiver::Ptr pasteDataReceiver;
    
    Callback<String>::Ptr selectionRequestedCallback;

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
