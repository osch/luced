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

#include "FindPanel.h"
#include "GuiLayoutColumn.h"
#include "GuiLayoutRow.h"
#include "GlobalConfig.h"
#include "GuiLayoutSpacer.h"
#include "LabelWidget.h"

using namespace LucED;

FindPanel::FindPanel(GuiWidget* parent, TextEditorWidget* editorWidget, Callback1<MessageBoxParameter> messageBoxInvoker)
    : DialogPanel(parent),
      editorWidget(editorWidget),
      messageBoxInvoker(messageBoxInvoker)
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



void FindPanel::handleButtonPressed(Button* button)
{
    if (button == cancelButton) {
        requestCloseFor(this);
    } else if (button == goBackButton) {
        messageBoxInvoker.call(MessageBoxParameter().setTitle("xxx title xxx").setMessage("xxx message xxx"));
    }
}

