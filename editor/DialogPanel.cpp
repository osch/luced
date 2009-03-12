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

DialogPanel::DialogPanel(GuiWidget*                  parent, 
                         Callback<DialogPanel*>::Ptr requestCloseCallback)
    : GuiWidget(parent, 0, 0, 1, 1, 0),
      wasNeverShown(true),
      
      hotKeyMapping(HotKeyMapping::create()),
      
      focusedElementTakesAwayDefaultKey(false),
      defaultKeyWidgets(WidgetQueue::create()),
      
      hasFocusFlag(false),
      requestCloseCallback(requestCloseCallback)
{
    addToXEventMask(ExposureMask|ButtonPressMask);

    setBackgroundColor(getGuiRoot()->getGuiColor03());

    GuiWidget::addActionMethods(Actions::create(this));
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



GuiElement::ProcessingResult DialogPanel::processEvent(const XEvent* event)
{
    if (GuiWidget::processEvent(event) == EVENT_PROCESSED) {
        return EVENT_PROCESSED;
    } else {
        switch (event->type)
        {
            case ButtonPress:
                if (!hasFocusFlag) {
                    GuiWidget::reportMouseClickFrom(this);
                }
                break;
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
    {
        KeyMapping::Id defaultKey(KeyModifier::NONE, KeyId("Return"));
        GuiWidget*     last = defaultKeyWidgets->getLast();

        if (!focusedElementTakesAwayDefaultKey) {
            if (last != NULL) {
                last->treatNewHotKeyRegistration(defaultKey);
            }
        }
    }

    hasFocusFlag = true;
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
        GuiWidget* w = widgets->getLast();
        if (w != NULL) {
            w->treatNewHotKeyRegistration(i.getKey());
        }        
    }
}


void DialogPanel::treatFocusOut()
{
    {
        KeyMapping::Id defaultKey(KeyModifier::NONE, KeyId("Return"));
        GuiWidget*     last = defaultKeyWidgets->getLast();

        if (!focusedElementTakesAwayDefaultKey) {
            if (last != NULL) {
                last->treatLostHotKeyRegistration(defaultKey);
            }
        }
    }

    hasFocusFlag = false;
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
            GuiWidget* w = widgets->getLast();
            if (w != NULL) {
                w->treatLostHotKeyRegistration(i.getKey());
            }        
        }
    }
}

void DialogPanel::Actions::focusNext()
{
    if (thisPanel->focusedElement.isValid()) {
        GuiWidget* e = thisPanel->focusedElement;
        do {
            e = e->getNextFocusWidget();
        } while (e != thisPanel->focusedElement && e != NULL && !e->isFocusable());
        if (e != NULL) {
            thisPanel->setFocus(e);
        }
    }
}


void DialogPanel::Actions::focusPrevious()
{
    if (thisPanel->focusedElement.isValid()) {
        GuiWidget* e = thisPanel->focusedElement;
        do {
            e = e->getPrevFocusWidget();
        } while (e != thisPanel->focusedElement && e != NULL && !e->isFocusable());
        if (e != NULL) {
            thisPanel->setFocus(e);
        }
    }
}

void DialogPanel::Actions::focusRight()
{
    if (thisPanel->focusedElement.isValid())
    {
        GuiWidget* e = thisPanel->focusedElement;
        GuiWidget* best = NULL;
        int minX = e->getPosition().x + e->getPosition().w;
        int minY = e->getPosition().y;
        int maxY = e->getPosition().y + e->getPosition().h;
        int bestX = INT_MAX;
        int bestY = INT_MAX;
//printf("\n-----           %d %d %d %d\n", e->getPosition().y, e->getPosition().x, e->getPosition().h, e->getPosition().w);
        do {
            e = e->getNextFocusWidget();
            if (e != NULL && e->isFocusable())
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
        } while (e != thisPanel->focusedElement && e != NULL);

        if (best == NULL)
        {
            int bestX = INT_MAX;
            int bestY = INT_MAX;
            int bestH = 0;
            
            e = thisPanel->focusedElement;
            do {
                e = e->getNextFocusWidget();
                if (e != NULL && e->isFocusable())
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
            } while (e != thisPanel->focusedElement && e != NULL);
        }
        if (best != NULL) {
            thisPanel->setFocus(best);
        }
    }
}


