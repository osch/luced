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

#include "HilitingBase.hpp"

using namespace LucED;


HilitingBase::HilitingBase()
{
    stack.append('\0');
    
    BreakData* first = breaks.appendAmount(1);
    first->nextStartOffset = 0;
    first->breakLength = 0;
    first->type = Break_NULL;
    first->stackLength = 1;
    lastFreeIteratorIndex = 0;
    iterators.appendAmount(1);
    iterators[0].inUseCounter = 0;
}

HilitingBase::Iterator HilitingBase::createNewIterator()
{
    long i;
    if (lastFreeIteratorIndex == iterators.getLength()) {
        for (i = 0; i < iterators.getLength(); ++i) {
            if (iterators[i].inUseCounter == 0) {
                break;
            }
        }
        if (i == iterators.getLength()) {
            iterators.appendAmount(2);
            iterators[i].inUseCounter = 0;
            iterators[i + 1].inUseCounter = 0;
            lastFreeIteratorIndex = i + 1;
        }
    } else {
        i = lastFreeIteratorIndex++;
    }
    iterators[i].breakIndex = 0;
    iterators[i].stackStartPos = 0;
    iterators[i].textStartPos = 0;
    return Iterator(this, i);
}


void HilitingBase::moveIteratorToNextBefore(IteratorHandle iterator, long textPos)
{
    long dist = abs(textPos - getBreakEndPos(iterator));
    if (textPos - 0  < dist) {
        IteratorData& idata = iterators[iterator.index];
        idata.breakIndex = 0;
        idata.stackStartPos = 0;
        idata.textStartPos = 0;
        dist = textPos - 0;
    }
    for (int i = 0; i < iterators.getLength(); ++i) {
        if (iterators[i].inUseCounter > 0 && i != iterator.index) {
            IteratorHandle I; I.index = i;
            long dist2 = abs(textPos - getBreakEndPos(I));
            if (dist2 < dist) {
                copyToIteratorFromIterator(iterator, I);
                dist = dist2;
            }
        }
    }
    if (isEndOfBreaks(iterator)) {
        while (!isFirstBreak(iterator)) {
            decIterator(iterator);
            if (getBreakEndPos(iterator) <= textPos) {
                break;
            }
        }
    } else {
        if (getBreakEndPos(iterator) < textPos) {
            while (!isLastBreak(iterator)) {
                incIterator(iterator);
                if (textPos < getBreakEndPos(iterator)) {
                    decIterator(iterator);
                    break;
                }
            }
        } else if (getBreakEndPos(iterator) > textPos) {
            while (!isFirstBreak(iterator)) {
                decIterator(iterator);
                if (getBreakEndPos(iterator) <= textPos) {
                    break;
                }
            }
        }
    }
}

void HilitingBase::deleteBreaks(IteratorHandle startIter, IteratorHandle endIter)
{
    long startBreak  = getIteratorData(startIter)->breakIndex;
    long endBreak    = getIteratorData(  endIter)->breakIndex;

    ASSERT(0 < startBreak);
    ASSERT(endBreak <= breaks.getLength());
    
    
    if (startBreak < endBreak)
    {
        long stackBegin  = getBreakStackBegin(startIter);
        long stackEnd    = getBreakStackBegin(endIter);
        long stackLength = stackEnd - stackBegin;
        stack.removeAmount(stackBegin, stackLength);

        long textStart = getIteratorData(startIter)->textStartPos;
        long textEnd   = getIteratorData(  endIter)->textStartPos;
        breaks[startBreak - 1].nextStartOffset += textEnd - textStart;

        long breakCount = endBreak - startBreak;
        breaks.removeAmount(startBreak, breakCount);
        
        for (int i = 0; i < iterators.getLength(); ++i) {
            IteratorData* I = iterators.getPtr(i);
            if (I->inUseCounter > 0)
            {
                if (I->stackStartPos >= stackEnd) {
                    I->stackStartPos -= stackLength;
                } else if (I->stackStartPos > stackBegin) {
                    I->stackStartPos = stackBegin;
                }
                
                if (I->breakIndex >= endBreak) {
                    I->breakIndex -= breakCount;
                } else if (I->breakIndex >= startBreak) {
                    I->breakIndex = startBreak;
                    I->textStartPos = textEnd; // because former start break is now former end break
                }
            }
        }
    }
}

