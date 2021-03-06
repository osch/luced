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

#include "GotoLinePanel.hpp"
#include "GuiLayoutColumn.hpp"
#include "GuiLayoutRow.hpp"
#include "GlobalConfig.hpp"
#include "GuiLayoutSpacer.hpp"
#include "LabelWidget.hpp"

using namespace LucED;

GotoLinePanel::GotoLinePanel(TextEditorWidget* editorWidget,
                             Callback<>::Ptr   panelCloser)
    : DialogPanel(panelCloser),
      editorWidget(editorWidget)
{
    gotoButton = Button::create("G]oto Line");
    cancelButton = Button::create("C]ancel");
   
    this->editField = SingleLineEditField::create(GlobalConfig::getInstance()->getDefaultLanguageMode());
    editField->setDesiredWidthInChars(5, 10, 15);

    LabelWidget::Ptr label0 = LabelWidget::create("Line Number:");
    GuiLayoutRow::Ptr row0 = GuiLayoutRow::create();
    GuiLayoutSpacerFrame::Ptr frame0 = GuiLayoutSpacerFrame::create(row0, 0);
    setRootElement(frame0);

    label0   ->setLayoutHeight(gotoButton->getStandardHeight(), VerticalAdjustment::CENTER);
    editField->setLayoutHeight(gotoButton->getStandardHeight(), VerticalAdjustment::CENTER);

    row0->addElement(label0);
    row0->addElement(editField);

    row0->addElement(gotoButton);
    row0->addElement(cancelButton);
    row0->addElement(GuiLayoutSpacer::create(0, 0, 0, 0, INT_MAX, 0));
    
    Callback<Button*,Button::ActivationVariant>::Ptr buttonCallback = newCallback(this, &GotoLinePanel::handleButtonPressed);
    gotoButton->setButtonPressedCallback(buttonCallback);
    cancelButton->setButtonPressedCallback(buttonCallback);

    label0->show();
    editField->show();
    gotoButton->show();
    cancelButton->show();  
    
    editField->setNextFocusWidget(gotoButton);
    gotoButton->setNextFocusWidget(cancelButton);
    cancelButton->setNextFocusWidget(editField);
    setFocus(editField);
    gotoButton->setAsDefaultButton();

    editField->getTextData()->setInsertFilterCallback(newCallback(this, &GotoLinePanel::filterInsert));

    label0->setMiddleMouseButtonCallback(newCallback(editField, &SingleLineEditField::replaceTextWithPrimarySelection));
}



void GotoLinePanel::handleButtonPressed(Button* button, Button::ActivationVariant variant)
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
                m.moveToLineAndWCharColumn(newLineNumber, m.getWCharColumn());
                editorWidget->moveCursorToTextMarkAndAdjustVisibility(m);
                editorWidget->rememberCursorPixX();
                if (!wasNegative) {
                    m.moveToBeginOfLine();    long spos = m.getPos();
                    m.moveToNextLineBegin();  long epos = m.getPos();
                    
                    editorWidget->setPrimarySelection(spos, epos);

                } else {
                    editorWidget->releaseSelection();
                }
            }
        }
        requestClose();
        editField->getTextData()->clear();
    }
    else if (button == cancelButton) {
        requestClose();
        editField->getTextData()->clear();
    }
}


void GotoLinePanel::filterInsert(const byte** buffer, long* length)
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

void GotoLinePanel::show()
{
    setFocus(editField);
    DialogPanel::show();
}

