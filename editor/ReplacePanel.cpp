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

using namespace LucED;

ReplacePanel::ReplacePanel(GuiWidget* parent, TextEditorWidget* editorWidget, FindPanel* findPanel, 
                           Callback<MessageBoxParameter>::Ptr messageBoxInvoker,
                           Callback<DialogPanel*>::Ptr        panelInvoker,
                           Callback<GuiWidget*>::Ptr          requestCloseCallback)
    : DialogPanel(parent, requestCloseCallback),
      e(editorWidget),
      messageBoxInvoker(messageBoxInvoker),
      panelInvoker(panelInvoker),
      defaultDirection(Direction::DOWN),
      historyIndex(-1),
      selectSearchRegexFlag(false),
      replaceUtil(e->getTextData())
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
    
    label0   ->setLayoutHeight(findPrevButton->getStandardHeight(), VerticalAdjustment::CENTER);
    label1   ->setLayoutHeight(findPrevButton->getStandardHeight(), VerticalAdjustment::CENTER);
    findEditField   ->setLayoutHeight(findPrevButton->getStandardHeight(), VerticalAdjustment::CENTER);
    replaceEditField->setLayoutHeight(findPrevButton->getStandardHeight(), VerticalAdjustment::CENTER);
 
    Measures labelMeasures;
             labelMeasures.maximize(label0->getDesiredMeasures());
             labelMeasures.maximize(label1->getDesiredMeasures());
    label0->setDesiredMeasures(labelMeasures);
    label1->setDesiredMeasures(labelMeasures);

    Callback<Button*>::Ptr buttonPressedCallback      = newCallback(this, &ReplacePanel::handleButtonPressed);
    Callback<Button*>::Ptr buttonRightClickedCallback = newCallback(this, &ReplacePanel::handleButtonRightClicked);

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
    
    findEditField   ->getTextData()->registerModifiedFlagListener(newCallback(this, &ReplacePanel::handleModifiedEditField));
    replaceEditField->getTextData()->registerModifiedFlagListener(newCallback(this, &ReplacePanel::handleModifiedEditField));

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


void ReplacePanel::executeFind(bool isWrapping, Callback<>::Ptr handleContinueSearchButton)
{
    try
    {
/*        {
            if (e->hasSelectionOwnership()) {
                e->getBackliteBuffer()->deactivateSelection();
            }
            TextData::TextMark m = e->createNewMarkFromCursor();
            m.moveToPos(replaceUtil.getTextPosition());
            e->moveCursorToTextMarkAndAdjustVisibility(m);
        }
*/
        GuiRoot::getInstance()->flushDisplay();

        replaceUtil.setAllowMatchAtStartOfSearchFlag(isWrapping);
        replaceUtil.findNext();

        if (replaceUtil.wasFound())
        {
            TextData::TextMark m = e->createNewMarkFromCursor();
            if (replaceUtil.isSearchingForward()) {
                m.moveToPos(replaceUtil.getMatchBeginPos());
            } else {
                m.moveToPos(replaceUtil.getMatchEndPos());
            }
            e->moveCursorToTextMarkAndAdjustVisibility(m);
            if (replaceUtil.isSearchingForward()) {
                m.moveToPos(replaceUtil.getMatchEndPos());
            } else {
                m.moveToPos(replaceUtil.getMatchBeginPos());
            }
            e->moveCursorToTextMarkAndAdjustVisibility(m);
            e->rememberCursorPixX();
            if (replaceUtil.getMatchBeginPos() < replaceUtil.getMatchEndPos())
            {
                e->setPrimarySelection(replaceUtil.getMatchBeginPos(),
                                       replaceUtil.getMatchEndPos());
            } else {
                e->releaseSelection();
            }

            replacePrevButton->setAsDefaultButton(!replaceUtil.isSearchingForward());
            replaceNextButton->setAsDefaultButton( replaceUtil.isSearchingForward());
            findPrevButton   ->setAsDefaultButton(false);
            findNextButton   ->setAsDefaultButton(false);
        }
        else 
        {
            findPrevButton->setAsDefaultButton(!replaceUtil.isSearchingForward());
            findNextButton->setAsDefaultButton( replaceUtil.isSearchingForward());
            replacePrevButton->setAsDefaultButton(false);
            replaceNextButton->setAsDefaultButton(false);

            if (!isWrapping) {
                if (replaceUtil.isSearchingForward()) {
                    messageBoxInvoker->call(MessageBoxParameter()
                                            .setTitle("Not found")
                                            .setMessage("Continue search from beginning of file?")
                                            .setDefaultButton("C]ontinue", handleContinueSearchButton)
                                            .setCancelButton("Ca]ncel"));
                } else {
                    messageBoxInvoker->call(MessageBoxParameter()
                                            .setTitle("Not found")
                                            .setMessage("Continue search from end of file?")
                                            .setDefaultButton("C]ontinue", handleContinueSearchButton)
                                            .setCancelButton("Ca]ncel"));
                }
            } else {
                    messageBoxInvoker->call(MessageBoxParameter()
                                            .setTitle("Not found")
                                            .setMessage("String was not found"));
            }
        }
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
                                .setMessage(String() << "Error within regular expression: " << ex.getMessage()));
    }
    catch (LuaException& ex)
    {
        panelInvoker->call(this);
        messageBoxInvoker->call(MessageBoxParameter()
                                .setTitle("Lua Error")
                                .setMessage(ex.getMessage()));
    }
}

