/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch at luced dot de
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

#include "GotoLineDialog.h"
#include "GlobalConfig.h"
#include "GuiLayoutRow.h"
#include "GuiLayoutSpacer.h"
#include "TextData.h"
#include "GlobalConfig.h"
#include "LabelWidget.h"
#include "DialogPanel.h"
#include "Callback.h"

using namespace LucED;

GotoLineDialog::GotoLineDialog(TopWin* referingWindow, TextEditorWidget* editorWidget)
    : PanelDialogWin(referingWindow),
      slotForButtonPressed(this, &GotoLineDialog::handleButtonPressed),
      editorWidget(editorWidget)
{
    gotoButton = Button::create(this, "G]oto Line");
    cancelButton = Button::create(this, "C]ancel");
   
    this->editField = SingleLineEditField::create(this, 
                                                  GlobalConfig::getInstance()->getDefaultLanguageMode());
    editField->setDesiredWidthInChars(5, 10, 15);

    LabelWidget::Ptr label0 = LabelWidget::create(this, "Goto Line Number:");
    GuiLayoutColumn::Ptr column0 = GuiLayoutColumn::create();
    GuiLayoutRow::Ptr row0 = GuiLayoutRow::create();
    GuiLayoutSpacerFrame::Ptr frame0 = GuiLayoutSpacerFrame::create(column0, 10);
    setRootElement(frame0);

    column0->addSpacer();
    column0->addElement(label0);
    column0->addElement(editField);
    column0->addElement(GuiLayoutSpacer::create(0, 0, 0, 10, 0, INT_MAX));
    column0->addElement(row0);
    column0->addSpacer();

    row0->addElement(GuiLayoutSpacer::create(0, 0, 0, 0, INT_MAX, 0));
    row0->addElement(gotoButton);
    row0->addElement(GuiLayoutSpacer::create(3, 0, 10, 0, 10, 0));
    row0->addElement(cancelButton);
    row0->addElement(GuiLayoutSpacer::create(0, 0, 0, 0, INT_MAX, 0));
    
    gotoButton->setButtonPressedCallback(slotForButtonPressed);
    cancelButton->setButtonPressedCallback(slotForButtonPressed);

    label0->show();
    editField->show();
    gotoButton->show();
    cancelButton->show();  
    
    editField->setNextFocusWidget(gotoButton);
    gotoButton->setNextFocusWidget(cancelButton);
    cancelButton->setNextFocusWidget(editField);
    setFocus(editField);
    gotoButton->setAsDefaultButton();
    setTitle("Goto Line Number");

    editField->getTextData()->setInsertFilterCallback(
        Callback2<const byte**,long*>(this, &GotoLineDialog::filterInsert));
}


void GotoLineDialog::handleButtonPressed(Button* button)
{
    if (button == gotoButton)
    {
        if (!editorWidget->areCursorChangesDisabled())
        {
            int textLength = editField->getTextData()->getLength();
            if (textLength > 0)
            {
                byte* ptr = editField->getTextData()->getAmount(0, textLength);
                int newLineNumber = 0;
                for (int i = 0; i < textLength; ++i) {
                    switch (ptr[i]) {
                        case '0': newLineNumber = newLineNumber * 10 + 0; break;
                        case '1': newLineNumber = newLineNumber * 10 + 1; break;
                        case '2': newLineNumber = newLineNumber * 10 + 2; break;
                        case '3': newLineNumber = newLineNumber * 10 + 3; break;
                        case '4': newLineNumber = newLineNumber * 10 + 4; break;
                        case '5': newLineNumber = newLineNumber * 10 + 5; break;
                        case '6': newLineNumber = newLineNumber * 10 + 6; break;
                        case '7': newLineNumber = newLineNumber * 10 + 7; break;
                        case '8': newLineNumber = newLineNumber * 10 + 8; break;
                        case '9': newLineNumber = newLineNumber * 10 + 9; break;
                    }
                }
                newLineNumber -= 1;
                bool wasNegative = false;
                if (newLineNumber < 0) {
                    newLineNumber = 0;
                    wasNegative = true;
                }
                TextData::TextMark m = editorWidget->createNewMarkFromCursor();
                m.moveToLineAndColumn(newLineNumber, m.getColumn());
                editorWidget->moveCursorToTextMarkAndAdjustVisibility(m);
                editorWidget->rememberCursorPixX();
                if (!wasNegative) {
                    m.moveToBeginOfLine();
                    editorWidget->requestSelectionOwnership();
                    editorWidget->getBackliteBuffer()->activateSelection(m.getPos());
                    m.moveToNextLineBegin();
                    editorWidget->getBackliteBuffer()->extendSelectionTo(m.getPos());
                } else {
                    editorWidget->releaseSelectionOwnership();
                }
            }
        }
        this->requestCloseWindow();
        editField->getTextData()->clear();
    }
    else if (button == cancelButton) {
        this->requestCloseWindow();
        editField->getTextData()->clear();
    }
}

void GotoLineDialog::requestCloseWindow()
{
    editField->getTextData()->clear();
    PanelDialogWin::requestCloseWindow();
}

void GotoLineDialog::treatFocusIn()
{
    setFocus(editField);
    PanelDialogWin::treatFocusIn();
}

void GotoLineDialog::filterInsert(const byte** buffer, long* length)
{
    filterBuffer.clear();
    const long len = *length;
    const byte* buf = *buffer;
    for (int i = 0; i < len; ++i) {
        switch(buf[i]) {
            case '0': filterBuffer.append('0'); break;
            case '1': filterBuffer.append('1'); break;
            case '2': filterBuffer.append('2'); break;
            case '3': filterBuffer.append('3'); break;
            case '4': filterBuffer.append('4'); break;
            case '5': filterBuffer.append('5'); break;
            case '6': filterBuffer.append('6'); break;
            case '7': filterBuffer.append('7'); break;
            case '8': filterBuffer.append('8'); break;
            case '9': filterBuffer.append('9'); break;
        }
    }
    *buffer = filterBuffer.getPtr(0);
    *length = filterBuffer.getLength();
}


