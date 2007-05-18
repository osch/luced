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

PasteDataReceiver::PasteDataReceiver(GuiWidget* baseWidget)
  : baseWidget(baseWidget),
    isReceivingPasteDataFlag(false),
    isMultiPartPastingFlag(false),
    display(GuiRoot::getInstance()->getDisplay())
{
    x11AtomForTargets   = XInternAtom(display, "TARGETS", False);
    x11AtomForIncr      = XInternAtom(display, "INCR", False);
    addToXEventMaskForGuiWidget(baseWidget, PropertyChangeMask);
}

void PasteDataReceiver::requestSelectionPasting()
{
    pasteBuffer.clear();
    if (SelectionOwner::hasPrimarySelectionOwner()) {
        SelectionOwner* selectionOwner = SelectionOwner::getPrimarySelectionOwner();
        long length = initSelectionDataRequest(selectionOwner);
        if (length > 0) {
            const byte* data = getSelectionDataChunk(selectionOwner, 0, length);
            ByteArray buffer;
            buffer.append(data, length); // make copy, because data* could become invalid, if
                                         // paste is going into the same buffer
            notifyAboutReceivedPasteData(buffer.getPtr(0), buffer.getLength());
        }
        endSelectionDataRequest(selectionOwner);
        isReceivingPasteDataFlag = false;
    } else {
        XDeleteProperty(display, getGuiWidgetWid(baseWidget), XA_PRIMARY);
        XConvertSelection(display, XA_PRIMARY, XA_STRING, XA_PRIMARY, getGuiWidgetWid(baseWidget), CurrentTime);
        isMultiPartPastingFlag = false;
        isReceivingPasteDataFlag = true;
        notifyAboutBeginOfPastingData();
        EventDispatcher::getInstance()->registerTimerCallback(3, 0, 
                                                              Callback0(this, 
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
            notifyAboutReceivedPasteData(clipboardBuffer.getPtr(0), clipboardBuffer.getLength());
        }
        isReceivingPasteDataFlag = false;
    } else {
        XDeleteProperty(display, getGuiWidgetWid(baseWidget), Clipboard::getInstance()->getX11AtomForClipboard());
        XConvertSelection(display, Clipboard::getInstance()->getX11AtomForClipboard(),
                             XA_STRING, Clipboard::getInstance()->getX11AtomForClipboard(), getGuiWidgetWid(baseWidget), CurrentTime);
        isMultiPartPastingFlag = false;
        isReceivingPasteDataFlag = true;
        notifyAboutBeginOfPastingData();
        EventDispatcher::getInstance()->registerTimerCallback(3, 0, 
                                                              Callback0(this, 
                                                                        &PasteDataReceiver::handleTimerEvent));
        lastPasteEventTime.setToCurrentTime();
    }
}

bool PasteDataReceiver::isReceivingPasteData()
{
    return isReceivingPasteDataFlag;
}

GuiElement::ProcessingResult PasteDataReceiver::processPasteDataReceiverEvent(const XEvent *event)
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
                notifyAboutEndOfPastingData();
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
                    return GuiElement::EVENT_PROCESSED;
                }
                else if (format != 8 || remainingLength == 0)
                {
                    return GuiElement::EVENT_PROCESSED;
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
                        notifyAboutReceivedPasteData(portion, portionLength);
                    }
                    notifyAboutEndOfPastingData();
                    isMultiPartPastingFlag = false;
                    isReceivingPasteDataFlag = false;
                    XDeleteProperty(display, event->xselection.requestor, event->xselection.property);
                    XFree(portion);
                    return GuiElement::EVENT_PROCESSED;
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
                    return GuiElement::EVENT_PROCESSED;
                }
                else if (format != 8 || actualType != XA_STRING)
                {
                    return GuiElement::EVENT_PROCESSED;
                }
                else if (isReceivingPasteDataFlag && remainingLength == 0)
                {
                    if (isMultiPartPastingFlag && pasteBuffer.getLength() > 0) {
                        notifyAboutReceivedPasteData(pasteBuffer.getPtr(0), pasteBuffer.getLength());
                        pasteBuffer.clear();
                    }
                    XDeleteProperty(display, event->xproperty.window, event->xproperty.atom);
                    notifyAboutEndOfPastingData();
                    isMultiPartPastingFlag = false;
                    isReceivingPasteDataFlag = false;
                    return GuiElement::EVENT_PROCESSED;
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
                        notifyAboutReceivedPasteData(portion, portionLength);
                        notifyAboutEndOfPastingData();
                        isMultiPartPastingFlag = false;
                        isReceivingPasteDataFlag = false;
                    }
                    XFree(portion);
                    XDeleteProperty(display, event->xproperty.window, event->xproperty.atom);
                    return GuiElement::EVENT_PROCESSED;
                }

            }
            break;
        }
    }
    return GuiElement::NOT_PROCESSED;
}

void PasteDataReceiver::handleTimerEvent()
{
    if (isReceivingPasteDataFlag)
    {
        TimeVal t1, t2;
        t1 = lastPasteEventTime;
        t1.addSecs(3);
        t2.setToCurrentTime();

        if (t2.isLaterThan(t1)) {
            notifyAboutEndOfPastingData();
            isMultiPartPastingFlag = false;
            isReceivingPasteDataFlag = false;
        } else {
            EventDispatcher::getInstance()->registerTimerCallback(3, 0, 
                    Callback0(this, &PasteDataReceiver::handleTimerEvent));
        }
    }
}

