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

#include "MessageBox.hpp"
#include "GlobalConfig.hpp"
#include "GuiLayoutRow.hpp"
#include "GuiLayoutSpacer.hpp"
#include "GlobalConfig.hpp"
#include "LabelWidget.hpp"
#include "Callback.hpp"

using namespace LucED;

MessageBox::MessageBox(TopWin* referingWindow, const MessageBoxParameter& p)
    : PanelDialogWin(referingWindow),
      wasClosed(false)
{
    if (p.defaultButtonLabel == "" && p.cancelButtonLabel == "") {
        button1 = Button::create(this, "O]K");
    } else {
        if (p.defaultButtonLabel.getLength() > 0) {
            button1 = Button::create(this, p.defaultButtonLabel);
        }
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
    if (button1.isValid()) {
        row0->addElement(button1);
    }
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
    Callback<Button*>::Ptr buttonCallback = newCallback(this, &MessageBox::handleButtonPressed);
    
    if (button1.isValid()) {
        button1->setButtonPressedCallback(buttonCallback);
    }

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
    if (button1.isValid()) {
        button1->show();
    }
    if (button3.isValid()) {
        button3->show();
    }
    if (button2.isValid()) {
        button2->show();
    }
    
    if (button1.isValid()) {
        button1->setAsDefaultButton();
    } else if (button3.isValid()) {
        button3->setAsDefaultButton();
    } else if (button2.isValid()) {
        button2->setAsDefaultButton();
    }
    setTitle(p.title);

    if (button3.isValid()) {
        if (button2.isValid()) {
            button2->setNextFocusWidget(button3);
            if (button1.isValid()) {
                button1->setNextFocusWidget(button2);
                button3->setNextFocusWidget(button1);
            } else {
                button3->setNextFocusWidget(button2);
            }
        } else {
            if (button1.isValid()) {
                button1->setNextFocusWidget(button3);
                button3->setNextFocusWidget(button1);
            }
        }
    }
    if (button1.isValid()) {
        setFocus(button1);
    } else if (button3.isValid()) {
        setFocus(button3);
    } else if (button2.isValid()) {
        setFocus(button2);
    }
    
    if (p.closeCallback.isValid()) {
        this->closeCallback = p.closeCallback;
    }
    
    keyMapping = p.keyMapping;
    if (p.messageBoxQueue.isValid()) {
        p.messageBoxQueue->append(this);
    }
    
    this->invokeNotifyCallback = p.invokeNotifyCallback;
    this->closeNotifyCallback  = p.closeNotifyCallback;
}

void MessageBox::show()
{
    PanelDialogWin::show();
    invokeNotifyCallback->call(this);
}

void MessageBox::handleButtonPressed(Button* button)
{
    if (!wasClosed)
    {

        if (button == button1 && defaultButtonCallback->isEnabled()) {
                PanelDialogWin::requestCloseWindow(TopWin::CLOSED_BY_USER);
                wasClosed = true;
                defaultButtonCallback->call();
        } else {
            if (button == button1 && button3.isInvalid()) {
                PanelDialogWin::requestCloseWindow(TopWin::CLOSED_BY_USER);
                wasClosed = true;
            }
            else if (button == button3 && button3.isValid()) {
                PanelDialogWin::requestCloseWindow(TopWin::CLOSED_BY_USER);
                wasClosed = true;
                cancelButtonCallback->call();
            }
            else if (button == button2) {
                PanelDialogWin::requestCloseWindow(TopWin::CLOSED_BY_USER);
                wasClosed = true;
                alternativeButtonCallback->call();
            }
        }
    }
}

void MessageBox::requestCloseWindow(TopWin::CloseReason reason)
{
    PanelDialogWin::requestCloseWindow(reason);
    if (!wasClosed) {
        if (closeCallback->isEnabled()) {
            closeCallback->call();
        } else {
            cancelButtonCallback->call();
        }
        wasClosed = true;
        closeNotifyCallback->call(this);
    }
}


GuiElement::ProcessingResult MessageBox::processKeyboardEvent(const XEvent* event)
{
    ProcessingResult rslt = NOT_PROCESSED;

    if (keyMapping.isValid())
    {
        KeyId pressedKey = KeyId(XLookupKeysym((XKeyEvent*)&event->xkey, 0));
        
        KeyMapping::Id keyMappingId(event->xkey.state, pressedKey);
        
        Callback<>::Ptr keyAction = keyMapping->find(keyMappingId);
        
        if (keyAction.isEnabled()) {
            keyAction->call();
            PanelDialogWin::requestCloseWindow(TopWin::CLOSED_BY_USER);
            rslt = EVENT_PROCESSED;
        }
    }
    
    if (rslt == NOT_PROCESSED)
    {
        rslt = PanelDialogWin::processKeyboardEvent(event);
    }

    return rslt;
}

