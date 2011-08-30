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
        if (GlobalConfig::getConfigData()->getGeneralConfig()->getKeepRunningIfOwningClipboard() ) {
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
      : sendingMultiPart(false),
        shouldRequestedClipboardOwnership(false)
{
    guiWidget = GuiWidget::create(Null, this, Position(0,0,1,1));
    
    guiWidget->addToXEventMask(PropertyChangeMask);
    x11AtomForClipboard = XInternAtom(GuiWidget::getDisplay(), "CLIPBOARD", False);
    x11AtomForTargets   = XInternAtom(GuiWidget::getDisplay(), "TARGETS", False);
    x11AtomForIncr      = XInternAtom(GuiWidget::getDisplay(), "INCR", False);
    
    selectionOwner = SelectionOwner::create(guiWidget, 
                                            SelectionOwner::TYPE_CLIPBOARD, 
                                            SelectionContentHandler::create(this));

    pasteDataReceiver = PasteDataReceiver::create(guiWidget,
                                                  PasteDataContentHandler::create(this));
}



void Clipboard::copyToClipboard(const byte* ptr, long length)
{
    selectionOwner->requestSelectionOwnership();

    clipboardBuffer.clear();
    clipboardBuffer.append(ptr, length);
}

void Clipboard::copyActiveSelectionToClipboard()
{
    if (!pasteDataReceiver->isReceivingPasteData()) {
        pasteDataReceiver->requestPrimarySelectionPasting();
    }
    shouldRequestedClipboardOwnership = true;
}


const ByteArray& Clipboard::getClipboardBuffer() {
    return clipboardBuffer;
}


GuiWidget::ProcessingResult Clipboard::processGuiWidgetEvent(const XEvent* event)
{
    if (selectionOwner   ->processSelectionOwnerEvent(event)     == GuiWidget::EVENT_PROCESSED
     || pasteDataReceiver->processPasteDataReceiverEvent(event)  == GuiWidget::EVENT_PROCESSED)
    {
        return GuiWidget::EVENT_PROCESSED;
    }
    else
    {
        return GuiWidget::NOT_PROCESSED;
    }
}

void Clipboard::processGuiWidgetNewPositionEvent(const Position& newPosition)
{}


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
    if (newBuffer.getLength() > 0) 
    {
        if (shouldRequestedClipboardOwnership) {
            selectionOwner->requestSelectionOwnership();
            clipboardBuffer = newBuffer;
        }
    
        if (selectionRequestCallbacks.hasCallbacks()) {
            String selectionString = newBuffer.toString();
            selectionRequestCallbacks.invokeAllCallbacks(selectionString);
            selectionRequestCallbacks.clear();
        }
        newBuffer.clear();
    }
}

void Clipboard::treatNewWindowPosition(Position newPosition)
{}


void Clipboard::addActiveSelectionRequest(Callback<String>::Ptr selectionRequestCallback)
{
    selectionRequestCallbacks.registerCallback(selectionRequestCallback);
    if (!pasteDataReceiver->isReceivingPasteData()) {
        pasteDataReceiver->requestPrimarySelectionPasting();
    }
}
