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

#ifndef HILITINGBASE_H
#define HILITINGBASE_H

#include "HeapObject.hpp"
#include "ObjectArray.hpp"
#include "MemBuffer.hpp"
#include "ByteArray.hpp"
#include "ByteBuffer.hpp"

namespace LucED {

class HilitingBase : public HeapObject
{
public:
    
    class IteratorHandle
    {
    protected:
        friend class HilitingBase;
        long index;
    };
    
    class Iterator : public IteratorHandle
    {
    public:
        ~Iterator() {
            hiliting->getIteratorData(*this)->inUseCounter -= 1;
            if (index + 1 == hiliting->lastFreeIteratorIndex) {
                hiliting->lastFreeIteratorIndex = index;
            }
        }
        Iterator(const Iterator& src) {
            index = src.index;
            hiliting = src.hiliting;
            hiliting->getIteratorData(*this)->inUseCounter += 1;
        }
        Iterator& operator=(const Iterator& src) {
            hiliting->getIteratorData(*this)->inUseCounter -= 1;
            index = src.index;
            hiliting = src.hiliting;
            hiliting->getIteratorData(*this)->inUseCounter += 1;
        }
    private:
        friend class HilitingBase;
        Iterator(HilitingBase *hiliting, long index) {
            this->index = index;
            this->hiliting = hiliting;
            hiliting->getIteratorData(*this)->inUseCounter += 1;
        }
        HilitingBase *hiliting;
    };
    
    class IteratorData
    {
    private:
        friend class HilitingBase;
        friend class Iterator;
        int inUseCounter;
        long breakIndex;
        long stackStartPos;
        long textStartPos;
    };
    
    enum BreakType {
        Break_NULL = 0,
        Break_BEGIN,
        Break_INTER,
        Break_END
    };
    
    class BreakData
    {
    private:
        friend class HilitingBase;
        
        long nextStartOffset;
        long breakLength;
        BreakType type;
        int stackLength;
    };

    Iterator createNewIterator();
    
