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

#include <X11/Xatom.h>

#include "util.hpp"
#include "Clipboard.hpp"
#include "GlobalConfig.hpp"
#include "RawPtr.hpp"

using namespace LucED;


SingletonInstance<Clipboard> Clipboard::instance;


Clipboard* Clipboard::getInstance()
{
    return instance.getPtr();
}

class Clipboard::SelectionContentHandler : public SelectionOwner::ContentHandler
{
public:
    typedef OwningPtr<ContentHandler> Ptr;

    static Ptr create(RawPtr<Clipboard> clipboard) {
        return Ptr(new SelectionContentHandler(clipboard));
    }
    virtual long initSelectionDataRequest() {
        return clipboard->clipboardBuffer.getLength();
    }
    virtual const byte* getSelectionDataChunk(long pos, long length) {
        return clipboard->clipboardBuffer.getPtr(pos);
    }
    virtual void endSelectionDataRequest() {
    }
    virtual void notifyAboutObtainedSelectionOwnership() {
        if (GlobalConfig::getInstance()->shouldKeepRunningIfOwningClipboard() ) {
            clipboard->programRunningKeeper.keepProgramRunning();
        }
    }
    virtual void notifyAboutLostSelectionOwnership() {
        clipboard->programRunningKeeper.doNotKeepProgramRunning();
    }
private:
    SelectionContentHandler(RawPtr<Clipboard> clipboard)
        : clipboard(clipboard)
    {}
    RawPtr<Clipboard> clipboard;
};

class Clipboard::PasteDataContentHandler : public PasteDataReceiver::ContentHandler
{
public:
    typedef OwningPtr<ContentHandler> Ptr;

    static Ptr create(RawPtr<Clipboard> clipboard) {
        return Ptr(new PasteDataContentHandler(clipboard));
    }
    
    virtual void notifyAboutBeginOfPastingData() {
        clipboard->notifyAboutBeginOfPastingData();
    }
    virtual void notifyAboutReceivedPasteData(const byte* data, long length) {
        clipboard->notifyAboutReceivedPasteData(data, length);
    }
    virtual void notifyAboutEndOfPastingData() {
        clipboard->notifyAboutEndOfPastingData();
    }
private:
    PasteDataContentHandler(RawPtr<Clipboard> clipboard)
        : clipboard(clipboard)
    {}
    RawPtr<Clipboard> clipboard;
};

Clipboard::Clipboard()
      : GuiWidget(0,0,1,1,0),
        selectionOwner(SelectionOwner::create(this, 
                                              SelectionOwner::TYPE_CLIPBOARD, 
                                              SelectionContentHandler::create(this))),

        pasteDataReceiver(PasteDataReceiver::create(this,
                                                    PasteDataContentHandler::create(this))),

        sendingMultiPart(false)
{
    addToXEventMask(PropertyChangeMask);
    x11AtomForClipboard = XInternAtom(getDisplay(), "CLIPBOARD", False);
    x11AtomForTargets   = XInternAtom(getDisplay(), "TARGETS", False);
    x11AtomForIncr      = XInternAtom(getDisplay(), "INCR", False);
}



void Clipboard::copyToClipboard(const byte* ptr, long length)
{
    if (selectionOwner->requestSelectionOwnership()) {
        clipboardBuffer.clear();
        clipboardBuffer.append(ptr, length);
    }
}

void Clipboard::copyActiveSelectionToClipboard()
{
    if (selectionOwner->requestSelectionOwnership()) {
        pasteDataReceiver->requestSelectionPasting();
    }
}


const ByteArray& Clipboard::getClipboardBuffer() {
    return clipboardBuffer;
}


GuiElement::ProcessingResult Clipboard::processEvent(const XEvent *event)
{
    if (selectionOwner   ->processSelectionOwnerEvent(event)     == GuiElement::EVENT_PROCESSED
     || pasteDataReceiver->processPasteDataReceiverEvent(event)  == GuiElement::EVENT_PROCESSED)
    {
        return GuiElement::EVENT_PROCESSED;
    }
    else
    {
        return GuiElement::NOT_PROCESSED;
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
    if (selectionOwner->hasSelectionOwnership() && newBuffer.getLength() > 0) {
        clipboardBuffer = newBuffer;
    }
    newBuffer.clear();
}



