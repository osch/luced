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

#include <X11/Xatom.h>

#include "util.h"
#include "Clipboard.h"
#include "EventDispatcher.h"

using namespace LucED;


Clipboard::Ptr Clipboard::instance;


Clipboard* Clipboard::getInstance()
{
    if (!instance.isValid()) {
        instance = Ptr(new Clipboard());
    }
    return instance.getRawPtr();
}


Clipboard::Clipboard()
      : GuiWidget(0,0,1,1,0),
        SelectionOwner(this, XInternAtom(getDisplay(), "CLIPBOARD", False)),
        sendingMultiPart(false)
{
    addToXEventMask(PropertyChangeMask);
    x11AtomForClipboard = XInternAtom(getDisplay(), "CLIPBOARD", False);
    x11AtomForTargets   = XInternAtom(getDisplay(), "TARGETS", False);
    x11AtomForIncr      = XInternAtom(getDisplay(), "INCR", False);
}



void Clipboard::copyToClipboard(const byte* ptr, long length)
{
    if (requestSelectionOwnership()) {
        clipboardBuffer.clear();
        clipboardBuffer.append(ptr, length);
    }
}


const ByteArray& Clipboard::getClipboardBuffer() {
    return clipboardBuffer;
}


bool Clipboard::processEvent(const XEvent *event)
{
    return processSelectionOwnerEvent(event);
}


long  Clipboard::initSelectionDataRequest()
{
    return clipboardBuffer.getLength();
}


const byte* Clipboard::getSelectionDataChunk(long pos, long length)
{
    return clipboardBuffer.getPtr(pos);
}


void  Clipboard::endSelectionDataRequest()
{
}

void  Clipboard::notifyAboutLostSelectionOwnership()
{
}
