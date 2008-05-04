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

#include <X11/keysym.h>

#include "ReplacePanel.hpp"
#include "GuiLayoutColumn.hpp"
#include "GuiLayoutRow.hpp"
#include "GlobalConfig.hpp"
#include "GuiLayoutSpacer.hpp"
#include "LabelWidget.hpp"
#include "RegexException.hpp"
#include "SearchHistory.hpp"
#include "SubstitutionException.hpp"
#include "PasteDataCollector.hpp"
#include "ReplaceUtil.hpp"
          
using namespace LucED;

ReplacePanel::ReplacePanel(GuiWidget* parent, TextEditorWidget* editorWidget, FindPanel* findPanel, 
                           Callback<const MessageBoxParameter&>::Ptr messageBoxInvoker,
                           Callback<DialogPanel*>::Ptr               panelInvoker,
                           Callback<GuiWidget*>::Ptr                 requestCloseCallback)
    : DialogPanel(parent, requestCloseCallback),

      pasteDataReceiver(PasteDataReceiver::create(this,
                                                  PasteDataCollector<ReplacePanel>::create(this))),

      e(editorWidget),
      messageBoxInvoker(messageBoxInvoker),
      panelInvoker(panelInvoker),
      defaultDirection(Direction::DOWN),
      historyIndex(-1),
      selectSearchRegexFlag(false),
      findPanel(findPanel),

      interactionCallbacks(messageBoxInvoker,
                           SearchHistory::getInstance()->getMessageBoxQueue(),
                           newCallback(this, &ReplacePanel::requestCloseFromInteraction),
                           newCallback(this, &ReplacePanel::requestCurrentSelectionForInteraction),
                           newCallback(this, &ReplacePanel::handleException),
                           this)
      
