/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#include "ReplacePanel.h"
#include "GuiLayoutColumn.h"
#include "GuiLayoutRow.h"
#include "GlobalConfig.h"
#include "GuiLayoutSpacer.h"
#include "LabelWidget.h"
#include "RegexException.h"
#include "SearchHistory.h"
#include "FindUtil.h"

using namespace LucED;



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
    GuiLayoutColumn::Ptr  c2 = GuiLayoutColumn::create();
    GuiLayoutRow::Ptr     r0 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r1 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r2 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r3 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r4 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r5 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r6 = GuiLayoutRow::create();
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

    findNextButton         = Button::create(this, "Find N]ext");
    findPrevButton         = Button::create(this, "Find P]revious");
    replaceNextButton      = Button::create(this, "Replace & Nex]t");
    replacePrevButton      = Button::create(this, "Replace & Pre]v");
    replaceSelectionButton = Button::create(this, "Replace in S]election");
    replaceWindowButton    = Button::create(this, "Replace in W]indow");
    cancelButton = Button::create(this, "C]lose");
    LabelWidget::Ptr label0 = LabelWidget::create(this, "Find:");
    LabelWidget::Ptr label1 = LabelWidget::create(this, "Replace:");
    
    
    ignoreCaseCheckBox  = CheckBox::create(this, "I]gnore Case");
    wholeWordCheckBox   = CheckBox::create(this, "Wh]ole Word");
    regularExprCheckBox = CheckBox::create(this, "R]egular Expression");
    
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
    replaceNextButton     ->setNextFocusWidget(ignoreCaseCheckBox);
    ignoreCaseCheckBox    ->setNextFocusWidget(wholeWordCheckBox);
    wholeWordCheckBox     ->setNextFocusWidget(regularExprCheckBox);
    regularExprCheckBox   ->setNextFocusWidget(replaceSelectionButton);
    replaceSelectionButton->setNextFocusWidget(replaceWindowButton);
    replaceWindowButton   ->setNextFocusWidget(cancelButton);
    cancelButton          ->setNextFocusWidget(findEditField);

    setFocus(findEditField);

    c0->addElement(r0);
    r0->addElement(label0);
    r0->addElement(findEditField);
    r0->addElement(findPrevButton);
    r0->addElement(findNextButton);

    c0->addElement(r5);
    r5->addElement(label1);
    r5->addElement(replaceEditField);
    r5->addElement(replacePrevButton);
    r5->addElement(replaceNextButton);

    c0->addElement(r3);
    r3->addElement(ignoreCaseCheckBox);
    r3->addElement(wholeWordCheckBox);
    r3->addElement(regularExprCheckBox);
    r3->addSpacer();
    r3->addElement(replaceSelectionButton);
    r3->addElement(replaceWindowButton);
    r3->addElement(cancelButton);
    
    ignoreCaseCheckBox->setChecked(true);
    
    findNextButton->setAsDefaultButton();

    Measures buttonMeasures;
             buttonMeasures.maximize(findPrevButton   ->getDesiredMeasures());
             buttonMeasures.maximize(findNextButton   ->getDesiredMeasures());
             buttonMeasures.maximize(replacePrevButton->getDesiredMeasures());
             buttonMeasures.maximize(replaceNextButton->getDesiredMeasures());

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
    ignoreCaseCheckBox    ->show();
    regularExprCheckBox   ->show();
    wholeWordCheckBox     ->show();
    
    findEditField   ->getTextData()->registerModifiedFlagListener(Callback1<bool>(this, &ReplacePanel::handleModifiedEditField));
    replaceEditField->getTextData()->registerModifiedFlagListener(Callback1<bool>(this, &ReplacePanel::handleModifiedEditField));
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


