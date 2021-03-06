/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2010 Oliver Schmidt, oliver at luced dot de
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

#include "config.h"
#include "headers.hpp"

#if LUCED_USE_XPM
#  include <X11/xpm.h>
#  include <X11/Xutil.h>
#endif

#include <stdlib.h>

#include "TopWin.hpp"
#include "KeyPressRepeater.hpp"
#include "GlobalConfig.hpp"
#include "SingletonInstance.hpp"
#include "File.hpp"
#include "ProgramName.hpp"
#include "EncodingConverter.hpp"


#define KEYSYM2UCS_INCLUDED
#include "keysym2ucs.c"

using namespace LucED;


TopWin::TopWin()
    : ownedTopWins(OwnedTopWins::create()),
      mapped(false),
      requestFocusAfterMapped(false),
      raiseAfterMapped(false),
      focusFlag(false),
      shouldRaiseAfterFocusIn(false),
      isClosingFlag(false),
      position(0,0,1,1),
      isVisibleFlag(false),
      sizeHints(NULL),
      initialWidth(-1),
      initialHeight(-1),
      x11InputContext(NULL),
      lastKeyPressWasPartOfComposeSequence(false)
{
    GlobalConfig::getInstance()->registerConfigChangedCallback(newCallback(this, &TopWin::handleConfigChanged));
}

void TopWin::createWidget()
{
    ASSERT(!guiWidget.isValid());

    guiWidget = GuiWidget::create(Null, this, getPosition());
    
    raiseWindowAtom = XInternAtom(guiWidget->getDisplay(), "_NET_ACTIVE_WINDOW", False),

    x11InternAtomForDeleteWindow = XInternAtom(GuiRoot::getInstance()->getDisplay(), 
            "WM_DELETE_WINDOW", False);
//    x11InternAtomForTakeFocus = XInternAtom(getDisplay(), 
//            "WM_TAKE_FOCUS", False);

    x11InternAtomForUtf8WindowTitle = XInternAtom(guiWidget->getDisplay(), "_NET_WM_NAME", False);
                        
    setTitle(title);

    Atom atoms[] = { x11InternAtomForDeleteWindow, 
                     //x11InternAtomForTakeFocus 
                   };
    XSetWMProtocols(guiWidget->getDisplay(), guiWidget->getWid(), 
                    atoms, sizeof(atoms) / sizeof(Atom));
    
    guiWidget->addToXEventMask(KeyPressMask|KeyReleaseMask|FocusChangeMask|StructureNotifyMask);
    setTransientFor(referingTopWin);
    
    if (sizeHints != NULL) {
        internalSetSizeHints();
    }
    if (initialWidth != -1 && initialHeight != -1) {
        XResizeWindow(guiWidget->getDisplay(), guiWidget->getWid(), initialWidth, initialHeight);
    }
    
    setWindowManagerHints();

    XIM x11InputMethod = GuiRoot::getInstance()->getInputMethod();
    if (x11InputMethod != NULL) {
        x11InputContext =  XCreateIC(x11InputMethod,
                                     XNInputStyle,   XIMPreeditNothing|XIMStatusNothing,
                                     XNClientWindow, GuiRoot::getInstance()->getRootWid().toX11Type(),
                                     XNFocusWindow,  guiWidget->getWid().toX11Type(),
                                     NULL);
    }
    processGuiWidgetCreatedEvent();
}

void TopWin::internalSetSizeHints()
{
    ASSERT(sizeHints != NULL);

    XSetWMNormalHints(guiWidget->getDisplay(), guiWidget->getWid(), sizeHints);
    XFree(sizeHints);
    sizeHints = NULL;
}

static TopWin* expectedFocusTopWin; // cannot be WeakPtr, because it is used in the destructor

