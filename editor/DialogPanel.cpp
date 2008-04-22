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

#include <X11/keysym.h>

#include "util.hpp"
#include "Callback.hpp"
#include "DialogPanel.hpp"
#include "Button.hpp"
#include "GlobalConfig.hpp"

using namespace LucED;

DialogPanel::DialogPanel(GuiWidget*                parent, 
                         Callback<GuiWidget*>::Ptr requestCloseCallback)
    : GuiWidget(parent, 0, 0, 1, 1, 0),
      wasNeverShown(true),
      
      keyMapping1(KeyMapping::create()),
      keyMapping2(KeyMapping::create()),
      hotKeyMapping(HotKeyMapping::create()),
      
      hasFocus(false),
      requestCloseCallback(requestCloseCallback)
{
    addToXEventMask(ExposureMask);
    setBackgroundColor(getGuiRoot()->getGuiColor03());
    keyMapping1->set(            0, KeyId("Tab"),      newCallback(this, &DialogPanel::switchFocusToNextWidget));
    keyMapping1->set(    ShiftMask, KeyId("Tab"),      newCallback(this, &DialogPanel::switchFocusToPrevWidget));
    keyMapping2->set(            0, KeyId("Left"),      newCallback(this, &DialogPanel::switchFocusToPrevWidget));
    keyMapping2->set(            0, KeyId("Right"),     newCallback(this, &DialogPanel::switchFocusToNextWidget));
    keyMapping2->set(            0, KeyId("KP_Left"),   newCallback(this, &DialogPanel::switchFocusToPrevWidget));
    keyMapping2->set(            0, KeyId("KP_Right"),  newCallback(this, &DialogPanel::switchFocusToNextWidget));
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

void DialogPanel::setPosition(Position p)
{
    Measures m = rootElement->getDesiredMeasures();
    
    if (m.minWidth > p.w) {
        p.w = m.minWidth;
    }
    if (m.minHeight > p.h) {
        p.h = m.minHeight;
    }
    
    GuiWidget::setPosition(p);
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
    for (HotKeyMapping::Iterator i = hotKeyMapping->getIterator(); !i.isAtEnd(); i.gotoNext()) {
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
    for (HotKeyMapping::Iterator i = hotKeyMapping->getIterator(); !i.isAtEnd(); i.gotoNext()) {
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
    } else
    {
        
        KeyId       pressedKey      = KeyId      (XLookupKeysym((XKeyEvent*)&event->xkey, 0));
        KeyModifier pressedModifier = KeyModifier(event->xkey.state);
        
        for (int i = 0; !processed && i < 2; ++i)
        {
            bool hotKeyProcessed = false;
            KeyMapping::Id keyMappingId(0, KeyId());
            switch (i) {
                case 0: keyMappingId = KeyMapping::Id(pressedModifier, pressedKey);
                        break;
                case 1: keyMappingId = KeyMapping::Id(       Mod1Mask, pressedKey);
                        break;
            }
            HotKeyMapping::Value foundHotKeyWidgets = hotKeyMapping->get(keyMappingId);
            
            if (i == 0 && !foundHotKeyWidgets.isValid() && pressedModifier.containsModifier1()) {
                foundHotKeyWidgets = hotKeyMapping->get(KeyMapping::Id(Mod1Mask, pressedKey));
            }
            
            if (foundHotKeyWidgets.isValid()) {
                WidgetQueue::Ptr widgets = foundHotKeyWidgets.get();
                ASSERT(widgets.isValid());
                WeakPtr<GuiWidget> w = widgets->getLast();
                if (w.isValid()) {
                    w->treatHotKeyEvent(KeyMapping::Id(pressedModifier, pressedKey));
                    if (focusedElement.isValid() && w != focusedElement) {
                        focusedElement->notifyAboutHotKeyEventForOtherWidget();
                    }
                    hotKeyProcessed = true;
                    processed = true;
                }
            } 
            if (!hotKeyProcessed) {
                Callback<>::Ptr keyAction = keyMapping1->find(keyMappingId);
                if (keyAction->isEnabled()) {
                    keyAction->call();
                    processed = true;
                } else {
                    if (focusedElement.isValid()) {
                        ProcessingResult rslt = focusedElement->processKeyboardEvent(event);
                        if (rslt == EVENT_PROCESSED) {
                            processed = true;
                        }
                    }
                    if (!processed) {
                        Callback<>::Ptr keyAction = keyMapping2->find(keyMappingId);
                        if (keyAction->isEnabled()) {
                            keyAction->call();
                            processed = true;
                        }
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
    
    HotKeyMapping::Value foundWidgets = hotKeyMapping->get(id);
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
        this->hotKeyMapping->set(id, widgets);
    }
    widgets->append(w);
    if (hasFocus) {
        w->treatNewHotKeyRegistration(id);
    }
}

void DialogPanel::requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w)
{
    ASSERT(w != NULL);
    HotKeyMapping::Value foundWidgets = hotKeyMapping->get(id);
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

void DialogPanel::requestClose()
{
    requestCloseCallback->call(this);
}
