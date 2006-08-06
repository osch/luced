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

#include <X11/xpm.h>

#include "TopWin.h"
#include "KeyPressRepeater.h"
#include "GlobalConfig.h"
#include "SingletonInstance.h"

using namespace LucED;

TopWin::TopWin()
    : GuiWidget(0, 0, 1, 1, 0),
      mapped(false),
      requestFocusAfterMapped(false)
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

}

static TopWin* expectedFocusTopWin = NULL;

TopWin::~TopWin()
{
    if (this == expectedFocusTopWin) {
        expectedFocusTopWin = NULL;
        if (GlobalConfig::getInstance()->getUseKeyPressRepeater())
        {
            KeyPressRepeater::getInstance()->reset();
            GuiRoot::getInstance()->setKeyboardAutoRepeatOriginal();
        }
    }
}

void TopWin::setSizeHints(int minWidth, int minHeight, int dx, int dy)
{
    XSizeHints *hints = XAllocSizeHints();
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
    XSizeHints *hints = XAllocSizeHints();
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

GuiElement::ProcessingResult TopWin::processEvent(const XEvent *event)
{
    if (GuiWidget::processEvent(event) == EVENT_PROCESSED) {
        return EVENT_PROCESSED;
    } else {
        
        switch (event->type) {

            case MapNotify: {
                if (event->xmap.window == getWid()) {
                    mapped = true;
                    if (requestFocusAfterMapped) {
                        XSetInputFocus(getDisplay(), getWid(), RevertToNone, CurrentTime);
                        requestFocusAfterMapped = false;
                    }
                    return EVENT_PROCESSED;
                } else {
                    return NOT_PROCESSED;
                }
            }

            case UnmapNotify: {
                if (event->xunmap.window == getWid()) {
                    mapped = false;
                    return EVENT_PROCESSED;
                } else {
                    return NOT_PROCESSED;
                }
            }

            case ClientMessage: {
                if (event->xclient.data.l[0] == this->x11InternAtomForDeleteWindow) {
                    this->requestCloseWindow();
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
                
                if (GlobalConfig::getInstance()->getUseKeyPressRepeater())
                {
                    if (KeyPressRepeater::getInstance()->isRepeating())
                    {
                        if (!KeyPressRepeater::getInstance()->addKeyModifier(event)) {
                            ProcessingResult processed = processKeyboardEvent(event);
                            KeyPressRepeater::getInstance()->repeatEvent(event);
                            return processed;
                        } else {
                            return EVENT_PROCESSED;
                        }
                    }
                    else {
                        ProcessingResult processed = processKeyboardEvent(event);
                        KeyPressRepeater::getInstance()->repeatEvent(event);
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
                if (GlobalConfig::getInstance()->getUseKeyPressRepeater())
                {
                    if (KeyPressRepeater::getInstance()->isRepeatingEvent(event)) {
                        KeyPressRepeater::getInstance()->reset();
                    }
                    if (KeyPressRepeater::getInstance()->isRepeating()) {
                       KeyPressRepeater::getInstance()->removeKeyModifier(event);
                    }
                }
                return EVENT_PROCESSED;
            }

            case FocusOut: {
                if (GlobalConfig::getInstance()->getUseKeyPressRepeater())
                {
                    KeyPressRepeater::getInstance()->reset();
                    GuiRoot::getInstance()->setKeyboardAutoRepeatOriginal();
                }
                treatFocusOut();
                if (expectedFocusTopWin == this)
                {
                    expectedFocusTopWin = NULL;
                }
                else if (expectedFocusTopWin != NULL)
                {
                    // I don't understand, why this can happen: 
                    // but without this workaround sometimes a window
                    // in the background still has a blinking cursor
                    // this also happens with NEdit, perhaps a bug in
                    // some window manager
                    
                    expectedFocusTopWin->treatFocusOut();
                    expectedFocusTopWin = NULL;
                }
                return EVENT_PROCESSED;
            }

            case FocusIn: {
                if (GlobalConfig::getInstance()->getUseKeyPressRepeater())
                {
                    KeyPressRepeater::getInstance()->reset();
                    GuiRoot::getInstance()->setKeyboardAutoRepeatOff();
                }
                if (expectedFocusTopWin != NULL && expectedFocusTopWin != this)
                {
                    // There was no FocusOut for the focus top win
                    // this sometimes happens, perhaps a bug in
                    // the window manager, but I'm not sure.
                    
                    expectedFocusTopWin->treatFocusOut();
                }
                expectedFocusTopWin = this;
                treatFocusIn();
                reportFocusOwnershipToTopWinOwner(this, owner);
                reportFocusOwnershipToTopWinOwner(this, this);
                return EVENT_PROCESSED;
            }


            default: {
                return NOT_PROCESSED;
            }
        }
    }
}

void TopWin::setTitle(const char* title)
{
    XStoreName(getDisplay(), getWid(), title);
}
   

#include "luced.xpm"

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

        if (XpmCreatePixmapFromData(
                GuiRoot::getInstance()->getDisplay(),
                GuiRoot::getInstance()->getRootWid(), luced_xpm, &pixMap, NULL, NULL) != 0) {
            pixMap = 0;
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
        XWMHints *hints = XAllocWMHints();
        
        hints->flags  = IconPixmapHint|InputHint;// | IconMaskHint ;//| IconWindowHint;
        hints->icon_pixmap = pixMap;
        hints->input = True;
        XSetWMHints(getDisplay(), getWid(), hints);
        
        XFree(hints);
    }
}

void TopWin::requestCloseWindow()
{
    owner->requestCloseChildWindow(this);
}