void DialogPanel::Actions::focusLeft()
{
    if (thisPanel->focusedElement.isValid())
    {
        GuiWidget* e = thisPanel->focusedElement;
        GuiWidget* best = NULL;
        int maxX = e->getPosition().x;
        int minY = e->getPosition().y;
        int maxY = e->getPosition().y + e->getPosition().h;
        int bestX = -1;
        int bestY = -1;
//printf("\n-----           %d %d %d %d\n", e->getPosition().y, e->getPosition().x, e->getPosition().h, e->getPosition().w);
        do {
            e = e->getPrevFocusWidget();
            if (e != NULL && e->isFocusable())
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
        } while (e != thisPanel->focusedElement && e != NULL);

        if (best == NULL)
        {
            int bestX = 0;
            int bestY = 0;
            int bestH = 0;
            
            e = thisPanel->focusedElement;
            do {
                e = e->getPrevFocusWidget();
                if (e != NULL && e->isFocusable())
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
            } while (e != thisPanel->focusedElement && e != NULL);
        }
        if (best != NULL) {
            thisPanel->setFocus(best);
        }
    }
}


void DialogPanel::Actions::focusUp()
{
    if (thisPanel->focusedElement.isValid())
    {
        GuiWidget* e = thisPanel->focusedElement;
        GuiWidget* best = NULL;
        int maxY = e->getPosition().y;
        int minX = e->getPosition().x;
        int maxX = e->getPosition().x + e->getPosition().w;
        int bestY = -1;
        int bestX = -1;
//printf("\n-----           %d %d %d %d\n", e->getPosition().x, e->getPosition().y, e->getPosition().w, e->getPosition().h);
        do {
            e = e->getPrevFocusWidget();
            if (e != NULL && e->isFocusable())
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
        } while (e != thisPanel->focusedElement && e != NULL);
        if (best != NULL) {
            thisPanel->setFocus(best);
        }
    }
}

void DialogPanel::Actions::focusDown()
{
    if (thisPanel->focusedElement.isValid())
    {
        GuiWidget* e = thisPanel->focusedElement;
        GuiWidget* best = NULL;
        int minY = e->getPosition().y + e->getPosition().h;
        int minX = e->getPosition().x;
        int maxX = e->getPosition().x + e->getPosition().w;
        int bestY = INT_MAX;
        int bestX = INT_MAX;
//printf("\n-----           %d %d %d %d\n", e->getPosition().x, e->getPosition().y, e->getPosition().w, e->getPosition().h);
        do {
            e = e->getNextFocusWidget();
            if (e != NULL && e->isFocusable())
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
        } while (e != thisPanel->focusedElement && e != NULL);
        if (best != NULL) {
            thisPanel->setFocus(best);
        }
    }
}


bool DialogPanel::processHotKey(KeyMapping::Id keyMappingId)
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
            WeakPtr<GuiWidget> w = widgets->getLast();
            if (w.isValid()) {
                w->treatHotKeyEvent(KeyMapping::Id(keyMappingId.getKeyModifier(), keyMappingId.getKeyId()));
                if (focusedElement.isValid() && w != focusedElement) {
                    focusedElement->notifyAboutHotKeyEventForOtherWidget();
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
    GuiWidget*     w = thisPanel->defaultKeyWidgets->getLast();

    if (w != NULL) {
        w->treatHotKeyEvent(defaultKey);
        if (thisPanel->focusedElement.isValid() && w != thisPanel->focusedElement) {
            thisPanel->focusedElement->notifyAboutHotKeyEventForOtherWidget();
        }
    }
}

bool DialogPanel::handleLowPriorityKeyPress(const KeyPressEvent& keyPressEvent)
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
    if (focusedElement.isValid()) {
        processed = focusedElement->handleLowPriorityKeyPress(keyPressEvent);
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

    if (keyPressEvent.getKeyId() != KeyId("Return"))
    {
        processed = processHotKey(KeyMapping::Id(keyPressEvent.getKeyModifier(), keyPressEvent.getKeyId()));
    }
    return processed;
}


GuiElement::ProcessingResult DialogPanel::processKeyboardEvent(const KeyPressEvent& keyPressEvent)
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
        focusedElement->processKeyboardEvent(keyPressEvent);
        processed = true;
    }
    else
    {
        processed = handleHighPriorityKeyPress(keyPressEvent);
    }

    if (!processed) {
        if (focusedElement.isValid()) {
            ProcessingResult rslt = focusedElement->processKeyboardEvent(keyPressEvent);
            if (rslt == EVENT_PROCESSED) {
                processed = true;
            }
        }
    }
    return processed ? EVENT_PROCESSED : NOT_PROCESSED;
}


