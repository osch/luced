/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch at luced dot de
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

#ifndef PASTEDATARECEIVER_H
#define PASTEDATARECEIVER_H

#include "ByteArray.h"
#include "GuiWidget.h"
#include "SelectionOwner.h"
#include "TimeVal.h"

namespace LucED {

class PasteDataReceiver : public virtual HeapObject,
                          public GuiWidgetAccessForEventProcessors, 
                          public SelectionOwnerAccessForPasteDataReceiver
{
public:
    void requestSelectionPasting();
    void requestClipboardPasting();
    bool isReceivingPasteData();
    
protected:
    PasteDataReceiver(GuiWidget* baseWidget);

    bool processPasteDataReceiverEvent(const XEvent *event);

    virtual void notifyAboutBeginOfPastingData() = 0;
    virtual void notifyAboutReceivedPasteData(const byte* data, long length) = 0;    
    virtual void notifyAboutEndOfPastingData() = 0;    

private:

    void handleTimerEvent();
    TimeVal lastPasteEventTime;

    GuiWidget* baseWidget;
    bool isReceivingPasteDataFlag;
    bool isMultiPartPastingFlag;
    ByteArray pasteBuffer;
    Atom x11AtomForTargets;
    Atom x11AtomForIncr;
    Display* const display;
};

} // namespace LucED

#endif // PASTEDATARECEIVER_H
