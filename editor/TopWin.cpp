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

#include "headers.hpp"

#ifdef USE_X11_XPM_LIB
#include <X11/xpm.h>
#include <X11/Xutil.h>
#endif


#include "TopWin.hpp"
#include "KeyPressRepeater.hpp"
#include "GlobalConfig.hpp"
#include "SingletonInstance.hpp"
#include "EditorServer.hpp"

using namespace LucED;


TopWin::TopWin()
    : GuiWidget(0, 0, 1, 1, 0),
      ownedTopWins(OwnedTopWins::create()),
      mapped(false),
      requestFocusAfterMapped(false),
      focusFlag(false),
      raiseWindowAtom(XInternAtom(getDisplay(), "_NET_ACTIVE_WINDOW", False)),
      shouldRaiseAfterFocusIn(false),
      isClosingFlag(false)
{
    setWindowManagerHints();

    x11InternAtomForDeleteWindow = XInternAtom(getDisplay(), 
            "WM_DELETE_WINDOW", False);
//    x11InternAtomForTakeFocus = XInternAtom(getDisplay(), 
//            "WM_TAKE_FOCUS", False);

    Atom atoms[] = { x11InternAtomForDeleteWindow, 
                     //x11InternAtomForTakeFocus 
                   };
    XSetWMProtocols(getDisplay(), getWid(), 
                    atoms, sizeof(atoms) / sizeof(Atom));
    
    addToXEventMask(KeyPressMask|KeyReleaseMask|FocusChangeMask|StructureNotifyMask);

    GlobalConfig::getInstance()->registerConfigChangedCallback(newCallback(this, &TopWin::handleConfigChanged));
}

static TopWin* expectedFocusTopWin; // cannot be WeakPtr, because it is used in the destructor

TopWin::~TopWin()
{
    if (this == expectedFocusTopWin) {
        expectedFocusTopWin = NULL;
        if (GlobalConfig::getInstance()->getUseOwnKeyPressRepeater())
        {
            if (KeyPressRepeater::isInstanceValid()) {
                KeyPressRepeater::getInstance()->reset();
            }
            GuiRoot::getInstance()->setKeyboardAutoRepeatOriginal();
        }
    }
}

static inline bool hasCurrentX11Focus(TopWin* topWin)
{
    Window focusWidget;
    int    revertToReturn;
    
    XGetInputFocus(GuiRoot::getInstance()->getDisplay(), 
                   &focusWidget,
                   &revertToReturn);
                   
    return (focusWidget == topWin->getWid());
}


void TopWin::checkTopWinFocus()
{
    if (expectedFocusTopWin != NULL)
    {
        if (!hasCurrentX11Focus(expectedFocusTopWin))
        {
#ifdef DEBUG
fprintf(stderr, "****************** expected FocusTopWin mismatch\n");
#endif
            if (expectedFocusTopWin->focusFlag) {
                expectedFocusTopWin->focusFlag = false;
                expectedFocusTopWin->treatFocusOut();
            }
            expectedFocusTopWin = NULL;
        }
    }
}


void TopWin::setSizeHints(int minWidth, int minHeight, int dx, int dy)
{
    XSizeHints* hints = XAllocSizeHints();
    hints->flags = PMinSize|PResizeInc;
    hints->min_width  = minWidth;
    hints->min_height = minHeight;
    hints->width_inc  = dx;
    hints->height_inc = dy;
    XSetWMNormalHints(getDisplay(), getWid(), hints);
    XFree(hints);
}

void TopWin::setSizeHints(int x, int y, int minWidth, int minHeight, int dx, int dy)
{
    XSizeHints* hints = XAllocSizeHints();
    hints->flags = USPosition|PMinSize|PResizeInc;
    hints->x = x;
    hints->y = y;
    hints->min_width  = minWidth;
    hints->min_height = minHeight;
    hints->width_inc  = dx;
    hints->height_inc = dy;
    XSetWMNormalHints(getDisplay(), getWid(), hints);
    XFree(hints);
}

void TopWin::requestFocus()
{
    if (!mapped) {
        this->show();
        this->requestFocusAfterMapped = true;
    } else {
        XSetInputFocus(getDisplay(), getWid(), RevertToNone, CurrentTime);
    }
}

