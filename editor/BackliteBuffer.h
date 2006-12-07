/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#ifndef BACKLITEBUFFER_H
#define BACKLITEBUFFER_H

#include "HeapObject.h"
#include "TextData.h"
#include "ByteArray.h"
#include "CallbackContainer.h"
#include "HilitingBuffer.h"
#include "OwningPtr.h"

namespace LucED {

class BackliteBuffer : public HeapObject
{
public:
    typedef OwningPtr<BackliteBuffer> Ptr;
    
    static Ptr create(TextData::Ptr textData) {
        return Ptr(new BackliteBuffer(textData));
    }
    
    byte getBackground(long textPos) {
        if (!hasSelection) {
            return 0;
        } else {
            return (beginSelection.getPos() <= textPos && textPos < endSelection.getPos()) ? 1 : 0;
        }
    }
    void activateSelection(long textPos);
    void deactivateSelection();
    void extendSelectionTo(long textPos);
    bool hasActiveSelection() {
        return hasSelection;
    }

    void registerUpdateListener(const Callback1<HilitingBuffer::UpdateInfo>& updateCallback);

    long getBeginSelectionPos() {
        ASSERT(hasSelection);
        return beginSelection.getPos();
    }
    long getEndSelectionPos() {
        ASSERT(hasSelection);
        return endSelection.getPos();
    }
    long getSelectionAnchorPos() {
        ASSERT(hasSelection);
        if (isSelectionAnchorAtBegin) {
            return beginSelection.getPos();
        } else {
            return endSelection.getPos();
        }
    }
    bool isAnchorAtBegin() {
        ASSERT(hasSelection);
        return isSelectionAnchorAtBegin;
    }
    void setAnchorToEndOfSelection() {
        ASSERT(hasSelection);
        isSelectionAnchorAtBegin = false;
    }
    void setAnchorToBeginOfSelection() {
        ASSERT(hasSelection);
        isSelectionAnchorAtBegin = true;
    }

private:
    BackliteBuffer(TextData::Ptr textData);
    byte getNonBufferedBackground(long textPos);
    
    TextData::Ptr textData;
    ByteArray buffer;
    long startPos;
    Callback1Container<HilitingBuffer::UpdateInfo> updateListeners;

    bool hasSelection;
    bool isSelectionAnchorAtBegin;
    TextData::TextMark beginSelection;
    TextData::TextMark endSelection;
};

} // namespace LucED

#endif // BACKLITEBUFFER_H
