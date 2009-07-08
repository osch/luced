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

#ifndef CLIPBOARD_HPP
#define CLIPBOARD_HPP

#include "debug.hpp"
#include "WidgetId.hpp"
#include "GuiWidget.hpp"
#include "ByteArray.hpp"
#include "SelectionOwner.hpp"
#include "SingletonInstance.hpp"
#include "PasteDataReceiver.hpp"
#include "ProgramRunningKeeper.hpp"

namespace LucED
{

class Clipboard : public HeapObject,
                  public GuiWidget::EventListener
{
public:
    static Clipboard* getInstance();
    
    void copyToClipboard(const byte* buffer, long length);
    void copyActiveSelectionToClipboard();
    
    void addActiveSelectionRequest(Callback<String>::Ptr selectionRequestCallback);
    
    Atom getX11AtomForClipboard() {
        return x11AtomForClipboard;
    }
    Atom getX11AtomForIncr() {
        return x11AtomForIncr;
    }
    const ByteArray& getClipboardBuffer();    
    
    virtual void show() {ASSERT(false);} // always hidden
    
    bool hasClipboardOwnership() {
        return selectionOwner->hasSelectionOwnership();
    }

private: // GuiWidget::EventListener interface implementation
    virtual GuiWidget::ProcessingResult processGuiWidgetEvent(const XEvent* event);
    virtual void                        processGuiWidgetNewPositionEvent(const Position& newPosition);

private:
    friend class SingletonInstance<Clipboard>;

    static SingletonInstance<Clipboard> instance;
    
    Clipboard();
    
    virtual void treatNewWindowPosition(Position newPosition);
    
    void notifyAboutReceivedPasteData(const byte* data, long length);
    void notifyAboutEndOfPastingData();
    void notifyAboutBeginOfPastingData();

    Atom x11AtomForClipboard;
    Atom x11AtomForTargets;
    Atom x11AtomForIncr;
    ByteArray clipboardBuffer;
    ByteArray newBuffer;
    bool hasRequestedClipboardOwnership;
    
    bool sendingMultiPart;
    long alreadySentPos;
    WidgetId multiPartTargetWid;
    Atom   multiPartTargetProp;

    ProgramRunningKeeper programRunningKeeper;

    class SelectionContentHandler;
    SelectionOwner::Ptr selectionOwner;

    class PasteDataContentHandler;
    PasteDataReceiver::Ptr pasteDataReceiver;
    
    GuiWidget::Ptr guiWidget;
    
    CallbackContainer<String> selectionRequestCallbacks;
    
    bool hasRequestedActiveSelectionForClipboard;
};

} // namespace LucED

#endif // CLIPBOARD_HPP
