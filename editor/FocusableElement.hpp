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

#ifndef FOCUSABLE_ELEMENT_HPP
#define FOCUSABLE_ELEMENT_HPP

#include "GuiElement.hpp"
#include "GuiWidget.hpp"
#include "KeyActionHandler.hpp"
#include "KeyMapping.hpp"
#include "FocusManager.hpp"

namespace LucED
{

class FocusableElement : public GuiElement
{
public:
    typedef OwningPtr<FocusableElement> Ptr;
    typedef OwningPtr<const FocusableElement> ConstPtr;

    enum FocusType {
        NO_FOCUS,
        NORMAL_FOCUS,
        TOTAL_FOCUS
    };

    bool      hasFocus()     const { return focusFlag; }
    bool      isFocusable()  const { return focusableFlag; }
    FocusType getFocusType() const { return focusType; }

    virtual void treatFocusIn()  = 0;
    virtual void treatFocusOut() = 0;

    virtual void notifyAboutHotKeyEventForOtherWidget();
    virtual void treatLostHotKeyRegistration(const KeyMapping::Id& id);
    virtual void treatNewHotKeyRegistration(const KeyMapping::Id& id);
    virtual void treatHotKeyEvent(const KeyMapping::Id& id);
    
    void setNextFocusWidget(RawPtr<FocusableElement> n) { nextFocusWidget = n; n->prevFocusWidget = this; }

    RawPtr<FocusableElement> getNextFocusWidget() { return nextFocusWidget; }
    RawPtr<FocusableElement> getPrevFocusWidget() { return prevFocusWidget; }
    
    OwningPtr<KeyActionHandler> getKeyActionHandler() const {
        return keyActionHandler;
    }
    
protected:
    FocusableElement(RawPtr<GuiWidget> parent, int x = 0, int y = 0, unsigned int width = 0, unsigned int height = 0)
        : GuiElement(x, y, width, height),
          parent(parent),
          focusManager(parent->getFocusManagerForChildWidgets()),
          focusableFlag(true),
          focusFlag(false),
          focusType(NORMAL_FOCUS)
    {}
            
    virtual ~FocusableElement();

    void setFocusFlag(bool focusFlag) {
        this->focusFlag = focusFlag;
    }
    void setFocusableFlag(bool focusableFlag) {
        this->focusableFlag = focusableFlag;
    }
    void setKeyActionHandler(KeyActionHandler::Ptr keyActionHandler) {
        this->keyActionHandler = keyActionHandler;
    }
    
    void requestHotKeyRegistration(const KeyMapping::Id& id) {
        focusManager->requestHotKeyRegistrationFor(id, this);
    }

    void requestRemovalOfHotKeyRegistration(const KeyMapping::Id& id) {
        focusManager->requestRemovalOfHotKeyRegistrationFor(id, this);
    }

    void requestFocus() {
        focusManager->requestFocusFor(this);
    }
    
    void reportMouseClick() {
        focusManager->reportMouseClickFrom(this);
    }

private:
    RawPtr<GuiWidget>    parent;
    RawPtr<FocusManager> focusManager;

    bool focusableFlag;
    bool focusFlag;
    FocusType focusType;
    WeakPtr<FocusableElement>   nextFocusWidget;
    WeakPtr<FocusableElement>   prevFocusWidget;
    OwningPtr<KeyActionHandler> keyActionHandler;
};

} // namespace LucED

#endif // FOCUSABLE_ELEMENT_HPP
