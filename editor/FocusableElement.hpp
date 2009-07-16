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
    typedef WeakPtr<FocusableElement> Ptr;
    typedef WeakPtr<const FocusableElement> ConstPtr;

    enum FocusType {
        NO_FOCUS,
        NORMAL_FOCUS,
        TOTAL_FOCUS
    };

    bool      hasFocus()     const { return focusFlag; }
    bool      isFocusable()  const { return focusableFlag; }
    FocusType getFocusType() const { return focusType; }

    virtual void treatFocusIn() {
        this->focusFlag = true;
    }
    virtual void treatFocusOut() {
        this->focusFlag = false;
    }

    virtual void treatNotificationOfHotKeyEventForOtherWidget();
    virtual void treatLostHotKeyRegistration(const KeyMapping::Id& id);
    virtual void treatNewHotKeyRegistration(const KeyMapping::Id& id);
    virtual void treatHotKeyEvent(const KeyMapping::Id& id);
    
    void setNextFocusWidget(RawPtr<FocusableElement> n) { nextFocusWidget = n; n->prevFocusWidget = this; }

    RawPtr<FocusableElement> getNextFocusWidget() { return nextFocusWidget; }
    RawPtr<FocusableElement> getPrevFocusWidget() { return prevFocusWidget; }
    
    OwningPtr<KeyActionHandler> getKeyActionHandler() const {
        return keyActionHandler;
    }
    
    bool hasThisFocusManager(RawPtr<FocusManager> rhs) const {
        return focusManagerForThis == rhs;
    }
    
    class FocusManagers
    {
    public:
        FocusManagers()
        {}
        FocusManagers& setFocusManagerForThis(RawPtr<FocusManager> focusManagerForThis) {
            this->focusManagerForThis = focusManagerForThis;
            return *this;
        }
        FocusManagers& setFocusManagerForChilds(RawPtr<FocusManager> focusManagerForChilds) {
            this->focusManagerForChilds = focusManagerForChilds;
            return *this;
        }
        RawPtr<FocusManager> getFocusManagerForThis() const {
            return focusManagerForThis;
        }
        RawPtr<FocusManager> getFocusManagerForChilds() const {
            return focusManagerForChilds;
        }
    private:
        RawPtr<FocusManager> focusManagerForThis;
        RawPtr<FocusManager> focusManagerForChilds;
    };


    virtual void adopt(RawPtr<GuiElement>    parentElement,
                       RawPtr<GuiWidget>     parentWidget,
                       const FocusManagers&  focusManagers);

    virtual void adopt(RawPtr<GuiElement>   parentElement,
                       RawPtr<GuiWidget>    parentWidget,
                       RawPtr<FocusManager> focusManager);

protected:
    FocusableElement(Visibility defaultVisibility = VISIBLE, int x = 0, int y = 0, int width = 0, int height = 0)
        : GuiElement(defaultVisibility, x, y, width, height),
          focusableFlag(true),
          focusFlag(false),
          focusType(NORMAL_FOCUS)
    {}
            
    virtual ~FocusableElement();

    void setFocusableFlag(bool focusableFlag) {
        this->focusableFlag = focusableFlag;
    }
    void setKeyActionHandler(KeyActionHandler::Ptr keyActionHandler) {
        this->keyActionHandler = keyActionHandler;
    }
    
    void requestHotKeyRegistration(const KeyMapping::Id& id);

    void requestRemovalOfHotKeyRegistration(const KeyMapping::Id& id);

    void requestFocus();
    
    void reportMouseClick() {
        focusManagerForThis->reportMouseClickFrom(this);
    }

private:
    class RequestQueue;
    
    RawPtr<FocusManager> focusManagerForThis;

    bool focusableFlag;
    bool focusFlag;
    FocusType focusType;
    WeakPtr<FocusableElement>   nextFocusWidget;
    WeakPtr<FocusableElement>   prevFocusWidget;
    OwningPtr<KeyActionHandler> keyActionHandler;
};

} // namespace LucED

#endif // FOCUSABLE_ELEMENT_HPP