void TopWin::repeatKeyPress(const XEvent* event)
{
    KeyPressRepeater::getInstance()->triggerNextRepeatEventFor(event, this);
    processKeyboardEvent(event);
}


GuiElement::ProcessingResult TopWin::processEvent(const XEvent* event)
{
    if (GuiWidget::processEvent(event) == EVENT_PROCESSED)
    {
        return EVENT_PROCESSED;
    } 
    else
    {
        switch (event->type)
        {
            case MapNotify: {
                if (event->xmap.window == getWid()) {
                    mapped = true;
                    if (requestFocusAfterMapped) {
                        XSetInputFocus(getDisplay(), getWid(), RevertToNone, CurrentTime);
                        requestFocusAfterMapped = false;
                    }
                    notifyAboutBeingMapped();
                    mappingNotifyCallbacks.invokeAllCallbacks(true);
                    return EVENT_PROCESSED;
                } else {
                    return NOT_PROCESSED;
                }
            }

            case UnmapNotify: {
                if (event->xunmap.window == getWid()) {
                    mapped = false;
                    notifyAboutBeingUnmapped();
                    mappingNotifyCallbacks.invokeAllCallbacks(false);
                    return EVENT_PROCESSED;
                } else {
                    return NOT_PROCESSED;
                }
            }

            case ClientMessage: {
                if (event->xclient.data.l[0] == this->x11InternAtomForDeleteWindow) {
                    this->requestCloseWindow(TopWin::CLOSED_BY_USER);
                    return EVENT_PROCESSED;
/*                } else if (event->xclient.data.l[0] == this->x11InternAtomForTakeFocus) {
printf("TakeFocus\n");
                    long timestamp = event->xclient.data.l[1];
                    bool focusInFromExternal = (expectedFocusTopWin == NULL);
                    XSetInputFocus(getDisplay(), getWid(), RevertToNone, timestamp);
                    return true;
*/
                } else {
                    return NOT_PROCESSED;
                }   
            }
            
            case KeyPress: {
                if (GlobalConfig::getInstance()->getUseOwnKeyPressRepeater())
                {
                    if (KeyPressRepeater::getInstance()->isRepeating())
                    {
                        if (KeyPressRepeater::getInstance()->addKeyModifier(event)) {
                            return EVENT_PROCESSED;
                        }
                        if (KeyPressRepeater::getInstance()->isRepeatingEvent(event)) {
                            return EVENT_PROCESSED;
                        }
                        KeyPressRepeater::getInstance()->triggerNextRepeatEventFor(event, this);
                        ProcessingResult processed = processKeyboardEvent(event);
                        return processed;
                    }
                    else {
                        KeyPressRepeater::getInstance()->triggerNextRepeatEventFor(event, this);
                        ProcessingResult processed = processKeyboardEvent(event);
                        return processed;
                    }
                }
                else
                {
                    return processKeyboardEvent(event);
                }
            
/*                if (KeyPressRepeater::getInstance()->isRepeating())
                {
                    if (KeyPressRepeater::getInstance()->isRepeatingEvent(event)) {
                        bool processed = processKeyboardEvent(event);
                        KeyPressRepeater::getInstance()->repeatEvent(event);
                        return processed;
                    }
                    else {
                        KeyPressRepeater::getInstance()->addKeyModifier(event);
                        return true;
                    }
                }
                else {
                    bool processed = processKeyboardEvent(event);
                    KeyPressRepeater::getInstance()->repeatEvent(event);
                    return processed;
                }
*/            
            }
            
            case KeyRelease: {
                if (GlobalConfig::getInstance()->getUseOwnKeyPressRepeater())
                {
                    bool isTrueRelease = true;
                    
                    if (   !GuiRoot::getInstance()->hasDetectableAutorepeat()
                        && XEventsQueued(getDisplay(), QueuedAlready) > 0)
                    {
                        XEvent nextEvent;
                        XPeekEvent(getDisplay(), &nextEvent);
                        if (   nextEvent.type == KeyPress
                            && nextEvent.xkey.keycode == event->xkey.keycode
                            && nextEvent.xkey.time    == event->xkey.time)
                        {
                            isTrueRelease = false;
                        }
                    }
                    if (isTrueRelease)
                    {
                        if (KeyPressRepeater::getInstance()->isRepeatingEvent(event)) {
                            KeyPressRepeater::getInstance()->reset();
                        }
                        if (KeyPressRepeater::getInstance()->isRepeating()) {
                           KeyPressRepeater::getInstance()->removeKeyModifier(event);
                        }
                    }
                }
                return EVENT_PROCESSED;
            }

            case FocusOut:
            {
                if (GlobalConfig::getInstance()->getUseOwnKeyPressRepeater())
                {
                    KeyPressRepeater::getInstance()->reset();
                    GuiRoot::getInstance()->setKeyboardAutoRepeatOriginal();
                }
                if (focusFlag) {
                    focusFlag = false;
                    treatFocusOut();
                }
                if (expectedFocusTopWin == this)
                {
                    expectedFocusTopWin = NULL;
                }
                else if (expectedFocusTopWin != NULL && !hasCurrentX11Focus(expectedFocusTopWin))
                {
                    // I don't understand, why this can happen: 
                    // but without this workaround sometimes a window
                    // in the background still has a blinking cursor
                    // this also happens with NEdit, perhaps a bug in
                    // some window manager
                    
                    if (expectedFocusTopWin->focusFlag) {
                        expectedFocusTopWin->focusFlag = false;
                        expectedFocusTopWin->treatFocusOut();
                    }
                    expectedFocusTopWin = NULL;
                }
                return EVENT_PROCESSED;
            }

            case FocusIn:
            {
                if (GlobalConfig::getInstance()->getUseOwnKeyPressRepeater())
                {
                    KeyPressRepeater::getInstance()->reset();
#if 0
                    if (!GuiRoot::getInstance()->hasDetectableAutorepeat()) {
                        GuiRoot::getInstance()->setKeyboardAutoRepeatOff();
                    }
#endif
                }
                if (expectedFocusTopWin != NULL && expectedFocusTopWin != this)
                {
                    // There was no FocusOut for the focus top win
                    // this sometimes happens, perhaps a bug in
                    // the window manager, but I'm not sure.
                    
                    if (expectedFocusTopWin->focusFlag) {
                        expectedFocusTopWin->focusFlag = false;
                        expectedFocusTopWin->treatFocusOut();
                    }
                }
                if (shouldRaiseAfterFocusIn) {
                    shouldRaiseAfterFocusIn = false;
                    internalRaise();
                }
                expectedFocusTopWin = this;
                if (!focusFlag) {
                    focusFlag = true;
                    treatFocusIn();
                }
                return EVENT_PROCESSED;
            }


            default: {
                return NOT_PROCESSED;
            }
        }
    }
}

