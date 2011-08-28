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

#include "util.hpp"
#include "GuiRoot.hpp"
#include "SelectionOwner.hpp"
#include "EventDispatcher.hpp"
#include "GlobalConfig.hpp"
#include "EncodingConverter.hpp"

using namespace LucED;


WeakPtr<SelectionOwner> SelectionOwner::primarySelectionOwner          = NULL;
WeakPtr<SelectionOwner> SelectionOwner::primarySelectionOwnerCandidate = NULL;


SelectionOwner::SelectionOwner(RawPtr<GuiWidget> baseWidget, Type type, ContentHandler::Ptr contentHandler)
      : baseWidget(baseWidget),
        contentHandler(contentHandler),
        x11AtomForSelection(type == TYPE_PRIMARY ? XA_PRIMARY : XInternAtom(GuiRoot::getInstance()->getDisplay(), "CLIPBOARD", False)),
        hasSelectionOwnershipFlag(false),
        display              (GuiRoot::getInstance()->getDisplay()),
        x11AtomForTargets    (XInternAtom(display, "TARGETS", False)),
        x11AtomForIncr       (XInternAtom(display, "INCR", False)),
        x11AtomForUtf8String (GuiRoot::getInstance()->getX11Utf8StringAtom()),
        lastX11Timestamp(0)
{
    GuiWidget::EventProcessorAccess::addToXEventMaskForGuiWidget(baseWidget, PropertyChangeMask);
}


SelectionOwner::~SelectionOwner()
{
    stopCurrentMultiPartSending();
    
    if (primarySelectionOwner == this) {
        primarySelectionOwner = NULL;
    }
}

void SelectionOwner::stopCurrentMultiPartSending()
{
    if (multiPartState != Null) {
        WidgetId targetWid = multiPartState.getPtr()->multiPartTargetWid;

        XSelectInput(display, targetWid, 0);

        EventDispatcher::getInstance()->removeEventReceiver(
            GuiWidget::EventProcessorAccess::createEventRegistration(baseWidget, targetWid)
        );
        multiPartState = Null;
        contentHandler->endSelectionDataRequest();
    }
}


void SelectionOwner::requestSelectionOwnership()
{
    stopCurrentMultiPartSending();
    
    if (!hasSelectionOwnership())
    {
        lastX11Timestamp = EventDispatcher::getInstance()->getLastX11Timestamp();
        XSetSelectionOwner(display, x11AtomForSelection, GuiWidget::EventProcessorAccess::getGuiWidgetWid(baseWidget), lastX11Timestamp);
        
        if (isPrimary()) {
            primarySelectionOwnerCandidate = this;
        }
        checkSelectionOwnership();
    }
}


void SelectionOwner::checkSelectionOwnership()
{
    bool hasIt = (XGetSelectionOwner(display, x11AtomForSelection) == GuiWidget::EventProcessorAccess::getGuiWidgetWid(baseWidget));
    
    if (hasIt && isPrimary()) 
    {
        if (primarySelectionOwner != Null && primarySelectionOwner != this) {
            // No SelectionClear event if selection owner changes within LucED (why!?!?!)
            primarySelectionOwner->contentHandler->notifyAboutLostSelectionOwnership();
            primarySelectionOwner->hasSelectionOwnershipFlag = false;
        }
        else if (primarySelectionOwnerCandidate != Null && primarySelectionOwnerCandidate != this) {
            primarySelectionOwnerCandidate->contentHandler->notifyAboutLostSelectionOwnership();
            primarySelectionOwnerCandidate->hasSelectionOwnershipFlag = false;
        }
        primarySelectionOwner          = this;
        primarySelectionOwnerCandidate = Null;
    }
    if (!hasIt && isPrimary())
    {
        if (primarySelectionOwner == this) {
            primarySelectionOwner = Null;
        }
        if (primarySelectionOwnerCandidate == this) {
            primarySelectionOwnerCandidate = Null;
        }
    }
    
    if (hasIt && !hasSelectionOwnershipFlag) {
        hasSelectionOwnershipFlag = true;
        contentHandler->notifyAboutObtainedSelectionOwnership();
    }
    else if (!hasIt && hasSelectionOwnershipFlag) {
        hasSelectionOwnershipFlag = false;
        contentHandler->notifyAboutLostSelectionOwnership();
    }
}

void SelectionOwner::checkPrimarySelectionOwnership()
{
    if (primarySelectionOwner != Null) {
        primarySelectionOwner->checkSelectionOwnership();
    }
    else if (primarySelectionOwnerCandidate != Null) {
        primarySelectionOwnerCandidate->checkSelectionOwnership();
    }
}


void SelectionOwner::releaseSelectionOwnership()
{
    if (hasSelectionOwnership())
    {
        if (isPrimary()) {
            if (primarySelectionOwner == this) {
                primarySelectionOwner = Null;
            }
            else if (primarySelectionOwnerCandidate == this) {
                primarySelectionOwnerCandidate = Null;
            }
        }
        hasSelectionOwnershipFlag = false;
        contentHandler->notifyAboutLostSelectionOwnership();

        lastX11Timestamp = EventDispatcher::getInstance()->getLastX11Timestamp();
        XSetSelectionOwner(display, x11AtomForSelection, None, lastX11Timestamp);
    }
}

