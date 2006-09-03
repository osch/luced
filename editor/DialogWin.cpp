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
}


void DialogWin::treatFocusOut()
{
    if (focusedElement.isValid()) {
        focusedElement->treatFocusOut();
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
            bool hotKeyProcessed = false;
            KeyMapping::Id keyMappingId(event->xkey.state, XLookupKeysym((XKeyEvent*)&event->xkey, 0));
            HotKeyMapping::Value foundHotKeyWidgets = hotKeyMapping.get(keyMappingId);
            if (foundHotKeyWidgets.isValid()) {
                WidgetQueue::Ptr widgets = foundHotKeyWidgets.get();
                ASSERT(widgets.isValid());
                GuiWidget* w = widgets->getLast();
                if (w != NULL) {
                    w->treatHotKeyEvent(keyMappingId);
                    hotKeyProcessed = true;
                }
            } 
            if (!hotKeyProcessed) {
                Callback0 keyAction = keyMapping.find(keyMappingId);
                if (keyAction.isValid()) {
                    keyAction.call();
                    return EVENT_PROCESSED;
                } else {
                    return focusedElement->processKeyboardEvent(event);
                }
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



void DialogWin::requestFocusFor(GuiWidget* w)
{
    setFocus(w);
}

void DialogWin::requestHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w)
{
    ASSERT(w != NULL);
    
    HotKeyMapping::Value foundWidgets = hotKeyMapping.get(id);
    WidgetQueue::Ptr widgets;

    if (foundWidgets.isValid()) {
        widgets = foundWidgets.get();
        ASSERT(widgets.isValid());
        GuiWidget* activeWidget = widgets->getLast();
        if (activeWidget != NULL) {
            activeWidget->treatLostHotKeyRegistration(id);
        }
    } else {
        widgets = WidgetQueue::create();
        this->hotKeyMapping.set(id, widgets);
    }
    widgets->append(w);
    w->treatNewHotKeyRegistration(id);
}

void DialogWin::requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w)
{
    ASSERT(w != NULL);
    HotKeyMapping::Value foundWidgets = hotKeyMapping.get(id);
    if (foundWidgets.isValid()) {
        WidgetQueue::Ptr widgets = foundWidgets.get();
        ASSERT(widgets.isValid());
        if (widgets->getLast() == w) {
            widgets->removeLast();
            w->treatLostHotKeyRegistration(id);
            GuiWidget* newActive = widgets->getLast();
            if (newActive != NULL) {
                newActive->treatNewHotKeyRegistration(id);
            }
        }
    }
}
