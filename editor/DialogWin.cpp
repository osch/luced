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

#include <X11/keysym.h>

#include "util.h"
#include "Callback.h"
#include "DialogWin.h"
#include "Button.h"

using namespace LucED;

DialogWin::DialogWin(TopWin* referingWindow)
    : wasNeverShown(true),
      referingWindow(referingWindow)
{
    if (referingWindow != NULL) {
        XSetTransientForHint(getDisplay(), getWid(), referingWindow->getWid());
    }
    addToXEventMask(ExposureMask);
    setBackgroundColor(getGuiRoot()->getGuiColor03());
    keyMapping.set(            0, XK_Tab,      Callback0(this, &DialogWin::switchFocusToNextWidget));
    keyMapping.set(    ShiftMask, XK_Tab,      Callback0(this, &DialogWin::switchFocusToPrevWidget));
    keyMapping.set(            0, XK_Escape,   Callback0(this, &DialogWin::requestCloseWindow));
    keyMapping.set(            0, XK_Return,   Callback0(this, &DialogWin::handleDefaultButtonPress));
    keyMapping.set(            0, XK_KP_Enter, Callback0(this, &DialogWin::handleDefaultButtonPress));
}

void DialogWin::setRootElement(OwningPtr<GuiElement> rootElement)
{
    this->rootElement = rootElement;
}

void DialogWin::show()
{
    if (rootElement.isValid()) {
        Measures m = rootElement->getDesiredMeasures();
        m.minHeight += 4;   m.minWidth  += 4;
        m.bestHeight += 4;  m.bestWidth += 4;
        if (wasNeverShown) {
            if (referingWindow != NULL)  {
                Position pp = referingWindow->getAbsolutePosition();
                int x = pp.x + (pp.w - m.bestWidth)/2;
                int y = pp.y + (pp.h - m.bestHeight)/2;
                setPosition(Position(x, y, m.bestWidth, m.bestHeight));
                setSizeHints(x, y, m.minWidth, m.minHeight, m.incrWidth, m.incrHeight);
            } else {
                setSize(m.bestWidth, m.bestHeight);
                setSizeHints(m.minWidth, m.minHeight, m.incrWidth, m.incrHeight);
            }
        } else {
            setSizeHints(getPosition().x, getPosition().y, m.minWidth, m.minHeight, m.incrWidth, m.incrHeight);
        }
        wasNeverShown = false;
    }
    TopWin::show();
}

void DialogWin::treatNewWindowPosition(Position newPosition)
{
    if (rootElement.isValid())
    {
        TopWin::treatNewWindowPosition(newPosition);
        int dx = 2;
        int dy = 2;
        if (newPosition.w < 2*dx)
           dx = (2*dx - newPosition.w)/2;
        if (newPosition.h < 2*dy)
           dy = (2*dy - newPosition.h)/2;
        rootElement->setPosition(Position(dx, dy, newPosition.w - 2*dx, newPosition.h - 2*dy));
    }
}




GuiElement::ProcessingResult DialogWin::processEvent(const XEvent *event)
{
    if (TopWin::processEvent(event) == EVENT_PROCESSED) {
        return EVENT_PROCESSED;
    } else {
        switch (event->type)
        {
            case GraphicsExpose:
                if (event->xgraphicsexpose.count > 0) {
                    break;
                }
            case Expose: {
                if (event->xexpose.count > 0) {
                    break;
                }
                drawRaisedBox(0, 0, getPosition().w, getPosition().h);
                return EVENT_PROCESSED;
            }
        }
        return NOT_PROCESSED;
    }
}


void DialogWin::treatFocusIn()
{
    if (focusedElement.isValid()) {
        focusedElement->treatFocusIn();
    }
    if (this->actualDefaultButtonWidget.isValid()) {
        this->actualDefaultButtonWidget->treatNewDefaultButtonState();
    } else if (this->defaultDefaultButtonWidget.isValid()) {
        this->actualDefaultButtonWidget = this->defaultDefaultButtonWidget;
        this->actualDefaultButtonWidget->treatNewDefaultButtonState();
    }
}