TopWin::~TopWin()
{
    if (x11InputContext != NULL) {
        XDestroyIC(x11InputContext);
    }
    if (sizeHints != NULL) {
        XFree(sizeHints);
        sizeHints = NULL;
    }
    if (this == expectedFocusTopWin) {
        expectedFocusTopWin = NULL;
        if (GlobalConfig::getConfigData()->getGeneralConfig()->getUseOwnKeyPressRepeater())
        {
            if (KeyPressRepeater::isInstanceValid()) {
                KeyPressRepeater::getInstance()->reset();
            }
            GuiRoot::getInstance()->setKeyboardAutoRepeatOriginal();
        }
    }
}

inline bool TopWin::hasCurrentX11Focus(TopWin* topWin)
{
    Window focusWidget;
    int    revertToReturn;
    
    XGetInputFocus(topWin->guiWidget->getDisplay(), 
                   &focusWidget,
                   &revertToReturn);
                   
    return (focusWidget == topWin->guiWidget->getWid());
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
                expectedFocusTopWin->internalTreatFocusOut();
            }
            expectedFocusTopWin = NULL;
        }
    }
}


void TopWin::setSizeHints(int minWidth, int minHeight, int dx, int dy)
{
    if (sizeHints == NULL) {
        sizeHints = XAllocSizeHints();
    }
    sizeHints->flags = PMinSize|PResizeInc;
    sizeHints->min_width  = minWidth;
    sizeHints->min_height = minHeight;
    sizeHints->width_inc  = dx;
    sizeHints->height_inc = dy;
    if (guiWidget.isValid()) {
        internalSetSizeHints();
    }
}

void TopWin::setSizeHints(int x, int y, int minWidth, int minHeight, int dx, int dy)
{
    if (sizeHints == NULL) {
        sizeHints = XAllocSizeHints();
    }
    sizeHints->flags = USPosition|PMinSize|PResizeInc;
    sizeHints->x = x;
    sizeHints->y = y;
    sizeHints->min_width  = minWidth;
    sizeHints->min_height = minHeight;
    sizeHints->width_inc  = dx;
    sizeHints->height_inc = dy;
    if (guiWidget.isValid()) {
        internalSetSizeHints();
    }
}

void TopWin::setSize(int width, int height)
{
    if (guiWidget.isValid()) {
        XResizeWindow(guiWidget->getDisplay(), guiWidget->getWid(), width, height);
    } else {
        this->initialWidth = width;
        this->initialHeight = height;
    }
}


void TopWin::requestFocus()
{
    if (!mapped) {
        this->show();
        this->requestFocusAfterMapped = true;
    } else {
        XSetInputFocus(guiWidget->getDisplay(), guiWidget->getWid(), RevertToNone, EventDispatcher::getInstance()->getLastX11Timestamp());
    }
}

void TopWin::repeatKeyPress(unsigned int keycode, const XEvent* event)
{
    KeyPressRepeater::getInstance()->triggerNextRepeatEventFor(keycode, event, this);
    
    processKeyboardEvent(event);
    
    XSync(GuiRoot::getInstance()->getDisplay(), False); // prevent too fast key press event for slow XServer
}


