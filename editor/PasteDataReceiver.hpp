/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
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

#ifndef PASTE_DATA_RECEIVER_HPP
#define PASTE_DATA_RECEIVER_HPP

#include "ByteArray.hpp"
#include "GuiWidget.hpp"
#include "SelectionOwner.hpp"
#include "TimeVal.hpp"
#include "GuiElement.hpp"
#include "OwningPtr.hpp"
#include "RawPtr.hpp"

namespace LucED
{

class PasteDataReceiver : public HeapObject
{
public:
    typedef OwningPtr<PasteDataReceiver> Ptr;

    class ContentHandler : public HeapObject
    {
    public:
        typedef OwningPtr<ContentHandler> Ptr;
        
        virtual void notifyAboutBeginOfPastingData() = 0;
        virtual void notifyAboutReceivedPasteData(const byte* data, long length) = 0;    
        virtual void notifyAboutEndOfPastingData() = 0;    
    protected:
        ContentHandler()
        {}
    };
    
    static Ptr create(RawPtr<GuiWidget> baseWidget, ContentHandler::Ptr contentHandler) {
        return Ptr(new PasteDataReceiver(baseWidget, contentHandler));
    }

    void requestSelectionPasting();
    void requestClipboardPasting();
    bool isReceivingPasteData();

    GuiElement::ProcessingResult processPasteDataReceiverEvent(const XEvent* event);

private:
    PasteDataReceiver(RawPtr<GuiWidget> baseWidget, ContentHandler::Ptr contentHandler);

    void handleTimerEvent();
    TimeVal lastPasteEventTime;

    RawPtr<GuiWidget>       baseWidget;
    OwningPtr<ContentHandler> contentHandler;

    bool isReceivingPasteDataFlag;
    bool isMultiPartPastingFlag;
    ByteArray pasteBuffer;
    Atom x11AtomForTargets;
    Atom x11AtomForIncr;
    Display* const display;
};

} // namespace LucED

#endif // PASTE_DATA_RECEIVER_HPP
