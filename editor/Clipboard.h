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

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "debug.h"
#include "GuiWidget.h"
#include "ByteArray.h"
#include "SelectionOwner.h"
#include "SingletonInstance.h"

namespace LucED {

class Clipboard : public GuiWidget, SelectionOwner
{
public:
    
    static Clipboard* getInstance();
    
    void copyToClipboard(const byte* buffer, long length);

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
    
    Clipboard();
    
    virtual long  initSelectionDataRequest();
    virtual const byte* getSelectionDataChunk(long pos, long length);
    virtual void  endSelectionDataRequest();
    virtual void notifyAboutLostSelectionOwnership();
    
    Atom x11AtomForClipboard;
    Atom x11AtomForTargets;
    Atom x11AtomForIncr;
    ByteArray clipboardBuffer;
    bool hasRequestedClipboardOwnership;
    
    bool sendingMultiPart;
    long alreadySentPos;
    Window multiPartTargetWid;
    Atom   multiPartTargetProp;
};

} // namespace LucED

#endif // CLIPBOARD_H
