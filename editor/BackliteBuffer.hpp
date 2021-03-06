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

#ifndef BACKLITEBUFFER_HPP
#define BACKLITEBUFFER_HPP

#include "HeapObject.hpp"
#include "TextData.hpp"
#include "ByteArray.hpp"
#include "CallbackContainer.hpp"
#include "HilitingBuffer.hpp"
#include "OwningPtr.hpp"

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
            if (beginSelection.getPos() <= textPos && textPos < endSelection.getPos()) {
                return isSecondarySelection ? (byte) 2 : (byte) 1;
            } else {
                return 0;
            }
        }
    }
    void activateSelection(long textPos);
    void deactivateSelection();
    void makeSelectionToSecondarySelection();
    void makeSecondarySelectionToPrimarySelection();

    bool isSelectionPrimary() const {
        return !isSecondarySelection;
    }

    void extendSelectionTo(long textPos);

    bool hasActiveSelection() {
        return hasSelection && getBeginSelectionPos() != getEndSelectionPos();
    }

    void registerUpdateListener(Callback<HilitingBuffer::UpdateInfo>::Ptr updateCallback);
    
    void registerListenerForNextChange(Callback<>::Ptr callback);

    long getBeginSelectionPos() {
        ASSERT(hasSelection);
        return beginSelection.getPos();
    }
    long getBeginSelectionLine() {
        ASSERT(hasSelection);
        return beginSelection.getLine();
    }
    TextData::TextMark createMarkToBeginOfSelection() const {
        ASSERT(hasSelection);
        return textData->createNewMark(beginSelection);
    }
    TextData::TextMark createMarkToEndOfSelection() const {
        ASSERT(hasSelection);
        return textData->createNewMark(endSelection);
    }
    long getEndSelectionPos() {
        ASSERT(hasSelection);
        return endSelection.getPos();
    }
    long getEndSelectionLine() {
        ASSERT(hasSelection);
        return endSelection.getLine();
    }
    long getSelectionByteLength() {
        ASSERT(hasSelection);
        return endSelection.getPos() - beginSelection.getPos();
    }
    long getSelectionWCharsInSameLineLength() {
        ASSERT(hasSelection);
        if (beginSelection.getLine() == endSelection.getLine()) {
            return endSelection.getWCharColumn() - beginSelection.getWCharColumn();
        } else {
            return 0;
        }
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
    CallbackContainer<HilitingBuffer::UpdateInfo> updateListeners;
    CallbackContainer<> nextChangeListeners;

    bool hasSelection;
    bool isSelectionAnchorAtBegin;
    TextData::TextMark beginSelection;
    TextData::TextMark endSelection;
    bool isSecondarySelection;
};

} // namespace LucED

#endif // BACKLITEBUFFER_HPP