{
    GuiLayoutColumn::Ptr  c0 = GuiLayoutColumn::create();
    GuiLayoutColumn::Ptr  c1 = GuiLayoutColumn::create();
    GuiLayoutRow::Ptr     r0 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r1 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r2 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r3 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r4 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r5 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r7 = GuiLayoutRow::create();
    GuiLayoutSpacerFrame::Ptr frame0 = GuiLayoutSpacerFrame::create(c0, 0);
    setRootElement(frame0);

    editFieldGroup = EditFieldGroup::create();

    findEditField    = SingleLineEditField::create(this, 
                                               GlobalConfig::getInstance()->getDefaultLanguageMode(),
                                               FindPanelAccess::getFindEditFieldTextData(findPanel));
    findEditField->setDesiredWidthInChars(5, 10, INT_MAX);
    findEditField->setToEditFieldGroup(editFieldGroup);

    replaceEditField  = SingleLineEditField::create(this, 
                                               GlobalConfig::getInstance()->getDefaultLanguageMode());
    replaceEditField->setDesiredWidthInChars(5, 10, INT_MAX);
    replaceEditField->setToEditFieldGroup(editFieldGroup);

    findNextButton          = Button::create     (this, "Find N]ext");
    findPrevButton          = Button::create     (this, "Find P]revious");
    replaceNextButton       = Button::create     (this, "Replace & Nex]t");
    replacePrevButton       = Button::create     (this, "Replace & Pre]v");
    replaceSelectionButton  = Button::create     (this, "All in S]election");
    replaceWindowButton     = Button::create     (this, "All in W]indow");
    cancelButton            = Button::create     (this, "Cl]ose");
    LabelWidget::Ptr label0 = LabelWidget::create(this, "Find:");
    LabelWidget::Ptr label1 = LabelWidget::create(this, "Replace:");
    
    
    caseSensitiveCheckBox   = CheckBox::create   (this, "C]ase Sensitive");
    wholeWordCheckBox       = CheckBox::create   (this, "Wh]ole Word");
    regularExprCheckBox     = CheckBox::create   (this, "R]egular Expression");
    
    Callback<CheckBox*>::Ptr checkBoxCallback = newCallback(this, &ReplacePanel::handleCheckBoxPressed);

    caseSensitiveCheckBox->setButtonPressedCallback(checkBoxCallback);
    wholeWordCheckBox    ->setButtonPressedCallback(checkBoxCallback);
    regularExprCheckBox  ->setButtonPressedCallback(checkBoxCallback);

    label0   ->setLayoutHeight(findPrevButton->getStandardHeight(), VerticalAdjustment::CENTER);
    label1   ->setLayoutHeight(findPrevButton->getStandardHeight(), VerticalAdjustment::CENTER);
    findEditField   ->setLayoutHeight(findPrevButton->getStandardHeight(), VerticalAdjustment::CENTER);
    replaceEditField->setLayoutHeight(findPrevButton->getStandardHeight(), VerticalAdjustment::CENTER);
 
    Measures labelMeasures;
             labelMeasures.maximize(label0->getDesiredMeasures());
             labelMeasures.maximize(label1->getDesiredMeasures());
    label0->setDesiredMeasures(labelMeasures);
    label1->setDesiredMeasures(labelMeasures);

    Callback<Button*>::Ptr buttonPressedCallback      = newCallback(this, &ReplacePanel::handleButtonPressed,
                                                                          &ReplacePanel::handleException);
    Callback<Button*>::Ptr buttonRightClickedCallback = newCallback(this, &ReplacePanel::handleButtonRightClicked,
                                                                          &ReplacePanel::handleException);

    findNextButton        ->setButtonPressedCallback(buttonPressedCallback);
    findPrevButton        ->setButtonPressedCallback(buttonPressedCallback);
    replaceNextButton     ->setButtonPressedCallback(buttonPressedCallback);
    replacePrevButton     ->setButtonPressedCallback(buttonPressedCallback);

    replaceSelectionButton->setButtonPressedCallback(buttonPressedCallback);
    replaceSelectionButton->setButtonRightClickedCallback(buttonRightClickedCallback);

    replaceWindowButton   ->setButtonPressedCallback(buttonPressedCallback);
    cancelButton          ->setButtonPressedCallback(buttonPressedCallback);
    
    findEditField         ->setNextFocusWidget(replaceEditField);
    replaceEditField      ->setNextFocusWidget(findPrevButton);
    findPrevButton        ->setNextFocusWidget(findNextButton);
    findNextButton        ->setNextFocusWidget(replacePrevButton);
    replacePrevButton     ->setNextFocusWidget(replaceNextButton);
    replaceNextButton     ->setNextFocusWidget(caseSensitiveCheckBox);
    caseSensitiveCheckBox ->setNextFocusWidget(wholeWordCheckBox);
    wholeWordCheckBox     ->setNextFocusWidget(regularExprCheckBox);
    regularExprCheckBox   ->setNextFocusWidget(replaceSelectionButton);
    replaceSelectionButton->setNextFocusWidget(replaceWindowButton);
    replaceWindowButton   ->setNextFocusWidget(cancelButton);
    cancelButton          ->setNextFocusWidget(findEditField);

    setFocus(findEditField);

    Measures buttonMeasures;
             buttonMeasures.maximize(findPrevButton   ->getDesiredMeasures());
             buttonMeasures.maximize(findNextButton   ->getDesiredMeasures());
             buttonMeasures.maximize(replacePrevButton->getDesiredMeasures());
             buttonMeasures.maximize(replaceNextButton->getDesiredMeasures());

    c0->addElement(r0);
    r0->addElement(c1);
    
    c1->addElement(r4);
    r4->addElement(label0);
    r4->addElement(findEditField);
    r4->addElement(findPrevButton);
    r4->addElement(findNextButton);

    c1->addElement(r5);
    r5->addElement(label1);
    r5->addElement(replaceEditField);
    r5->addElement(replacePrevButton);
    r5->addElement(replaceNextButton);

    c1->addElement(r3);
    r3->addElement(caseSensitiveCheckBox);
    r3->addElement(wholeWordCheckBox);
    r3->addElement(regularExprCheckBox);
    r3->addSpacer();
    r3->addElement(replaceSelectionButton);
    r3->addElement(replaceWindowButton);
    r3->addElement(cancelButton);
    
    caseSensitiveCheckBox->setChecked(false);
    
    findNextButton->setAsDefaultButton();

    findPrevButton   ->setDesiredMeasures(buttonMeasures);
    findNextButton   ->setDesiredMeasures(buttonMeasures);
    replacePrevButton->setDesiredMeasures(buttonMeasures);
    replaceNextButton->setDesiredMeasures(buttonMeasures);

    label0                ->show();
    findEditField         ->show();
    label1                ->show();
    replaceEditField      ->show();
    findPrevButton        ->show();
    findNextButton        ->show();
    replacePrevButton     ->show();
    replaceNextButton     ->show();
    replaceSelectionButton->show();
    replaceWindowButton   ->show();
    cancelButton          ->show();
    caseSensitiveCheckBox ->show();
    regularExprCheckBox   ->show();
    wholeWordCheckBox     ->show();
    
    findEditField   ->getTextData()->registerModifiedFlagListener(newCallback(this, &ReplacePanel::handleModifiedEditField,
                                                                                    &ReplacePanel::handleException));
    replaceEditField->getTextData()->registerModifiedFlagListener(newCallback(this, &ReplacePanel::handleModifiedEditField,
                                                                                    &ReplacePanel::handleException));

    label0->setMiddleMouseButtonCallback(newCallback(findEditField,    &SingleLineEditField::replaceTextWithPrimarySelection));
    label1->setMiddleMouseButtonCallback(newCallback(replaceEditField, &SingleLineEditField::replaceTextWithPrimarySelection));
}

