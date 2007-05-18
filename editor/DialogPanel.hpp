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

#ifndef DIALOGPANEL_H
#define DIALOGPANEL_H

#include "GuiWidget.hpp"
#include "KeyMapping.hpp"
#include "HeapObjectArray.hpp"
#include "WeakPtrQueue.hpp"

namespace LucED {


class DialogPanel : public GuiWidget
{
public:
    typedef OwningPtr<DialogPanel> Ptr;
    
    static Ptr create(GuiWidget* parent) {
        return Ptr(new DialogPanel(parent));
    }

    virtual Measures getDesiredMeasures();

    virtual void treatNewWindowPosition(Position newPosition);
    virtual ProcessingResult processEvent(const XEvent *event);
    virtual ProcessingResult processKeyboardEvent(const XEvent *event);

    virtual void treatFocusIn();
    virtual void treatFocusOut();

    virtual void requestHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w);
    virtual void requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w);

    void setRootElement(OwningPtr<GuiElement> rootElement);
    void setFocus(GuiWidget* element);
    virtual void notifyAboutHotKeyEventForOtherWidget();
    
protected:
    DialogPanel(GuiWidget* parent);
    
    GuiElement* getRootElement() {return rootElement.getRawPtr();}
    
    void switchFocusToNextWidget();
    void switchFocusToPrevWidget();
    virtual void requestFocusFor(GuiWidget* w);
    
private:
    OwningPtr<GuiElement> rootElement;
    bool wasNeverShown;
    
    KeyMapping keyMapping1;
    KeyMapping keyMapping2;
    typedef WeakPtrQueue<GuiWidget> WidgetQueue;
    typedef HashMap< KeyMapping::Id, WidgetQueue::Ptr > HotKeyMapping;
    HotKeyMapping hotKeyMapping;
    
    WeakPtr<GuiWidget> focusedElement;
    
    bool hasFocus;
};

} // namespace LucED

#endif // DIALOGPANEL_H
