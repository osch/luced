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

using namespace LucED;

TopWin::TopWin(int x, int y, unsigned int width, unsigned int height, unsigned border_width)
    : GuiWidget(x, y, width, height, border_width)
{
    setWindowIcon();
    {
#define W_INC 10
#define H_INC 10
        setSizeHints(x, y, 5*W_INC, 5*W_INC, W_INC, H_INC);
    }
    x11InternAtomForDeleteWindow = XInternAtom(getDisplay(), 
            "WM_DELETE_WINDOW", True);
    XSetWMProtocols(getDisplay(), getWid(), 
            &(x11InternAtomForDeleteWindow), 1);
    
    addToXEventMask(StructureNotifyMask|KeyPressMask|KeyReleaseMask|FocusChangeMask);
}

static TopWin* expectedFocusTopWin = NULL;

TopWin::~TopWin()
{
    if (this == expectedFocusTopWin) {
        expectedFocusTopWin = NULL;
    }
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

bool TopWin::processEvent(const XEvent *event)
{
    if (GuiWidget::processEvent(event)) {
        return true;
    } else {
        
        switch (event->type) {

            case ConfigureNotify: {
                int x = event->xconfigure.x;
                int y = event->xconfigure.y;
                int w = event->xconfigure.width;
                int h = event->xconfigure.height;
                Position newPosition(x,y,w,h);
                this->treatNewWindowPosition(newPosition);
                return true;
            }

            case ClientMessage: {
                if (event->xclient.data.l[0] == this->x11InternAtomForDeleteWindow) {
                    this->requestCloseWindow();
                    return true;
                } else {
                    return false;
                }   
            }
            
            case KeyPress: {
                
                if (GlobalConfig::getInstance()->getUseKeyPressRepeater())
                {
                    if (KeyPressRepeater::getInstance()->isRepeating())
                    {
                        if (!KeyPressRepeater::getInstance()->addKeyModifier(event)) {
                            bool processed = processKeyboardEvent(event);
                            KeyPressRepeater::getInstance()->repeatEvent(event);
                            return processed;
                        } else {
                            return true;
                        }
                    }
                    else {
                        bool processed = processKeyboardEvent(event);
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
                return true;
            }

            case FocusOut: {
                if (GlobalConfig::getInstance()->getUseKeyPressRepeater())
                {
                    KeyPressRepeater::getInstance()->reset();
                    GuiRoot::getInstance()->setKeyboardAutoRepeatOriginal();
                }
                treatFocusOut();
                if (expectedFocusTopWin == this) {
                    expectedFocusTopWin = NULL;
                } else if (expectedFocusTopWin != NULL) {
                    expectedFocusTopWin->treatFocusOut();
                    expectedFocusTopWin = NULL;
                }
                return true;
            }

            case FocusIn: {
                if (GlobalConfig::getInstance()->getUseKeyPressRepeater())
                {
                    KeyPressRepeater::getInstance()->reset();
                    GuiRoot::getInstance()->setKeyboardAutoRepeatOff();
                }
                if (expectedFocusTopWin != NULL && expectedFocusTopWin != this) {
                    expectedFocusTopWin->treatFocusOut();
                }
                expectedFocusTopWin = this;
                treatFocusIn();
                return true;
            }


            default: {
                return false;
            }
        }
    }
}

void TopWin::setTitle(const char* title)
{
    XStoreName(getDisplay(), getWid(), title);
}
   

#include "luced.xpm"

static bool staticallyInitialized = false;
static Pixmap pixMap;

static void initPixMap()
{
    pixMap = 0;
    
    if (XpmCreatePixmapFromData(
            GuiRoot::getInstance()->getDisplay(),
            GuiRoot::getInstance()->getRootWid(), luced_xpm, &pixMap, NULL, NULL) != 0) {
        pixMap = 0;
    }

}

void TopWin::setWindowIcon()
{
    if (!staticallyInitialized) {
        initPixMap();
        staticallyInitialized = true;
    }
    
    if (pixMap != 0)
    {
        XWMHints *hints = XAllocWMHints();
        
        hints->flags  = IconPixmapHint;// | IconMaskHint ;//| IconWindowHint;
        hints->icon_pixmap = pixMap;
        XSetWMHints(getDisplay(), getWid(), hints);
        
        XFree(hints);
    }
}

