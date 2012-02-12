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

#include "GuiRoot.hpp"
#include "PasteDataReceiver.hpp"
#include "Clipboard.hpp"
#include "SelectionOwner.hpp"
#include "EventDispatcher.hpp"
#include "EncodingConverter.hpp"
                      
using namespace LucED;

PasteDataReceiver::PasteDataReceiver(RawPtr<GuiWidget> baseWidget, ContentHandler::Ptr contentHandler)
  : baseWidget(baseWidget),
    contentHandler(contentHandler),
    isRequestingTargetTypes(false),
    receivingPasteDataFlag(false),
    isMultiPartPastingFlag(false),
    display             (GuiRoot::getInstance()->getDisplay()),
    x11AtomForTargets   (XInternAtom(display, "TARGETS", False)),
    x11AtomForIncr      (XInternAtom(display, "INCR", False)),
    x11AtomForUtf8String(GuiRoot::getInstance()->getX11Utf8StringAtom())
{
    GuiWidget::EventProcessorAccess::addToXEventMaskForGuiWidget(baseWidget, PropertyChangeMask);
}

void PasteDataReceiver::requestPrimarySelectionPasting()
{
    pasteBuffer.clear();
    if (SelectionOwner::hasPrimarySelectionOwner()) {
        SelectionOwner* selectionOwner = SelectionOwner::getPrimarySelectionOwner();
        long length = SelectionOwner::ReceiverAccess::initSelectionDataRequest(selectionOwner);
        if (length > 0) {
            const byte* data = SelectionOwner::ReceiverAccess::getSelectionDataChunk(selectionOwner, 0, length);
            SelectionOwner::ReceiverAccess::endSelectionDataRequest(selectionOwner);
            ByteArray buffer;
            buffer.append(data, length); // make copy, because data* could become invalid, if
                                         // paste is going into the same buffer
            contentHandler->notifyAboutBeginOfPastingData();
            contentHandler->notifyAboutReceivedPasteData(buffer.getPtr(0), buffer.getLength());
            contentHandler->notifyAboutEndOfPastingData();
        } else {
            SelectionOwner::ReceiverAccess::endSelectionDataRequest(selectionOwner);
        }
        isRequestingTargetTypes  = false;
        receivingPasteDataFlag   = false;
    } else {
        XDeleteProperty(display, GuiWidget::EventProcessorAccess::getGuiWidgetWid(baseWidget), 
                                 /*XA_PRIMARY*/x11AtomForTargets);

        pasteTargetAtom = XA_PRIMARY;

        XConvertSelection(display,              pasteTargetAtom, 
                          x11AtomForTargets,    x11AtomForTargets, 
                                                GuiWidget::EventProcessorAccess::getGuiWidgetWid(baseWidget), 
                                                EventDispatcher::getInstance()->getLastX11Timestamp());
        isRequestingTargetTypes  = true;
        receivingPasteDataFlag   = false;
        isMultiPartPastingFlag   = false;
        contentHandler->notifyAboutBeginOfPastingData();
        EventDispatcher::getInstance()->registerTimerCallback(Seconds(3), MicroSeconds(0), 
                                                              newCallback(this, 
                                                                          &PasteDataReceiver::handleTimerEvent));
        lastPasteEventTime = TimeStamp::now();
    }
}

void PasteDataReceiver::requestClipboardPasting()
{
    pasteBuffer.clear();
    if (Clipboard::getInstance()->hasClipboardOwnership()) {
        const ByteArray& clipboardBuffer = Clipboard::getInstance()->getClipboardBuffer();
        if (clipboardBuffer.getLength() > 0) {
            contentHandler->notifyAboutBeginOfPastingData();
            contentHandler->notifyAboutReceivedPasteData(clipboardBuffer.getPtr(0), clipboardBuffer.getLength());
            contentHandler->notifyAboutEndOfPastingData();
        }
        isRequestingTargetTypes  = false;
        receivingPasteDataFlag   = false;
    } else {
        XDeleteProperty(display, GuiWidget::EventProcessorAccess::getGuiWidgetWid(baseWidget), 
                                 Clipboard::getInstance()->getX11AtomForClipboard());

        pasteTargetAtom = Clipboard::getInstance()->getX11AtomForClipboard();

        XConvertSelection(display,              pasteTargetAtom,
                          x11AtomForTargets,    x11AtomForTargets, 
                                                GuiWidget::EventProcessorAccess::getGuiWidgetWid(baseWidget), 
                                                EventDispatcher::getInstance()->getLastX11Timestamp());
        isRequestingTargetTypes  = true;
        receivingPasteDataFlag   = false;
        isMultiPartPastingFlag   = false;
        contentHandler->notifyAboutBeginOfPastingData();
        EventDispatcher::getInstance()->registerTimerCallback(Seconds(3), MicroSeconds(0), 
                                                              newCallback(this, 
                                                                          &PasteDataReceiver::handleTimerEvent));
        lastPasteEventTime = TimeStamp::now();
    }
}

