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

static const char* const REPLACE_IN_SELECTION_BUTTON_TEXT = "All in S]election";

ReplacePanel::ReplacePanel(GuiWidget* parent, TextEditorWidget* editorWidget, FindPanel* findPanel, 
                           Callback1<MessageBoxParameter> messageBoxInvoker,
                           Callback1<DialogPanel*>        panelInvoker)
    : DialogPanel(parent),
      e(editorWidget),
      messageBoxInvoker(messageBoxInvoker),
      panelInvoker(panelInvoker),
      defaultDirection(Direction::DOWN),
      historyIndex(-1),
      selectSearchRegexFlag(false)
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
    replaceSelectionButton  = Button::create     (this, REPLACE_IN_SELECTION_BUTTON_TEXT);
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

    Callback1<Button*> buttonCallback(this, &ReplacePanel::handleButtonPressed);
    findNextButton        ->setButtonPressedCallback(buttonCallback);
    findPrevButton        ->setButtonPressedCallback(buttonCallback);
    replaceNextButton     ->setButtonPressedCallback(buttonCallback);
    replacePrevButton     ->setButtonPressedCallback(buttonCallback);
    replaceSelectionButton->setButtonPressedCallback(buttonCallback);
    replaceWindowButton   ->setButtonPressedCallback(buttonCallback);
    cancelButton          ->setButtonPressedCallback(buttonCallback);
    
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
    
    findEditField   ->getTextData()->registerModifiedFlagListener(Callback1<bool>(this, &ReplacePanel::handleModifiedEditField));
    replaceEditField->getTextData()->registerModifiedFlagListener(Callback1<bool>(this, &ReplacePanel::handleModifiedEditField));

    replaceUtil.setTextData(e->getTextData());
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