void ReplacePanel::treatFocusOut()
{
    if (isVisible()) {
        if (editFieldGroup->getLastFocusObject() == findEditField) {
            lastFocusedEditField = findEditField;
        } else if (editFieldGroup->getLastFocusObject() == replaceEditField) {
            lastFocusedEditField = replaceEditField;
        } else {
            lastFocusedEditField.invalidate();
        }
    } else {
        lastFocusedEditField.invalidate();
    }
    DialogPanel::treatFocusOut();
}

void ReplacePanel::treatFocusIn()
{
    if (lastFocusedEditField.isValid()) {
        setFocus(lastFocusedEditField);
    } else {
        setFocus(findEditField);
    }
    DialogPanel::treatFocusIn();
}


void ReplacePanel::requestCurrentSelectionForInteraction(SearchInteraction* interaction, Callback<String>::Ptr selectionRequestedCallback)
{
    if (currentInteraction.getRawPtr() == interaction) {
        this->selectionRequestedCallback = selectionRequestedCallback;
        pasteDataReceiver->requestSelectionPasting();
    }
}

void ReplacePanel::requestCloseFromInteraction(SearchInteraction* interaction)
{
    if (currentInteraction.getRawPtr() == interaction) {
        requestClose();
    }
}

void ReplacePanel::notifyAboutCollectedPasteData(String collectedSelectionData)
{
    selectionRequestedCallback->call(collectedSelectionData);
}


GuiElement::ProcessingResult ReplacePanel::processEvent(const XEvent* event)
{
    if (pasteDataReceiver->processPasteDataReceiverEvent(event) == EVENT_PROCESSED) {
        return EVENT_PROCESSED;
    } else {
        return DialogPanel::processEvent(event);
    }
}




void ReplacePanel::handleButtonPressed(Button* button)
{
    if (button == cancelButton)
    {
        requestClose();
    }
    else if (button == findNextButton    || button == findPrevButton
          || button == replaceNextButton || button == replacePrevButton)
    {
        SearchParameter p = getSearchParameterFromGuiControls();
                        p.setSearchForwardFlag(   button == findNextButton 
                                               || button == replaceNextButton);

        findEditField   ->getTextData()->setModifiedFlag(false);
        replaceEditField->getTextData()->setModifiedFlag(false);

        if (   (button == findNextButton || button == findPrevButton)
            && currentInteraction.isValid()
            && currentInteraction->isWaitingForContinue() 
            && currentInteraction->getSearchParameter().findsSameThan(p))
        {
            if (button == findNextButton) {
                currentInteraction->continueForwardAndKeepInvokingPanel();
            } else {
                currentInteraction->continueBackwardAndKeepInvokingPanel();
            }
        }
        else if (   (button == replaceNextButton || button == replacePrevButton)
                 && currentInteraction.isValid()
                 && currentInteraction->isWaitingForContinue() 
                 && currentInteraction->getSearchParameter().findsAndReplacesSameThan(p))
        {
            if (button == replaceNextButton) {
                currentInteraction->replaceAndContinueForwardAndKeepInvokingPanel();
            } else {
                currentInteraction->replaceAndContinueBackwardAndKeepInvokingPanel();
            }
        }
        else
        {
            SearchHistory::getInstance()->getMessageBoxQueue()->closeQueued();

            historyIndex = -1;
     
            currentInteraction = SearchInteraction::create(p, e, interactionCallbacks);
            
            if (   button == replaceNextButton 
                || button == replacePrevButton)
            {
                currentInteraction->replaceAndContinueWithFind();
            }
            else {   
                currentInteraction->startFind();
            }
        }
    }
    else if ((button == replaceSelectionButton && e->hasSelection())
           || button == replaceWindowButton)
    {
        long spos;
        long epos;

        if (button == replaceSelectionButton) {
            spos = e->getBeginSelectionPos();
            epos = e->getEndSelectionPos();
        } else {
            spos = 0;
            epos = e->getTextData()->getLength();
        }
        String newRememberedSelection;
        if (button == replaceSelectionButton) {
            newRememberedSelection = e->getTextData()->getSubstring(spos, epos - spos);
        }
        SearchParameter p = getSearchParameterFromGuiControls();
                        p.setSearchForwardFlag(true);
                       
        ReplaceUtil replaceUtil(e->getTextData());
                    replaceUtil.setParameter(p);

        findEditField   ->getTextData()->setModifiedFlag(false);
        replaceEditField->getTextData()->setModifiedFlag(false);

        historyIndex = -1;

        SearchHistory::getInstance()->append(p);

        bool wasAythingReplaced = replaceUtil.replaceAllBetween(spos, epos);

        if (wasAythingReplaced && button == replaceSelectionButton && newRememberedSelection.getLength() > 0) {
            this->rememberedSelection = newRememberedSelection;
            e->registerListenerForNextSelectionChange(newCallback(this, &ReplacePanel::forgetRememberedSelection,
                                                                        &ReplacePanel::handleException));
        }
    }
}