GuiWidget::ProcessingResult TopWin::processGuiWidgetEvent(const XEvent* event)
{
    switch (event->type)
    {
        case MapNotify: {
            if (event->xmap.window == guiWidget->getWid()) {
                mapped = true;
                if (requestFocusAfterMapped) {
                    XSetInputFocus(guiWidget->getDisplay(), guiWidget->getWid(), RevertToNone, EventDispatcher::getInstance()->getLastX11Timestamp());
                    requestFocusAfterMapped = false;
                }
                if (raiseAfterMapped) {
                    raiseAfterMapped = false;
                    raise();
                }
                notifyAboutBeingMapped();
                mappingNotifyCallbacks.invokeAllCallbacks(true);
                return GuiWidget::EVENT_PROCESSED;
            } else {
                return GuiWidget::NOT_PROCESSED;
            }
        }

        case UnmapNotify: {
            if (event->xunmap.window == guiWidget->getWid()) {
                mapped = false;
                notifyAboutBeingUnmapped();
                mappingNotifyCallbacks.invokeAllCallbacks(false);
                return GuiWidget::EVENT_PROCESSED;
            } else {
                return GuiWidget::NOT_PROCESSED;
            }
        }

        case ClientMessage: {
            if (event->xclient.data.l[0] == this->x11InternAtomForDeleteWindow) {
                this->requestCloseWindow(TopWin::CLOSED_BY_USER);
                return GuiWidget::EVENT_PROCESSED;
/*                } else if (event->xclient.data.l[0] == this->x11InternAtomForTakeFocus) {
printf("TakeFocus\n");
                long timestamp = event->xclient.data.l[1];
                bool focusInFromExternal = (expectedFocusTopWin == NULL);
                XSetInputFocus(getDisplay(), getWid(), RevertToNone, timestamp);
                return true;
*/
            } else {
                return GuiWidget::NOT_PROCESSED;
            }   
        }
        
        case KeyPress: {
            GuiWidget::ProcessingResult rslt = GuiWidget::EVENT_PROCESSED;
            
            XEvent filteredEvent = *event;
            bool ignoreEvent = XFilterEvent(&filteredEvent, None); // ignore for input of composed characters 
            
            if (GlobalConfig::getConfigData()->getGeneralConfig()->getUseOwnKeyPressRepeater())
            {
                if (KeyPressRepeater::getInstance()->isRepeating())
                {
                    if (ignoreEvent) {
                        KeyPressRepeater::getInstance()->reset();
                        goto returnRslt;
                    }
                    else
                    {
                        if (KeyPressRepeater::getInstance()->addKeyModifier(&filteredEvent)) {
                            goto returnRslt;
                        }
                        if (KeyPressRepeater::getInstance()->isRepeatingEventForKeyCode(event->xkey.keycode)) {
                            goto returnRslt;
                        }
                    }
                } else {
                    if (ignoreEvent) {
                        goto returnRslt;
                    }
                }
                if (   lastKeyPressWasPartOfComposeSequence
                    && lastEventTimeOfComposeSequence == filteredEvent.xkey.time)
                {
                    KeyPressRepeater::getInstance()->triggerNextRepeatEventFor(lastKeyCodeOfComposeSequence,
                                                                               &filteredEvent, this);
                } else {
                    KeyPressRepeater::getInstance()->triggerNextRepeatEventFor(filteredEvent.xkey.keycode,
                                                                               &filteredEvent, this);
                }
                rslt = processKeyboardEvent(&filteredEvent);

                if (KeyPressRepeater::getInstance()->isRepeating()) {
                    XSync(GuiRoot::getInstance()->getDisplay(), False);  // prevent too fast key press event for slow XServer
                }
                
                goto returnRslt;
            }
            else {
                if (!ignoreEvent) {
                    rslt = processKeyboardEvent(&filteredEvent);
                }
                goto returnRslt;
            }

        returnRslt:
            lastKeyPressWasPartOfComposeSequence = ignoreEvent;
            if (ignoreEvent) {
                lastKeyCodeOfComposeSequence   = event->xkey.keycode;
                lastEventTimeOfComposeSequence = event->xkey.time;
            }
            return rslt;
        }
        
        case KeyRelease: {
            if (GlobalConfig::getConfigData()->getGeneralConfig()->getUseOwnKeyPressRepeater())
            {
                bool isTrueRelease = true;
                
                if (   !GuiRoot::getInstance()->hasDetectableAutorepeat()
                    && XEventsQueued(guiWidget->getDisplay(), QueuedAlready) > 0)
                {
                    XEvent nextEvent;
                    XPeekEvent(guiWidget->getDisplay(), &nextEvent);

                    if (         nextEvent.type         == KeyPress
                        &&       nextEvent.xkey.keycode == event->xkey.keycode
                        && (   /*nextEvent.xkey.time    == event->xkey.time*/        // autorepeated keys used to have same timestamp
                            labs(nextEvent.xkey.time    -  event->xkey.time) <= 2))  // but newer xservers messed it up
                    {
                        isTrueRelease = false;
                    }
                }
                if (isTrueRelease)
                {
                    if (KeyPressRepeater::getInstance()->isRepeatingEventForKeyCode(event->xkey.keycode)) {
                        KeyPressRepeater::getInstance()->reset();
                    }
                    if (KeyPressRepeater::getInstance()->isRepeating()) {
                       KeyPressRepeater::getInstance()->removeKeyModifier(event);
                    }
                }
            }
            return GuiWidget::EVENT_PROCESSED;
        }

        case FocusOut:
        {
            if (GlobalConfig::getConfigData()->getGeneralConfig()->getUseOwnKeyPressRepeater())
            {
                KeyPressRepeater::getInstance()->reset();
                GuiRoot::getInstance()->setKeyboardAutoRepeatOriginal();
            }
            if (focusFlag) {
                focusFlag = false;
                internalTreatFocusOut();
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
                    expectedFocusTopWin->internalTreatFocusOut();
                }
                expectedFocusTopWin = NULL;
            }
            return GuiWidget::EVENT_PROCESSED;
        }

        case FocusIn:
        {
            if (event->xfocus.detail == NotifyPointer) {
                return GuiWidget::EVENT_PROCESSED;
            }
                   
            if (GlobalConfig::getConfigData()->getGeneralConfig()->getUseOwnKeyPressRepeater())
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
                    expectedFocusTopWin->internalTreatFocusOut();
                }
            }
            if (shouldRaiseAfterFocusIn) {
                shouldRaiseAfterFocusIn = false;
                internalRaise();
            }
            expectedFocusTopWin = this;
            if (!focusFlag) {
                focusFlag = true;
                if (x11InputContext != NULL) {
                    XSetICFocus(x11InputContext);
                }
                treatFocusIn();
            }
            return GuiWidget::EVENT_PROCESSED;
        }


        default: {
            return GuiWidget::NOT_PROCESSED;
        }
    }
}

