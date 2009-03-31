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

#include "OwnedTopWins.hpp"

#ifndef TOP_WIN_HPP
#define TOP_WIN_HPP

#include "HeapObject.hpp"
#include "GuiWidget.hpp"
#include "WeakPtr.hpp"
#include "OwningPtr.hpp"
#include "ObjectArray.hpp"
#include "CallbackContainer.hpp"

namespace LucED
{

class KeyPressRepeater;

class TopWin : public HeapObject,
               public GuiWidget
{
public:
    typedef WeakPtr<TopWin> Ptr;
    
    enum CloseReason
    {
        CLOSED_BY_USER,
        CLOSED_SILENTLY
    };
    
    virtual ~TopWin();

    virtual ProcessingResult processEvent(const XEvent* event);

    virtual void requestFocus();
    virtual void requestCloseWindow(TopWin::CloseReason reason);

    void requestCloseWindow() {
        requestCloseWindow(CLOSED_SILENTLY);
    }
    Position getPosition() const {
        return position;
    }
    
    virtual void treatNewWindowPosition(Position newPosition);
    virtual void treatFocusIn();
    virtual void treatFocusOut();

    virtual void show();
    virtual void hide();

    bool isVisible() const {
        return isVisibleFlag;
    }
    
    void setTitle(const char* title);
    void setTitle(const String& title) {
        setTitle(title.toCString());
    }
    virtual void raise();
    
    bool isMapped() const {
        return mapped;
    }

    void registerRequestForCloseNotifyCallback(Callback<TopWin*,CloseReason>::Ptr callback) {
        requestForCloseNotifyCallbacks.registerCallback(callback);
    }

    void registerMappingNotifyCallback(Callback<bool>::Ptr mappingNotifyCallback) {
        mappingNotifyCallbacks.registerCallback(mappingNotifyCallback);
    }
    
    bool hasFocus() const {
        return focusFlag;
    }
    
    static void checkTopWinFocus();

    class AccessForKeyPressRepeater
    {
        friend class KeyPressRepeater;
        
        static void repeat(TopWin* topWin, const XEvent* event) {
            topWin->repeatKeyPress(event);
        }
    };
    
    bool isClosing() const {
        return isClosingFlag;
    }

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
    
    virtual void notifyAboutBeingMapped();
    
    virtual void notifyAboutBeingUnmapped();

    virtual ProcessingResult processKeyboardEvent(const KeyPressEvent& keyPressEvent);
    
    void internalRaise();
    
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
    CallbackContainer<TopWin*,CloseReason> requestForCloseNotifyCallbacks;

    OwnedTopWins::Ptr ownedTopWins;
    Atom raiseWindowAtom;
    
    bool shouldRaiseAfterFocusIn;
    bool isClosingFlag;
    Position position;

    bool isVisibleFlag;
};



template
<
    class T, 
    class OwnerPtr
>
WeakPtr<T> TopWin::transferOwnershipTo(T* topWin, OwnerPtr owner)
{
    OwningPtr<T>  rslt(topWin);
    OwnedTopWins::AccessForTopWin::appendTopWinToOwnedTopWins(rslt, owner->getOwnedTopWins());
    topWin->myOwner = owner->getOwnedTopWins();
    return rslt;
}



} // namespace LucED

#endif // TOP_WIN_HPP