GuiWidget::ProcessingResult SelectionOwner::processSelectionOwnerEvent(const XEvent* event)
{
    switch (event->type) {
    
        case SelectionClear:
        {
            if (event->xselectionclear.time > lastX11Timestamp)
            {
                if (event->xselectionclear.selection ==  x11AtomForSelection) {
                    checkSelectionOwnership();
                    return GuiWidget::EVENT_PROCESSED;
                }
            }
            break;
        }
        case SelectionRequest:
        {
            XSelectionEvent e;
            e.type = SelectionNotify;
            e.requestor = event->xselectionrequest.requestor;
            e.selection = event->xselectionrequest.selection;
            e.target    = event->xselectionrequest.target;
            e.time      = event->xselectionrequest.time;
            e.property  = event->xselectionrequest.property;
            if (e.target == x11AtomForTargets) {
                Atom myTargets[] = { x11AtomForUtf8String, XA_STRING };
                static const int XSERVER_ATOM_BITSIZE = 32; // sizeof(Atom) == 64 on 64bit systems, but this value must be 32
                XChangeProperty(display, e.requestor, e.property,
                                XA_ATOM, XSERVER_ATOM_BITSIZE, 0, (unsigned char*)myTargets,
                                sizeof(myTargets)/sizeof(myTargets[0]));
            } else if (e.target == XA_STRING || e.target == x11AtomForUtf8String)
            {
                stopCurrentMultiPartSending();

                if (hasSelectionOwnership() && e.selection == x11AtomForSelection)
                {
                    long selectionDataLength = contentHandler->initSelectionDataRequest();

                    if (selectionDataLength < GlobalConfig::getConfigData()->getGeneralConfig()->getX11SelectionChunkLength())
                    {
                        // send all at once
                    
                        const byte* bytes = contentHandler->getSelectionDataChunk(0, selectionDataLength);
                        
                        if (e.target == x11AtomForUtf8String) {
                            XChangeProperty(display, e.requestor, e.property,
                                            e.target, 8, PropModeReplace,
                                            (unsigned char*) bytes, selectionDataLength);
                        } else {
                            String latin1String = EncodingConverter::convertUtf8ToLatin1String(bytes, selectionDataLength);
                            XChangeProperty(display, e.requestor, e.property,
                                            e.target, 8, PropModeReplace,
                                            (unsigned char*) latin1String.toCString(), latin1String.getLength());
                        }
                        contentHandler->endSelectionDataRequest();
                    } 
                    else 
                    {
                        // send multi-part

                        multiPartState = MultiPartState();

                        RawPtr<MultiPartState> m = multiPartState.getPtr();
                        
                        m->multiPartTargetWid  = WidgetId(e.requestor);
                        m->multiPartTargetProp = e.property;
                        m->sendingTypeAtom     = e.target;
                        m->selectionDataLength = selectionDataLength;
                        m->alreadySentPos      = 0;
                        
                        XChangeProperty(display, m->multiPartTargetWid, m->multiPartTargetProp,
                                        x11AtomForIncr, 32, PropModeReplace, 0, 0);
                        XSelectInput(display, m->multiPartTargetWid, PropertyChangeMask);
                        
                        // this method should only be used for Windows from other applications
                        ASSERT(EventDispatcher::getInstance()->isForeignWidget(m->multiPartTargetWid));
                        
                        EventDispatcher::getInstance()->registerEventReceiverForForeignWidget(GuiWidget::EventProcessorAccess::createEventRegistration(baseWidget, m->multiPartTargetWid));
                    }
                } else {
                    e.property = 0;
                }
            } else {
                e.property = 0;
            }
            XSendEvent(display, e.requestor, 0, 0, (XEvent*)&e);

            return GuiWidget::EVENT_PROCESSED;
            break;
        }
        case PropertyNotify:
        {
            if (multiPartState != Null && event->xproperty.state == PropertyDelete)
            {
                RawPtr<MultiPartState> m = multiPartState.getPtr();
                
                if (m->alreadySentPos == 0) {
                    m->selectionDataLength = contentHandler->initSelectionDataRequest();
                }

                if (m->alreadySentPos >= m->selectionDataLength)
                {
                    // multi-part finished
                    
                    XChangeProperty(display, 
                                    m->multiPartTargetWid, 
                                    m->multiPartTargetProp,
                                    m->sendingTypeAtom, 8, PropModeReplace,
                                    0, 0);

                    stopCurrentMultiPartSending();
                }
                else
                {
                    // send next part
                
                    long sendLength = util::minimum(GlobalConfig::getConfigData()->getGeneralConfig()->getX11SelectionChunkLength(),
                                                    m->selectionDataLength - m->alreadySentPos);
                    
                    
                    const byte* bytes = contentHandler->getSelectionDataChunk(m->alreadySentPos, sendLength);
                    
                    if (m->sendingTypeAtom == x11AtomForUtf8String) {
                        XChangeProperty(display, 
                                        m->multiPartTargetWid, 
                                        m->multiPartTargetProp,
                                        m->sendingTypeAtom, 8, PropModeReplace,
                                        (unsigned char*) bytes, sendLength);
                    } else {
                        String latin1String = EncodingConverter::convertUtf8ToLatin1String(bytes, sendLength);

                        XChangeProperty(display, 
                                        m->multiPartTargetWid, 
                                        m->multiPartTargetProp,
                                        m->sendingTypeAtom, 8, PropModeReplace,
                                        (unsigned char*) latin1String.toCString(), latin1String.getLength());
                    }
                    m->alreadySentPos += sendLength;
                }
                return GuiWidget::EVENT_PROCESSED;
            }
            break;
        }
    }
    return GuiWidget::NOT_PROCESSED;
}