void ReplacePanel::invalidateOutdatedInteraction()
{
    SearchParameter p = getSearchParameterFromGuiControls();

    if (   currentInteraction.isValid()
        && currentInteraction->isWaitingForContinue() 
        && !currentInteraction->getSearchParameter().findsAndReplacesSameThan(p))
    {
        currentInteraction.invalidate();
    }
}

void ReplacePanel::handleCheckBoxPressed(CheckBox* checkBox)
{
    invalidateOutdatedInteraction();
}

void ReplacePanel::forgetRememberedSelection()
{
    rememberedSelection = String();
}

void ReplacePanel::handleButtonRightClicked(Button* button)
{
    if (button == replaceSelectionButton && rememberedSelection.getLength() > 0) 
    {
        // right clicking replaceSelectionButton means "Restore Selection"
    
       TextData::TextMark mark = e->getNewMarkToBeginOfSelection();
       long selLength = e->getSelectionLength();
        
       e->getTextData()->insertAtMark(mark, rememberedSelection);
       mark.moveForwardToPos(mark.getPos() + rememberedSelection.getLength());
       e->getTextData()->removeAtMark(mark, selLength);
       e->adjustCursorVisibility();
    
       rememberedSelection = String();
    }
}



void ReplacePanel::findAgainForward()
{
    internalFindAgain(true);
}

void ReplacePanel::findAgainBackward()
{
    internalFindAgain(false);
}

void ReplacePanel::internalFindAgain(bool forwardFlag)
{
    try
    {
        if (this->isVisible() && findEditField->getTextData()->getLength() == 0) {
            return;
        }
        
        SearchHistory::getInstance()->getMessageBoxQueue()->closeQueued();

        SearchParameter p;
        
        if (this->isVisible()) {
            p = getSearchParameterFromGuiControls();
            p.setSearchForwardFlag(forwardFlag);
            findEditField->getTextData()->setModifiedFlag(false);
            replaceEditField->getTextData()->setModifiedFlag(false);
            requestClose();
        }
        else
        {
            if (SearchHistory::getInstance()->hasEntries()) {
                p = SearchHistory::getInstance()->getSearchParameterFromLastEntry();
                p.setSearchForwardFlag(forwardFlag);
            } else {
                return;
            }
        }
        historyIndex = -1;
        
        currentInteraction = SearchInteraction::create(p, e, interactionCallbacks);
        currentInteraction->startFind();
    }
    catch (...)
    {
        handleException();
    }

}

void ReplacePanel::replaceAgainForward()
{
    internalReplaceAgain(true);
}

void ReplacePanel::replaceAgainBackward()
{
    internalReplaceAgain(false);
}

void ReplacePanel::internalReplaceAgain(bool forwardFlag)
{
    try
    {
        if (this->isVisible() && findEditField->getTextData()->getLength() == 0) {
            return;
        }
    
        SearchHistory::getInstance()->getMessageBoxQueue()->closeQueued();

        SearchParameter p;
        
        if (this->isVisible()) {
            p = getSearchParameterFromGuiControls();
            p.setSearchForwardFlag(forwardFlag);
            findEditField   ->getTextData()->setModifiedFlag(false);
            replaceEditField->getTextData()->setModifiedFlag(false);
            requestClose();
        }
        else
        {
            if (SearchHistory::getInstance()->hasEntries()) {
                p = SearchHistory::getInstance()->getSearchParameterFromLastEntry();
                if (!p.hasReplaceString()) {
                    return;
                }
                p.setSearchForwardFlag(forwardFlag);
            } else {
                return;
            }
        }
    
        historyIndex = -1;
    
        currentInteraction = SearchInteraction::create(p, e, interactionCallbacks);
        currentInteraction->replaceAndDontContinueWithFind();
    }
    catch (...) {
        handleException();
    }
}