KeyPressEvent TopWin::createKeyPressEventObjectFromX11Event(const XEvent* event)
{
    KeyId       keyId;
    KeyModifier keyModifier;
    String      input;

    XEvent eventCopy = *event;

    unsigned int keyState = eventCopy.xkey.state;

    KeySym keySym;
    KeySym keySymWithoutModifier;
    
    {
        char buffer[1000];
        int len;
        
    #ifdef X_HAVE_UTF8_STRING
        if (x11InputContext != NULL)
        {
            eventCopy.xkey.state = 0;
            len = Xutf8LookupString(x11InputContext, 
                                    &eventCopy.xkey, buffer, sizeof(buffer), &keySymWithoutModifier, NULL);
    
            eventCopy.xkey.state = keyState;
            len = Xutf8LookupString(x11InputContext, 
                                    &eventCopy.xkey, buffer, sizeof(buffer), &keySym, NULL);
        } else
    #endif
        {
            bool wasXLookupString = false;
    #ifndef X_HAVE_UTF8_STRING
            if (x11InputContext != NULL)
            {
                eventCopy.xkey.state = 0;
                len = XmbLookupString(x11InputContext, 
                                      &eventCopy.xkey, buffer, sizeof(buffer), &keySymWithoutModifier, NULL);

                eventCopy.xkey.state = keyState;
                len = XmbLookupString(x11InputContext, 
                                      &eventCopy.xkey, buffer, sizeof(buffer), &keySym, NULL);
            } else
    #endif
            {
                eventCopy.xkey.state = 0;
                len = XLookupString  (&eventCopy.xkey, buffer, sizeof(buffer), &keySymWithoutModifier, NULL);

                eventCopy.xkey.state = keyState;
                len = XLookupString  (&eventCopy.xkey, buffer, sizeof(buffer), &keySym, NULL);

                wasXLookupString = true;
            }
            long c = keysym2ucs(keySym);
            
            if (c < 0) {
                // keysym2ucs failed
                if (wasXLookupString) {
                    // according to xlib documentation XLookupString returns latin-1 encoding
                    input = EncodingConverter::convertLatin1ToUtf8String((byte*)&buffer, len);
                    len = 0; // string "input" is set now
                } else {
                    // we don't know the encoding of XmbLookupString, so give up here
                    len = 0; // string "input" is empty now
                }
            } else if (c <= 0x7F) {
                buffer[0] = c;
                len = 1;
            } else if (c <= 0x7FF) {
                buffer[0] = 0xC0 | ((c >> 6) & 0x1F);
                buffer[1] = 0x80 | (c & 0x3F);
                len = 2;
            } else if (c <= 0xFFFF) {
                buffer[0] = 0xE0 | ((c >> 12) & 0x0F);
                buffer[1] = 0x80 | ((c >> 6) & 0x3F);
                buffer[2] = 0x80 | (c & 0x3F);
                len = 3;
            } else {
                buffer[0] = 0xF0 | ((c >> 18) & 0x07);
                buffer[1] = 0x80 | ((c >> 12) & 0x3f);
                buffer[2] = 0x80 | ((c >> 6) & 0x3f);
                buffer[3] = 0x80 | (c & 0x3f);
                len = 4;
            }
        }
        if (len > 0) {
            input = String(buffer, len);
        }
    }
        
/*    keySym = XKeycodeToKeysym(guiWidget->getDisplay(),
                              event->xkey.keycode, 0);*/
 
// printf("--- State: %p: %s - %s\n", (void*)keyState, KeyId(keySymWithoutModifier).toString().toCString(),
//                                                     KeyId(keySym).toString().toCString());
 
    
    if (!(XK_KP_Space <= keySymWithoutModifier && keySymWithoutModifier <= XK_KP_9))
    {
        KeySym lowerKeySym;
        KeySym upperKeySym;

        XConvertCase(keySymWithoutModifier, &lowerKeySym, &upperKeySym);
        keySym = upperKeySym;
    }
    

#ifdef XK_ISO_Left_Tab
    // Shift + Tab becomes XK_ISO_Left_Tab through XLookupString,
    // however we need XLookupString for otherwise correct
    // interpretation of num_lock
    if (keySym == XK_ISO_Left_Tab) {
        keySym = XK_Tab;
    }
#endif
#ifdef XK_KP_Tab
    if (keySym == XK_KP_Tab) {
        keySym = XK_Tab;
    }
#endif
    keyId       = KeyId(keySym);
    keyModifier = KeyModifier::createFromX11KeyState(keyState);
    
    return KeyPressEvent(keyModifier, keyId, input);
}

