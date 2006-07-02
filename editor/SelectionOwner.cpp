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

#include "util.h"
#include "GuiRoot.h"
#include "SelectionOwner.h"
#include "EventDispatcher.h"
#include "GlobalConfig.h"

using namespace LucED;

SelectionOwner* SelectionOwner::primarySelectionOwner = NULL;

static bool staticallyInitialized = false;
static Atom x11AtomForTargets;
static Atom x11AtomForIncr;

static void initializeStatically() {
    staticallyInitialized = true;
    x11AtomForTargets   = XInternAtom(GuiRoot::getInstance()->getDisplay(), "TARGETS", False);
    x11AtomForIncr      = XInternAtom(GuiRoot::getInstance()->getDisplay(), "INCR", False);
}


static inline Display* getDisplay() {
    return GuiRoot::getInstance()->getDisplay();
}


SelectionOwner::SelectionOwner(GuiWidget* baseWidget, Atom x11AtomForSelection)
      : baseWidget(baseWidget),
        x11AtomForSelection(x11AtomForSelection),
        hasRequestedSelectionOwnership(false),
        sendingMultiPart(false)
{
    if (!staticallyInitialized) {
        initializeStatically();
    }
    addToXEventMaskForGuiWidget(baseWidget, PropertyChangeMask);
}


SelectionOwner::~SelectionOwner()
{
    if (sendingMultiPart) {
        sendingMultiPart = false;
        XSelectInput(getDisplay(), multiPartTargetWid, 0);
        EventDispatcher::getInstance()->removeEventReceiver(createEventRegistration(baseWidget, multiPartTargetWid));
    }
    XSetSelectionOwner(getDisplay(), x11AtomForSelection, None, CurrentTime);
    if (primarySelectionOwner == this) {
        primarySelectionOwner = NULL;
    }
}


bool SelectionOwner::requestSelectionOwnership()
{
    if (sendingMultiPart) {
        sendingMultiPart = false;
        XSelectInput(getDisplay(), multiPartTargetWid, 0);
        EventDispatcher::getInstance()->removeEventReceiver(createEventRegistration(baseWidget, multiPartTargetWid));
    }
    if (!hasRequestedSelectionOwnership) {
        XSetSelectionOwner(getDisplay(), x11AtomForSelection, getGuiWidgetWid(baseWidget), CurrentTime);
        hasRequestedSelectionOwnership = (XGetSelectionOwner(getDisplay(), x11AtomForSelection) == getGuiWidgetWid(baseWidget));
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
        XSetSelectionOwner(getDisplay(), x11AtomForSelection, None, CurrentTime);
        hasRequestedSelectionOwnership = false;
    }
}

bool SelectionOwner::hasSelectionOwnership()
{
    return hasRequestedSelectionOwnership;
}

bool SelectionOwner::processSelectionOwnerEvent(const XEvent *event)
{
    switch (event->type) {
    
        case SelectionClear:
        {
            if (event->xselectionclear.selection ==  x11AtomForSelection) {
                hasRequestedSelectionOwnership = false;
                notifyAboutLostSelectionOwnership();
                return true;
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
                XChangeProperty(getDisplay(), e.requestor, e.property,
		        XA_ATOM, sizeof(Atom)*8, 0, (unsigned char*)myTargets,
		        sizeof(myTargets)/sizeof(Atom));
            } else if (e.target == XA_STRING)
            {
                if (sendingMultiPart) {
                    sendingMultiPart = false;
		    XSelectInput(getDisplay(), multiPartTargetWid, 0);
                    EventDispatcher::getInstance()->removeEventReceiver(createEventRegistration(baseWidget, multiPartTargetWid));
                }
                if (hasRequestedSelectionOwnership && e.selection == x11AtomForSelection)
                {
                    selectionDataLength = initSelectionDataRequest();
                    if (selectionDataLength < GlobalConfig::getInstance()->getX11SelectionChunkLength())
                    {
                        // send all at once
                    
                        XChangeProperty(getDisplay(), e.requestor, e.property,
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

                        XChangeProperty(getDisplay(), multiPartTargetWid, multiPartTargetProp,
		                x11AtomForIncr, 32, PropModeReplace, 0, 0);
			XSelectInput(getDisplay(), multiPartTargetWid, PropertyChangeMask);
                        
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
            XSendEvent(getDisplay(), e.requestor, 0, 0, (XEvent *)&e);

            return true;                
            break;
        }
        case PropertyNotify:
        {
            if (sendingMultiPart && event->xproperty.state == PropertyDelete)
            {
                if (alreadySentPos >= selectionDataLength)
                {
                    // multi-part finished
                    
                    XChangeProperty(getDisplay(), multiPartTargetWid, 
                            multiPartTargetProp,
		            XA_STRING, 8, PropModeReplace,
		            0, 0);
                    sendingMultiPart = false;
		    XSelectInput(getDisplay(), multiPartTargetWid, 0);
                    EventDispatcher::getInstance()->removeEventReceiver(createEventRegistration(baseWidget, multiPartTargetWid));
                    endSelectionDataRequest();
                }
                else
                {
                    // send next part
                
                    long sendLength = util::minimum(GlobalConfig::getInstance()->getX11SelectionChunkLength(),
                                                    selectionDataLength - alreadySentPos);
                    
                    XChangeProperty(getDisplay(), multiPartTargetWid, 
                            multiPartTargetProp,
		            XA_STRING, 8, PropModeReplace,
		            (unsigned char *)getSelectionDataChunk(alreadySentPos, sendLength),
                            sendLength);
                    alreadySentPos += sendLength;
                }
                return true;
            }
            break;
        }
    }
    return false;
}