void ReplacePanel::executeFind(bool isWrapping, FindUtil& f, const Callback0& handleContinueSearchButton)
{
    try
    {
        {
            if (e->hasSelectionOwnership()) {
                e->getBackliteBuffer()->deactivateSelection();
            }
            TextData::TextMark m = e->createNewMarkFromCursor();
            m.moveToPos(f.getTextPosition());
            e->moveCursorToTextMarkAndAdjustVisibility(m);
        }
        GuiRoot::getInstance()->flushDisplay();

        f.findNext();

        if (f.wasFound())
        {
            TextData::TextMark m = e->createNewMarkFromCursor();
            m.moveToPos(f.getMatchBeginPos());
            e->moveCursorToTextMarkAndAdjustVisibility(m);
            m.moveToPos(f.getMatchEndPos());
            e->moveCursorToTextMarkAndAdjustVisibility(m);
            e->rememberCursorPixX();
            e->requestSelectionOwnership();
            e->getBackliteBuffer()->activateSelection(f.getMatchBeginPos());
            e->getBackliteBuffer()->extendSelectionTo(f.getMatchEndPos());

            replacePrevButton->setAsDefaultButton(!f.isSearchingForward());
            replaceNextButton->setAsDefaultButton( f.isSearchingForward());
            findPrevButton   ->setAsDefaultButton(false);
            findNextButton   ->setAsDefaultButton(false);
        }
        else 
        {
            findPrevButton->setAsDefaultButton(!f.isSearchingForward());
            findNextButton->setAsDefaultButton( f.isSearchingForward());
            replacePrevButton->setAsDefaultButton(false);
            replaceNextButton->setAsDefaultButton(false);

            if (!isWrapping) {
                if (f.isSearchingForward()) {
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

void ReplacePanel::internalFindNext(bool forward, int textPosition, bool isWrapping)
{
    String editFieldContent = findEditField->getTextData()->getAsString();

    if (editFieldContent.getLength() <= 0) {
        return;
    }

    FindUtil f;
             f.setSearchForwardFlag(forward);
             f.setIgnoreCaseFlag   (ignoreCaseCheckBox->isChecked());
             f.setRegexFlag        (regularExprCheckBox->isChecked());
             f.setWholeWordFlag    (wholeWordCheckBox->isChecked());
             f.setSearchString     (editFieldContent);
             f.setTextPosition     (textPosition);
             f.setTextData         (e->getTextData());

    SearchHistory::Entry newEntry;
                         newEntry.setFindString    (editFieldContent);
                         newEntry.setReplaceString (replaceEditField->getTextData()->getAsString());
                         newEntry.setWholeWordFlag (wholeWordCheckBox->isChecked());
                         newEntry.setRegexFlag     (regularExprCheckBox->isChecked());
                         newEntry.setIgnoreCaseFlag(ignoreCaseCheckBox->isChecked());
    SearchHistory::getInstance()->append(newEntry);

    findEditField   ->getTextData()->setModifiedFlag(false);
    replaceEditField->getTextData()->setModifiedFlag(false);

    executeFind(isWrapping, f, forward ? Callback0(this, &ReplacePanel::handleContinueAtBeginningButton)
                                       : Callback0(this, &ReplacePanel::handleContinueAtEndButton));
}

void ReplacePanel::handleButtonPressed(Button* button)
{
    if (button == cancelButton)
    {
        requestCloseFor(this);
    }
    else if (button == findNextButton    || button == findPrevButton
          || button == replaceNextButton || button == replacePrevButton)
    {
        int   textPosition   = e->getCursorTextPosition();
        bool  isNextDirectionForward = true;
        
        if (button == findNextButton) 
        {
            isNextDirectionForward = true;
            if (e->getBackliteBuffer()->hasActiveSelection()) {
                textPosition = e->getBackliteBuffer()->getEndSelectionPos();
            }
        }
        else if (button == findPrevButton)
        {
            isNextDirectionForward = false;
            if (e->getBackliteBuffer()->hasActiveSelection()) {
                textPosition = e->getBackliteBuffer()->getBeginSelectionPos();
            }
        }
        else
        {
            ASSERT(button == replaceNextButton || button == replacePrevButton);
            
            long spos, epos;
            
            if (e->getBackliteBuffer()->hasActiveSelection()) {
                spos = e->getBackliteBuffer()->getBeginSelectionPos();
                epos = e->getBackliteBuffer()->getEndSelectionPos();
            } else {
                spos = textPosition;
                epos = textPosition;
            }
            FindUtil f;
                     f.setSearchForwardFlag(button == replaceNextButton);
                     f.setIgnoreCaseFlag   (ignoreCaseCheckBox->isChecked());
                     f.setRegexFlag        (regularExprCheckBox->isChecked());
                     f.setWholeWordFlag    (wholeWordCheckBox->isChecked());
                     f.setSearchString     (findEditField->getTextData()->getAsString());
                     f.setTextPosition     (spos);
                     f.setTextData         (e->getTextData());
            if (f.doesMatch()) {
                ASSERT(f.getMatchBeginPos() == spos);
                if (f.getMatchEndPos() == epos) {
                    e->moveCursorToTextPosition(spos);
                    e->removeAtCursor(epos - spos);
                    e->insertAtCursor(replaceEditField->getTextData());
                }
                if (button == replaceNextButton) {
                    textPosition = spos + replaceEditField->getTextData()->getLength();
                } else {
                    textPosition = spos;
                }
            } else {
                if (e->getBackliteBuffer()->hasActiveSelection()) {
                    if (button == replaceNextButton) {
                        textPosition = e->getBackliteBuffer()->getEndSelectionPos();
                    } else {
                        textPosition = e->getBackliteBuffer()->getBeginSelectionPos();
                    }
                }
            }
            isNextDirectionForward = (button == replaceNextButton);
        }


        findEditField   ->getTextData()->setModifiedFlag(false);
        replaceEditField->getTextData()->setModifiedFlag(false);
        
        historyIndex = -1;

        internalFindNext(isNextDirectionForward, textPosition, false);
    }
}

void ReplacePanel::handleContinueSelectionFindAtBeginningButton()
{
    FindUtil f;
             f.setSearchForwardFlag(true);
             f.setIgnoreCaseFlag   (true);
             f.setRegexFlag        (selectSearchRegexFlag);
             f.setWholeWordFlag    (false);
             f.setSearchString     (selectionSearchString);
             f.setTextPosition     (0);
             f.setTextData         (e->getTextData());
    executeFind(true, f, Callback0(this, &ReplacePanel::handleContinueSelectionFindAtBeginningButton));
}

void ReplacePanel::handleContinueAtBeginningButton()
{
    internalFindNext(true, 0, true);
}


void ReplacePanel::handleContinueSelectionFindAtEndButton()
{
    FindUtil f;
             f.setSearchForwardFlag(false);
             f.setIgnoreCaseFlag   (true);
             f.setRegexFlag        (selectSearchRegexFlag);
             f.setWholeWordFlag    (false);
             f.setSearchString     (selectionSearchString);
             f.setTextPosition     (e->getTextData()->getLength());
             f.setTextData         (e->getTextData());
    executeFind(true, f, Callback0(this, &ReplacePanel::handleContinueSelectionFindAtEndButton));
}

void ReplacePanel::handleContinueAtEndButton()
{
    internalFindNext(false, e->getTextData()->getLength(), true);
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
                                 newEntry.setIgnoreCaseFlag(ignoreCaseCheckBox->isChecked());

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
                textData->clear();
                textData->insertAtMark(textData->createNewMark(), lastFindString);
                textData->clearHistory();
                textData->setModifiedFlag(false);

                textData = replaceEditField->getTextData();
                textData->clear();
                textData->insertAtMark(textData->createNewMark(), lastReplaceString);
                textData->clearHistory();
                textData->setModifiedFlag(false);
                
                historyIndex = h;
                wholeWordCheckBox->setChecked(entry.getWholeWordFlag());
                regularExprCheckBox->setChecked(entry.getRegexFlag());
                ignoreCaseCheckBox->setChecked(entry.getIgnoreCaseFlag());
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
                                 newEntry.setIgnoreCaseFlag(ignoreCaseCheckBox->isChecked());

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
                    textData->clear();
                    textData->insertAtMark(textData->createNewMark(), nextFindString);
                    textData->clearHistory();
                    textData->setModifiedFlag(false);
                
                    textData = replaceEditField->getTextData();
                    textData->clear();
                    textData->insertAtMark(textData->createNewMark(), nextReplaceString);
                    textData->clearHistory();
                    textData->setModifiedFlag(false);
                    
                    historyIndex = h;
                    found = true;
                    wholeWordCheckBox->setChecked(entry.getWholeWordFlag());
                    regularExprCheckBox->setChecked(entry.getRegexFlag());
                    ignoreCaseCheckBox->setChecked(entry.getIgnoreCaseFlag());
                    break;
                }
            }
            if (!found) {
                historyIndex = -1;
                   findEditField->getTextData()->clear();
                replaceEditField->getTextData()->clear();
            }
        }
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
    lastFocusedEditField.invalidate();
    DialogPanel::hide();
}

void ReplacePanel::show()
{
    findPrevButton->setAsDefaultButton(defaultDirection != Direction::DOWN);
    findNextButton->setAsDefaultButton(defaultDirection == Direction::DOWN);
    replacePrevButton->setAsDefaultButton(false);
    replaceNextButton->setAsDefaultButton(false);

    DialogPanel::show();

    if (   findEditField->getTextData()->getModifiedFlag() == false
     && replaceEditField->getTextData()->getModifiedFlag() == false)
    {
        findEditField   ->getTextData()->clear();
        replaceEditField->getTextData()->clear();
        historyIndex = -1;
    }
}

void ReplacePanel::handleModifiedEditField(bool modifiedFlag)
{
    if (modifiedFlag == true)
    {
        historyIndex = -1;
    }
}