void HilitingBase::treatTextDataUpdate(IteratorHandle processingRestartedIterator,
        long beginChangedPos, long oldEndChangedPos, long changedAmount)
{
//ASSERTvalid();

    moveIteratorToNextBefore(processingRestartedIterator, beginChangedPos);
    ASSERT(getBreakEndPos(processingRestartedIterator) <= beginChangedPos);

    if (!isLastBreak(processingRestartedIterator))
    {
        // delete invalid breaks

        incIterator(processingRestartedIterator);
        Iterator iterator = createNewIterator();
        copyToIteratorFromIterator(iterator, processingRestartedIterator);
        do {
            incIterator(iterator);
        } while (!isEndOfBreaks(iterator) && getBreakStartPos(iterator) < oldEndChangedPos);

        ASSERT(isEndOfBreaks(iterator) || getBreakStartPos(iterator) >= oldEndChangedPos);
        deleteBreaks(processingRestartedIterator, iterator);
        ASSERT(isEndOfBreaks(iterator) || getBreakStartPos(iterator) >= oldEndChangedPos);
        
        ASSERT(isEndOfBreaks(processingRestartedIterator) 
                || getBreakStartPos(processingRestartedIterator) >= oldEndChangedPos); // because former start break is now former end break
        
        bool wasEndOfBreaksGreaterOrEqualThanOldEndChangedPos 
                = (getBreakStartPos(processingRestartedIterator) >= oldEndChangedPos);
        
//ASSERTvalid();

        if (wasEndOfBreaksGreaterOrEqualThanOldEndChangedPos)
        {
            decIterator(processingRestartedIterator);
            ASSERT(getBreakEndPos(processingRestartedIterator) <= beginChangedPos);

            getBreakData(processingRestartedIterator)->nextStartOffset += changedAmount;

            long breakIndex = getIteratorData(processingRestartedIterator)->breakIndex + 1;

            ASSERT(getBreakData(processingRestartedIterator)->nextStartOffset >= 0);

            for (int i = 0; i < iterators.getLength(); ++i) {
                IteratorData* I = iterators.getPtr(i);
                if (I->inUseCounter > 0 && I->breakIndex > 0) {
                    if (I->breakIndex >= breakIndex) {
                        ASSERT(I->textStartPos >= oldEndChangedPos);
                        I->textStartPos += changedAmount;
                    }
                }
            }
        }
        else
        {
            decIterator(processingRestartedIterator);
            ASSERT(isLastBreak(processingRestartedIterator));

            ASSERT(getBreakEndPos(processingRestartedIterator) <= beginChangedPos);

            getBreakData(processingRestartedIterator)->nextStartOffset = 0;

            long textStartPosForIteratorsAtEndOfBreaks = getBreakStartPos(processingRestartedIterator);

            long breakIndex = getIteratorData(processingRestartedIterator)->breakIndex + 1;

            ASSERT(getBreakData(processingRestartedIterator)->nextStartOffset >= 0);

            for (int i = 0; i < iterators.getLength(); ++i) {
                IteratorData* I = iterators.getPtr(i);
                if (I->inUseCounter > 0 && I->breakIndex > 0) {
                    if (I->breakIndex >= breakIndex) {
                        ASSERT(I->breakIndex == breaks.getLength());
                        I->textStartPos = textStartPosForIteratorsAtEndOfBreaks;
                    }
                }
            }
        }
    }
//ASSERTvalid();
}