void TopWin::handleConfigChanged()
{
    if (focusFlag)
    {
        if (GlobalConfig::getInstance()->getUseOwnKeyPressRepeater())
        {
#if 0
            if (!GuiRoot::getInstance()->hasDetectableAutorepeat()) {
                GuiRoot::getInstance()->setKeyboardAutoRepeatOff();
            }
#endif
        }
        else
        {
            KeyPressRepeater::getInstance()->reset();
            GuiRoot::getInstance()->setKeyboardAutoRepeatOriginal();
        }
    }
}

void TopWin::setTitle(const char* title)
{
    XStoreName(getDisplay(), getWid(), title);
}
   

#ifdef USE_X11_XPM_LIB
    #include "luced.xpm"
#endif

class TopWinSingletonData : public HeapObject
{
public:
    static TopWinSingletonData* getInstance() { return instance.getPtr(); }
    Pixmap getPixmap() { return pixMap; }
private:
    friend class SingletonInstance<TopWinSingletonData>;
    
    TopWinSingletonData()
    {
        pixMap = 0;

#ifdef USE_X11_XPM_LIB
        if (XpmCreatePixmapFromData(GuiRoot::getInstance()->getDisplay(),
                                    GuiRoot::getInstance()->getRootWid(), 
                                    const_cast<char**>(luced_xpm), 
                                    &pixMap, 
                                    NULL, 
                                    NULL) != 0)
        {
            pixMap = 0;
        }
#endif
        if (GuiRoot::getInstance()->hasXkbExtension()) {
            GuiRoot::getInstance()->setDetectableAutorepeat(true);
        }
    }    

