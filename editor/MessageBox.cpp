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

#include "MessageBox.hpp"
#include "GlobalConfig.hpp"
#include "GuiLayoutRow.hpp"
#include "GuiLayoutSpacer.hpp"
#include "GlobalConfig.hpp"
#include "LabelWidget.hpp"
#include "Callback.hpp"

using namespace LucED;

MessageBox::MessageBox(TopWin* referingWindow, MessageBoxParameter p)
    : PanelDialogWin(referingWindow)
{
    if (p.defaultButtonLabel == "") {
        button1 = Button::create(this, "O]K");
    } else {
        button1 = Button::create(this, p.defaultButtonLabel);
        if (p.cancelButtonLabel == "") {
            button3 = Button::create(this, "C]ancel");
        } else {
            button3 = Button::create(this, p.cancelButtonLabel);
        }
    }

    if (p.alternativeButtonLabel.getLength() > 0) {
        button2 = Button::create(this, p.alternativeButtonLabel);
    }
   
    LabelWidget::Ptr label0 = LabelWidget::create(this, p.message);
    GuiLayoutColumn::Ptr column0 = GuiLayoutColumn::create();
    GuiLayoutRow::Ptr row0 = GuiLayoutRow::create();
    GuiLayoutSpacerFrame::Ptr frame0 = GuiLayoutSpacerFrame::create(column0, 10);
    setRootElement(frame0);

    column0->addSpacer();
    column0->addElement(label0);
    column0->addElement(GuiLayoutSpacer::create(0, 0, 0, 10, 0, INT_MAX));
    column0->addElement(row0);
    column0->addSpacer();

    row0->addElement(GuiLayoutSpacer::create(0, 0, 0, 0, INT_MAX, 0));
    row0->addElement(button1);
    if (button2.isValid()) {
        row0->addElement(button2);
    }
    if (button3.isValid()) {
        row0->addElement(button3);
    }
    row0->addElement(GuiLayoutSpacer::create(3, 0, 10, 0, 10, 0));
    //row0->addElement(cancelButton);
    row0->addElement(GuiLayoutSpacer::create(0, 0, 0, 0, INT_MAX, 0));
    
    if (p.defaultButtonCallback.isValid()) {
        defaultButtonCallback = p.defaultButtonCallback;
    }
    Callback1<Button*> buttonCallback(this, &MessageBox::handleButtonPressed);
    button1->setButtonPressedCallback(buttonCallback);

    if (button2.isValid()) {
        if (p.alternativeButtonCallback.isValid()) {
            alternativeButtonCallback = p.alternativeButtonCallback;
        }
        button2->setButtonPressedCallback(buttonCallback);
    }
    if (button3.isValid()) {
        button3->setButtonPressedCallback(buttonCallback);
        cancelButtonCallback = p.cancelButtonCallback;
    }

    label0->show();
    button1->show();
    if (button3.isValid()) {
        button3->show();
    }
    if (button2.isValid()) {
        button2->show();
    }
    
    button1->setAsDefaultButton();
    setTitle(p.title);

    if (button3.isValid()) {
        if (button2.isValid()) {
            button1->setNextFocusWidget(button2);
            button2->setNextFocusWidget(button3);
            button3->setNextFocusWidget(button1);
        } else {
            button1->setNextFocusWidget(button3);
            button3->setNextFocusWidget(button1);
        }
    }
    setFocus(button1);
}


void MessageBox::handleButtonPressed(Button* button)
{
    if (button == button1 && defaultButtonCallback.isValid()) {
            PanelDialogWin::requestCloseWindow();
            defaultButtonCallback.call();
    } else {
        if (button == button1 && button3.isInvalid()) {
            PanelDialogWin::requestCloseWindow();
        }
        else if (button == button3 && button3.isValid()) {
            PanelDialogWin::requestCloseWindow();
            cancelButtonCallback.call();
        }
        else if (button == button2) {
            PanelDialogWin::requestCloseWindow();
            alternativeButtonCallback.call();
        }
    }
}

void MessageBox::requestCloseWindow()
{
    PanelDialogWin::requestCloseWindow();
    cancelButtonCallback.call();
}