void ReplacePanel::internalFindNext(bool isWrapping)
{
    if (replaceUtil.getSearchString().getLength() <= 0) {
        return;
    }

    SearchHistory::Entry newEntry;
                         newEntry.setFindString    (replaceUtil.getSearchString());
                         newEntry.setReplaceString (replaceUtil.getReplaceString());
                         newEntry.setWholeWordFlag (replaceUtil.getWholeWordFlag());
                         newEntry.setRegexFlag     (replaceUtil.getRegexFlag());
                         newEntry.setIgnoreCaseFlag(replaceUtil.getIgnoreCaseFlag());

    SearchHistory::getInstance()->append(newEntry);

    bool forward = replaceUtil.getSearchForwardFlag();

    executeFind(isWrapping, forward ? newCallback(this, &ReplacePanel::handleContinueAtBeginningButton)
                                    : newCallback(this, &ReplacePanel::handleContinueAtEndButton));
}

void ReplacePanel::handleButtonPressed(Button* button)
{
    try
    {
        if (button == cancelButton)
        {
            requestClose();
        }
        else if (button == findNextButton    || button == findPrevButton
              || button == replaceNextButton || button == replacePrevButton)
        {
            replaceUtil.setTextPosition(e->getCursorTextPosition());

            replaceUtil.setSearchForwardFlag(button == findNextButton || button == replaceNextButton);
            replaceUtil.setIgnoreCaseFlag   (!caseSensitiveCheckBox->isChecked());
            replaceUtil.setRegexFlag        (regularExprCheckBox->isChecked());
            replaceUtil.setWholeWordFlag    (wholeWordCheckBox->isChecked());
            replaceUtil.setSearchString     (findEditField   ->getTextData()->getAsString());
            replaceUtil.setReplaceString    (replaceEditField->getTextData()->getAsString());

            findEditField   ->getTextData()->setModifiedFlag(false);
            replaceEditField->getTextData()->setModifiedFlag(false);

            historyIndex = -1;
            
            bool wasReplaced = false;
            
            if (button == replaceNextButton || button == replacePrevButton)
            {
                long spos, epos;

                if (e->hasPrimarySelection()) {
                    spos = e->getBeginSelectionPos();
                    epos = e->getEndSelectionPos();
                    replaceUtil.setTextPosition(spos);
                } else {
                    spos = replaceUtil.getTextPosition();
                    epos = replaceUtil.getTextPosition();
                }

                if (replaceUtil.doesMatch())
                {
                    ASSERT(replaceUtil.getMatchBeginPos() == spos);
                    if (replaceUtil.getMatchEndPos() == epos)
                    {
                        String substitutedString = replaceUtil.getSubstitutedString();
                        e->moveCursorToTextPosition(spos);
                        e->removeAtCursor(epos - spos);
                        e->insertAtCursor(substitutedString);
                        wasReplaced = true;
                        if (button == replaceNextButton) {
                            int offs = (replaceUtil.getMatchLength() == 0) ? 1 : 0;
                            e->moveCursorToTextPosition(spos + substitutedString.getLength() + offs);
                        } else {
                            e->moveCursorToTextPosition(spos);
                        }
                    }
                    else {
                        if (button == replaceNextButton) {
                            e->moveCursorToTextPosition(epos);
                        } else {
                            e->moveCursorToTextPosition(spos);
                        }
                    }
                }
            }
            int textPosition = e->getCursorTextPosition();
            if (button == findNextButton || (button == replaceNextButton && !wasReplaced)) {
                if (e->hasPrimarySelection()) {
                    replaceUtil.setTextPosition(e->getBeginSelectionPos());
                    if (replaceUtil.doesMatch() && replaceUtil.getMatchEndPos() == e->getEndSelectionPos()) {
                        textPosition = e->getEndSelectionPos();
                    }
                }
            } else if (button == findPrevButton || (button == replacePrevButton && !wasReplaced)) {
                if (e->hasPrimarySelection()) {
                    replaceUtil.setTextPosition(e->getBeginSelectionPos());
                    if (replaceUtil.doesMatch() && replaceUtil.getMatchEndPos() == e->getEndSelectionPos()) {
                        textPosition = e->getBeginSelectionPos();
                    }
                }
            }
            replaceUtil.setTextPosition(textPosition);
            
            internalFindNext(false);
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
            replaceUtil.setSearchForwardFlag(true);
            replaceUtil.setIgnoreCaseFlag   (!caseSensitiveCheckBox->isChecked());
            replaceUtil.setRegexFlag        (regularExprCheckBox->isChecked());
            replaceUtil.setWholeWordFlag    (wholeWordCheckBox->isChecked());
            replaceUtil.setSearchString     (findEditField   ->getTextData()->getAsString());
            replaceUtil.setReplaceString    (replaceEditField->getTextData()->getAsString());

            findEditField   ->getTextData()->setModifiedFlag(false);
            replaceEditField->getTextData()->setModifiedFlag(false);

            historyIndex = -1;

            SearchHistory::Entry newEntry;
                                 newEntry.setFindString    (replaceUtil.getSearchString());
                                 newEntry.setReplaceString (replaceUtil.getReplaceString());
                                 newEntry.setWholeWordFlag (replaceUtil.getWholeWordFlag());
                                 newEntry.setRegexFlag     (replaceUtil.getRegexFlag());
                                 newEntry.setIgnoreCaseFlag(replaceUtil.getIgnoreCaseFlag());

            SearchHistory::getInstance()->append(newEntry);

            bool wasAythingReplaced = replaceUtil.replaceAllBetween(spos, epos);

            if (wasAythingReplaced && button == replaceSelectionButton && newRememberedSelection.getLength() > 0) {
                this->rememberedSelection = newRememberedSelection;
                e->registerListenerForNextSelectionChange(newCallback(this, &ReplacePanel::forgetRememberedSelection));
            }
        }
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
                                .setMessage(String() << "Error within replace string: " << ex.getMessage()));
    }
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

