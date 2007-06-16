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

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "debug.hpp"
#include "GuiWidget.hpp"
#include "ByteArray.hpp"
#include "SelectionOwner.hpp"
#include "SingletonInstance.hpp"
#include "PasteDataReceiver.hpp"
#include "ProgramRunningKeeper.hpp"

namespace LucED {

class Clipboard : public GuiWidget, 
                  public SelectionOwner,
                  public PasteDataReceiver
{
public:
    
    static Clipboard* getInstance();
    
    void copyToClipboard(const byte* buffer, long length);
    void copyActiveSelectionToClipboard();
    
    Atom getX11AtomForClipboard() {
        return x11AtomForClipboard;
    }
    Atom getX11AtomForIncr() {
        return x11AtomForIncr;
    }
    const ByteArray& getClipboardBuffer();    
    
    virtual void show() {ASSERT(false);} // always hidden
    virtual ProcessingResult processEvent(const XEvent *event);
    
    bool hasClipboardOwnership() {
        return hasSelectionOwnership();
    }

private:
    friend class SingletonInstance<Clipboard>;
    static SingletonInstance<Clipboard> instance;
    
    Clipboard();
    
    virtual long  initSelectionDataRequest();
    virtual const byte* getSelectionDataChunk(long pos, long length);
    virtual void  endSelectionDataRequest();
    virtual void notifyAboutLostSelectionOwnership();
    virtual void notifyAboutObtainedSelectionOwnership();

    virtual void notifyAboutReceivedPasteData(const byte* data, long length);
    virtual void notifyAboutEndOfPastingData();
    virtual void notifyAboutBeginOfPastingData();

    Atom x11AtomForClipboard;
    Atom x11AtomForTargets;
    Atom x11AtomForIncr;
    ByteArray clipboardBuffer;
    ByteArray newBuffer;
    bool hasRequestedClipboardOwnership;
    
    bool sendingMultiPart;
    long alreadySentPos;
    Window multiPartTargetWid;
    Atom   multiPartTargetProp;

    ProgramRunningKeeper programRunningKeeper;
};

} // namespace LucED

#endif // CLIPBOARD_H
