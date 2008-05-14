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

#ifndef DIALOG_PANEL_HPP
#define DIALOG_PANEL_HPP

#include "GuiWidget.hpp"
#include "KeyMapping.hpp"
#include "HeapObjectArray.hpp"
#include "WeakPtrQueue.hpp"
#include "Callback.hpp"
#include "HeapHashMap.hpp"

namespace LucED
{

class DialogPanel : public GuiWidget
{
public:
    typedef OwningPtr<DialogPanel> Ptr;
    
    static Ptr create(GuiWidget* parent) {
        return Ptr(new DialogPanel(parent, Callback<DialogPanel*>::Ptr()));
    }
    
    void setHotKeyPredecessor(DialogPanel* hotKeyPredecessor) {
        ASSERT(this->hotKeyMapping->isEmpty());
        this->hotKeyPredecessor = hotKeyPredecessor;
        hotKeyPredecessor->setHotKeySuccessor(this);
    }

    virtual Measures getDesiredMeasures();

    virtual void treatNewWindowPosition(Position newPosition);
    virtual ProcessingResult processEvent(const XEvent* event);
    virtual ProcessingResult processKeyboardEvent(const XEvent* event);

    virtual void treatFocusIn();
    virtual void treatFocusOut();

    virtual void requestHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w);
    virtual void requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w);

    void setRootElement(OwningPtr<GuiElement> rootElement);
    void setFocus(GuiWidget* element);
    virtual void notifyAboutHotKeyEventForOtherWidget();

    virtual void setPosition(Position newPosition);
    
    bool hasFocus() const {
        return hasFocusFlag;
    }
    
protected:
    DialogPanel(GuiWidget* parent, Callback<DialogPanel*>::Ptr requestCloseCallback);
    
    GuiElement* getRootElement() {return rootElement.getRawPtr();}
    
    void switchFocusToNextWidget();
    void switchFocusToPrevWidget();
    virtual void requestFocusFor(GuiWidget* w);
    
    virtual void requestClose();
    
private:
    void setHotKeySuccessor(DialogPanel* hotKeySuccessor) {
        ASSERT(hotKeySuccessor->hotKeyMapping->isEmpty());
        this->hotKeySuccessor = hotKeySuccessor;
    }

    OwningPtr<GuiElement> rootElement;
    bool wasNeverShown;
    
    KeyMapping::Ptr keyMapping1;
    KeyMapping::Ptr keyMapping2;
    
    typedef WeakPtrQueue<GuiWidget> WidgetQueue;
    typedef HeapHashMap< KeyMapping::Id, WidgetQueue::Ptr > HotKeyMapping;
    HotKeyMapping::Ptr hotKeyMapping;
    
    WeakPtr<GuiWidget> focusedElement;
    
    bool hasFocusFlag;
    
    Callback<DialogPanel*>::Ptr requestCloseCallback;
    
    WeakPtr<DialogPanel> hotKeySuccessor;
    WeakPtr<DialogPanel> hotKeyPredecessor;
};

} // namespace LucED

#endif // DIALOG_PANEL_HPP
