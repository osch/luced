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

using namespace LucED;

SelectionOwner* SelectionOwner::primarySelectionOwner = NULL;


SelectionOwner::SelectionOwner(GuiWidget* baseWidget, Atom x11AtomForSelection)
      : baseWidget(baseWidget),
        x11AtomForSelection(x11AtomForSelection),
        hasRequestedSelectionOwnership(false),
        sendingMultiPart(false),
        display(GuiRoot::getInstance()->getDisplay())
{
    x11AtomForTargets   = XInternAtom(display, "TARGETS", False);
    x11AtomForIncr      = XInternAtom(display, "INCR", False);
    addToXEventMaskForGuiWidget(baseWidget, PropertyChangeMask);
}


SelectionOwner::~SelectionOwner()
{
    if (sendingMultiPart) {
        sendingMultiPart = false;
        XSelectInput(display, multiPartTargetWid, 0);
        EventDispatcher::getInstance()->removeEventReceiver(createEventRegistration(baseWidget, multiPartTargetWid));
    }
    if (primarySelectionOwner == this) {
        primarySelectionOwner = NULL;
    }
}


bool SelectionOwner::requestSelectionOwnership()
{
    if (sendingMultiPart) {
        sendingMultiPart = false;
        XSelectInput(display, multiPartTargetWid, 0);
        EventDispatcher::getInstance()->removeEventReceiver(createEventRegistration(baseWidget, multiPartTargetWid));
    }
    if (!hasRequestedSelectionOwnership) {
        XSetSelectionOwner(display, x11AtomForSelection, getGuiWidgetWid(baseWidget), CurrentTime);
        hasRequestedSelectionOwnership = (XGetSelectionOwner(display, x11AtomForSelection) == getGuiWidgetWid(baseWidget));
        if (x11AtomForSelection == XA_PRIMARY && hasRequestedSelectionOwnership) {
            if (primarySelectionOwner != NULL && primarySelectionOwner != this) {
                // No SelectionClear event if selection owner changes within LucED (why!?!?!)
                primarySelectionOwner->notifyAboutLostSelectionOwnership();
                primarySelectionOwner->hasRequestedSelectionOwnership = false;
            }
            primarySelectionOwner = this;
        }
    }
    return hasRequestedSelectionOwnership;
}

void SelectionOwner::releaseSelectionOwnership()
{
    if (hasRequestedSelectionOwnership)
    {
        if (x11AtomForSelection == XA_PRIMARY && primarySelectionOwner == this) {
            primarySelectionOwner = NULL;
        }
        if (hasRequestedSelectionOwnership) {
            notifyAboutLostSelectionOwnership();
        }
        XSetSelectionOwner(display, x11AtomForSelection, None, CurrentTime);
        hasRequestedSelectionOwnership = false;
    }
}

bool SelectionOwner::hasSelectionOwnership()
{
    return hasRequestedSelectionOwnership;
}

GuiElement::ProcessingResult SelectionOwner::processSelectionOwnerEvent(const XEvent *event)
{
    switch (event->type) {
    
        case SelectionClear:
        {
            if (event->xselectionclear.selection ==  x11AtomForSelection) {
                hasRequestedSelectionOwnership = false;
                if (x11AtomForSelection == XA_PRIMARY && primarySelectionOwner == this) {
                    primarySelectionOwner = NULL;
                }
                notifyAboutLostSelectionOwnership();
                return GuiElement::EVENT_PROCESSED;
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
                Atom myTargets[] = {XA_STRING};
                XChangeProperty(display, e.requestor, e.property,
                                XA_ATOM, sizeof(Atom)*8, 0, (unsigned char*)myTargets,
                                sizeof(myTargets)/sizeof(Atom));
            } else if (e.target == XA_STRING)
            {
                if (sendingMultiPart) {
                    sendingMultiPart = false;
                    XSelectInput(display, multiPartTargetWid, 0);
                    EventDispatcher::getInstance()->removeEventReceiver(createEventRegistration(baseWidget, multiPartTargetWid));
                }
                if (hasRequestedSelectionOwnership && e.selection == x11AtomForSelection)
                {
                    selectionDataLength = initSelectionDataRequest();
                    if (selectionDataLength < GlobalConfig::getInstance()->getX11SelectionChunkLength())
                    {
                        // send all at once
                    
                        XChangeProperty(display, e.requestor, e.property,
                        XA_STRING, 8, PropModeReplace,
                                (unsigned char *)getSelectionDataChunk(0, selectionDataLength),
                                selectionDataLength);
                        endSelectionDataRequest();
                    } 
                    else 
                    {
                        // send multi-part
                        
                        this->multiPartTargetWid = e.requestor;
                        this->multiPartTargetProp = e.property;
                        this->sendingMultiPart = true;
                        this->alreadySentPos = 0;

                        XChangeProperty(display, multiPartTargetWid, multiPartTargetProp,
                                        x11AtomForIncr, 32, PropModeReplace, 0, 0);
                        XSelectInput(display, multiPartTargetWid, PropertyChangeMask);
                        
                        // this method should only be used for Windows from other applications
                        ASSERT(EventDispatcher::getInstance()->isForeignWidget(multiPartTargetWid));
                        
                        EventDispatcher::getInstance()->registerEventReceiverForForeignWidget(createEventRegistration(baseWidget, multiPartTargetWid));
                    }
                } else {
                    e.property = 0;
                }
            } else {
                e.property = 0;
            }
            XSendEvent(display, e.requestor, 0, 0, (XEvent *)&e);

            return GuiElement::EVENT_PROCESSED;
            break;
        }
        case PropertyNotify:
        {
            if (sendingMultiPart && event->xproperty.state == PropertyDelete)
            {
                if (alreadySentPos >= selectionDataLength)
                {
                    // multi-part finished
                    
                    XChangeProperty(display, multiPartTargetWid, 
                            multiPartTargetProp,
                            XA_STRING, 8, PropModeReplace,
                            0, 0);
                    sendingMultiPart = false;
                    XSelectInput(display, multiPartTargetWid, 0);
                    EventDispatcher::getInstance()->removeEventReceiver(createEventRegistration(baseWidget, multiPartTargetWid));
                    endSelectionDataRequest();
                }
                else
                {
                    // send next part
                
                    long sendLength = util::minimum(GlobalConfig::getInstance()->getX11SelectionChunkLength(),
                                                    selectionDataLength - alreadySentPos);
                    
                    XChangeProperty(display, multiPartTargetWid, 
                            multiPartTargetProp,
                            XA_STRING, 8, PropModeReplace,
                            (unsigned char *)getSelectionDataChunk(alreadySentPos, sendLength),
                            sendLength);
                    alreadySentPos += sendLength;
                }
                return GuiElement::EVENT_PROCESSED;
            }
            break;
        }
    }
    return GuiElement::NOT_PROCESSED;
}
