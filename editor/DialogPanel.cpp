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

#include <X11/keysym.h>

#include "util.hpp"
#include "Callback.hpp"
#include "DialogPanel.hpp"
#include "Button.hpp"
#include "GlobalConfig.hpp"

using namespace LucED;

class DialogPanel::MyKeyActionHandler : public KeyActionHandler
{
public:
    typedef OwningPtr<MyKeyActionHandler> Ptr;
    
    static Ptr create(RawPtr<DialogPanel> p) {
        return Ptr(new MyKeyActionHandler(p));
    }
    
    virtual bool invokeActionMethod(ActionId actionId)
    {
        return  (   (   p->focusedElement.isValid() 
                     && p->focusedElement->getKeyActionHandler()->invokeActionMethod(actionId))
                 || KeyActionHandler::invokeActionMethod(actionId));
    }
    
    virtual bool hasActionMethod(ActionId actionId)
    {
        return  (   (   p->focusedElement.isValid() 
                     && p->focusedElement->getKeyActionHandler()->hasActionMethod(actionId))
                 || KeyActionHandler::hasActionMethod(actionId));
    }
    
    virtual bool handleHighPriorityKeyPress(const KeyPressEvent& keyPressEvent)
    {
        return p->handleHighPriorityKeyPress(keyPressEvent);
    }
    
    virtual bool handleLowPriorityKeyPress(const KeyPressEvent& keyPressEvent)
    {
        return p->handleLowPriorityKeyPress(keyPressEvent);
    }
    
private:
    MyKeyActionHandler(RawPtr<DialogPanel> p)
        : p(p)
    {}
    
    RawPtr<DialogPanel> p;
};

DialogPanel::DialogPanel(Callback<>::Ptr requestCloseCallback)
    : hotKeyMapping(HotKeyMapping::create()),
      
      focusedElementTakesAwayDefaultKey(false),
      defaultKeyWidgets(WidgetQueue::create()),
      
      requestCloseCallback(requestCloseCallback)
{
    MyKeyActionHandler::Ptr keyActionHandler = MyKeyActionHandler::create(this);
    setKeyActionHandler(keyActionHandler);

    keyActionHandler->addActionMethods(Actions::create(this));
}


void DialogPanel::setPosition(const Position& newPosition)
{
    Position p = newPosition;
    Measures m = getRootElement()->getDesiredMeasures();
    
    if (m.minWidth > p.w) {
        p.w = m.minWidth;
    }
    if (m.minHeight > p.h) {
        p.h = m.minHeight;
    }
    
    BaseClass::setPosition(p);
}


void DialogPanel::processGuiWidgetNewPositionEvent(const Position& newPosition)
{
    if (getRootElement().isValid())
    {
        int guiSpacing = GlobalConfig::getConfigData()->getGeneralConfig()->getGuiSpacing();

        int border = guiSpacing;
        int dx = border;
        int dy = border;
        if (newPosition.w < 2*dx)
           dx = (2*dx - newPosition.w)/2;
        if (newPosition.h < 2*dy)
           dy = (2*dy - newPosition.h)/2;
        getRootElement()->setPosition(Position(dx, dy, newPosition.w - 2*dx - guiSpacing, newPosition.h - 2*dy - guiSpacing));
    }
    BaseClass::processGuiWidgetNewPositionEvent(newPosition);
}