    ~TopWinSingletonData()
    {
        if (pixMap != 0) {
            XFreePixmap(GuiRoot::getInstance()->getDisplay(), pixMap);
        }
    }

    static SingletonInstance<TopWinSingletonData> instance;
    Pixmap pixMap;
};

SingletonInstance<TopWinSingletonData> TopWinSingletonData::instance;



void TopWin::setWindowManagerHints()
{
    Pixmap pixMap = TopWinSingletonData::getInstance()->getPixmap();
    
    if (pixMap != 0)
    {
        XWMHints* hints = XAllocWMHints();
        
        if (hints != NULL) {
            hints->flags  = IconPixmapHint|InputHint;// | IconMaskHint ;//| IconWindowHint;
            hints->icon_pixmap = pixMap;
            hints->input = True;
            XSetWMHints(getDisplay(), getWid(), hints);
            
            XFree(hints);
        }        
    }
    XClassHint* classHint = XAllocClassHint();

    if (classHint != NULL)
    {
        String resName;

        String instanceName = EditorServer::getInstance()->getInstanceName();
        if (instanceName.getLength() > 0) {
            resName = instanceName;
        } else {
            const char* envString = ::getenv("RESOURCE_NAME");
            if (envString != NULL) {
                resName = envString;
            } else {
                String programName = EditorServer::getInstance()->getProgramName();
                resName = programName;
            }
        }
        classHint->res_name  = const_cast<char*>(resName.toCString());
        classHint->res_class = const_cast<char*>("LucED");
        XSetClassHint(getDisplay(), getWid(), classHint);
        XFree(classHint);
    }
}

void TopWin::requestCloseWindow(TopWin::CloseReason reason)
{
    this->isClosingFlag = true;
    myOwner->requestCloseChildWindow(this);
    requestForCloseNotifyCallbacks.invokeAllCallbacks(this, reason);
#if 0
    if (   GuiRoot::getInstance()->getX11ServerVendorString().startsWith("Hummingbird")
        && GuiRoot::getInstance()->getX11ServerVendorRelease() == 6100
        && expectedFocusTopWin != NULL
        && expectedFocusTopWin != this
        && reason == CLOSED_SILENTLY)
    {
        // vendor <Hummingbird Communications Ltd.> <6100>
        this->internalRaise(); // strange workaround for exceed
        expectedFocusTopWin->internalRaise();
    }
#endif
}


void TopWin::raise()
{
    if (   GuiRoot::getInstance()->getX11ServerVendorString().startsWith("Hummingbird")
        && GuiRoot::getInstance()->getX11ServerVendorRelease() == 6100)
    {
        // vendor <Hummingbird Communications Ltd.> <6100>
        this->shouldRaiseAfterFocusIn = true; // delayed raise is Workaround for Exceed X11-Server
    }
    internalRaise();
}

void TopWin::internalRaise()
{
    this->requestFocus();

    {
        // This is needed to raise the Window under Gnome-Desktop or KDE with
        // Focus Stealing Prevention Level set to "normal".
        // (see http://standards.freedesktop.org/wm-spec/wm-spec-latest.html)
        
        XEvent event;
        
        memset(&event, 0, sizeof(event));
        event.xclient.type = ClientMessage;
        //unsigned long serial;     /* # of last request processed by server */
        event.xclient.send_event = true;        /* true if this came from a SendEvent request */
        event.xclient.display = getDisplay();       /* Display the event was read from */
        event.xclient.window = getWid();
        event.xclient.message_type = raiseWindowAtom;
        event.xclient.format = 32;
        event.xclient.data.l[0] = 1;
        event.xclient.data.l[1] = EventDispatcher::getInstance()->getLastX11Timestamp();
        event.xclient.data.l[2] = 0;

        XSendEvent(getDisplay(), getRootWid(), false, 
                                               SubstructureNotifyMask|SubstructureRedirectMask,
                                               &event);
    }

    // older Windowmanager only need XRaiseWindow
        
    XRaiseWindow(getGuiRoot()->getDisplay(), this->getWid());
}

