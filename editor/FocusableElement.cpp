/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#include "FocusableElement.hpp"
#include "Nullable.hpp"

using namespace LucED;


FocusableElement::~FocusableElement()
{
    if (nextFocusWidget.isValid() && nextFocusWidget->prevFocusWidget == this) {
        nextFocusWidget->prevFocusWidget = prevFocusWidget;
    }
    if (prevFocusWidget.isValid() && prevFocusWidget->nextFocusWidget == this) {
        prevFocusWidget->nextFocusWidget = nextFocusWidget;
    }
}


void FocusableElement::treatNotificationOfHotKeyEventForOtherWidget()
{}

void FocusableElement::treatLostHotKeyRegistration(const KeyMapping::Id& id)
{}

void FocusableElement::treatNewHotKeyRegistration(const KeyMapping::Id& id)
{}

void FocusableElement::treatHotKeyEvent(const KeyMapping::Id& id)
{}



void FocusableElement::adopt(RawPtr<GuiElement>    parentElement,
                             RawPtr<GuiWidget>     parentWidget,
                             const FocusManagers&  focusManagers)
{
    RawPtr<FocusManager> focusManagerForThis   = focusManagers.getFocusManagerForThis();
    RawPtr<FocusManager> focusManagerForChilds = focusManagers.getFocusManagerForChilds();
                                                 
    if (!focusManagerForChilds.isValid()) {
        focusManagerForChilds = focusManagerForThis;
    }
    FocusManager::Access::QueueTransaction queueTransaction(focusManagerForChilds);

    this->focusManagerForThis = focusManagerForThis;
    
    GuiElement::adopt(parentElement, parentWidget, focusManagerForChilds);
    
    queueTransaction.execute();
}

void FocusableElement::adopt(RawPtr<GuiElement>   parentElement,
                             RawPtr<GuiWidget>    parentWidget,
                             RawPtr<FocusManager> focusManager)
{
    adopt(parentElement, parentWidget, FocusManagers().setFocusManagerForThis  (focusManager)
                                                      .setFocusManagerForChilds(focusManager));
}


void FocusableElement::requestHotKeyRegistration(const KeyMapping::Id& id)
{
    if (wasAdopted() && FocusManager::Access::isReady(focusManagerForThis)) {
        focusManagerForThis->requestHotKeyRegistrationFor(id, this);
    } else {
        FocusManager::Access::queueHotKeyRegistrationFor(id, this);
    }
}

void FocusableElement::requestRemovalOfHotKeyRegistration(const KeyMapping::Id& id)
{
    if (wasAdopted() && FocusManager::Access::isReady(focusManagerForThis)) {
        focusManagerForThis->requestRemovalOfHotKeyRegistrationFor(id, this);
    } else {
        FocusManager::Access::queueRemovalOfHotKeyRegistrationFor(id, this);
    }
}

void FocusableElement::requestFocus()
{
    if (wasAdopted() && FocusManager::Access::isReady(focusManagerForThis)) {
        focusManagerForThis->requestFocusFor(this);
    } else {
        FocusManager::Access::queueFocusFor(this);
    }
}