GuiElement::ProcessingResult ReplacePanel::processKeyboardEvent(const XEvent *event)
{
    KeyId pressedKey = KeyId(XLookupKeysym((XKeyEvent*)&event->xkey, 0));

    bool processed = false;
    KeyMapping::Id keyMappingId(event->xkey.state, pressedKey);

    if (KeyMapping::Id(0, KeyId("Up"))    == keyMappingId
     || KeyMapping::Id(0, KeyId("KP_Up")) == keyMappingId)
    {
        SearchHistory* history = SearchHistory::getInstance();

        if (historyIndex < 0) {
            String editFieldContent = findEditField->getTextData()->getAsString();

            SearchHistory::Entry newEntry;
                                 newEntry.setFindString    (editFieldContent);
                                 newEntry.setReplaceString (replaceEditField->getTextData()->getAsString());
                                 newEntry.setWholeWordFlag (wholeWordCheckBox->isChecked());
                                 newEntry.setRegexFlag     (regularExprCheckBox->isChecked());
                                 newEntry.setIgnoreCaseFlag(!caseSensitiveCheckBox->isChecked());

            history->append(newEntry);
        }

        int h = historyIndex;
        if (h < 0) {
            h = history->getEntryCount();
        }
        --h;
        String findFieldContent    = findEditField->getTextData()->getAsString();
        String replaceFieldContent = replaceEditField->getTextData()->getAsString();
        while (h >= 0) {
            SearchHistory::Entry entry = history->getEntry(h);
            String lastFindString    = entry.getFindString();
            String lastReplaceString = entry.getReplaceString();
            if (lastFindString != findFieldContent || lastReplaceString != replaceFieldContent)
            {
                RawPtr<TextData> textData = findEditField->getTextData();
                textData->setToString(lastFindString);
                textData->clearHistory();
                textData->setModifiedFlag(false);

                textData = replaceEditField->getTextData();
                textData->setToString(lastReplaceString);
                textData->clearHistory();
                textData->setModifiedFlag(false);
                
                historyIndex = h;
                wholeWordCheckBox->setChecked(entry.getWholeWordFlag());
                regularExprCheckBox->setChecked(entry.getRegexFlag());
                caseSensitiveCheckBox->setChecked(!entry.getIgnoreCaseFlag());
                break;
            }
            --h;
        }
        processed = true;
    }
    else if (KeyMapping::Id(0, KeyId("Down"))    == keyMappingId
          || KeyMapping::Id(0, KeyId("KP_Down")) == keyMappingId)
    {
        SearchHistory* history = SearchHistory::getInstance();

        if (historyIndex < 0) {
            String editFieldContent = findEditField->getTextData()->getAsString();

            SearchHistory::Entry newEntry;
                                 newEntry.setFindString    (editFieldContent);
                                 newEntry.setReplaceString (replaceEditField->getTextData()->getAsString());
                                 newEntry.setWholeWordFlag (wholeWordCheckBox->isChecked());
                                 newEntry.setRegexFlag     (regularExprCheckBox->isChecked());
                                 newEntry.setIgnoreCaseFlag(!caseSensitiveCheckBox->isChecked());

            history->append(newEntry);

            historyIndex = history->getEntryCount() - 1;
        }

        if (historyIndex >= 0) {
            String findFieldContent    = findEditField   ->getTextData()->getAsString();
            String replaceFieldContent = replaceEditField->getTextData()->getAsString();
            int h = historyIndex;
            bool found = false;
            while (h + 1 < history->getEntryCount()) {
                ++h;
                SearchHistory::Entry entry = history->getEntry(h);
                String nextFindString    = entry.getFindString();
                String nextReplaceString = entry.getReplaceString();
                if (nextFindString != findFieldContent || nextReplaceString != replaceFieldContent)
                {
                    RawPtr<TextData> textData = findEditField->getTextData();
                    textData->setToString(nextFindString);
                    textData->clearHistory();
                    textData->setModifiedFlag(false);
                
                    textData = replaceEditField->getTextData();
                    textData->setToString(nextReplaceString);
                    textData->clearHistory();
                    textData->setModifiedFlag(false);
                    
                    historyIndex = h;
                    found = true;
                    wholeWordCheckBox->setChecked(entry.getWholeWordFlag());
                    regularExprCheckBox->setChecked(entry.getRegexFlag());
                    caseSensitiveCheckBox->setChecked(!entry.getIgnoreCaseFlag());
                    break;
                }
            }
            if (!found) {
                historyIndex = -1;
                   findEditField->getTextData()->clear();
                replaceEditField->getTextData()->clear();
                caseSensitiveCheckBox->setChecked(false);
                regularExprCheckBox->setChecked(false);
                wholeWordCheckBox->setChecked(false);
            }
        }
        processed = true;
    }
    else if (KeyMapping::Id(ControlMask, KeyId("g")) == keyMappingId)
    {
        findAgainForward();
        processed = true;
    }
    else if (KeyMapping::Id(ShiftMask|ControlMask, KeyId("g")) == keyMappingId)
    {
        findAgainBackward();
        processed = true;
    }
    
    if (!processed) {
        return DialogPanel::processKeyboardEvent(event);
    } else {
        findEditField->showCursor();
        return EVENT_PROCESSED;
    }
}


