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

#ifndef TOP_WIN_HPP
#define TOP_WIN_HPP

#include <iostream>

#include "GuiWidget.hpp"
#include "WeakPtr.hpp"
#include "OwningPtr.hpp"
#include "ObjectArray.hpp"
#include "OwnedTopWins.hpp"
#include "CallbackContainer.hpp"

namespace LucED
{

class KeyPressRepeater;

class TopWin : public GuiWidget, private OwnedTopWinsAccessForTopWin
{
public:
    typedef WeakPtr<TopWin> Ptr;
    
    virtual ~TopWin();

    virtual ProcessingResult processEvent(const XEvent* event);

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

    void registerRequestForCloseNotifyCallback(Callback<TopWin*>::Ptr callback) {
        requestForCloseNotifyCallbacks.registerCallback(callback);
    }

    void registerMappingNotifyCallback(Callback<bool>::Ptr mappingNotifyCallback) {
        mappingNotifyCallbacks.registerCallback(mappingNotifyCallback);
    }
    
    bool hasFocus() const {
        return focusFlag;
    }
    
    static void checkTopWinFocus();

    class KeyPressRepeaterAccess
    {
    private:
        friend class KeyPressRepeater;
        
        static void repeat(TopWin* topWin, const XEvent* event) {
            topWin->repeatKeyPress(event);
        }
    };
    
protected:
    TopWin();
    
    void setSizeHints(int minWidth, int minHeight, int dx, int dy);
    void setSizeHints(int x, int y, int minWidth, int minHeight, int dx, int dy);
    
    template
    <
        class T, 
        class OwnerPtr
    > 
    static WeakPtr<T> transferOwnershipTo(T* topWin, OwnerPtr owner);
    
    virtual void notifyAboutBeingMapped()
    {}
    
    virtual void notifyAboutBeingUnmapped()
    {}

private:
    void setWindowManagerHints();
    void handleConfigChanged();
    
    void repeatKeyPress(const XEvent* event);
    
    RawPtr<OwnedTopWins> getOwnedTopWins() {
        return ownedTopWins;
    }
    
    Atom x11InternAtomForDeleteWindow;
//  Atom x11InternAtomForTakeFocus;
    RawPtr<OwnedTopWins> myOwner;
    bool mapped;
    bool requestFocusAfterMapped;
    
    bool focusFlag;
    
    CallbackContainer<bool> mappingNotifyCallbacks;
    CallbackContainer<TopWin*> requestForCloseNotifyCallbacks;

    OwnedTopWins::Ptr ownedTopWins;
};



template
<
    class T, 
    class OwnerPtr
>
WeakPtr<T> TopWin::transferOwnershipTo(T* topWin, OwnerPtr owner)
{
    OwningPtr<T>  rslt(topWin);
    OwnedTopWinsAccessForTopWin::appendTopWinToOwnedTopWins(rslt, owner->getOwnedTopWins());
    topWin->myOwner = owner->getOwnedTopWins();
    return rslt;
}



} // namespace LucED

#endif // TOP_WIN_HPP

