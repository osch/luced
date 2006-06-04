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

#ifndef HILITINGBUFFER_H
#define HILITINGBUFFER_H

#include "HeapObject.h"
#include "Hiliting.h"
#include "CallbackContainer.h"
#include "MemArray.h"
#include "OwningPtr.h"

namespace LucED {

class HilitingBuffer : public HeapObject
{
public:

    typedef OwningPtr<HilitingBuffer> Ptr;
    
    struct UpdateInfo {
        long beginPos;
        long endPos;
        UpdateInfo(long beginPos, long endPos)
            : beginPos(beginPos), endPos(endPos) {}
    };

    static HilitingBuffer::Ptr create(Hiliting::Ptr hiliting) {
        return HilitingBuffer::Ptr(new HilitingBuffer(hiliting));
    }
    
    int HilitingBuffer::getTextStyle(long textPos) {
        if (textPos >= startPos && textPos - startPos < styleBuffer.getLength()) {
            return styleBuffer[textPos - startPos];
        } else {
            return getNonBufferedTextStyle(textPos);
        }
    }
    
    void registerUpdateListener(const Callback1<UpdateInfo>& updateCallback);

private:
    
    HilitingBuffer(Hiliting::Ptr hiliting);
    
    int getNonBufferedTextStyle(long textPos);


    void treatHilitingUpdate(Hiliting::UpdateInfo update);
    Slot1<Hiliting::UpdateInfo> slotForHilitingUpdateTreatment;
    
    void treatTextDataUpdate(TextData::UpdateInfo update);
    Slot1<TextData::UpdateInfo> slotForTextDataUpdateTreatment;
    
    Hiliting::Ptr hiliting;
    Hiliting::Iterator iterator;
    TextData::Ptr textData;
    SyntaxPatterns::Ptr syntaxPatterns;
    LanguageMode::Ptr languageMode;

    long startPos;
    long rememberedSearchRestartPos;
    ByteArray patternStack;
    ByteArray styleBuffer;
    Callback1Container<UpdateInfo> updateListeners;
    MemArray<int> ovector;

    const int maxDistance;
};

} // namespace LucED

#endif // HILITINGBUFFER_H
