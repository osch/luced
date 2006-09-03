/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch@luced.de
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

#ifndef DIALOGWIN_H
#define DIALOGWIN_H

#include "TopWin.h"
#include "KeyMapping.h"
#include "HeapObjectArray.h"
#include "WeakPtrQueue.h"

namespace LucED {


class DialogWin : public TopWin
{
public:
    typedef WeakPtr<DialogWin> Ptr;

    virtual void treatNewWindowPosition(Position newPosition);
    virtual ProcessingResult processEvent(const XEvent *event);
    virtual ProcessingResult processKeyboardEvent(const XEvent *event);

    virtual void show();
    virtual void treatFocusIn();
    virtual void treatFocusOut();

    virtual void requestHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w);
    virtual void requestRemovalOfHotKeyRegistrationFor(const KeyMapping::Id& id, GuiWidget* w);
    
protected:
    DialogWin(TopWin* referingWindow);
    
    void setRootElement(OwningPtr<GuiElement> rootElement);
    GuiElement* getRootElement() {return rootElement.getRawPtr();}
    void setFocus(GuiWidget* element);
    
    void switchFocusToNextWidget();
    void switchFocusToPrevWidget();
    virtual void requestFocusFor(GuiWidget* w);
    
private:
    OwningPtr<GuiElement> rootElement;
    bool wasNeverShown;
    WeakPtr<TopWin> referingWindow;
    
    KeyMapping keyMapping;
    typedef WeakPtrQueue<GuiWidget> WidgetQueue;
    typedef HashMap< KeyMapping::Id, WidgetQueue::Ptr > HotKeyMapping;
    HotKeyMapping hotKeyMapping;
    
    WeakPtr<GuiWidget> focusedElement;
};

} // namespace LucED

#endif // DIALOGWIN_H
