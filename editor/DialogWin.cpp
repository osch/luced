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

#include <X11/keysym.h>

#include "util.h"
#include "Callback.h"
#include "DialogWin.h"
#include "Button.h"

using namespace LucED;

DialogWin::DialogWin(TopWin* referingWindow)
    : wasNeverShown(true),
      referingWindow(referingWindow)
{
    if (referingWindow != NULL) {
        XSetTransientForHint(getDisplay(), getWid(), referingWindow->getWid());
    }
    setBackgroundColor(getGuiRoot()->getGuiColor03());
    keyMapping.set(            0, XK_Escape,   Callback0(this, &DialogWin::requestCloseWindow));
}

void DialogWin::setRootElement(OwningPtr<GuiElement> rootElement)
{
    this->rootElement = rootElement;
}

void DialogWin::show()
{
    if (rootElement.isValid()) {
        Measures m = rootElement->getDesiredMeasures();
        if (wasNeverShown) {
            if (referingWindow != NULL)  {
                Position pp = referingWindow->getAbsolutePosition();
                int x = pp.x + (pp.w - m.bestWidth)/2;
                int y = pp.y + (pp.h - m.bestHeight)/2;
                setPosition(Position(x, y, m.bestWidth, m.bestHeight));
                setSizeHints(x, y, m.minWidth, m.minHeight, m.incrWidth, m.incrHeight);
            } else {
                setSize(m.bestWidth, m.bestHeight);
                setSizeHints(m.minWidth, m.minHeight, m.incrWidth, m.incrHeight);
            }
        } else {
            setSizeHints(getPosition().x, getPosition().y, m.minWidth, m.minHeight, m.incrWidth, m.incrHeight);
        }
        wasNeverShown = false;
    }
    TopWin::show();
}

void DialogWin::treatNewWindowPosition(Position newPosition)
{
    if (rootElement.isValid())
    {
        TopWin::treatNewWindowPosition(newPosition);
        rootElement->setPosition(Position(0, 0, newPosition.w, newPosition.h));
    }
}





GuiElement::ProcessingResult DialogWin::processKeyboardEvent(const XEvent *event)
{
    bool processed = false;

    KeyMapping::Id keyMappingId(event->xkey.state, XLookupKeysym((XKeyEvent*)&event->xkey, 0));
    Callback0 keyAction = keyMapping.find(keyMappingId);
    if (keyAction.isValid()) {
        keyAction.call();
        processed = true;
    }
    return processed ? EVENT_PROCESSED : NOT_PROCESSED;
}



