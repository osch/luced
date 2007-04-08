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

#include "FindPanel.h"
#include "GuiLayoutColumn.h"
#include "GuiLayoutRow.h"
#include "GlobalConfig.h"
#include "GuiLayoutSpacer.h"
#include "LabelWidget.h"
#include "RegexException.h"
#include "SearchHistory.h"
#include "FindUtil.h"

using namespace LucED;

using std::string;

FindPanel::FindPanel(GuiWidget* parent, TextEditorWidget* editorWidget, Callback1<MessageBoxParameter> messageBoxInvoker,
                                                                        Callback1<DialogPanel*>        panelInvoker)
    : DialogPanel(parent),
      e(editorWidget),
      messageBoxInvoker(messageBoxInvoker),
      panelInvoker(panelInvoker),
      defaultDirection(Direction::DOWN),
      historyIndex(-1)  
{
    GuiLayoutColumn::Ptr  c0 = GuiLayoutColumn::create();
    GuiLayoutColumn::Ptr  c1 = GuiLayoutColumn::create();
    GuiLayoutColumn::Ptr  c2 = GuiLayoutColumn::create();
    GuiLayoutRow::Ptr     r0 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r1 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r2 = GuiLayoutRow::create();
    GuiLayoutRow::Ptr     r3 = GuiLayoutRow::create();
    GuiLayoutSpacerFrame::Ptr frame0 = GuiLayoutSpacerFrame::create(c0, 0);
    setRootElement(frame0);

    editField    = SingleLineEditField::create(this, 
                                               GlobalConfig::getInstance()->getDefaultLanguageMode());
    editField->setDesiredWidthInChars(5, 10, INT_MAX);
    findNextButton   = Button::create(this, "Find N]ext");
    findPrevButton   = Button::create(this, "Find P]revious");
    goBackButton   = Button::create(this, "Go B]ack");
    cancelButton = Button::create(this, "C]lose");
    LabelWidget::Ptr label0 = LabelWidget::create(this, "String to Find:");
    ignoreCaseCheckBox  = CheckBox::create(this, "I]gnore Case");
    wholeWordCheckBox   = CheckBox::create(this, "Wh]ole Word");
    regularExprCheckBox = CheckBox::create(this, "R]egular Expression");
    
    label0   ->setLayoutHeight(findPrevButton->getStandardHeight(), VerticalAdjustment::BOTTOM);
    editField->setLayoutHeight(findPrevButton->getStandardHeight(), VerticalAdjustment::CENTER);
 
    Callback1<Button*> buttonCallback(this, &FindPanel::handleButtonPressed);
    findNextButton->setButtonPressedCallback(buttonCallback);
    findPrevButton->setButtonPressedCallback(buttonCallback);
    goBackButton->setButtonPressedCallback(buttonCallback);
    cancelButton->setButtonPressedCallback(buttonCallback);
    
    editField->setNextFocusWidget(findPrevButton);
    findPrevButton->setNextFocusWidget(findNextButton);
    findNextButton->setNextFocusWidget(ignoreCaseCheckBox);

    ignoreCaseCheckBox->setNextFocusWidget(wholeWordCheckBox);
    wholeWordCheckBox->setNextFocusWidget(regularExprCheckBox);
    regularExprCheckBox->setNextFocusWidget(goBackButton);

    goBackButton->setNextFocusWidget(cancelButton);
    cancelButton->setNextFocusWidget(editField);

    setFocus(editField);

    c0->addElement(r0);
    r0->addElement(c1);
    c1->addElement(label0);
    c1->addElement(editField);
    c1->addElement(r3);
    r3->addElement(ignoreCaseCheckBox);
    r3->addElement(wholeWordCheckBox);
    r3->addElement(regularExprCheckBox);
    r0->addElement(c2);
    c2->addElement(findPrevButton);
    c2->addElement(findNextButton);
    c2->addElement(r1);
    r1->addElement(goBackButton);
    r1->addElement(cancelButton);
    
    ignoreCaseCheckBox->setChecked(true);
    
    findNextButton->setAsDefaultButton();
    label0->show();
    editField->show();
    findPrevButton->show();
    findNextButton->show();
    goBackButton->show();
    cancelButton->show();
    ignoreCaseCheckBox->show();
    regularExprCheckBox->show();
    wholeWordCheckBox->show();
    
    editField->getTextData()->registerModifiedFlagListener(Callback1<bool>(this, &FindPanel::handleModifiedEditField));
}

