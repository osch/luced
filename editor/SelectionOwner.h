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

    virtual long  initSelectionDataRequest() {return 0;}
    virtual const byte* getSelectionDataChunk(long pos, long length) {return NULL;}
    virtual void  endSelectionDataRequest() {}
    
protected:
    SelectionOwner(GuiWidget* baseWidget, Atom x11AtomForSelection = XA_PRIMARY);
    ~SelectionOwner();
    
    bool processSelectionOwnerEvent(const XEvent *event);
    
    virtual void notifyAboutLostSelectionOwnership() {}

    bool requestSelectionOwnership();
    void releaseSelectionOwnership();
    bool hasSelectionOwnership();
    
private:
    GuiWidget* baseWidget;
    Atom x11AtomForSelection;
    bool sendingMultiPart;
    bool hasRequestedSelectionOwnership;
    long selectionDataLength;
    long alreadySentPos;
    Window multiPartTargetWid;
    Atom   multiPartTargetProp;
    
    static SelectionOwner* primarySelectionOwner;
};

} // namespace LucED

#endif // SELECTIONOWNER_H
