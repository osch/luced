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

#include "FindPanel.h"
#include "GuiLayoutColumn.h"
#include "GuiLayoutRow.h"
#include "GlobalConfig.h"
#include "GuiLayoutSpacer.h"
#include "LabelWidget.h"
#include "RegexException.h"
#include "SearchHistory.h"

using namespace LucED;

FindPanel::FindPanel(GuiWidget* parent, TextEditorWidget* editorWidget, Callback1<MessageBoxParameter> messageBoxInvoker)
    : DialogPanel(parent),
      editorWidget(editorWidget),
      messageBoxInvoker(messageBoxInvoker),
      defaultDirection(Direction::DOWN)
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
    wholeWordCheckBox   = CheckBox::create(this, "W]hole Word");
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
}

void FindPanel::treatFocusIn()
{
    setFocus(editField);
    DialogPanel::treatFocusIn();
}



void FindPanel::internalFindNext(bool forward, int textPosition, bool isWrapping)
{
    int editFieldLength = editField->getTextData()->getLength();
    std::string editFieldContent((char*)editField->getTextData()->getAmount(0, editFieldLength), editFieldLength);

    if (editFieldContent.length() <= 0) {
        return;
    }

    try
    {
        regex = Regex(editFieldContent, Regex::MULTILINE | Regex::ANCHORED);
        SearchHistory::getInstance()->append(editFieldContent);
        int   textLength     = editorWidget->getTextData()->getLength();
        byte* textStart      = editorWidget->getTextData()->getAmount(0, textLength);
        MemArray<int> ovector(regex.getOvecSize());
        bool found = false;
        if (forward) 
        {
            while (!found && textPosition <= textLength) {
                if (regex.findMatch((char*)textStart, textLength, textPosition, Regex::MatchOptions(), ovector)) {
                    found = true;
                }
                else {
                    ++textPosition;
                }
            }
        } else {
            --textPosition;
            while (!found && textPosition >= 0) {
                if (regex.findMatch((char*)textStart, textLength, textPosition, Regex::MatchOptions(), ovector)) {
                    found = true;
                }
                else {
                    --textPosition;
                }
            }
        }
        if (found)
        {
            TextData::TextMark m = editorWidget->createNewMarkFromCursor();
            m.moveToPos(ovector[0]);
            editorWidget->moveCursorToTextMarkAndAdjustVisibility(m);
            m.moveToPos(ovector[1]);
            editorWidget->moveCursorToTextMarkAndAdjustVisibility(m);
            editorWidget->rememberCursorPixX();
            editorWidget->requestSelectionOwnership();
            editorWidget->getBackliteBuffer()->activateSelection(ovector[0]);
            editorWidget->getBackliteBuffer()->extendSelectionTo(ovector[1]);
        } else if (!isWrapping) {
            if (forward) {
                messageBoxInvoker.call(MessageBoxParameter()
                                       .setTitle("Not found")
                                       .setMessage("Continue search from beginning of file?")
                                       .setDefaultButton("C]ontinue", Callback0(this, &FindPanel::handleContinueAtBeginningButton))
                                       .setCancelButton("Ca]ncel"));
            } else {
                messageBoxInvoker.call(MessageBoxParameter()
                                       .setTitle("Not found")
                                       .setMessage("Continue search from end of file?")
                                       .setDefaultButton("C]ontinue", Callback0(this, &FindPanel::handleContinueAtEndButton))
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
        messageBoxInvoker.call(MessageBoxParameter()
                               .setTitle("Error")
                               .setMessage("Error within regular expression: " + ex.getMessage()));
    }
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
        int   textPosition   = editorWidget->getCursorTextPosition();
        if (button == findNextButton) 
        {
            findPrevButton->setAsDefaultButton(false);
            findNextButton->setAsDefaultButton(true);
            
            if (editorWidget->getBackliteBuffer()->hasActiveSelection()) {
                textPosition = editorWidget->getBackliteBuffer()->getEndSelectionPos();
            }
        } else {
            findPrevButton->setAsDefaultButton(true);
            findNextButton->setAsDefaultButton(false);

            if (editorWidget->getBackliteBuffer()->hasActiveSelection()) {
                textPosition = editorWidget->getBackliteBuffer()->getBeginSelectionPos();
            }
        }
        internalFindNext(button == findNextButton, textPosition, false);
    }
}

void FindPanel::handleContinueAtBeginningButton()
{
    internalFindNext(true, 0, true);
}


void FindPanel::handleContinueAtEndButton()
{
    internalFindNext(false, editorWidget->getTextData()->getLength(), true);
}


void FindPanel::findAgainForward()
{
    if (this->isVisible()) {
        requestCloseFor(this);
    } else {
        WeakPtr<TextData> textData = editField->getTextData();
        textData->clear();
        std::string lastFindString = SearchHistory::getInstance()->getLast().getFindString();
        textData->insertAtMark(textData->createNewMark(), (const byte*) lastFindString.c_str(), lastFindString.length());
    }
    int   textPosition   = editorWidget->getCursorTextPosition();
    if (editorWidget->getBackliteBuffer()->hasActiveSelection()) {
        textPosition = editorWidget->getBackliteBuffer()->getEndSelectionPos();
    }
    internalFindNext(true, textPosition, false);
}


void FindPanel::findAgainBackward()
{
    if (this->isVisible()) {
        requestCloseFor(this);
    }
    int   textPosition   = editorWidget->getCursorTextPosition();
    if (editorWidget->getBackliteBuffer()->hasActiveSelection()) {
        textPosition = editorWidget->getBackliteBuffer()->getBeginSelectionPos();
    }
    internalFindNext(false, textPosition, false);
}

    
