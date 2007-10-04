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

#ifndef TOPWIN_H
#define TOPWIN_H

#include <iostream>

#include "GuiWidget.hpp"
#include "WeakPtr.hpp"
#include "OwningPtr.hpp"
#include "ObjectArray.hpp"
#include "TopWinOwner.hpp"
#include "CallbackContainer.hpp"

namespace LucED {


class TopWin : public GuiWidget, public TopWinOwner, private TopWinOwnerAccessForTopWin
{
public:
    typedef WeakPtr<TopWin> Ptr;
    
    virtual ~TopWin();

    virtual ProcessingResult processEvent(const XEvent *event);

    virtual void requestFocus();
    virtual void requestCloseWindow();
    virtual void treatNewWindowPosition(Position newPosition) {}
    virtual void treatFocusIn() {}
    virtual void treatFocusOut() {}

    void setTitle(const char* title);
    void setTitle(const String& title) {
        setTitle(title.toCString());
    }
    virtual void raise();
    
    bool isMapped() const {
        return mapped;
    }

    void registerMappingNotifyCallback(Callback<bool>::Ptr mappingNotifyCallback) {
        mappingNotifyCallbacks.registerCallback(mappingNotifyCallback);
    }
    
    bool hasFocus() const {
        return focusFlag;
    }
    
protected:
    TopWin();
    
    void setSizeHints(int minWidth, int minHeight, int dx, int dy);
    void setSizeHints(int x, int y, int minWidth, int minHeight, int dx, int dy);
    
    template<class T> static WeakPtr<T> transferOwnershipTo(T *topWin, TopWinOwner* owner);
    template<class T> static WeakPtr<T> transferOwnershipTo(T *topWin, TopWin::Ptr owner);
    
    virtual void notifyAboutBeingMapped()
    {}
    
    virtual void notifyAboutBeingUnmapped()
    {}
    
private:
    void setWindowManagerHints();
    
    Atom x11InternAtomForDeleteWindow;
//  Atom x11InternAtomForTakeFocus;
    TopWinOwner* owner;
    bool mapped;
    bool requestFocusAfterMapped;
    
    bool focusFlag;
    
    CallbackContainer<bool> mappingNotifyCallbacks;
};



template<class T> WeakPtr<T> TopWin::transferOwnershipTo(T *topWin, TopWinOwner* owner)
{
    OwningPtr<T>  rslt(topWin);
    appendTopWinToTopWinOwner(rslt, owner);
    topWin->owner = owner;
    return rslt;
}

template<class T> WeakPtr<T> TopWin::transferOwnershipTo(T *topWin, TopWin::Ptr owner)
{
    return transferOwnershipTo(topWin, owner.getRawPtr());
}


} // namespace LucED

#endif // TOPWIN_H