void ReplacePanel::hide()
{
    rememberedSelection = String();
    
    lastFocusedEditField.invalidate();
    DialogPanel::hide();
}


void ReplacePanel::show()
{
    findPrevButton->setAsDefaultButton(defaultDirection != Direction::DOWN);
    findNextButton->setAsDefaultButton(defaultDirection == Direction::DOWN);
    replacePrevButton->setAsDefaultButton(false);
    replaceNextButton->setAsDefaultButton(false);

    rememberedSelection = String();

    if (e->hasPrimarySelection())
    {
        TextData::TextMark  beginMark = e->getNewMarkToBeginOfSelection();
        TextData::TextMark  endMark   = e->getNewMarkToEndOfSelection();
        
        if (beginMark.getLine() == endMark.getLine()) {
            String selectedText = e->getTextData()->getSubstring(beginMark, endMark);
            if (selectedText != findEditField->getTextData()->getAsString())
            {
                findEditField   ->getTextData()->setToString(selectedText);
                findEditField   ->setCursorPosition(findEditField->getTextData()->getLength());
                replaceEditField->getTextData()->clear();
                historyIndex = -1;
            }
        }
    } else if (   findEditField->getTextData()->getModifiedFlag() == false
            && replaceEditField->getTextData()->getModifiedFlag() == false)
    {
        findEditField   ->getTextData()->clear();
        replaceEditField->getTextData()->clear();
        historyIndex = -1;
        caseSensitiveCheckBox->setChecked(false);
        regularExprCheckBox->setChecked(false);
        wholeWordCheckBox->setChecked(false);
    }
    DialogPanel::show();
}

void ReplacePanel::handleModifiedEditField(bool modifiedFlag)
{
    if (modifiedFlag == true)
    {
        rememberedSelection = String();
        historyIndex = -1;
        
        invalidateOutdatedInteraction();
    }
}


void ReplacePanel::handleException()
{
    try
    {
        throw;
    }
    catch (SubstitutionException& ex)
    {
        int position = ex.getPosition();
        if (position >= 0) {
            replaceEditField->setCursorPosition(position);
            setFocus(replaceEditField);
        }
        panelInvoker->call(this);
        messageBoxInvoker->call(MessageBoxParameter()
                                .setTitle("Replace Error")
                                .setMessage(String() << "Error within replace string: " << ex.getMessage())
                                .setMessageBoxQueue(SearchHistory::getInstance()->getMessageBoxQueue()));
    }
    catch (RegexException& ex)
    {
        int position = ex.getPosition();
        if (position >= 0) {
            findEditField->setCursorPosition(position);
            setFocus(findEditField);
        }
        panelInvoker->call(this);
        messageBoxInvoker->call(MessageBoxParameter()
                                .setTitle("Regex Error")
                                .setMessage(String() << "Error within regular expression: " << ex.getMessage())
                                .setMessageBoxQueue(SearchHistory::getInstance()->getMessageBoxQueue()));
    }
    catch (LuaException& ex)
    {
        panelInvoker->call(this);
        messageBoxInvoker->call(MessageBoxParameter()
                                .setTitle("Lua Error")
                                .setMessage(ex.getMessage())
                                .setMessageBoxQueue(SearchHistory::getInstance()->getMessageBoxQueue()));
    }
}