    IteratorData* getIteratorData(IteratorHandle iterator) {
        return &(iterators[iterator.index]);
    }
    const IteratorData* getIteratorData(IteratorHandle iterator) const {
        return &(iterators[iterator.index]);
    }
    BreakData* getBreakData(IteratorHandle iterator) {
        return &(breaks[getIteratorData(iterator)->breakIndex]);
    }
    const BreakData* getBreakData(IteratorHandle iterator) const {
        ASSERT(!isEndOfBreaks(iterator));
        return &(breaks[getIteratorData(iterator)->breakIndex]);
    }
    long getBreakIndex(IteratorHandle iterator) const {
        return getIteratorData(iterator)->breakIndex;
    }
    void copyToIteratorFromIterator(IteratorHandle dst, IteratorHandle src) {
        IteratorData* to   = getIteratorData(dst);
        IteratorData* from = getIteratorData(src);
        to->breakIndex     = from->breakIndex;
        to->stackStartPos  = from->stackStartPos;
        to->textStartPos   = from->textStartPos;
    }
    long getBreakStartPos(IteratorHandle iterator) const {
        return getIteratorData(iterator)->textStartPos;
    }
    long getBreakEndPos(IteratorHandle iterator) const {
        if (isEndOfBreaks(iterator)) {
            return getIteratorData(iterator)->textStartPos;
        } else {
            return getIteratorData(iterator)->textStartPos + getBreakData(iterator)->breakLength;
        }
    }
    long getPrevBreakStartPos(IteratorHandle iterator) const {
        const IteratorData *I   = getIteratorData(iterator);
        ASSERT(I->breakIndex <= breaks.getLength());
        ASSERT(0 < I->breakIndex);
        return I->textStartPos - breaks[I->breakIndex - 1].nextStartOffset;
    }
    long getPrevBreakEndPos(IteratorHandle iterator) const {
        const IteratorData *I   = getIteratorData(iterator);
        ASSERT(I->breakIndex <= breaks.getLength());
        ASSERT(0 < I->breakIndex);
        return I->textStartPos - breaks[I->breakIndex - 1].nextStartOffset + breaks[I->breakIndex - 1].breakLength;
    }
    BreakType getBreakType(IteratorHandle iterator) const {
        return getBreakData(iterator)->type;
    }
    long getBreakStackBegin(IteratorHandle iterator) const {
        return getIteratorData(iterator)->stackStartPos;
    }
    long getBreakStackEnd(IteratorHandle iterator) const {
        ASSERT(!isEndOfBreaks(iterator));
        return getIteratorData(iterator)->stackStartPos + getBreakData(iterator)->stackLength;
    }
    long getBreakStackLength(IteratorHandle iterator) const {
        ASSERT(!isEndOfBreaks(iterator));
        return getBreakData(iterator)->stackLength;
    }
    byte getBreakStackByte(IteratorHandle iterator, long stackPos) {
        return stack[getBreakStackBegin(iterator) + stackPos];
    }
    byte getLastBreakStackByte(IteratorHandle iterator) {
        return stack[getBreakStackBegin(iterator) + getBreakStackLength(iterator) - 1];
    }
    long getPrevBreakStackBegin(IteratorHandle iterator) const {
        const IteratorData *I   = getIteratorData(iterator);
        ASSERT(I->breakIndex <= breaks.getLength());
        ASSERT(0 < I->breakIndex);
        return I->stackStartPos - breaks[I->breakIndex - 1].stackLength;
    }
    void copyBreakStackTo(IteratorHandle iterator, ByteArray& dest) {
        dest.clear();
        long stackLength = getBreakStackEnd(iterator) - getBreakStackBegin(iterator);
        dest.appendAmount(stackLength);
        memcpy(dest.getPtr(0), stack.getAmount(getBreakStackBegin(iterator), stackLength), stackLength);
    }
    bool hasEqualBreakStack(IteratorHandle iterator, const ByteArray& rhs) {
        long stackLength = getBreakStackEnd(iterator) - getBreakStackBegin(iterator);
        if (stackLength != rhs.getLength()) {
            return false;
        } else {
            return memcmp(rhs.getPtr(0), stack.getAmount(getBreakStackBegin(iterator), stackLength), stackLength) == 0;
        }
    }
    bool isEndOfBreaks(IteratorHandle iterator) const {
        return getIteratorData(iterator)->breakIndex == breaks.getLength();
    }
    bool isLastBreak(IteratorHandle iterator) const {
        return getIteratorData(iterator)->breakIndex >= breaks.getLength() - 1;
    }
    void incIterator(IteratorHandle iterator) {
        ASSERT(!isEndOfBreaks(iterator));
        IteratorData *I   = getIteratorData(iterator);
        I->stackStartPos += breaks[I->breakIndex].stackLength;
        I->textStartPos  += breaks[I->breakIndex].nextStartOffset;
        I->breakIndex    += 1;
        ASSERT(getIteratorData(iterator)->textStartPos >= 0);
    }
    bool isFirstBreak(IteratorHandle iterator) const {
        return getIteratorData(iterator)->breakIndex == 0;
    }
    void decIterator(IteratorHandle iterator) {
        IteratorData *I   = getIteratorData(iterator);
        ASSERT(0 < I->breakIndex);
        ASSERT(getIteratorData(iterator)->textStartPos >= 0);
        I->stackStartPos -= breaks[I->breakIndex - 1].stackLength;
        I->textStartPos  -= breaks[I->breakIndex - 1].nextStartOffset;
        I->breakIndex    -= 1;
        ASSERT(getIteratorData(iterator)->textStartPos >= 0);
        ASSERT(getBreakData(iterator)->breakLength >= 0);
    }
    void moveIteratorToNextBefore(IteratorHandle iterator, long textPos);
    void deleteBreaks(IteratorHandle start, IteratorHandle end);

    void fillInterBreaks(IteratorHandle iterator, long startPos,
            const ByteArray& parsingStack, long fillCount, long fillDistance) {
        insertBreaks(iterator, startPos, 1, Break_INTER, parsingStack, fillCount, fillDistance);
    }
            
    void insertBreak(IteratorHandle iterator, long startPos, long breakLength, BreakType type,
            const ByteArray& parsingStack) {
        insertBreaks(iterator, startPos, breakLength, type, parsingStack, 1, 0);
    }
    void treatTextDataUpdate(IteratorHandle processingRestartedIterator,
            long beginChangedPos, long oldEndChangedPos, long changedAmount);

#ifdef DEBUG    
    void ASSERTvalid();
    void ASSERTIteratorsEndPos(long endPos);
    void ASSERTvalidStacks();
#else
    void ASSERTvalid() {}
    void ASSERTIteratorsEndPos(long endPos) {}
    void ASSERTvalidStacks() {}
#endif
    
    
protected:
    HilitingBase();
    
    void clear();
    
private:

    void insertBreaks(IteratorHandle iterator, long startPos, long breakLength, BreakType type,
            const ByteArray& parsingStack, long fillCount, long fillDistance);

    ObjectArray<IteratorData> iterators;
    long lastFreeIteratorIndex;
    MemBuffer<BreakData> breaks;
    ByteBuffer stack;
};

} // namespace LucED

#endif // HILITINGBASE_H
