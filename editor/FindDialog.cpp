/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch@luced.de
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

#include "FindDialog.h"
#include "GlobalConfig.h"
#include "GuiLayoutRow.h"
#include "GuiLayoutSpacer.h"
#include "TextData.h"
#include "GlobalConfig.h"

using namespace LucED;

FindDialog::FindDialog(TopWin* referingWindow, int x, int y, unsigned int width, unsigned int height)
    : DialogWin(referingWindow),
      slotForButtonPressed(this, &FindDialog::handleButtonPressed)
{
    findButton = Button::create(this, "Find");
    findButton->setAsDefault();
    cancelButton = Button::create(this, "Cancel");

    TextData::Ptr          editData  = TextData::create();
    LanguageMode::Ptr      languageMode = GlobalConfig::getInstance()->getLanguageModeForFileName("");
    Hiliting::Ptr          hiliting = Hiliting::create(editData, languageMode);
    HilitingBuffer::Ptr    hilitingBuffer = HilitingBuffer::create(hiliting);
    
    this->editField = TextEditorWidget::create(this, editData, 
            GlobalConfig::getInstance()->getTextStyles(), 
            hilitingBuffer);
    editField->setDesiredMeasuresInChars(
            20, 1, 40, 1, -1 , 1);

    GuiLayoutColumn::Ptr column0 = GuiLayoutColumn::create();
    GuiLayoutRow::Ptr row0 = GuiLayoutRow::create();
    GuiLayoutSpacerFrame::Ptr frame0 = GuiLayoutSpacerFrame::create(column0, 10);
    setRootElement(frame0);

    column0->addSpacer();
    column0->addElement(editField);
    column0->addSpacer();
    column0->addElement(row0);
    column0->addSpacer();

    row0->addElement(GuiLayoutSpacer::create(0, 0, 0, 0, -1, 0));
    row0->addElement(findButton);
    row0->addElement(GuiLayoutSpacer::create(3, 0, 10, 0, 10, 0));
    row0->addElement(cancelButton);
    row0->addElement(GuiLayoutSpacer::create(0, 0, 0, 0, -1, 0));
    
    findButton->setButtonPressedCallback(slotForButtonPressed);
    cancelButton->setButtonPressedCallback(slotForButtonPressed);

    editField->show();
    findButton->show();
    cancelButton->show();    
}


void FindDialog::handleButtonPressed(Button* button)
{
    if (button == findButton) {
        printf("findButton\n");
    }
    else if (button == cancelButton) {
        this->requestCloseWindow();
    }
}

void FindDialog::treatFocusIn()
{
    editField->treatFocusIn();
}


void FindDialog::treatFocusOut()
{
    editField->treatFocusOut();
}

bool FindDialog::processKeyboardEvent(const XEvent *event)
{
    return editField->processKeyboardEvent(event);
}


