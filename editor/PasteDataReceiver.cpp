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

using namespace LucED;

PasteDataReceiver::PasteDataReceiver(RawPtr<GuiWidget> baseWidget, ContentHandler::Ptr contentHandler)
  : baseWidget(baseWidget),
    contentHandler(contentHandler),
    isReceivingPasteDataFlag(false),
    isMultiPartPastingFlag(false),
    display(GuiRoot::getInstance()->getDisplay())
{
    x11AtomForTargets   = XInternAtom(display, "TARGETS", False);
    x11AtomForIncr      = XInternAtom(display, "INCR", False);
    GuiWidget::EventProcessorAccess::addToXEventMaskForGuiWidget(baseWidget, PropertyChangeMask);
}

void PasteDataReceiver::requestSelectionPasting()
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
        isReceivingPasteDataFlag = false;
    } else {
        XDeleteProperty(display, GuiWidget::EventProcessorAccess::getGuiWidgetWid(baseWidget), XA_PRIMARY);
        XConvertSelection(display, XA_PRIMARY, XA_STRING, XA_PRIMARY, GuiWidget::EventProcessorAccess::getGuiWidgetWid(baseWidget), CurrentTime);
        isMultiPartPastingFlag = false;
        isReceivingPasteDataFlag = true;
        contentHandler->notifyAboutBeginOfPastingData();
        EventDispatcher::getInstance()->registerTimerCallback(Seconds(3), MicroSeconds(0), 
                                                              newCallback(this, 
                                                                          &PasteDataReceiver::handleTimerEvent));
        lastPasteEventTime.setToCurrentTime();
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
        isReceivingPasteDataFlag = false;
    } else {
        XDeleteProperty(display, GuiWidget::EventProcessorAccess::getGuiWidgetWid(baseWidget), Clipboard::getInstance()->getX11AtomForClipboard());
        XConvertSelection(display, Clipboard::getInstance()->getX11AtomForClipboard(),
                             XA_STRING, Clipboard::getInstance()->getX11AtomForClipboard(), GuiWidget::EventProcessorAccess::getGuiWidgetWid(baseWidget), CurrentTime);
        isMultiPartPastingFlag = false;
        isReceivingPasteDataFlag = true;
        contentHandler->notifyAboutBeginOfPastingData();
        EventDispatcher::getInstance()->registerTimerCallback(Seconds(3), MicroSeconds(0), 
                                                              newCallback(this, 
                                                                          &PasteDataReceiver::handleTimerEvent));
        lastPasteEventTime.setToCurrentTime();
    }
}

bool PasteDataReceiver::isReceivingPasteData()
{
    return isReceivingPasteDataFlag;
}

GuiWidget::ProcessingResult PasteDataReceiver::processPasteDataReceiverEvent(const XEvent *event)
{
    switch (event->type)
    {
        case SelectionNotify:
        {
            if (isReceivingPasteDataFlag && event->xselection.property == None)
            {
                lastPasteEventTime.setToCurrentTime();
                isReceivingPasteDataFlag = false;
                isMultiPartPastingFlag = false;
                contentHandler->notifyAboutEndOfPastingData();
            }
            else if (event->xselection.property == Clipboard::getInstance()->getX11AtomForClipboard()
                  || event->xselection.property == XA_PRIMARY)
            {
                lastPasteEventTime.setToCurrentTime();
                unsigned long remainingLength = 0;
                Atom actualType; int format; unsigned long portionLength;
                unsigned char* portion = NULL;

                XGetWindowProperty(display,
                        event->xselection.requestor,
                        event->xselection.property,
                        0, 0, False, AnyPropertyType,
                        &actualType, &format, &portionLength, &remainingLength,
                        &portion);
                XFree(portion);

                if (isReceivingPasteDataFlag && actualType == x11AtomForIncr)
                {
                    isMultiPartPastingFlag = true;
                    XDeleteProperty(display, event->xselection.requestor, event->xselection.property);
                    return GuiWidget::EVENT_PROCESSED;
                }
                else if (format != 8 || remainingLength == 0)
                {
                    return GuiWidget::EVENT_PROCESSED;
                }
                else if (isReceivingPasteDataFlag)
                {
                    portion = NULL;
                    XGetWindowProperty(display,
                            event->xselection.requestor,
                            event->xselection.property,
                            0, remainingLength, False, AnyPropertyType,
                            &actualType, &format, &portionLength, &remainingLength,
                            &portion);
                    if (portionLength > 0) {
                        contentHandler->notifyAboutReceivedPasteData(portion, portionLength);
                    }
                    contentHandler->notifyAboutEndOfPastingData();
                    isMultiPartPastingFlag = false;
                    isReceivingPasteDataFlag = false;
                    XDeleteProperty(display, event->xselection.requestor, event->xselection.property);
                    XFree(portion);
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
                lastPasteEventTime.setToCurrentTime();
                unsigned long remainingLength = 0;
                Atom actualType; int format; unsigned long portionLength;
                unsigned char* portion = NULL;

                XGetWindowProperty(display,
                        event->xproperty.window,
                        event->xproperty.atom,
                        0, 0, False, AnyPropertyType,
                        &actualType, &format, &portionLength, &remainingLength,
                        &portion);
                XFree(portion);

                if (isReceivingPasteDataFlag && actualType == x11AtomForIncr)
                {
                    isMultiPartPastingFlag = true;
                    XDeleteProperty(display, event->xproperty.window, event->xproperty.atom);
                    return GuiWidget::EVENT_PROCESSED;
                }
                else if (format != 8 || actualType != XA_STRING)
                {
                    return GuiWidget::EVENT_PROCESSED;
                }
                else if (isReceivingPasteDataFlag && remainingLength == 0)
                {
                    if (isMultiPartPastingFlag && pasteBuffer.getLength() > 0) {
                        contentHandler->notifyAboutReceivedPasteData(pasteBuffer.getPtr(0), pasteBuffer.getLength());
                        pasteBuffer.clear();
                    }
                    XDeleteProperty(display, event->xproperty.window, event->xproperty.atom);
                    contentHandler->notifyAboutEndOfPastingData();
                    isMultiPartPastingFlag = false;
                    isReceivingPasteDataFlag = false;
                    return GuiWidget::EVENT_PROCESSED;
                }
                else if (isReceivingPasteDataFlag)
                {
                    portion = NULL;
                    XGetWindowProperty(display,
                            event->xproperty.window,
                            event->xproperty.atom,
                            0, remainingLength, False, AnyPropertyType,
                            &actualType, &format, &portionLength, &remainingLength,
                            &portion);
                    if (isMultiPartPastingFlag) {
                        pasteBuffer.append(portion, portionLength);
                    } else if (portionLength > 0) {
                        contentHandler->notifyAboutReceivedPasteData(portion, portionLength);
                        contentHandler->notifyAboutEndOfPastingData();
                        isMultiPartPastingFlag = false;
                        isReceivingPasteDataFlag = false;
                    }
                    XFree(portion);
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
    if (isReceivingPasteDataFlag)
    {
        TimeVal t1, t2;
        t1 = lastPasteEventTime;
        t1.add(Seconds(3));
        t2.setToCurrentTime();

        if (t2.isLaterThan(t1)) {
            contentHandler->notifyAboutEndOfPastingData();
            isMultiPartPastingFlag = false;
            isReceivingPasteDataFlag = false;
        } else {
            EventDispatcher::getInstance()->registerTimerCallback(Seconds(3), MicroSeconds(0), 
                    newCallback(this, &PasteDataReceiver::handleTimerEvent));
        }
    }
}

