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

#include "FindDialog.h"
#include "GlobalConfig.h"
#include "GuiLayoutRow.h"
#include "GuiLayoutSpacer.h"
#include "TextData.h"
#include "GlobalConfig.h"
#include "LabelWidget.h"

using namespace LucED;

FindDialog::FindDialog(TopWin* referingWindow, int x, int y, unsigned int width, unsigned int height)
    : DialogWin(referingWindow),
      slotForButtonPressed(this, &FindDialog::handleButtonPressed)
{
    findButton = Button::create(this, "F]ind");
    cancelButton = Button::create(this, "C]ancel");
   
    this->editField = SingleLineEditField::create(this, 
                                                  GlobalConfig::getInstance()->getDefaultLanguageMode());
    editField->setDesiredWidthInChars(20, 40, -1);

    LabelWidget::Ptr label0 = LabelWidget::create(this, "String to Find:");
    GuiLayoutColumn::Ptr column0 = GuiLayoutColumn::create();
    GuiLayoutRow::Ptr row0 = GuiLayoutRow::create();
    GuiLayoutSpacerFrame::Ptr frame0 = GuiLayoutSpacerFrame::create(column0, 10);
    setRootElement(frame0);

    column0->addSpacer();
    column0->addElement(label0);
    column0->addElement(editField);
    column0->addElement(GuiLayoutSpacer::create(0, 0, 0, 10, 0, -1));
    column0->addElement(row0);
    column0->addSpacer();

    row0->addElement(GuiLayoutSpacer::create(0, 0, 0, 0, -1, 0));
    row0->addElement(findButton);
    row0->addElement(GuiLayoutSpacer::create(3, 0, 10, 0, 10, 0));
    row0->addElement(cancelButton);
    row0->addElement(GuiLayoutSpacer::create(0, 0, 0, 0, -1, 0));
    
    findButton->setButtonPressedCallback(slotForButtonPressed);
    cancelButton->setButtonPressedCallback(slotForButtonPressed);

    label0->show();
    editField->show();
    findButton->show();
    cancelButton->show();  
    
    editField->setNextFocusWidget(findButton);
    findButton->setNextFocusWidget(cancelButton);
    cancelButton->setNextFocusWidget(editField);
    setFocus(editField);
    //setDefaultButtonWidget(findButton);
    findButton->setAsDefaultButton();
    setTitle("Find");
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
    setFocus(editField);
    DialogWin::treatFocusIn();
}