void HilitingBase::insertBreaks(IteratorHandle iterator, long startPos, long breakLength, BreakType type,
            const ByteArray& parsingStack, long fillCount, long fillDistance)
{
    ASSERT(parsingStack[0] == 0);
    ASSERT(!isFirstBreak(iterator));
    ASSERT(getPrevBreakEndPos(iterator) <= startPos);
    ASSERT(type == Break_INTER || fillCount == 1);
    ASSERT((fillCount == 1) || (breakLength == 0));
    ASSERT(isEndOfBreaks(iterator)
            || (startPos + (fillCount - 1) * fillDistance + breakLength <= getBreakStartPos(iterator)));
    
    bool iteratorIsEndOfBreaks = isEndOfBreaks(iterator);
    long breakIndex        = getBreakIndex(iterator);
    long nextBreakStartPos = getBreakStartPos(iterator);
    long prevBreakStartPos = getPrevBreakStartPos(iterator);
    long stackBegin        = getBreakStackBegin(iterator);
    
    long stackLength = parsingStack.getLength();

    byte* stackStart = stack.insertAmount(stackBegin, fillCount * stackLength);
    for (long i = 0; i < fillCount; ++i) {
        memcpy(stackStart + i * stackLength, parsingStack.getPtr(0), stackLength);
    }

    breaks.insertAmount(breakIndex, fillCount);
    breaks[breakIndex - 1].nextStartOffset = startPos - prevBreakStartPos;
    
    long i;
    for (i = 0; i < fillCount - 1; ++i) {
        breaks[breakIndex + i].nextStartOffset = fillDistance;
        breaks[breakIndex + i].breakLength     = breakLength;
        breaks[breakIndex + i].type            = type;
        breaks[breakIndex + i].stackLength     = stackLength;
    }
    if (iteratorIsEndOfBreaks) {
        breaks[breakIndex + i].nextStartOffset = breakLength;
        ASSERT(breakLength >= 0);
    } else {
        breaks[breakIndex + i].nextStartOffset = nextBreakStartPos - (startPos + (fillCount - 1) * fillDistance);
        ASSERT(breaks[breakIndex + i].nextStartOffset >= 0);
    }
    breaks[breakIndex + i].breakLength     = breakLength;
    breaks[breakIndex + i].type            = type;
    breaks[breakIndex + i].stackLength     = stackLength;

    // Correcting iterators
    
    iterators.getPtr(iterator.index)->textStartPos = startPos;

    if (iteratorIsEndOfBreaks) {
        for (int i = 0; i < iterators.getLength(); ++i) {
            IteratorData* I = iterators.getPtr(i);
            if (I->inUseCounter > 0 && i != iterator.index) {
                if (I->breakIndex == breakIndex) {
                    I->textStartPos = startPos + (fillCount - 1) * fillDistance + breakLength;
                }
            } 
        }
    }

    for (int i = 0; i < iterators.getLength(); ++i)
    {
        if (i != iterator.index)
        {
            IteratorData* I = iterators.getPtr(i);
            if (I->inUseCounter > 0) {
                if (I->stackStartPos >= stackBegin) {
                    I->stackStartPos += fillCount * stackLength;
                }
                if (I->breakIndex >= breakIndex) {
                    I->breakIndex += fillCount;
                }
            }
        }
    }
    
}

#ifdef DEBUG
void HilitingBase::ASSERTvalid()
{
    Iterator breakIter = createNewIterator();
    int i;
    IteratorHandle I;
    
    for (i = 0; i < iterators.getLength(); ++i) {
        if (iterators.getPtr(i)->inUseCounter > 0
                && i != breakIter.index) {
        
            I.index = i;
            
            copyToIteratorFromIterator(breakIter, I);
            
            while (!isFirstBreak(breakIter)) {
                decIterator(breakIter);
            }
            long breakStartPos = getBreakStartPos(breakIter);
            long breakEndPos   = getBreakEndPos(breakIter);
            ASSERT(breakStartPos == 0);
            ASSERT(breakEndPos == 0);
        }
    }
}
#endif

#ifdef DEBUG
void HilitingBase::ASSERTvalidStacks()
{
fprintf(stderr, "Check Stacks\n");
    Iterator breakIter = createNewIterator();
    int i;
    IteratorHandle I;
    
    for (i = 0; i < iterators.getLength(); ++i) {
        if (iterators.getPtr(i)->inUseCounter > 0
                && i != breakIter.index) {
        
            I.index = i;
            
            copyToIteratorFromIterator(breakIter, I);
            
            
            
            if (isEndOfBreaks(breakIter)) {
                long stackBegin = getBreakStackBegin(breakIter);
                long stackEnd   = getBreakStackEnd(breakIter);
                ASSERT(stackBegin == stack.getLength());
                ASSERT(stackEnd == stack.getLength());
            } else {
                long stackBegin = getBreakStackBegin(breakIter);
                long stackEnd   = getBreakStackEnd(breakIter);
                ASSERT(stackEnd <= stack.getLength());
            }
            
            while (!isFirstBreak(breakIter)) {
                decIterator(breakIter);
            }
            long stackBegin = getBreakStackBegin(breakIter);
            long stackEnd   = getBreakStackEnd(breakIter);
            ASSERT(stackBegin == 0);
            ASSERT(stackEnd == 1);
        }
    }
}
#endif

#ifdef DEBUG
void HilitingBase::ASSERTIteratorsEndPos(long endPos)
{
    Iterator breakIter = createNewIterator();
    int i;
    IteratorHandle I;
    
    for (i = 0; i < iterators.getLength(); ++i) {
        if (iterators.getPtr(i)->inUseCounter > 0
                && i != breakIter.index) {
        
            I.index = i;
            
            copyToIteratorFromIterator(breakIter, I);
            
            while (!isLastBreak(breakIter)) {
                incIterator(breakIter);
            }
            if (!isEndOfBreaks(breakIter)) {
                ASSERT(getBreakEndPos(breakIter) <= endPos);
            }
        }
    }
}
#endif

