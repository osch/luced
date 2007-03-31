/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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
#include "TopWinList.h"

using namespace LucED;


SingletonInstance<Clipboard> Clipboard::instance;


Clipboard* Clipboard::getInstance()
{
    return instance.getPtr();
}


Clipboard::Clipboard()
      : GuiWidget(0,0,1,1,0),
        SelectionOwner(this, XInternAtom(getDisplay(), "CLIPBOARD", False)),
        PasteDataReceiver(this),
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

void Clipboard::copyActiveSelectionToClipboard()
{
    if (requestSelectionOwnership()) {
        requestSelectionPasting();
    }
}


const ByteArray& Clipboard::getClipboardBuffer() {
    return clipboardBuffer;
}


GuiElement::ProcessingResult Clipboard::processEvent(const XEvent *event)
{
    if (processSelectionOwnerEvent(event)    == GuiElement::EVENT_PROCESSED
     || processPasteDataReceiverEvent(event) == GuiElement::EVENT_PROCESSED)
    {
        return GuiElement::EVENT_PROCESSED;
    }
    else
    {
        return GuiElement::NOT_PROCESSED;
    }
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
    if (TopWinList::getInstance()->getNumberOfTopWins() == 0) {
        EventDispatcher::getInstance()->requestProgramTermination();
    }
}

void Clipboard::notifyAboutBeginOfPastingData()
{
    newBuffer.clear();
}

void Clipboard::notifyAboutReceivedPasteData(const byte* data, long length)
{
    newBuffer.append(data, length);
}


void Clipboard::notifyAboutEndOfPastingData()
{
    if (hasSelectionOwnership() && newBuffer.getLength() > 0) {
        clipboardBuffer = newBuffer;
    }
    newBuffer.clear();
}