void TopWin::processGuiWidgetNewPositionEvent(const Position& newPosition)
{
    this->position = newPosition;
}

void TopWin::setPosition(const Position& position)
{
    if (guiWidget.isValid()) {
        guiWidget->setPosition(position);
    } else {
        this->position = position;
    }
}


void TopWin::handleConfigChanged()
{
    if (focusFlag)
    {
        if (GlobalConfig::getConfigData()->getGeneralConfig()->getUseOwnKeyPressRepeater())
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

void TopWin::setTitle(const String& title)
{
    this->title = title;
    if (guiWidget.isValid())
    {
        String latin1Title = EncodingConverter::convertUtf8ToLatin1String(title);
        XStoreName(guiWidget->getDisplay(), guiWidget->getWid(), latin1Title.toCString());

        XChangeProperty(guiWidget->getDisplay(), 
                        guiWidget->getWid(),
                        x11InternAtomForUtf8WindowTitle,
                        GuiRoot::getInstance()->getX11Utf8StringAtom(),
                        8, PropModeReplace, 
                        (unsigned char*) title.toCString(),
                                         title.getLength());
        
    }
}
   

#if LUCED_USE_XPM
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

#if LUCED_USE_XPM
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
            XSetWMHints(guiWidget->getDisplay(), guiWidget->getWid(), hints);
            
            XFree(hints);
        }        
    }
    XClassHint* classHint = XAllocClassHint();

    if (classHint != NULL)
    {
        String resName;

        String instanceName = GuiRoot::getInstance()->getInstanceName();
        if (instanceName.getLength() > 0) {
            resName = instanceName;
        } else {
            const char* envString = ::getenv("RESOURCE_NAME");
            if (envString != NULL) {
                resName = envString;
            } else {
                String programName = File(ProgramName::get()).getBaseName();
                resName = programName;
            }
        }
        classHint->res_name  = const_cast<char*>(resName.toCString());
        classHint->res_class = const_cast<char*>("LucED");
        XSetClassHint(guiWidget->getDisplay(), guiWidget->getWid(), classHint);
        XFree(classHint);
    }
}

