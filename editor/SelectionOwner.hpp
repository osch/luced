/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#ifndef SELECTION_OWNER_HPP
#define SELECTION_OWNER_HPP

#include "WidgetId.hpp"
#include "GuiWidget.hpp"
#include "RawPtr.hpp"

namespace LucED
{

class SelectionOwner : public HeapObject
{
public:
    typedef OwningPtr<SelectionOwner> Ptr;

    enum Type
    {
        TYPE_PRIMARY,
        TYPE_CLIPBOARD
    };

    class ContentHandler : public HeapObject
    {
    public:
        typedef OwningPtr<ContentHandler> Ptr;
        
        virtual long        initSelectionDataRequest()                   = 0;
        virtual const byte* getSelectionDataChunk(long pos, long length) = 0;
        virtual void        endSelectionDataRequest()                    = 0;
        virtual void        notifyAboutLostSelectionOwnership()          = 0;
        virtual void        notifyAboutObtainedSelectionOwnership()      = 0;
    protected:
        ContentHandler()
        {}
    };

    static Ptr create(RawPtr<GuiWidget> baseWidget, Type type, ContentHandler::Ptr contentHandler) {
        return Ptr(new SelectionOwner(baseWidget, type, contentHandler));
    }
    ~SelectionOwner();
    
    GuiWidget::ProcessingResult processSelectionOwnerEvent(const XEvent *event);
    
    bool requestSelectionOwnership();
    void releaseSelectionOwnership();
    bool hasSelectionOwnership();
    
    static SelectionOwner* getPrimarySelectionOwner() {return primarySelectionOwner;}
    static bool hasPrimarySelectionOwner() {return primarySelectionOwner != NULL;}

    class ReceiverAccess
    {
        friend class PasteDataReceiver;
        
        static long initSelectionDataRequest(SelectionOwner* o) {
            return o->contentHandler->initSelectionDataRequest();
        }
        static const byte* getSelectionDataChunk(SelectionOwner* o, long pos, long length) {
            return o->contentHandler->getSelectionDataChunk(pos, length);
        }
        static void endSelectionDataRequest(SelectionOwner* o) {
            o->contentHandler->endSelectionDataRequest();
        }
    };
private:

    friend class SelectionOwnerAccessForPasteDataReceiver;

    SelectionOwner(RawPtr<GuiWidget> baseWidget, Type type, ContentHandler::Ptr contentHandler);

    RawPtr<GuiWidget> baseWidget;
    OwningPtr<ContentHandler> contentHandler;
    
    Atom x11AtomForSelection;
    bool sendingMultiPart;
    Atom sendingTypeAtom;
    bool hasRequestedSelectionOwnership;
    long selectionDataLength;
    long alreadySentPos;
    WidgetId multiPartTargetWid;
    Atom   multiPartTargetProp;
    
    Atom x11AtomForTargets;
    Atom x11AtomForIncr;
    Atom x11AtomForUtf8String;
    Display* const display;
    
    static WeakPtr<SelectionOwner> primarySelectionOwner;
};

} // namespace LucED

#endif // SELECTION_OWNER_HPP