void ReplacePanel::handleContinueSelectionFindAtBeginningButton()
{
    replaceUtil.setSearchForwardFlag(true);
    replaceUtil.setIgnoreCaseFlag   (true);
    replaceUtil.setRegexFlag        (selectSearchRegexFlag);
    replaceUtil.setWholeWordFlag    (false);
    replaceUtil.setSearchString     (selectionSearchString);
    replaceUtil.setTextPosition     (0);

    executeFind(true, newCallback(this, &ReplacePanel::handleContinueSelectionFindAtBeginningButton));
}

void ReplacePanel::handleContinueAtBeginningButton()
{
    ASSERT(replaceUtil.getSearchForwardFlag() == true);
    replaceUtil.setTextPosition(0);
    internalFindNext(true);
}


void ReplacePanel::handleContinueSelectionFindAtEndButton()
{
    replaceUtil.setSearchForwardFlag(false);
    replaceUtil.setIgnoreCaseFlag   (true);
    replaceUtil.setRegexFlag        (selectSearchRegexFlag);
    replaceUtil.setWholeWordFlag    (false);
    replaceUtil.setSearchString     (selectionSearchString);
    replaceUtil.setTextPosition     (e->getTextData()->getLength());

    executeFind(true, newCallback(this, &ReplacePanel::handleContinueSelectionFindAtEndButton));
}

