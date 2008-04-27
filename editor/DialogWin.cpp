/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2007 Oliver Schmidt, oliver at luced dot de
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

#include "util.hpp"
#include "Callback.hpp"
#include "DialogWin.hpp"
#include "Button.hpp"

using namespace LucED;

DialogWin::DialogWin(TopWin* referingWindow)
    : wasNeverShown(true),
      referingWindow(referingWindow),
      keyMapping(KeyMapping::create()),
      shouldBeMapped(false)
{
    if (referingWindow != NULL) {
        XSetTransientForHint(getDisplay(), getWid(), referingWindow->getWid());
        referingWindow->registerMappingNotifyCallback(newCallback(this, &DialogWin::notifyAboutReferingWindowMapping));
    }
    setBackgroundColor(getGuiRoot()->getGuiColor03());
    keyMapping->set(            0, KeyId("Escape"),   newCallback(this, &DialogWin::requestCloseWindowByUser));
}

void DialogWin::requestCloseWindowByUser()
{
    requestCloseWindow(TopWin::CLOSED_BY_USER);
}

void DialogWin::setRootElement(OwningPtr<GuiElement> rootElement)
{
    this->rootElement = rootElement;
}

void DialogWin::prepareSizeHints()
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
}

void DialogWin::show()
{
    if (referingWindow.isValid() && !referingWindow->isMapped())
    {
        shouldBeMapped = true;
    }
    else
    {
        prepareSizeHints();
        TopWin::show();
    }
}

void DialogWin::notifyAboutReferingWindowMapping(bool isReferingWindowMapped)
{
    if (isReferingWindowMapped && !this->isVisible() && shouldBeMapped)
    {
        prepareSizeHints();
        TopWin::show();
    }
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
    KeyId pressedKey = KeyId(XLookupKeysym((XKeyEvent*)&event->xkey, 0));
    
    bool processed = false;

    KeyMapping::Id keyMappingId(event->xkey.state, pressedKey);
    Callback<>::Ptr keyAction = keyMapping->find(keyMappingId);
    if (keyAction->isEnabled()) {
        keyAction->call();
        processed = true;
    }
    return processed ? EVENT_PROCESSED : NOT_PROCESSED;
}



void DialogWin::setReferingWindowForPositionHintsOnly(TopWin* referingWindow)
{
    ASSERT(!this->referingWindow.isValid());
    
    if (referingWindow != NULL)
    {
        this->referingWindow = referingWindow;
        referingWindow->registerMappingNotifyCallback(newCallback(this, &DialogWin::notifyAboutReferingWindowMapping));
    }
}


