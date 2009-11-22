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
               public GuiWidget::EventListener
{
public:
    typedef WeakPtr<TopWin> Ptr;
    
    enum CloseReason
    {
        CLOSED_BY_USER,
        CLOSED_SILENTLY
    };
    
    virtual ~TopWin();

    virtual void requestFocus();
    virtual void requestCloseWindow(TopWin::CloseReason reason);

    void requestCloseWindow() {
        requestCloseWindow(CLOSED_SILENTLY);
    }
    Position getPosition() const {
        return position;
    }
    virtual void setPosition(const Position& position);
    
    virtual void treatFocusIn();
    virtual void treatFocusOut();

    virtual void show();
    virtual void hide();

    bool isVisible() const {
        return isVisibleFlag;
    }
    
    void setTitle(const String& title);

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
        
        static void repeat(TopWin* topWin, unsigned int keycode, const XEvent* event) {
            topWin->repeatKeyPress(keycode, event);
        }
    };
    
    bool isClosing() const {
        return isClosingFlag;
    }
    
    bool hasAbsolutePosition() const {
        return guiWidget.isValid();
    }
    
    Position getAbsolutePosition() const {
        return guiWidget->getAbsolutePosition();
    }

protected:
    TopWin();

    static Display* getDisplay() { return GuiRoot::getInstance()->getDisplay(); }
    static GuiRoot* getGuiRoot() { return GuiRoot::getInstance(); }
    
    void setSizeHints(int minWidth, int minHeight, int dx, int dy);
    void setSizeHints(int x, int y, int minWidth, int minHeight, int dx, int dy);
    void setSize(int width, int height);
    
    template
    <
        class T, 
        class OwnerPtr
    > 
    static WeakPtr<T> transferOwnershipTo(T* topWin, OwnerPtr owner);
    
    virtual void notifyAboutBeingMapped();
    
    virtual void notifyAboutBeingUnmapped();

    virtual GuiWidget::ProcessingResult processKeyboardEvent(const KeyPressEvent& keyPressEvent);
    
    void createWidget();

    void internalRaise();
    
    void setTransientFor(RawPtr<TopWin> referingTopWin);

    RawPtr<GuiWidget> getGuiWidget() {
        return guiWidget;
    }
    
    void setRootElement(GuiElement::Ptr rootElement);

    void setFocusManager(RawPtr<FocusManager> focusManager);

protected:
    virtual void processGuiWidgetCreatedEvent();

protected: // GuiWidget::EventListener interface implementation
    virtual GuiWidget::ProcessingResult processGuiWidgetEvent(const XEvent* event);
    virtual void                        processGuiWidgetNewPositionEvent(const Position& newPosition);

private:
    static bool hasCurrentX11Focus(TopWin* topWin);

    KeyPressEvent createKeyPressEventObjectFromX11Event(const XEvent* event);

    GuiWidget::ProcessingResult processKeyboardEvent(const XEvent* event) {
        return processKeyboardEvent(createKeyPressEventObjectFromX11Event(event));
    }
    
    void internalTreatFocusOut();
    
    void internalSetSizeHints();
    
    void setWindowManagerHints();
    void handleConfigChanged();
    
    void repeatKeyPress(unsigned int keycode, const XEvent* event);
    
    RawPtr<OwnedTopWins> getOwnedTopWins() {
        return ownedTopWins;
    }
    
    Atom x11InternAtomForDeleteWindow;
//  Atom x11InternAtomForTakeFocus;
    Atom x11InternAtomForUtf8WindowTitle;
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

    GuiWidget::Ptr guiWidget;

    WeakPtr<TopWin> referingTopWin;
    XSizeHints*     sizeHints;
    int             initialWidth;
    int             initialHeight;
    
    OwningPtr<GuiElement> rootElement;
    
    String title;
    
    RawPtr<FocusManager> focusManager;
    
    XIC x11InputContext;
    bool lastKeyPressWasPartOfComposeSequence;
    unsigned int lastKeyCodeOfComposeSequence;
    Time         lastEventTimeOfComposeSequence;
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