void DialogWin::treatFocusOut()
{
    if (focusedElement.isValid()) {
        focusedElement->treatFocusOut();
    }
    if (this->actualDefaultButtonWidget.isValid()) {
        this->actualDefaultButtonWidget->treatLostDefaultButtonState();
        this->actualDefaultButtonWidget.invalidate();
    }
}

void DialogWin::switchFocusToNextWidget()
{
    if (focusedElement.isValid()) {
        GuiWidget* e = focusedElement;
        do {
            e = e->getNextFocusWidget();
        } while (e != focusedElement && e != NULL && !e->isFocusable());
        if (e != NULL) {
            setFocus(e);
        }
    }
}


void DialogWin::switchFocusToPrevWidget()
{
    if (focusedElement.isValid()) {
        GuiWidget* e = focusedElement;
        do {
            e = e->getPrevFocusWidget();
        } while (e != focusedElement && e != NULL && !e->isFocusable());
        if (e != NULL) {
            setFocus(e);
        }
    }
}



GuiElement::ProcessingResult DialogWin::processKeyboardEvent(const XEvent *event)
{
    if (focusedElement.isValid()) {
        if (focusedElement->getFocusType() == NO_FOCUS) {
            GuiWidget* e = focusedElement;
            do {
                e = e->getNextFocusWidget();
            } while (e != focusedElement && !e->isFocusable());
            focusedElement = e;
        }
        if (focusedElement->getFocusType() == TOTAL_FOCUS) {
            return focusedElement->processKeyboardEvent(event);
        } else {
            Callback0 keyAction = keyMapping.find(event->xkey.state, XLookupKeysym((XKeyEvent*)&event->xkey, 0));
            if (keyAction.isValid()) {
                keyAction.call();
                return EVENT_PROCESSED;
            } else {
                return focusedElement->processKeyboardEvent(event);
            }
        }
    }
    return NOT_PROCESSED;
}


void DialogWin::setFocus(GuiWidget* element)
{
    if (focusedElement != element) {
        if (focusedElement.isValid()) {
            focusedElement->treatFocusOut();
        }
        focusedElement = element;
        if (focusedElement.isValid()) {
            focusedElement->treatFocusIn();
        }
    }
}



void DialogWin::handleDefaultButtonPress()
{
    Button* defaultButton = dynamic_cast<Button*>(getActualDefaultButtonWidget());
    if (defaultButton != NULL) {
        defaultButton->emulateButtonPress();
    }
}

void DialogWin::requestFocusFor(GuiWidget* w)
{
    setFocus(w);
}

GuiWidget* DialogWin::getActualDefaultButtonWidget()
{
    return actualDefaultButtonWidget;
}

void DialogWin::setDefaultButtonWidget(GuiWidget* widget)
{
    defaultDefaultButtonWidget = widget;
    requestToBeActualDefaultButtonWidget(widget);
}

void DialogWin::requestToBeActualDefaultButtonWidget(GuiWidget* widget)
{
    GuiWidget* oldDefaultButtonWidget = this->actualDefaultButtonWidget;
    this->actualDefaultButtonWidget = widget;
    if (oldDefaultButtonWidget != NULL && oldDefaultButtonWidget != this->actualDefaultButtonWidget) {
        oldDefaultButtonWidget->treatLostDefaultButtonState();
    }
    if (this->actualDefaultButtonWidget.isValid()) {
        this->actualDefaultButtonWidget->treatNewDefaultButtonState();
    }
}

void DialogWin::requestNotToBeActualDefaultButtonWidget(GuiWidget* widget)
{
    if (actualDefaultButtonWidget == widget) {
        actualDefaultButtonWidget.invalidate();
        widget->treatLostDefaultButtonState();
    }
    if (defaultDefaultButtonWidget.isValid()) {
        actualDefaultButtonWidget = defaultDefaultButtonWidget;
        defaultDefaultButtonWidget->treatNewDefaultButtonState();
    }
}