void FindPanel::treatFocusIn()
{
    setFocus(editField);
    DialogPanel::treatFocusIn();
}


void FindPanel::executeFind(bool isWrapping, FindUtil& f, const Callback0& handleContinueSearchButton)
{
    try
    {
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
        } else if (!isWrapping) {
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
    catch (RegexException& ex)
    {
        int position = ex.getPosition();
        if (position >= 0) {
            editField->setCursorPosition(position);
        }
        panelInvoker.call(this);
        messageBoxInvoker.call(MessageBoxParameter()
                               .setTitle("Regex Error")
                               .setMessage("Error within regular expression: " + ex.getMessage()));
    }
    catch (LuaException& ex)
    {
        panelInvoker.call(this);
        messageBoxInvoker.call(MessageBoxParameter()
                               .setTitle("Lua Error")
                               .setMessage(ex.getMessage()));
    }
}

void FindPanel::findSelectionForward()
{
    requestCloseFor(this);
    if (!e->areCursorChangesDisabled())
    {
        if (e->hasSelectionOwnership()) {
            long selBegin = e->getBackliteBuffer()->getBeginSelectionPos();
            long selLength = e->getBackliteBuffer()->getEndSelectionPos() - selBegin;
            if (selLength > 0) {
                selectionSearchString = string((char*)e->getTextData()->getAmount(selBegin, selLength), selLength);

                SearchHistory::Entry newEntry;
                                     newEntry.setFindString    (selectionSearchString);
                                     newEntry.setWholeWordFlag (false);
                                     newEntry.setRegexFlag     (false);
                                     newEntry.setIgnoreCaseFlag(true);
                SearchHistory::getInstance()->append(newEntry);

                editField->getTextData()->setModifiedFlag(false);
                FindUtil f;
                         f.setSearchForwardFlag(true);
                         f.setIgnoreCaseFlag   (true);
                         f.setRegexFlag        (false);
                         f.setWholeWordFlag    (false);
                         f.setSearchString     (selectionSearchString);
                         f.setTextPosition     (selBegin + selLength);
                         f.setTextData         (e->getTextData());
                executeFind(false, f, Callback0(this, &FindPanel::handleContinueSelectionFindAtBeginningButton));
            }
        }
    }
    e->assureCursorVisible();
}


void FindPanel::findSelectionBackward()
{
    requestCloseFor(this);
    if (!e->areCursorChangesDisabled())
    {
        if (e->hasSelectionOwnership()) {
            long selBegin = e->getBackliteBuffer()->getBeginSelectionPos();
            long selLength = e->getBackliteBuffer()->getEndSelectionPos() - selBegin;
            if (selLength > 0) {
                selectionSearchString = string((char*)e->getTextData()->getAmount(selBegin, selLength), selLength);
                SearchHistory::Entry newEntry;
                                     newEntry.setFindString    (selectionSearchString);
                                     newEntry.setWholeWordFlag (false);
                                     newEntry.setRegexFlag     (false);
                                     newEntry.setIgnoreCaseFlag(true);
                SearchHistory::getInstance()->append(newEntry);
                editField->getTextData()->setModifiedFlag(false);
                FindUtil f;
                         f.setSearchForwardFlag(false);
                         f.setIgnoreCaseFlag   (true);
                         f.setRegexFlag        (false);
                         f.setWholeWordFlag    (false);
                         f.setSearchString     (selectionSearchString);
                         f.setTextPosition     (selBegin);
                         f.setTextData         (e->getTextData());
                executeFind(false, f, Callback0(this, &FindPanel::handleContinueSelectionFindAtEndButton));
            }
        }
    }
    e->assureCursorVisible();
}

void FindPanel::internalFindNext(bool forward, int textPosition, bool isWrapping)
{
    long editFieldLength = editField->getTextData()->getLength();
    string editFieldContent((char*)editField->getTextData()->getAmount(0, editFieldLength), editFieldLength);

    if (editFieldContent.length() <= 0) {
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

    if (historyIndex == -1) {
        SearchHistory::Entry newEntry;
                             newEntry.setFindString    (editFieldContent);
                             newEntry.setWholeWordFlag (wholeWordCheckBox->isChecked());
                             newEntry.setRegexFlag     (regularExprCheckBox->isChecked());
                             newEntry.setIgnoreCaseFlag(ignoreCaseCheckBox->isChecked());
        SearchHistory::getInstance()->append(newEntry);
    } else {
        SearchHistory::Entry& entry = SearchHistory::getInstance()->getEntry(historyIndex);
                              entry.setWholeWordFlag (wholeWordCheckBox->isChecked());
                              entry.setRegexFlag     (regularExprCheckBox->isChecked());
                              entry.setIgnoreCaseFlag(ignoreCaseCheckBox->isChecked());
    }
    editField->getTextData()->setModifiedFlag(false);

    executeFind(isWrapping, f, forward ? Callback0(this, &FindPanel::handleContinueAtBeginningButton)
                                       : Callback0(this, &FindPanel::handleContinueAtEndButton));
}

void FindPanel::handleButtonPressed(Button* button)
{
    if (button == cancelButton)
    {
        requestCloseFor(this);
    }
    else if (button == goBackButton)
    {
        messageBoxInvoker.call(MessageBoxParameter().setTitle("xxx title xxx").setMessage("xxx message xxx"));
    }
    else if (button == findNextButton || button == findPrevButton)
    {
        int   textPosition   = e->getCursorTextPosition();
        if (button == findNextButton) 
        {
            findPrevButton->setAsDefaultButton(false);
            findNextButton->setAsDefaultButton(true);
            
            if (e->getBackliteBuffer()->hasActiveSelection()) {
                textPosition = e->getBackliteBuffer()->getEndSelectionPos();
            }
        } else {
            findPrevButton->setAsDefaultButton(true);
            findNextButton->setAsDefaultButton(false);

            if (e->getBackliteBuffer()->hasActiveSelection()) {
                textPosition = e->getBackliteBuffer()->getBeginSelectionPos();
            }
        }
        internalFindNext(button == findNextButton, textPosition, false);
    }
}

void FindPanel::handleContinueSelectionFindAtBeginningButton()
{
    FindUtil f;
             f.setSearchForwardFlag(true);
             f.setIgnoreCaseFlag   (true);
             f.setRegexFlag        (false);
             f.setWholeWordFlag    (false);
             f.setSearchString     (selectionSearchString);
             f.setTextPosition     (0);
             f.setTextData         (e->getTextData());
    executeFind(true, f, Callback0(this, &FindPanel::handleContinueSelectionFindAtBeginningButton));
}

void FindPanel::handleContinueAtBeginningButton()
{
    internalFindNext(true, 0, true);
}


void FindPanel::handleContinueSelectionFindAtEndButton()
{
    FindUtil f;
             f.setSearchForwardFlag(false);
             f.setIgnoreCaseFlag   (true);
             f.setRegexFlag        (false);
             f.setWholeWordFlag    (false);
             f.setSearchString     (selectionSearchString);
             f.setTextPosition     (e->getTextData()->getLength());
             f.setTextData         (e->getTextData());
    executeFind(false, f, Callback0(this, &FindPanel::handleContinueSelectionFindAtEndButton));
}

void FindPanel::handleContinueAtEndButton()
{
    internalFindNext(false, e->getTextData()->getLength(), true);
}


void FindPanel::findAgainForward()
{
    if (this->isVisible()) {
        requestCloseFor(this);
    } else {
        WeakPtr<TextData> textData = editField->getTextData();
        textData->clear();
        string lastFindString = SearchHistory::getInstance()->getLast().getFindString();
        textData->insertAtMark(textData->createNewMark(), (const byte*) lastFindString.c_str(), lastFindString.length());
        textData->clearHistory();
    }
    int   textPosition   = e->getCursorTextPosition();
    if (e->getBackliteBuffer()->hasActiveSelection()) {
        textPosition = e->getBackliteBuffer()->getEndSelectionPos();
    }
    internalFindNext(true, textPosition, false);
}


void FindPanel::findAgainBackward()
{
    if (this->isVisible()) {
        requestCloseFor(this);
    }
    int   textPosition   = e->getCursorTextPosition();
    if (e->getBackliteBuffer()->hasActiveSelection()) {
        textPosition = e->getBackliteBuffer()->getBeginSelectionPos();
    }
    internalFindNext(false, textPosition, false);
}

    
GuiElement::ProcessingResult FindPanel::processKeyboardEvent(const XEvent *event)
{
    bool processed = false;
    KeyMapping::Id pressedKey(event->xkey.state, XLookupKeysym((XKeyEvent*)&event->xkey, 0));

    if (KeyMapping::Id(0, XK_Up)    == pressedKey
     || KeyMapping::Id(0, XK_KP_Up) == pressedKey)
    {
        SearchHistory* history = SearchHistory::getInstance();

        if (historyIndex < 0) {
            int editFieldLength = editField->getTextData()->getLength();
            string editFieldContent((char*)editField->getTextData()->getAmount(0, editFieldLength), editFieldLength);

            SearchHistory::Entry newEntry;
                                 newEntry.setFindString    (editFieldContent);
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
        WeakPtr<TextData> textData = editField->getTextData();
        string editFieldContent((char*)textData->getAmount(0, textData->getLength()), textData->getLength());
        while (h >= 0) {
            SearchHistory::Entry entry = history->getEntry(h);
            string lastFindString = entry.getFindString();
            if (lastFindString != editFieldContent) {
                textData->clear();
                textData->insertAtMark(textData->createNewMark(), (const byte*) lastFindString.c_str(), lastFindString.length());
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
        TextData* textData     = editField->getTextData();

        if (historyIndex < 0) {
            int editFieldLength = editField->getTextData()->getLength();
            string editFieldContent((char*)editField->getTextData()->getAmount(0, editFieldLength), editFieldLength);

            SearchHistory::Entry newEntry;
                                 newEntry.setFindString    (editFieldContent);
                                 newEntry.setWholeWordFlag (wholeWordCheckBox->isChecked());
                                 newEntry.setRegexFlag     (regularExprCheckBox->isChecked());
                                 newEntry.setIgnoreCaseFlag(ignoreCaseCheckBox->isChecked());

            history->append(newEntry);

            historyIndex = history->getEntryCount() - 1;
        }

        if (historyIndex >= 0) {
            string editFieldContent((char*)textData->getAmount(0, textData->getLength()), textData->getLength());
            int h = historyIndex;
            bool found = false;
            while (h + 1 < history->getEntryCount()) {
                ++h;
                SearchHistory::Entry entry = history->getEntry(h);
                string nextFindString = entry.getFindString();
                if (nextFindString != editFieldContent) {
                    textData->clear();
                    textData->insertAtMark(textData->createNewMark(), (const byte*) nextFindString.c_str(), nextFindString.length());
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
                textData->clear();
            }
        }
        processed = true;
    }
    if (!processed) {
        return DialogPanel::processKeyboardEvent(event);
    } else {
        editField->showCursor();
        return EVENT_PROCESSED;
    }
}


void FindPanel::show()
{
    DialogPanel::show();
    if (editField->getTextData()->getModifiedFlag() == false) {
        editField->getTextData()->clear();
        historyIndex = -1;
    }
}

void FindPanel::handleModifiedEditField(bool modifiedFlag)
{
    if (modifiedFlag == true)
    {
        historyIndex = -1;
    }
}