void ReplacePanel::handleContinueAtEndButton()
{
    ASSERT(replaceUtil.getSearchForwardFlag() == false);
    replaceUtil.setTextPosition(e->getTextData()->getLength());
    internalFindNext(true);
}


    
void ReplacePanel::findAgainForward()
{
    ASSERT(this->isVisible());

    if (findEditField->getTextData()->getLength() == 0) {
        return;
    }

    replaceUtil.setIgnoreCaseFlag   (!caseSensitiveCheckBox->isChecked());
    replaceUtil.setRegexFlag        (regularExprCheckBox->isChecked());
    replaceUtil.setWholeWordFlag    (wholeWordCheckBox->isChecked());
    replaceUtil.setSearchString     (findEditField   ->getTextData()->getAsString());
    replaceUtil.setReplaceString    (replaceEditField->getTextData()->getAsString());
    findEditField   ->getTextData()->setModifiedFlag(false);
    replaceEditField->getTextData()->setModifiedFlag(false);
    requestClose();

    historyIndex = -1;

    int textPosition = e->getCursorTextPosition();
    if (e->hasPrimarySelection()) {
        replaceUtil.setTextPosition(e->getBeginSelectionPos());
        if (replaceUtil.doesMatch() && replaceUtil.getMatchEndPos() == e->getEndSelectionPos()) {
            textPosition = e->getEndSelectionPos();
        }
    }
    replaceUtil.setTextPosition(textPosition);
    
    replaceUtil.setSearchForwardFlag(true);
    
    internalFindNext(false);
}


void ReplacePanel::findAgainBackward()
{
    ASSERT(this->isVisible());

    if (findEditField->getTextData()->getLength() == 0) {
        return;
    }

    replaceUtil.setIgnoreCaseFlag   (!caseSensitiveCheckBox->isChecked());
    replaceUtil.setRegexFlag        (regularExprCheckBox->isChecked());
    replaceUtil.setWholeWordFlag    (wholeWordCheckBox->isChecked());
    replaceUtil.setSearchString     (findEditField   ->getTextData()->getAsString());
    replaceUtil.setReplaceString    (replaceEditField->getTextData()->getAsString());
    findEditField   ->getTextData()->setModifiedFlag(false);
    replaceEditField->getTextData()->setModifiedFlag(false);
    requestClose();

    historyIndex = -1;

    int textPosition = e->getCursorTextPosition();
    if (e->hasPrimarySelection()) {
        replaceUtil.setTextPosition(e->getBeginSelectionPos());
        if (replaceUtil.doesMatch() && replaceUtil.getMatchEndPos() == e->getEndSelectionPos()) {
            textPosition = e->getBeginSelectionPos();
        }
    }
    replaceUtil.setTextPosition(textPosition);

    replaceUtil.setSearchForwardFlag(false);
    
    internalFindNext(false);
}