bool PasteDataReceiver::isReceivingPasteData()
{
    return receivingPasteDataFlag;
}

namespace // anonymous namespace
{

class XMemoryHolder
{
public:
    XMemoryHolder(void* ptr)
        : ptr(ptr)
    {}
    ~XMemoryHolder() {
        XFree(ptr);
    }
private:
    void* ptr;
};

} // anonymous namespace

GuiWidget::ProcessingResult PasteDataReceiver::processPasteDataReceiverEvent(const XEvent* event)
{
    switch (event->type)
    {
        case SelectionNotify:
        {
            if (isRequestingTargetTypes && event->xselection.property == x11AtomForTargets)
            {
                lastPasteEventTime = TimeStamp::now();
                unsigned long remainingLength = 0;
                Atom actualType; int format; unsigned long numberItems;
                unsigned char* itemData = NULL;

                XGetWindowProperty(display,
                        event->xselection.requestor,
                        event->xselection.property,
                        0,  XMaxRequestSize(display), True, AnyPropertyType,
                        &actualType, &format, &numberItems, &remainingLength,
                        &itemData);
                XMemoryHolder memoryHolder(itemData);
                
                bool canUtf8   = false;
                bool canLatin1 = false;
                
                if (actualType == XA_ATOM)
                {
                    Atom* atoms = (Atom*) itemData;

                    for (int i = 0; i < numberItems; ++i) {

                        if (atoms[i] == XA_STRING) {
                            canLatin1 = true;
                        } else if (atoms[i] == x11AtomForUtf8String) {
                            canUtf8 = true;
                            break;
                        }
                    }
                    if (numberItems == 0) {
                        canLatin1 = true; // Fallback for empty TARGETS works with older programs
                    }
                }

                if (canUtf8 || canLatin1)
                {
                    if (canUtf8) {
                        pasteDataTypeAtom = x11AtomForUtf8String;
                    } else {
                        pasteDataTypeAtom = XA_STRING;
                    }
                    XDeleteProperty(display, GuiWidget::EventProcessorAccess::getGuiWidgetWid(baseWidget), 
                                             XA_PRIMARY);
                    XConvertSelection(display,              pasteTargetAtom, 
                                      pasteDataTypeAtom,    pasteTargetAtom, 
                                                            GuiWidget::EventProcessorAccess::getGuiWidgetWid(baseWidget), 
                                                            EventDispatcher::getInstance()->getLastX11Timestamp());
                    isRequestingTargetTypes  = false;
                    receivingPasteDataFlag   = true;
                    isMultiPartPastingFlag   = false;
                }
                else
                {
                    isRequestingTargetTypes  = false;
                    receivingPasteDataFlag   = false;
                    isMultiPartPastingFlag   = false;
                }
            } 
            else if ((isRequestingTargetTypes || receivingPasteDataFlag) && event->xselection.property == None)
            {
                lastPasteEventTime = TimeStamp::now();
                receivingPasteDataFlag = false;
                isMultiPartPastingFlag = false;
                contentHandler->notifyAboutEndOfPastingData();
            }
            else if (event->xselection.property == Clipboard::getInstance()->getX11AtomForClipboard()
                  || event->xselection.property == XA_PRIMARY)
            {
                lastPasteEventTime = TimeStamp::now();
                unsigned long remainingLength = 0;
                Atom actualType; int format; unsigned long portionLength;
                unsigned char* portion = NULL;

                XGetWindowProperty(display,
                        event->xselection.requestor,
                        event->xselection.property,
                        0, 0, False, AnyPropertyType,
                        &actualType, &format, &portionLength, &remainingLength,
                        &portion);
                XMemoryHolder memoryHolder(portion);

                if (receivingPasteDataFlag && actualType == x11AtomForIncr)
                {
                    isMultiPartPastingFlag = true;
                    XDeleteProperty(display, event->xselection.requestor, event->xselection.property);
                    return GuiWidget::EVENT_PROCESSED;
                }
                else if (format != 8 || remainingLength == 0)
                {
                    return GuiWidget::EVENT_PROCESSED;
                }
                else if (receivingPasteDataFlag)
                {
                    portion = NULL;
                    XGetWindowProperty(display,
                            event->xselection.requestor,
                            event->xselection.property,
                            0, remainingLength, False, AnyPropertyType,
                            &actualType, &format, &portionLength, &remainingLength,
                            &portion);
                    XMemoryHolder memoryHolder(portion);
                    if (portionLength > 0) {
                        if (actualType == XA_STRING) {
                            String utf8Bytes = EncodingConverter::convertLatin1ToUtf8String(portion, portionLength);
                            contentHandler->notifyAboutReceivedPasteData((const byte*)utf8Bytes.toCString(), 
                                                                                      utf8Bytes.getLength());
                        } else {
                            contentHandler->notifyAboutReceivedPasteData(portion, portionLength);
                        }
                    }
                    contentHandler->notifyAboutEndOfPastingData();
                    isMultiPartPastingFlag = false;
                    receivingPasteDataFlag = false;
                    XDeleteProperty(display, event->xselection.requestor, event->xselection.property);

                    return GuiWidget::EVENT_PROCESSED;
                }
            }
            break;
        }
        case PropertyNotify:
        {
            if ((event->xproperty.atom == Clipboard::getInstance()->getX11AtomForClipboard()
              || event->xproperty.atom == XA_PRIMARY) 
                    && event->xproperty.state == PropertyNewValue)
            {
                lastPasteEventTime = TimeStamp::now();
                unsigned long remainingLength = 0;
                Atom actualType; int format; unsigned long portionLength;
                unsigned char* portion = NULL;

                XGetWindowProperty(display,
                        event->xproperty.window,
                        event->xproperty.atom,
                        0, 0, False, AnyPropertyType,
                        &actualType, &format, &portionLength, &remainingLength,
                        &portion);
                XMemoryHolder memoryHolder(portion);

                if (receivingPasteDataFlag && actualType == x11AtomForIncr)
                {
                    isMultiPartPastingFlag = true;
                    XDeleteProperty(display, event->xproperty.window, event->xproperty.atom);
                    return GuiWidget::EVENT_PROCESSED;
                }
                else if (format != 8 || (   actualType != x11AtomForUtf8String
                                         && actualType != XA_STRING))
                {
                    return GuiWidget::EVENT_PROCESSED;
                }
                else if (receivingPasteDataFlag && remainingLength == 0)
                {
                    if (isMultiPartPastingFlag && pasteBuffer.getLength() > 0) {
                        contentHandler->notifyAboutReceivedPasteData(pasteBuffer.getPtr(0), pasteBuffer.getLength());
                        pasteBuffer.clear();
                    }
                    XDeleteProperty(display, event->xproperty.window, event->xproperty.atom);
                    contentHandler->notifyAboutEndOfPastingData();
                    isMultiPartPastingFlag = false;
                    receivingPasteDataFlag = false;
                    return GuiWidget::EVENT_PROCESSED;
                }
                else if (receivingPasteDataFlag)
                {
                    portion = NULL;
                    XGetWindowProperty(display,
                            event->xproperty.window,
                            event->xproperty.atom,
                            0, remainingLength, False, AnyPropertyType,
                            &actualType, &format, &portionLength, &remainingLength,
                            &portion);
                    XMemoryHolder memoryHolder(portion);

                    if (isMultiPartPastingFlag) {
                        if (actualType == XA_STRING) {
                            String utf8Bytes = EncodingConverter::convertLatin1ToUtf8String(portion, portionLength);
                            pasteBuffer.append((const byte*)utf8Bytes.toCString(), 
                                                            utf8Bytes.getLength());
                        } else {
                            pasteBuffer.append(portion, portionLength);
                        }
                    } else if (portionLength > 0) {
                        if (actualType == XA_STRING) {
                            String utf8Bytes = EncodingConverter::convertLatin1ToUtf8String(portion, portionLength);
                            contentHandler->notifyAboutReceivedPasteData((const byte*)utf8Bytes.toCString(), 
                                                                                      utf8Bytes.getLength());
                        } else {
                            contentHandler->notifyAboutReceivedPasteData(portion, portionLength);
                        }
                        contentHandler->notifyAboutEndOfPastingData();
                        isMultiPartPastingFlag = false;
                        receivingPasteDataFlag = false;
                    }
                    XDeleteProperty(display, event->xproperty.window, event->xproperty.atom);
                    return GuiWidget::EVENT_PROCESSED;
                }
            }

            break;
        }
    }
    return GuiWidget::NOT_PROCESSED;
}

void PasteDataReceiver::handleTimerEvent()
{
    if (receivingPasteDataFlag)
    {
        if (TimeStamp::now() > lastPasteEventTime.get() + Seconds(3)) {
            contentHandler->notifyAboutEndOfPastingData();
            isMultiPartPastingFlag = false;
            receivingPasteDataFlag = false;
        } else {
            EventDispatcher::getInstance()->registerTimerCallback(Seconds(3), MicroSeconds(0), 
                    newCallback(this, &PasteDataReceiver::handleTimerEvent));
        }
    }
}