void TopWin::requestCloseWindow(TopWin::CloseReason reason)
{
    if (!isClosingFlag)
    {
        isClosingFlag = true;
        myOwner->requestCloseChildWindow(this);
        requestForCloseNotifyCallbacks.invokeAllCallbacks(this, reason);
    }
}


void TopWin::raise()
{
    if (!isClosingFlag)
    {
        if (isMapped())
        {
            if (   GuiRoot::getInstance()->getX11ServerVendorString().startsWith("Hummingbird")
                && GuiRoot::getInstance()->getX11ServerVendorRelease() == 6100)
            {
                // vendor <Hummingbird Communications Ltd.> <6100>
                this->shouldRaiseAfterFocusIn = true; // delayed raise is Workaround for Exceed X11-Server
            }
            internalRaise();
        } else {
            raiseAfterMapped = true;
        }
    }
}

void TopWin::internalRaise()
{
    if (!isClosingFlag)
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
            event.xclient.display = guiWidget->getDisplay();       /* Display the event was read from */
            event.xclient.window = guiWidget->getWid();
            event.xclient.message_type = raiseWindowAtom;
            event.xclient.format = 32;
            event.xclient.data.l[0] = 1;
            event.xclient.data.l[1] = EventDispatcher::getInstance()->getLastX11Timestamp();
            event.xclient.data.l[2] = 0;
    
            XSendEvent(guiWidget->getDisplay(), guiWidget->getRootWid(), false, 
                                                   SubstructureNotifyMask|SubstructureRedirectMask,
                                                   &event);
        }
    
        // older Windowmanager only need XRaiseWindow
            
        XRaiseWindow(guiWidget->getDisplay(), guiWidget->getWid());
    }
}

void TopWin::treatFocusIn()
{}

void TopWin::internalTreatFocusOut()
{
    if (x11InputContext != NULL) {
        XUnsetICFocus(x11InputContext);
    }
    treatFocusOut();
}

void TopWin::treatFocusOut()
{}

void TopWin::notifyAboutBeingMapped()
{}

void TopWin::notifyAboutBeingUnmapped()
{}

GuiWidget::ProcessingResult TopWin::processKeyboardEvent(const KeyPressEvent& keyPressEvent)
{
    return GuiWidget::NOT_PROCESSED;
}

void TopWin::show()
{
    if (!guiWidget.isValid())
    {
        createWidget();
    }
    guiWidget->show();
    isVisibleFlag = true;
}


void TopWin::setTransientFor(RawPtr<TopWin> referingTopWin)
{
    this->referingTopWin = referingTopWin;
    
    if (guiWidget.isValid() && referingTopWin.isValid()) {
        XSetTransientForHint(getDisplay(), guiWidget->getWid(), referingTopWin->guiWidget->getWid());
    }
}


void TopWin::setRootElement(GuiElement::Ptr rootElement)
{
    ASSERT(!this->rootElement.isValid());
    this->rootElement = rootElement;
}

void TopWin::setFocusManager(RawPtr<FocusManager> focusManager)
{
    this->focusManager = focusManager;
}

void TopWin::processGuiWidgetCreatedEvent()
{
    ASSERT(rootElement.isValid());
    
    rootElement->adopt(Null,             // parentElement
                       guiWidget,        // parentWidget
                       focusManager,     // focusManagerForThis
                       focusManager);    // focusManagerForChilds
}


void TopWin::hide()
{
    guiWidget->hide();
    isVisibleFlag = false;
}

