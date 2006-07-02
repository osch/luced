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

#include "DialogWin.h"
#include "util.h"

using namespace LucED;

DialogWin::DialogWin(TopWin* referingWindow)
    : wasNeverShown(true),
      referingWindow(referingWindow)
{
    if (referingWindow != NULL) {
        XSetTransientForHint(getDisplay(), getWid(), referingWindow->getWid());
    }
    addToXEventMask(ExposureMask);
    setBackgroundColor(getGuiRoot()->getGuiColor03());
}

void DialogWin::setRootElement(OwningPtr<GuiElement> rootElement)
{
    this->rootElement = rootElement;
}

void DialogWin::show()
{
    if (rootElement.isValid()) {
        Measures m = rootElement->getDesiredMeasures();
        m.minHeight += 4;   m.minWidth  += 4;
        m.bestHeight += 4;  m.bestWidth += 4;
        if (wasNeverShown) {
            if (referingWindow != NULL)  {
                Position pp = referingWindow->getPosition();
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
        int dx = 2;
        int dy = 2;
        if (newPosition.w < 2*dx)
           dx = (2*dx - newPosition.w)/2;
        if (newPosition.h < 2*dy)
           dy = (2*dy - newPosition.h)/2;
        rootElement->setPosition(Position(dx, dy, newPosition.w - 2*dx, newPosition.h - 2*dy));
    }
}



bool DialogWin::processKeyboardEvent(const XEvent *event)
{
    return false;
}


bool DialogWin::processEvent(const XEvent *event)
{
    if (TopWin::processEvent(event)) {
        return true;
    } else {
        switch (event->type)
        {
            case GraphicsExpose:
                if (event->xgraphicsexpose.count > 0) {
                    break;
                }
            case Expose: {
                if (event->xexpose.count > 0) {
                    break;
                }
                drawRaisedBox(0, 0, getPosition().w, getPosition().h);
                return true;
            }
        }
        return false;
    }
}


void DialogWin::treatFocusIn()
{
}


void DialogWin::treatFocusOut()
{
}




