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
#include "ActionMethodBinding.hpp"
#include "FocusableWidget.hpp"
                    
namespace LucED
{

class DialogPanel : public  FocusableWidget,
                    public  FocusManager
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

    virtual void treatFocusIn();
    virtual void treatFocusOut();

    void setRootElement(OwningPtr<GuiElement> rootElement);
    void setFocus(RawPtr<FocusableElement> element);
    virtual void notifyAboutHotKeyEventForOtherWidget();

    virtual void setPosition(Position newPosition);
    
    
protected:
    DialogPanel(RawPtr<GuiWidget> parent, Callback<DialogPanel*>::Ptr requestCloseCallback);
    
    RawPtr<GuiElement> getRootElement() { return rootElement; }
    
    virtual void reportMouseClickFrom(RawPtr<FocusableElement> w);
    
    virtual void requestClose();

private: // FocusManager
    virtual void requestHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w);
    virtual void requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, RawPtr<FocusableElement> w);
    virtual void requestFocusFor(RawPtr<FocusableElement> w);
    
private:
    class MyKeyActionHandler;
    
    class Actions : public ActionMethodBinding<Actions>
    {
    public:
        typedef OwningPtr<Actions> Ptr;

        static Ptr create(RawPtr<DialogPanel> thisPanel) {
            return Ptr(new Actions(thisPanel));
        }

        void focusNext();
        void focusPrevious();

        void focusRight();
        void focusLeft();

        void focusUp();        
        void focusDown();        

        void pressDefaultButton();

    private:
        
        Actions(RawPtr<DialogPanel> thisPanel)
            : ActionMethodBinding<Actions>(this),
              thisPanel(thisPanel)
        {}
        
        RawPtr<DialogPanel> thisPanel;
    };
    friend class ActionMethodBinding<Actions>;

    bool handleHighPriorityKeyPress(const KeyPressEvent& keyPressEvent);
    bool handleLowPriorityKeyPress(const KeyPressEvent& keyPressEvent);
    
    void setHotKeySuccessor(DialogPanel* hotKeySuccessor) {
        ASSERT(hotKeySuccessor->hotKeyMapping->isEmpty());
        this->hotKeySuccessor = hotKeySuccessor;
    }
    
    bool takesAwayDefaultKey(RawPtr<FocusableElement> widget);
    
    bool processHotKey(KeyMapping::Id keyMappingId);

    OwningPtr<GuiElement> rootElement;
    bool wasNeverShown;
    
    typedef WeakPtrQueue<FocusableElement> WidgetQueue;
    typedef HeapHashMap< KeyMapping::Id, WidgetQueue::Ptr > HotKeyMapping;
    HotKeyMapping::Ptr hotKeyMapping;
    
    WeakPtr<FocusableElement> focusedElement;
    bool                     focusedElementTakesAwayDefaultKey;
    WidgetQueue::Ptr         defaultKeyWidgets;
    
    Callback<DialogPanel*>::Ptr requestCloseCallback;
    
    WeakPtr<DialogPanel> hotKeySuccessor;
    WeakPtr<DialogPanel> hotKeyPredecessor;
    
};

} // namespace LucED

#endif // DIALOG_PANEL_HPP