GuiElement::Measures DialogPanel::internalGetDesiredMeasures()
{
    if (getRootElement().isValid())
    {
        int guiSpacing = GlobalConfig::getConfigData()->getGeneralConfig()->getGuiSpacing();
        
        Measures m = getRootElement()->getDesiredMeasures();
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

void DialogPanel::processGuiWidgetRedrawEvent(Region redrawRegion)
{
    getGuiWidget()->drawRaisedBox(0, 0, getPosition().w, getPosition().h);
}


GuiWidget::ProcessingResult DialogPanel::processGuiWidgetEvent(const XEvent* event)
{
    switch (event->type)
    {
        case ButtonPress:
            if (!hasFocus()) {
                reportMouseClick();
            }
            break;
    }
    return GuiWidget::NOT_PROCESSED;
}

void DialogPanel::processGuiWidgetCreatedEvent()
{
    getGuiWidget()->addToXEventMask(ExposureMask|ButtonPressMask);
    getGuiWidget()->setBackgroundColor(getGuiRoot()->getGuiColor03());
}

void DialogPanel::treatFocusIn()
{
    {
        KeyMapping::Id defaultKey(KeyModifier::NONE, KeyId("Return"));
        RawPtr<FocusableElement> last = defaultKeyWidgets->getLast();

        if (!focusedElementTakesAwayDefaultKey) {
            if (last != Null) {
                last->treatNewHotKeyRegistration(defaultKey);
            }
        }
    }

    BaseClass::treatFocusIn();
    if (focusedElement.isValid()) {
        focusedElement->treatFocusIn();
    }
    for (HotKeyMapping::Iterator i =   hotKeyPredecessor.isValid()
                                     ? hotKeyPredecessor->hotKeyMapping->getIterator()
                                     : this             ->hotKeyMapping->getIterator();
         !i.isAtEnd(); 
          i.gotoNext())
    {
        WidgetQueue::Ptr widgets = i.getValue();
        ASSERT(widgets.isValid());
        RawPtr<FocusableElement> w = widgets->getLast();
        if (w != Null) {
            w->treatNewHotKeyRegistration(i.getKey());
        }        
    }
}


void DialogPanel::treatFocusOut()
{
    {
        KeyMapping::Id defaultKey(KeyModifier::NONE, KeyId("Return"));
        RawPtr<FocusableElement> last = defaultKeyWidgets->getLast();

        if (!focusedElementTakesAwayDefaultKey) {
            if (last != Null) {
                last->treatLostHotKeyRegistration(defaultKey);
            }
        }
    }

    BaseClass::treatFocusOut();
    if (focusedElement.isValid()) {
        focusedElement->treatFocusOut();
    }
    if (hotKeySuccessor.isInvalid())
    {
        for (HotKeyMapping::Iterator i =   hotKeyPredecessor.isValid()
                                         ? hotKeyPredecessor->hotKeyMapping->getIterator()
                                         : this             ->hotKeyMapping->getIterator();
             !i.isAtEnd();
              i.gotoNext())
        {
            WidgetQueue::Ptr widgets = i.getValue();
            ASSERT(widgets.isValid());
            RawPtr<FocusableElement> w = widgets->getLast();
            if (w != Null) {
                w->treatLostHotKeyRegistration(i.getKey());
            }        
        }
    }
}

void DialogPanel::Actions::focusNext()
{
    if (thisPanel->focusedElement.isValid()) {
        RawPtr<FocusableElement> e = thisPanel->focusedElement;
        do {
            e = e->getNextFocusWidget();
        } while (e != thisPanel->focusedElement && e.isValid() && !e->isFocusable());
        if (e.isValid()) {
            thisPanel->setFocus(e);
        }
    }
}


void DialogPanel::Actions::focusPrevious()
{
    if (thisPanel->focusedElement.isValid()) {
        RawPtr<FocusableElement> e = thisPanel->focusedElement;
        do {
            e = e->getPrevFocusWidget();
        } while (e != thisPanel->focusedElement && e.isValid() && !e->isFocusable());
        if (e.isValid()) {
            thisPanel->setFocus(e);
        }
    }
}

void DialogPanel::Actions::focusRight()
{
    if (thisPanel->focusedElement.isValid())
    {
        RawPtr<FocusableElement> e = thisPanel->focusedElement;
        RawPtr<FocusableElement> best;
        int minX = e->getPosition().x + e->getPosition().w;
        int minY = e->getPosition().y;
        int maxY = e->getPosition().y + e->getPosition().h;
        int bestX = INT_MAX;
        int bestY = INT_MAX;
//printf("\n-----           %d %d %d %d\n", e->getPosition().y, e->getPosition().x, e->getPosition().h, e->getPosition().w);
        do {
            e = e->getNextFocusWidget();
            if (e.isValid() && e->isFocusable())
            {
//printf("                %d %d %d %d\n", e->getPosition().y, e->getPosition().x, e->getPosition().h, e->getPosition().w);

                if (   (e->getPosition().x < bestX && e->getPosition().x + e->getPosition().w > minX)
                    && (e->getPosition().y + e->getPosition().h > minY  && e->getPosition().y < maxY 
                        && e->getPosition().y < bestY))
                {
//printf("best            %d %d %d %d\n", e->getPosition().y, e->getPosition().x, e->getPosition().h, e->getPosition().w);
                    best = e;
                    bestY = e->getPosition().y;
                    bestX = e->getPosition().x;
                }
            }
        } while (e != thisPanel->focusedElement && e.isValid());

        if (!best.isValid())
        {
            int bestX = INT_MAX;
            int bestY = INT_MAX;
            int bestH = 0;
            
            e = thisPanel->focusedElement;
            do {
                e = e->getNextFocusWidget();
                if (e.isValid() && e->isFocusable())
                {
    //printf("                %d %d %d %d\n", e->getPosition().y, e->getPosition().x, e->getPosition().h, e->getPosition().w);
    
                    if (   (e->getPosition().x < bestX)
                        && (e->getPosition().y >= maxY 
                            && e->getPosition().y < bestY + bestH))
                    {
    //printf("best            %d %d %d %d\n", e->getPosition().y, e->getPosition().x, e->getPosition().h, e->getPosition().w);
                        best = e;
                        bestY = e->getPosition().y;
                        bestX = e->getPosition().x;
                        bestH = e->getPosition().h;
                    }
                }
            } while (e != thisPanel->focusedElement && e.isValid());
        }
        if (best.isValid()) {
            thisPanel->setFocus(best);
        }
    }
}


void DialogPanel::Actions::focusLeft()
{
    if (thisPanel->focusedElement.isValid())
    {
        RawPtr<FocusableElement> e = thisPanel->focusedElement;
        RawPtr<FocusableElement> best;
        int maxX = e->getPosition().x;
        int minY = e->getPosition().y;
        int maxY = e->getPosition().y + e->getPosition().h;
        int bestX = -1;
        int bestY = -1;
//printf("\n-----           %d %d %d %d\n", e->getPosition().y, e->getPosition().x, e->getPosition().h, e->getPosition().w);
        do {
            e = e->getPrevFocusWidget();
            if (e.isValid() && e->isFocusable())
            {
//printf("                %d %d %d %d\n", e->getPosition().y, e->getPosition().x, e->getPosition().h, e->getPosition().w);

                if (   (e->getPosition().x + e->getPosition().w > bestX && e->getPosition().x < maxX)
                    && (e->getPosition().y + e->getPosition().h > minY  && e->getPosition().y < maxY 
                        && e->getPosition().y + e->getPosition().h > bestY))
                {
//printf("best            %d %d %d %d\n", e->getPosition().y, e->getPosition().x, e->getPosition().h, e->getPosition().w);
                    best = e;
                    bestY = e->getPosition().y + e->getPosition().h;
                    bestX = e->getPosition().x + e->getPosition().w;
                }
            }
        } while (e != thisPanel->focusedElement && e.isValid());

        if (!best.isValid())
        {
            int bestX = 0;
            int bestY = 0;
            int bestH = 0;
            
            e = thisPanel->focusedElement;
            do {
                e = e->getPrevFocusWidget();
                if (e.isValid() && e->isFocusable())
                {
    //printf("                %d %d %d %d\n", e->getPosition().y, e->getPosition().x, e->getPosition().h, e->getPosition().w);
    
                    if (   (e->getPosition().x + e->getPosition().w > bestX)
                        && (e->getPosition().y < minY 
                            && e->getPosition().y + e->getPosition().h > bestY + bestH))
                    {
    //printf("best            %d %d %d %d\n", e->getPosition().y, e->getPosition().x, e->getPosition().h, e->getPosition().w);
                        best = e;
                        bestY = e->getPosition().y;
                        bestH = e->getPosition().h;
                        bestX = e->getPosition().x + e->getPosition().w;
                    }
                }
            } while (e != thisPanel->focusedElement && e.isValid());
        }
        if (best.isValid()) {
            thisPanel->setFocus(best);
        }
    }
}


void DialogPanel::Actions::focusUp()
{
    if (thisPanel->focusedElement.isValid())
    {
        RawPtr<FocusableElement> e = thisPanel->focusedElement;
        RawPtr<FocusableElement> best;
        int maxY = e->getPosition().y;
        int minX = e->getPosition().x;
        int maxX = e->getPosition().x + e->getPosition().w;
        int bestY = -1;
        int bestX = -1;
//printf("\n-----           %d %d %d %d\n", e->getPosition().x, e->getPosition().y, e->getPosition().w, e->getPosition().h);
        do {
            e = e->getPrevFocusWidget();
            if (e.isValid() && e->isFocusable())
            {
//printf("                %d %d %d %d\n", e->getPosition().x, e->getPosition().y, e->getPosition().w, e->getPosition().h);

                if (   (e->getPosition().y + e->getPosition().h > bestY && e->getPosition().y < maxY)
                    && (e->getPosition().x + e->getPosition().w > minX  && e->getPosition().x < maxX 
                        && e->getPosition().x + e->getPosition().w > bestX))
                {
//printf("best            %d %d %d %d\n", e->getPosition().x, e->getPosition().y, e->getPosition().w, e->getPosition().h);
                    best = e;
                    bestX = e->getPosition().x + e->getPosition().w;
                    bestY = e->getPosition().y + e->getPosition().h;
                }
            }
        } while (e != thisPanel->focusedElement && e.isValid());
        if (best.isValid()) {
            thisPanel->setFocus(best);
        }
    }
}

void DialogPanel::Actions::focusDown()
{
    if (thisPanel->focusedElement.isValid())
    {
        RawPtr<FocusableElement> e = thisPanel->focusedElement;
        RawPtr<FocusableElement> best;
        int minY = e->getPosition().y + e->getPosition().h;
        int minX = e->getPosition().x;
        int maxX = e->getPosition().x + e->getPosition().w;
        int bestY = INT_MAX;
        int bestX = INT_MAX;
//printf("\n-----           %d %d %d %d\n", e->getPosition().x, e->getPosition().y, e->getPosition().w, e->getPosition().h);
        do {
            e = e->getNextFocusWidget();
            if (e.isValid() && e->isFocusable())
            {
//printf("                %d %d %d %d\n", e->getPosition().x, e->getPosition().y, e->getPosition().w, e->getPosition().h);

                if (   (e->getPosition().y < bestY && e->getPosition().y + e->getPosition().h > minY)
                    && (e->getPosition().x + e->getPosition().w > minX  && e->getPosition().x < maxX 
                        && e->getPosition().x < bestX))
                {
//printf("best            %d %d %d %d\n", e->getPosition().x, e->getPosition().y, e->getPosition().w, e->getPosition().h);
                    best = e;
                    bestX = e->getPosition().x;
                    bestY = e->getPosition().y;
                }
            }
        } while (e != thisPanel->focusedElement && e.isValid());
        if (best.isValid()) {
            thisPanel->setFocus(best);
        }
    }
}


bool DialogPanel::processHotKey(KeyMapping::Id keyMappingId)
{
    bool processed = false;
    
    if (focusedElement.isValid()) {
        if (focusedElement->getFocusType() == NO_FOCUS) {
            RawPtr<FocusableElement> e = focusedElement;
            do {
                e = e->getNextFocusWidget();
            } while (e.isValid() && e != focusedElement && !e->isFocusable());
            if (e.isValid()) {
                e->treatFocusIn();
                focusedElement = e;
            }
        }
    }

    {
        HotKeyMapping::Ptr mapping =   hotKeyPredecessor.isValid()
                                     ? hotKeyPredecessor->hotKeyMapping
                                     : this             ->hotKeyMapping;
        
        HotKeyMapping::Value foundHotKeyWidgets = mapping->get(keyMappingId);
        
        if (!foundHotKeyWidgets.isValid() && keyMappingId.getKeyModifier().containsAltKey()) {
            foundHotKeyWidgets = mapping->get(KeyMapping::Id(KeyModifier::ALT, keyMappingId.getKeyId()));
        }
        
        if (foundHotKeyWidgets.isValid()) {
            WidgetQueue::Ptr widgets = foundHotKeyWidgets.get();
            ASSERT(widgets.isValid());
            RawPtr<FocusableElement> w = widgets->getLast();
            if (w.isValid()) {
                w->treatHotKeyEvent(KeyMapping::Id(keyMappingId.getKeyModifier(), keyMappingId.getKeyId()));
                if (focusedElement.isValid() && w != focusedElement) {
                    focusedElement->treatNotificationOfHotKeyEventForOtherWidget();
                }
                processed = true;
            }
        }
    }
    return processed;

}

void DialogPanel::Actions::pressDefaultButton()
{
    KeyMapping::Id defaultKey(KeyModifier::NONE, KeyId("Return"));
    RawPtr<FocusableElement>  w = thisPanel->defaultKeyWidgets->getLast();

    if (w != Null) {
        w->treatHotKeyEvent(defaultKey);
        if (thisPanel->focusedElement.isValid() && w != thisPanel->focusedElement) {
            thisPanel->focusedElement->treatNotificationOfHotKeyEventForOtherWidget();
        }
    }
}

bool DialogPanel::handleLowPriorityKeyPress(const KeyPressEvent& keyPressEvent)
{
    bool processed = false;
    
    if (focusedElement.isValid()) {
        if (focusedElement->getFocusType() == NO_FOCUS) {
            RawPtr<FocusableElement> e = focusedElement;
            do {
                e = e->getNextFocusWidget();
            } while (e.isValid() && e != focusedElement && !e->isFocusable());
            if (e.isValid()) {
                e->treatFocusIn();
                focusedElement = e;
            }
        }
    }
    if (focusedElement.isValid()) {
        processed = focusedElement->getKeyActionHandler()->handleLowPriorityKeyPress(keyPressEvent);
    }
    if (!processed) {
        processed = handleHighPriorityKeyPress(KeyPressEvent(KeyModifier::ALT, keyPressEvent.getKeyId()));
    }
    return processed;
}


bool DialogPanel::handleHighPriorityKeyPress(const KeyPressEvent& keyPressEvent)
{
    bool processed = false;
    
    if (focusedElement.isValid()) {
        if (focusedElement->getFocusType() == NO_FOCUS) {
            RawPtr<FocusableElement> e = focusedElement;
            do {
                e = e->getNextFocusWidget();
            } while (e.isValid() && e != focusedElement && !e->isFocusable());
            if (e.isValid()) {
                e->treatFocusIn();
                focusedElement = e;
            }
        }
    }

    if (keyPressEvent.getKeyId() != KeyId("Return"))
    {
        processed = processHotKey(KeyMapping::Id(keyPressEvent.getKeyModifier(), keyPressEvent.getKeyId()));
    }
    return processed;
}



bool DialogPanel::takesAwayDefaultKey(RawPtr<FocusableElement> widget)
{
    if (widget == Null) {
        return false;
    }
    ActionKeyConfig::Ptr config = GlobalConfig::getInstance()->getActionKeyConfig();
    
    ActionKeyConfig::KeyCombinations::Ptr keys = config->getKeyCombinationsForAction(ActionId::PRESS_DEFAULT_BUTTON);

    if (keys.isValid()) {
        for (int i = 0, n = keys->getLength(); i < n; i++)
        {
            ActionKeyConfig::ActionIds::Ptr actions = config->getActionIdsForKeyCombination(keys->get(i));
            
            if (actions.isValid()) {
                for (int j = 0, m = actions->getLength(); j < m; ++j) 
                {
                    ActionId a = actions->get(j);
                    
                    if (widget->getKeyActionHandler()->hasActionMethod(a)) {
                        return true;
                    }
                    if (a == ActionId::PRESS_DEFAULT_BUTTON) {
                        return false;
                    }
                }
            }
        }
    }
    return false;
}


void DialogPanel::setFocus(RawPtr<FocusableElement> element)
{
    if (focusedElement != element)
    {
        if (hasFocus())
        {
            if (focusedElement.isValid()) {
                focusedElement->treatFocusOut();
            }
            focusedElement = element;
            if (focusedElement.isValid())
            {
                KeyMapping::Id defaultKey(KeyModifier::NONE, KeyId("Return"));
                
                bool newFocusedElementTakesAwayDefaultKey = takesAwayDefaultKey(focusedElement);
                
                RawPtr<FocusableElement> last = defaultKeyWidgets->getLast();
                if (last != Null)
                {
                    if (focusedElementTakesAwayDefaultKey) {
                        if (!newFocusedElementTakesAwayDefaultKey) {
                            last->treatNewHotKeyRegistration(defaultKey);
                        }
                    } else {
                        if (newFocusedElementTakesAwayDefaultKey)
                        {
                            last->treatLostHotKeyRegistration(defaultKey);
                        }
                    }
                }
                focusedElementTakesAwayDefaultKey = newFocusedElementTakesAwayDefaultKey;
                
                focusedElement->treatFocusIn();
            }
        } else {
            focusedElement = element;
            focusedElementTakesAwayDefaultKey = takesAwayDefaultKey(focusedElement);
        }
    }
}


void DialogPanel::reportMouseClickFrom(RawPtr<FocusableElement> w)
{
    if (!hasFocus()) {
        reportMouseClick();
    }
}

void DialogPanel::requestFocusFor(RawPtr<FocusableElement> w)
{
    setFocus(w);
}

void DialogPanel::requestHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w)
{
    ASSERT(w != Null);
    
    KeyMapping::Id defaultKey(KeyModifier::NONE, KeyId("Return"));
    
    if (id == defaultKey)
    {
        RawPtr<FocusableElement> last = defaultKeyWidgets->getLast();
                                       defaultKeyWidgets->append(w);
        if (!focusedElementTakesAwayDefaultKey) {
            if (last != Null) {
                last->treatLostHotKeyRegistration(id);
            }
            if (hasFocus()) {
                w->treatNewHotKeyRegistration(id);
            }
        }
        return;
    }


    HotKeyMapping::Value foundWidgets = hotKeyMapping->get(id);
    WidgetQueue::Ptr widgets;
    
    if (foundWidgets.isValid()) {
        widgets = foundWidgets.get();
        ASSERT(widgets.isValid());
        RawPtr<FocusableElement> activeWidget = widgets->getLast();
        if (hotKeyPredecessor.isInvalid() && activeWidget != Null) {
            if (activeWidget == w) {
                return;
            }
            activeWidget->treatLostHotKeyRegistration(id);
        }
    } else {
        widgets = WidgetQueue::create();
        this->hotKeyMapping->set(id, widgets);
    }
    widgets->append(w);
    if (hasFocus()) {
        w->treatNewHotKeyRegistration(id);
    }
    if (hotKeyPredecessor.isValid())
    {
        hotKeyPredecessor->requestHotKeyRegistrationFor(id, w);
    }
}

void DialogPanel::requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w)
{
    ASSERT(w != Null);

    KeyMapping::Id defaultKey(KeyModifier::NONE, KeyId("Return"));
    
    if (id == defaultKey)
    {
        w->treatLostHotKeyRegistration(id);
        defaultKeyWidgets->removeAll(w);

        if (hasFocus() && !focusedElementTakesAwayDefaultKey) {
            RawPtr<FocusableElement> last = defaultKeyWidgets->getLast();
            if (last != Null) {
                last->treatNewHotKeyRegistration(id);
            }
        }
        return;
    }

    HotKeyMapping::Value foundWidgets = hotKeyMapping->get(id);

    bool doIt = !hotKeyPredecessor.isValid();

    if (foundWidgets.isValid()) {
        WidgetQueue::Ptr widgets = foundWidgets.get();
        ASSERT(widgets.isValid());
        if (widgets->getLast() == w) {
            widgets->removeLast();
            if (doIt) {
                w->treatLostHotKeyRegistration(id);
            }
            RawPtr<FocusableElement> newActive = widgets->getLast();
            if (doIt && newActive != Null && hasFocus()) {
                newActive->treatNewHotKeyRegistration(id);
            }
        }
    }
    if (hotKeyPredecessor.isValid())
    {
        hotKeyPredecessor->requestRemovalOfHotKeyRegistrationFor(id, w);
    }
}

void DialogPanel::treatNotificationOfHotKeyEventForOtherWidget()
{
    if (focusedElement.isValid()) {
        focusedElement->treatNotificationOfHotKeyEventForOtherWidget();
    }    
}

void DialogPanel::requestClose()
{
    requestCloseCallback->call();
}


void DialogPanel::adopt(RawPtr<GuiElement>   parentElement,
                        RawPtr<GuiWidget>    parentWidget,
                        RawPtr<FocusManager> focusManagerForThis,
                        RawPtr<FocusManager> focusManagerForChilds)
{
    BaseClass::adopt(parentElement, 
                     parentWidget, 
                     focusManagerForThis,  // FocusManager for this
                     this);                // FocusManager for childs
}