void ReplacePanel::replaceAgainForward()
{
    if (this->isVisible() && findEditField->getTextData()->getLength() == 0) {
        return;
    }

    if (this->isVisible()) {
        replaceUtil.setIgnoreCaseFlag   (!caseSensitiveCheckBox->isChecked());
        replaceUtil.setRegexFlag        (regularExprCheckBox->isChecked());
        replaceUtil.setWholeWordFlag    (wholeWordCheckBox->isChecked());
        replaceUtil.setSearchString     (findEditField   ->getTextData()->getAsString());
        replaceUtil.setReplaceString    (replaceEditField->getTextData()->getAsString());
        findEditField   ->getTextData()->setModifiedFlag(false);
        replaceEditField->getTextData()->setModifiedFlag(false);
        requestClose();
    }
    else
    {
        SearchHistory* history = SearchHistory::getInstance();
        if (history->getEntryCount() >= 1) {
            SearchHistory::Entry entry = history->getEntry(history->getEntryCount() - 1);

            if (!entry.hasReplaceString())
            {
                return;
            }

            replaceUtil.setIgnoreCaseFlag   (entry.getIgnoreCaseFlag());
            replaceUtil.setRegexFlag        (entry.getRegexFlag());
            replaceUtil.setWholeWordFlag    (entry.getWholeWordFlag());
            replaceUtil.setSearchString     (entry.getFindString());
            replaceUtil.setReplaceString    (entry.getReplaceString());
        }
    }

    historyIndex = -1;

    int   textPosition   = e->getCursorTextPosition();
    if (e->hasPrimarySelection())
    {
        long spos = e->getBeginSelectionPos();
        long epos = e->getEndSelectionPos();

        ValidPtr<TextData> textData = e->getTextData();

        replaceUtil.setTextPosition(spos);
        
        if (replaceUtil.doesMatch() && replaceUtil.getMatchEndPos() == epos)
        {
            String substitutedString = replaceUtil.getSubstitutedString();
            
            TextData::TextMark textMark = e->createNewMarkFromCursor();
            textMark.moveToPos(spos);
            textData->insertAtMark(textMark, substitutedString);

            textMark.moveToPos(spos + substitutedString.getLength());
            textData->removeAtMark(textMark, replaceUtil.getMatchLength());

            epos += substitutedString.getLength() - replaceUtil.getMatchLength();

            e->moveCursorToTextPosition(epos);
            e->assureCursorVisible();
        }
//        textPosition = epos;
    }

//    replaceUtil.setSearchForwardFlag(true);
//    replaceUtil.setTextPosition(textPosition);

//    internalFindNext(false);
}


void ReplacePanel::replaceAgainBackward()
{
    if (this->isVisible() && findEditField->getTextData()->getLength() == 0) {
        return;
    }

    if (this->isVisible()) {
        replaceUtil.setIgnoreCaseFlag   (!caseSensitiveCheckBox->isChecked());
        replaceUtil.setRegexFlag        (regularExprCheckBox->isChecked());
        replaceUtil.setWholeWordFlag    (wholeWordCheckBox->isChecked());
        replaceUtil.setSearchString     (findEditField   ->getTextData()->getAsString());
        replaceUtil.setReplaceString    (replaceEditField->getTextData()->getAsString());
        findEditField   ->getTextData()->setModifiedFlag(false);
        replaceEditField->getTextData()->setModifiedFlag(false);
        requestClose();
    }
    else
    {
        SearchHistory* history = SearchHistory::getInstance();
        if (history->getEntryCount() >= 1) {
            SearchHistory::Entry entry = history->getEntry(history->getEntryCount() - 1);

            if (!entry.hasReplaceString())
            {
                return;
            }
            replaceUtil.setIgnoreCaseFlag   (entry.getIgnoreCaseFlag());
            replaceUtil.setRegexFlag        (entry.getRegexFlag());
            replaceUtil.setWholeWordFlag    (entry.getWholeWordFlag());
            replaceUtil.setSearchString     (entry.getFindString());
            replaceUtil.setReplaceString    (entry.getReplaceString());
        }
    }

    historyIndex = -1;

    int   textPosition   = e->getCursorTextPosition();

    if (e->hasPrimarySelection())
    {
        long spos = e->getBeginSelectionPos();
        long epos = e->getEndSelectionPos();

        replaceUtil.setTextPosition(spos);
        
        if (replaceUtil.doesMatch() && replaceUtil.getMatchEndPos() == epos)
        {
            String substitutedString = replaceUtil.getSubstitutedString();
            ValidPtr<TextData> textData = e->getTextData();
            
            TextData::TextMark textMark = e->createNewMarkFromCursor();
            textMark.moveToPos(spos);
            textData->insertAtMark(textMark, substitutedString);

            textMark.moveToPos(spos + substitutedString.getLength());
            textData->removeAtMark(textMark, replaceUtil.getMatchLength());

            epos += substitutedString.getLength() - replaceUtil.getMatchLength();
            
            e->moveCursorToTextPosition(spos);
            e->assureCursorVisible();
        }
//        textPosition = spos;
    }

//    replaceUtil.setSearchForwardFlag(false);
//    replaceUtil.setTextPosition(textPosition);
//    internalFindNext(false);
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
                ValidPtr<TextData> textData = findEditField->getTextData();
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
                    ValidPtr<TextData> textData = findEditField->getTextData();
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
    }
}


