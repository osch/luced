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

#include <X11/keysym.h>

#include "util.h"
#include "Callback.h"
#include "DialogPanel.h"
#include "Button.h"
#include "GlobalConfig.h"

using namespace LucED;

DialogPanel::DialogPanel(GuiWidget* parent)
    : GuiWidget(parent, 0, 0, 1, 1, 0),
      wasNeverShown(true),
      hasFocus(false)
{
    addToXEventMask(ExposureMask);
    setBackgroundColor(getGuiRoot()->getGuiColor03());
    keyMapping1.set(            0, XK_Tab,      Callback0(this, &DialogPanel::switchFocusToNextWidget));
    keyMapping1.set(    ShiftMask, XK_Tab,      Callback0(this, &DialogPanel::switchFocusToPrevWidget));
    keyMapping2.set(                    0, XK_Left,      Callback0(this, &DialogPanel::switchFocusToPrevWidget));
    keyMapping2.set(                    0, XK_Right,     Callback0(this, &DialogPanel::switchFocusToNextWidget));
    keyMapping2.set(                    0, XK_KP_Left,   Callback0(this, &DialogPanel::switchFocusToPrevWidget));
    keyMapping2.set(                    0, XK_KP_Right,  Callback0(this, &DialogPanel::switchFocusToNextWidget));
}

void DialogPanel::setRootElement(OwningPtr<GuiElement> rootElement)
{
    this->rootElement = rootElement;
}

void DialogPanel::treatNewWindowPosition(Position newPosition)
{
    if (rootElement.isValid())
    {
        int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();

        GuiWidget::treatNewWindowPosition(newPosition);
        int border = guiSpacing;
        int dx = border;
        int dy = border;
        if (newPosition.w < 2*dx)
           dx = (2*dx - newPosition.w)/2;
        if (newPosition.h < 2*dy)
           dy = (2*dy - newPosition.h)/2;
        rootElement->setPosition(Position(dx, dy, newPosition.w - 2*dx - guiSpacing, newPosition.h - 2*dy - guiSpacing));
    }
}


GuiElement::Measures DialogPanel::getDesiredMeasures()
{
    if (rootElement.isValid())
    {
        int guiSpacing = GlobalConfig::getInstance()->getGuiSpacing();
        
        Measures m = rootElement->getDesiredMeasures();
        int border = guiSpacing;
        m.minWidth += 2*border + guiSpacing;
        m.minHeight += 2*border + guiSpacing;
        m.bestWidth += 2*border + guiSpacing;
        m.bestHeight += 2*border + guiSpacing;
        if (m.maxHeight != INT_MAX) {
            m.maxHeight += 2*border + guiSpacing;
        }
        if (m.maxWidth != INT_MAX) {
            m.maxWidth += 2*border + guiSpacing;
        }
        return m;
    }
    else {
        return Measures();
    }
}



GuiElement::ProcessingResult DialogPanel::processEvent(const XEvent *event)
{
    if (GuiWidget::processEvent(event) == EVENT_PROCESSED) {
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


void DialogPanel::treatFocusIn()
{
    hasFocus = true;
    if (focusedElement.isValid()) {
        focusedElement->treatFocusIn();
    }
    for (HotKeyMapping::Iterator i = hotKeyMapping.getIterator(); !i.isAtEnd(); i.gotoNext()) {
        WidgetQueue::Ptr widgets = i.getValue();
        ASSERT(widgets.isValid());
        GuiWidget* w = widgets->getLast();
        if (w != NULL) {
            w->treatNewHotKeyRegistration(i.getKey());
        }        
    }
}


void DialogPanel::treatFocusOut()
{
    hasFocus = false;
    if (focusedElement.isValid()) {
        focusedElement->treatFocusOut();
    }
    for (HotKeyMapping::Iterator i = hotKeyMapping.getIterator(); !i.isAtEnd(); i.gotoNext()) {
        WidgetQueue::Ptr widgets = i.getValue();
        ASSERT(widgets.isValid());
        GuiWidget* w = widgets->getLast();
        if (w != NULL) {
            w->treatLostHotKeyRegistration(i.getKey());
        }        
    }
}

void DialogPanel::switchFocusToNextWidget()
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


void DialogPanel::switchFocusToPrevWidget()
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



GuiElement::ProcessingResult DialogPanel::processKeyboardEvent(const XEvent *event)
{
    bool processed = false;
    
    if (focusedElement.isValid()) {
        if (focusedElement->getFocusType() == NO_FOCUS) {
            GuiWidget* e = focusedElement;
            do {
                e = e->getNextFocusWidget();
            } while (e != NULL && e != focusedElement && !e->isFocusable());
            if (e != NULL) {
                e->treatFocusIn();
                focusedElement = e;
            }
        }
    }

    if (focusedElement.isValid() && focusedElement->getFocusType() == TOTAL_FOCUS) {
        focusedElement->processKeyboardEvent(event);
        processed = true;
    } else {
        bool hotKeyProcessed = false;
        KeyMapping::Id keyMappingId(event->xkey.state, XLookupKeysym((XKeyEvent*)&event->xkey, 0));
        HotKeyMapping::Value foundHotKeyWidgets = hotKeyMapping.get(keyMappingId);
        if (foundHotKeyWidgets.isValid()) {
            WidgetQueue::Ptr widgets = foundHotKeyWidgets.get();
            ASSERT(widgets.isValid());
            WeakPtr<GuiWidget> w = widgets->getLast();
            if (w.isValid()) {
                w->treatHotKeyEvent(keyMappingId);
                if (focusedElement.isValid() && w != focusedElement) {
                    focusedElement->notifyAboutHotKeyEventForOtherWidget();
                }
                hotKeyProcessed = true;
                processed = true;
            }
        } 
        if (!hotKeyProcessed) {
            Callback0 keyAction = keyMapping1.find(keyMappingId);
            if (keyAction.isValid()) {
                keyAction.call();
                processed = true;
            } else {
                if (focusedElement.isValid()) {
                    ProcessingResult rslt = focusedElement->processKeyboardEvent(event);
                    if (rslt == EVENT_PROCESSED) {
                        processed = true;
                    }
                }
                if (!processed) {
                    Callback0 keyAction = keyMapping2.find(keyMappingId);
                    if (keyAction.isValid()) {
                        keyAction.call();
                        processed = true;
                    }
                }
            }
        }
    }

    return processed ? EVENT_PROCESSED : NOT_PROCESSED;
}


void DialogPanel::setFocus(GuiWidget* element)
{
    if (focusedElement != element) {
        if (hasFocus) {
            if (focusedElement.isValid()) {
                focusedElement->treatFocusOut();
            }
            focusedElement = element;
            if (focusedElement.isValid()) {
                focusedElement->treatFocusIn();
            }
        } else {
            focusedElement = element;
        }
    }
}



void DialogPanel::requestFocusFor(GuiWidget* w)
{
    setFocus(w);
}

void DialogPanel::requestHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w)
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
    if (hasFocus) {
        w->treatNewHotKeyRegistration(id);
    }
}

void DialogPanel::requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w)
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
            if (newActive != NULL && hasFocus) {
                newActive->treatNewHotKeyRegistration(id);
            }
        }
    }
}

void DialogPanel::notifyAboutHotKeyEventForOtherWidget()
{
    if (focusedElement.isValid()) {
        focusedElement->notifyAboutHotKeyEventForOtherWidget();
    }    
}