void ReplacePanel::executeFind(bool isWrapping, const Callback0& handleContinueSearchButton)
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

        replaceUtil.findNext();

        if (replaceUtil.wasFound())
        {
            TextData::TextMark m = e->createNewMarkFromCursor();
            m.moveToPos(replaceUtil.getMatchBeginPos());
            e->moveCursorToTextMarkAndAdjustVisibility(m);
            m.moveToPos(replaceUtil.getMatchEndPos());
            e->moveCursorToTextMarkAndAdjustVisibility(m);
            e->rememberCursorPixX();
            e->requestSelectionOwnership();
            e->getBackliteBuffer()->activateSelection(replaceUtil.getMatchBeginPos());
            e->getBackliteBuffer()->extendSelectionTo(replaceUtil.getMatchEndPos());

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
                    messageBoxInvoker.call(MessageBoxParameter()
                                           .setTitle("Not found")
                                           .setMessage("Continue search from beginning of file?")
                                           .setDefaultButton("C]ontinue", handleContinueSearchButton)
                                           .setCancelButton("Ca]ncel"));
                } else {
                    messageBoxInvoker.call(MessageBoxParameter()
                                           .setTitle("Not found")
                                           .setMessage("Continue search from end of file?")
                                           .setDefaultButton("C]ontinue", handleContinueSearchButton)
                                           .setCancelButton("Ca]ncel"));
                }
            } else {
                    messageBoxInvoker.call(MessageBoxParameter()
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
        panelInvoker.call(this);
        messageBoxInvoker.call(MessageBoxParameter()
                               .setTitle("Regex Error")
                               .setMessage(String() << "Error within regular expression: " << ex.getMessage()));
    }
    catch (LuaException& ex)
    {
        panelInvoker.call(this);
        messageBoxInvoker.call(MessageBoxParameter()
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

    executeFind(isWrapping, forward ? Callback0(this, &ReplacePanel::handleContinueAtBeginningButton)
                                    : Callback0(this, &ReplacePanel::handleContinueAtEndButton));
}

void ReplacePanel::handleButtonPressed(Button* button)
{
    try
    {
        if (button == cancelButton)
        {
            requestCloseFor(this);
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

            if (button == findNextButton) 
            {
                if (e->getBackliteBuffer()->hasActiveSelection()) {
                    replaceUtil.setTextPosition(e->getBackliteBuffer()->getEndSelectionPos());
                }
            }
            else if (button == findPrevButton)
            {
                if (e->getBackliteBuffer()->hasActiveSelection()) {
                    replaceUtil.setTextPosition(e->getBackliteBuffer()->getBeginSelectionPos());
                }
            }
            else
            {
                ASSERT(button == replaceNextButton || button == replacePrevButton);

                long spos, epos;

                if (e->getBackliteBuffer()->hasActiveSelection()) {
                    spos = e->getBackliteBuffer()->getBeginSelectionPos();
                    epos = e->getBackliteBuffer()->getEndSelectionPos();
                    replaceUtil.setTextPosition(spos);
                } else {
                    spos = replaceUtil.getTextPosition();
                    epos = replaceUtil.getTextPosition();
                }

                if (replaceUtil.doesMatch()) {
                    ASSERT(replaceUtil.getMatchBeginPos() == spos);
                    if (replaceUtil.getMatchEndPos() == epos) {
                        String substitutedString = replaceUtil.getSubstitutedString();
                        e->moveCursorToTextPosition(spos);
                        e->removeAtCursor(epos - spos);
                        e->insertAtCursor(substitutedString);
                    }
                    if (button == replaceNextButton) {
                        replaceUtil.setTextPosition(epos);
                    } else {
                        replaceUtil.setTextPosition(spos);
                    }
                } else {
                    if (e->getBackliteBuffer()->hasActiveSelection()) {
                        if (button == replaceNextButton) {
                            replaceUtil.setTextPosition(e->getBackliteBuffer()->getEndSelectionPos());
                        } else {
                            replaceUtil.setTextPosition(e->getBackliteBuffer()->getBeginSelectionPos());
                        }
                    }
                }
            }

            internalFindNext(false);
        }
        else if ((button == replaceSelectionButton && e->getBackliteBuffer()->hasActiveSelection())
               || button == replaceWindowButton)
        {
            if (button == replaceSelectionButton && rememberedSelection.getLength() > 0) 
            {
                // replaceSelectionButton means "Restore Selection"

               TextData::TextMark mark = e->getBackliteBuffer()->createMarkToBeginOfSelection();
               long selLength = e->getBackliteBuffer()->getSelectionLength();
                
               e->getTextData()->insertAtMark(mark, rememberedSelection);
               mark.moveForwardToPos(mark.getPos() + rememberedSelection.getLength());
               e->getTextData()->removeAtMark(mark, selLength);
               e->adjustCursorVisibility();

               rememberedSelection = String();
               replaceSelectionButton->setButtonText(REPLACE_IN_SELECTION_BUTTON_TEXT);
            }
            else
            {
                // replaceSelectionButton means "Normal Replace"
                
                long spos;
                long epos;

                if (button == replaceSelectionButton) {
                    spos = e->getBackliteBuffer()->getBeginSelectionPos();
                    epos = e->getBackliteBuffer()->getEndSelectionPos();
                } else {
                    spos = 0;
                    epos = e->getTextData()->getLength();
                }
                String rememberedSelection;
                if (button == replaceSelectionButton) {
                    rememberedSelection = e->getTextData()->getSubstring(spos, epos - spos);
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

                if (wasAythingReplaced && button == replaceSelectionButton && rememberedSelection.getLength() > 0) {
                    this->rememberedSelection = rememberedSelection;
                    button->setButtonText("Restore S]election");
                }
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
        panelInvoker.call(this);
        messageBoxInvoker.call(MessageBoxParameter()
                               .setTitle("Replace Error")
                               .setMessage(String() << "Error within replace string: " << ex.getMessage()));
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

    executeFind(true, Callback0(this, &ReplacePanel::handleContinueSelectionFindAtBeginningButton));
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

    executeFind(true, Callback0(this, &ReplacePanel::handleContinueSelectionFindAtEndButton));
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
    requestCloseFor(this);

    historyIndex = -1;

    int   textPosition   = e->getCursorTextPosition();
    if (e->getBackliteBuffer()->hasActiveSelection()) {
        textPosition = e->getBackliteBuffer()->getEndSelectionPos();
    }
    
    replaceUtil.setSearchForwardFlag(true);
    replaceUtil.setTextPosition(textPosition);
    
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
    requestCloseFor(this);

    historyIndex = -1;

    int   textPosition   = e->getCursorTextPosition();
    if (e->getBackliteBuffer()->hasActiveSelection()) {
        textPosition = e->getBackliteBuffer()->getBeginSelectionPos();
    }

    replaceUtil.setSearchForwardFlag(false);
    replaceUtil.setTextPosition(textPosition);
    
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
        requestCloseFor(this);
    }
    else
    {
        SearchHistory* history = SearchHistory::getInstance();
        if (history->getEntryCount() >= 1) {
            SearchHistory::Entry entry = history->getEntry(history->getEntryCount() - 1);

            replaceUtil.setIgnoreCaseFlag   (entry.getIgnoreCaseFlag());
            replaceUtil.setRegexFlag        (entry.getRegexFlag());
            replaceUtil.setWholeWordFlag    (entry.getWholeWordFlag());
            replaceUtil.setSearchString     (entry.getFindString());
        }
    }

    historyIndex = -1;

    int   textPosition   = e->getCursorTextPosition();
    if (e->getBackliteBuffer()->hasActiveSelection())
    {
        long spos = e->getBackliteBuffer()->getBeginSelectionPos();
        long epos = e->getBackliteBuffer()->getEndSelectionPos();

        TextData* textData = e->getTextData();

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
        }
        textPosition = epos;
    }

    replaceUtil.setSearchForwardFlag(true);
    replaceUtil.setTextPosition(textPosition);

    internalFindNext(false);
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
        requestCloseFor(this);
    }
    else
    {
        SearchHistory* history = SearchHistory::getInstance();
        if (history->getEntryCount() >= 1) {
            SearchHistory::Entry entry = history->getEntry(history->getEntryCount() - 1);

            replaceUtil.setIgnoreCaseFlag   (entry.getIgnoreCaseFlag());
            replaceUtil.setRegexFlag        (entry.getRegexFlag());
            replaceUtil.setWholeWordFlag    (entry.getWholeWordFlag());
            replaceUtil.setSearchString     (entry.getFindString());
        }
    }

    historyIndex = -1;

    int   textPosition   = e->getCursorTextPosition();

    if (e->getBackliteBuffer()->hasActiveSelection())
    {
        long spos = e->getBackliteBuffer()->getBeginSelectionPos();
        long epos = e->getBackliteBuffer()->getEndSelectionPos();

        replaceUtil.setTextPosition(spos);
        
        if (replaceUtil.doesMatch() && replaceUtil.getMatchEndPos() == epos)
        {
            String substitutedString = replaceUtil.getSubstitutedString();
            TextData* textData = e->getTextData();
            
            TextData::TextMark textMark = e->createNewMarkFromCursor();
            textMark.moveToPos(spos);
            textData->insertAtMark(textMark, substitutedString);

            textMark.moveToPos(spos + substitutedString.getLength());
            textData->removeAtMark(textMark, replaceUtil.getMatchLength());

            epos += substitutedString.getLength() - replaceUtil.getMatchLength();
        }
        textPosition = spos;
    }

    replaceUtil.setSearchForwardFlag(false);
    replaceUtil.setTextPosition(textPosition);

    internalFindNext(false);
}


GuiElement::ProcessingResult ReplacePanel::processKeyboardEvent(const XEvent *event)
{
    bool processed = false;
    KeyMapping::Id pressedKey(event->xkey.state, XLookupKeysym((XKeyEvent*)&event->xkey, 0));

    if (KeyMapping::Id(0, XK_Up)    == pressedKey
     || KeyMapping::Id(0, XK_KP_Up) == pressedKey)
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
                TextData* textData = findEditField->getTextData();
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
    else if (KeyMapping::Id(0, XK_Down)    == pressedKey
          || KeyMapping::Id(0, XK_KP_Down) == pressedKey)
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
                    TextData* textData = findEditField->getTextData();
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
    else if (KeyMapping::Id(ControlMask, XK_g) == pressedKey)
    {
        findAgainForward();
        processed = true;
    }
    else if (KeyMapping::Id(ShiftMask|ControlMask, XK_g) == pressedKey)
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

#if 0
    if (e->getBackliteBuffer()->hasActiveSelection())
    {
        e->requestSelectionOwnership();
        e->getBackliteBuffer()->makeSecondarySelectionToPrimarySelection();
        e->getBackliteBuffer()->turnOffSelectionPersistence();
    }
#endif
}


void ReplacePanel::show()
{
    findPrevButton->setAsDefaultButton(defaultDirection != Direction::DOWN);
    findNextButton->setAsDefaultButton(defaultDirection == Direction::DOWN);
    replacePrevButton->setAsDefaultButton(false);
    replaceNextButton->setAsDefaultButton(false);

    rememberedSelection = String();
    replaceSelectionButton->setButtonText(REPLACE_IN_SELECTION_BUTTON_TEXT);

    if (e->getBackliteBuffer()->hasActiveSelection())
    {
        TextData::TextMark  beginMark = e->getBackliteBuffer()->createMarkToBeginOfSelection();
        TextData::TextMark  endMark = e->getBackliteBuffer()->createMarkToEndOfSelection();
        
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
        replaceSelectionButton->setButtonText(REPLACE_IN_SELECTION_BUTTON_TEXT);
        historyIndex = -1;
    }
}

