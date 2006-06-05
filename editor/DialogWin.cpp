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

using namespace LucED;

DialogWin::DialogWin(TopWin* referingWindow, int x, int y, unsigned int width, unsigned int height)
    : TopWin(x, y, width, height, 0)
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
    Measures m = rootElement->getDesiredMeasures();
    setPosition(Position(getPosition().x, getPosition().y, 
                         m.bestWidth + 4, m.bestHeight + 4));
    setSizeHints(getPosition().x, getPosition().y, 
                         m.minWidth + 4, m.minHeight + 4, 1, 1);
}

void DialogWin::treatNewWindowPosition(Position newPosition)
{
    TopWin::treatNewWindowPosition(newPosition);
    rootElement->setPosition(Position(2, 2, newPosition.w - 4, newPosition.h - 4));
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