bool DialogPanel::takesAwayDefaultKey(GuiWidget* widget)
{
    if (widget == NULL) {
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
                    
                    if (widget->hasActionMethod(a)) {
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


void DialogPanel::setFocus(GuiWidget* element)
{
    if (focusedElement != element)
    {
        if (hasFocusFlag)
        {
            if (focusedElement.isValid()) {
                focusedElement->treatFocusOut();
            }
            focusedElement = element;
            if (focusedElement.isValid())
            {
                KeyMapping::Id defaultKey(KeyModifier::NONE, KeyId("Return"));
                
                bool newFocusedElementTakesAwayDefaultKey = takesAwayDefaultKey(focusedElement);
                
                GuiWidget* last = defaultKeyWidgets->getLast();
                if (last != NULL)
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


void DialogPanel::reportMouseClickFrom(GuiWidget* w)
{
    if (!hasFocusFlag) {
        GuiWidget::reportMouseClickFrom(this);
    }
}

void DialogPanel::requestFocusFor(GuiWidget* w)
{
    setFocus(w);
}

void DialogPanel::requestHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w)
{
    ASSERT(w != NULL);
    
    KeyMapping::Id defaultKey(KeyModifier::NONE, KeyId("Return"));
    
    if (id == defaultKey)
    {
        GuiWidget* last = defaultKeyWidgets->getLast();
                          defaultKeyWidgets->append(w);
        if (!focusedElementTakesAwayDefaultKey) {
            if (last != NULL) {
                last->treatLostHotKeyRegistration(id);
            }
            if (hasFocusFlag) {
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
        GuiWidget* activeWidget = widgets->getLast();
        if (hotKeyPredecessor.isInvalid() && activeWidget != NULL) {
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
    if (hasFocusFlag) {
        w->treatNewHotKeyRegistration(id);
    }
    if (hotKeyPredecessor.isValid())
    {
        hotKeyPredecessor->requestHotKeyRegistrationFor(id, w);
    }
}

void DialogPanel::requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w)
{
    ASSERT(w != NULL);

    KeyMapping::Id defaultKey(KeyModifier::NONE, KeyId("Return"));
    
    if (id == defaultKey)
    {
        w->treatLostHotKeyRegistration(id);
        defaultKeyWidgets->removeAll(w);

        if (hasFocusFlag && !focusedElementTakesAwayDefaultKey) {
            GuiWidget* last = defaultKeyWidgets->getLast();
            if (last != NULL) {
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
            GuiWidget* newActive = widgets->getLast();
            if (doIt && newActive != NULL && hasFocusFlag) {
                newActive->treatNewHotKeyRegistration(id);
            }
        }
    }
    if (hotKeyPredecessor.isValid())
    {
        hotKeyPredecessor->requestRemovalOfHotKeyRegistrationFor(id, w);
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


bool DialogPanel::invokeActionMethod(ActionId actionId)
{
    return  (   (focusedElement.isValid() && focusedElement->invokeActionMethod(actionId))
             || GuiWidget::invokeActionMethod(actionId));
}

bool DialogPanel::hasActionMethod(ActionId actionId)
{
    return  (   (focusedElement.isValid() && focusedElement->hasActionMethod(actionId))
             || GuiWidget::hasActionMethod(actionId));
}

