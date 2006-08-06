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

#ifndef SELECTIONOWNER_H
#define SELECTIONOWNER_H

#include <X11/Xatom.h>
#include "GuiWidget.h"

namespace LucED {

class SelectionOwner : GuiWidgetAccessForEventProcessors
{
public:

    static SelectionOwner* getPrimarySelectionOwner() {return primarySelectionOwner;}
    static bool hasPrimarySelectionOwner() {return primarySelectionOwner != NULL;}

    bool requestSelectionOwnership();
    void releaseSelectionOwnership();
    bool hasSelectionOwnership();
    
protected:
    friend class SelectionOwnerAccessForPasteDataReceiver;
    
    virtual long  initSelectionDataRequest() {return 0;}
    virtual const byte* getSelectionDataChunk(long pos, long length) {return NULL;}
    virtual void  endSelectionDataRequest() {}

    SelectionOwner(GuiWidget* baseWidget, Atom x11AtomForSelection = XA_PRIMARY);
    ~SelectionOwner();
    
    GuiElement::ProcessingResult processSelectionOwnerEvent(const XEvent *event);
    
    virtual void notifyAboutLostSelectionOwnership() {}

    
private:
    GuiWidget* baseWidget;
    Atom x11AtomForSelection;
    bool sendingMultiPart;
    bool hasRequestedSelectionOwnership;
    long selectionDataLength;
    long alreadySentPos;
    Window multiPartTargetWid;
    Atom   multiPartTargetProp;
    
    Atom x11AtomForTargets;
    Atom x11AtomForIncr;
    
    static SelectionOwner* primarySelectionOwner;
};

class SelectionOwnerAccessForPasteDataReceiver
{
protected:
    static long initSelectionDataRequest(SelectionOwner* o) {
        return o->initSelectionDataRequest();
    }
    static const byte* getSelectionDataChunk(SelectionOwner* o, long pos, long length) {
        return o->getSelectionDataChunk(pos, length);
    }
    static void endSelectionDataRequest(SelectionOwner* o) {
        o->endSelectionDataRequest();
    }
};

} // namespace LucED

#endif // SELECTIONOWNER_H
