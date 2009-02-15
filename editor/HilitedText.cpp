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

#include "util.hpp"
#include "HilitedText.hpp"
#include "EventDispatcher.hpp"
#include "GlobalConfig.hpp"

//#define processAmountUnit (10 * breakPointDistance)  // TODO: muss größer sein als das größte vorkommende Pattern
//#define processAmountUnit 500  // TODO: muss größer sein als das größte vorkommende Pattern
//#define breakPointDistance  500
//#define processAmountUnit (80 * breakPointDistance)  // TODO: muss größer sein als das größte vorkommende Pattern

using namespace LucED;


HilitedText::HilitedText(TextData::Ptr textData, LanguageMode::Ptr languageMode)
        : processingEndBeforeRestartIterator(createNewIterator()),
          processingEndBeforeRestartFlag(false),
          textData(textData),
          rememberedLastProcessingRestartedIterator(createNewIterator()),
          languageMode(languageMode),
          startNextProcessIterator(createNewIterator()),
          tryToBeLastBreakIterator(createNewIterator()),
          processHandler(ProcessHandler::create(this, &HilitedText::process, &HilitedText::needsProcessing))
{
    textData                      ->registerUpdateListener(newCallback(this, &HilitedText::treatTextDataUpdate));
    EventDispatcher::getInstance()->registerUpdateSource  (newCallback(this, &HilitedText::flushPendingUpdates));

    this->beginChangedPos = 0;
    this->endChangedPos = 0;
    this->processingEndBeforeRestartFlag = false;
    this->needsProcessingFlag = false;

    this->syntaxPatternUpdateCallback = newCallback(this, &HilitedText::treatSyntaxPatternsUpdate);
    this->syntaxPatterns = GlobalConfig::getInstance()->getSyntaxPatternsForLanguageMode(this->languageMode,
                                                                                         this->syntaxPatternUpdateCallback);
    if (languageMode.isValid()) {
        this->breakPointDistance = languageMode->getHilitingBreakPointDistance();
    } else {
        this->breakPointDistance = 50;
    }

    ASSERT(syntaxPatterns.isValid());
    
    if (syntaxPatterns->hasPatterns()) {
        this->ovector.increaseTo(syntaxPatterns->getMaxOvecSize());
    }
    EventDispatcher::getInstance()->registerProcess(processHandler);
}


void HilitedText::setLanguageMode(LanguageMode::Ptr languageMode)
{
    if (languageMode != this->languageMode)
    {
        this->languageMode = languageMode;
        if (languageMode.isValid()) {
            this->breakPointDistance = languageMode->getHilitingBreakPointDistance();
        } else {
            this->breakPointDistance = 50;
        }
        this->syntaxPatternUpdateCallback->disable();
        this->syntaxPatternUpdateCallback = newCallback(this, &HilitedText::treatSyntaxPatternsUpdate);

        treatSyntaxPatternsUpdate(GlobalConfig::getInstance()->getSyntaxPatternsForLanguageMode(this->languageMode,
                                                                                                this->syntaxPatternUpdateCallback));
    }
}

void HilitedText::treatSyntaxPatternsUpdate(SyntaxPatterns::Ptr newSyntaxPatterns)
{
    if (this->syntaxPatterns != newSyntaxPatterns)
    {
        this->syntaxPatterns = newSyntaxPatterns;
        if (syntaxPatterns->hasPatterns()) {
            this->ovector.increaseTo(syntaxPatterns->getMaxOvecSize());
        }
        HilitingBase::clear();
    
        this->beginChangedPos = 0;
        this->endChangedPos = 0;
        this->processingEndBeforeRestartFlag = false;
        this->needsProcessingFlag = newSyntaxPatterns.isValid();
        
        hilitingChangedCallbacks.invokeAllCallbacks(this);
        
        syntaxPatternsChangedCallbacks.invokeAllCallbacks(newSyntaxPatterns);
    }
}


bool HilitedText::setBreak(IteratorHandle iterator, 
                           long startPos1, long startPos, long endPos, BreakType type, 
                           const PatternStack& parsingStack)
{
    ASSERT(!isFirstBreak(iterator));
    ASSERT(startPos1 == startPos 
            || (type == Break_INTER && startPos == endPos));
    ASSERT(startPos1 <= startPos && startPos <= endPos);
    
    bool canBeStopped = false;
    {    
        Iterator beginIterator = createNewIterator();
        copyToIteratorFromIterator(beginIterator, iterator);
        while (!isEndOfBreaks(iterator) && getBreakStartPos(iterator) <= endPos)
        {
            if (!canBeStopped
                    && (!processingEndBeforeRestartFlag 
                            || getBreakIndex(iterator) >= getBreakIndex(processingEndBeforeRestartIterator))
                    && getBreakStartPos(iterator) >= startPos1
                    && getBreakStartPos(iterator) <= startPos
                    && (startPos1 != startPos || getBreakEndPos(iterator) == endPos)
                    && type == getBreakType(iterator)
                    && hasEqualBreakStack(iterator, parsingStack))
            {
                canBeStopped = true;
            }
            incIterator(iterator);
        }
        deleteBreaks(beginIterator, iterator);
    }
    ASSERT(isEndOfBreaks(iterator) || getBreakStartPos(iterator) > endPos);

    ASSERT(endPos <= textData->getLength());

    insertBreak(iterator, startPos, endPos - startPos, type, parsingStack);

    return canBeStopped;   
}


bool HilitedText::fillWithBreaks(IteratorHandle iterator, 
        long fillStart, long fillEnd,
        long *lastFillEnd, PatternStack& patternStack)
{
    long p;
    long p1 = fillStart;
    bool canBeStopped = false;

    if (*lastFillEnd + breakPointDistance <= fillEnd) {
        p = *lastFillEnd + breakPointDistance;
        util::maximize(&p, p1);
        do {
            ASSERT(!isEndOfBreaks(iterator));
            incIterator(iterator);
            canBeStopped = setBreak(iterator, p1, p, p, 
                    Break_INTER, patternStack) || canBeStopped;
            ASSERT(!isEndOfBreaks(iterator));
                    
            *lastFillEnd = getBreakEndPos(iterator); // p;
            p1  = p;
            p  += breakPointDistance;
        } while (p < fillEnd);
    }
    return canBeStopped;
}


inline int HilitedText::getReparseDistance(IteratorHandle iterator)
{
    BreakType type = getBreakType(iterator);
    
    int rslt = 0;
    PatternStack patternStack;
    copyBreakStackTo(iterator, patternStack);
    
    if (type == Break_BEGIN && patternStack.getLength() > 0) {
        patternStack.removeLast();
    }
    while (patternStack.getLength() > 0)
    {
        byte patternId = patternStack.getLast();
        SyntaxPattern* pattern = syntaxPatterns->get(patternId);
        util::maximize(&rslt, pattern->maxREBytesExtend);
        patternStack.removeLast();
    }
    return rslt;
}


void HilitedText::gotoReparseStart(long textPos, IteratorHandle iterator)
{
    moveIteratorToNextBefore(iterator, textPos);
    int reparseDistance = getReparseDistance(iterator);
    while (true)
    {
        if (isFirstBreak(iterator)) {
            return;
        }
        if (textPos - getBreakStartPos(iterator) <= reparseDistance) {
            // noch weiter vorne Reparsen
            decIterator(iterator);
            continue;
        } else {
            return;
        }
    }
}


bool HilitedText::needsProcessing()
{
    return needsProcessingFlag;
}

void HilitedText::treatTextDataUpdate(TextData::UpdateInfo u)
{
    if (!syntaxPatterns->hasPatterns()) {
        return;
    }
    HilitingBase::treatTextDataUpdate(rememberedLastProcessingRestartedIterator, 
            u.beginChangedPos, u.oldEndChangedPos, u.changedAmount);

#if 0
printf("------HilitedText::gotoReparseStart davor!!\n");
#endif
    gotoReparseStart(u.beginChangedPos, rememberedLastProcessingRestartedIterator);
#if 0
printf("------HilitedText::gotoReparseStart start = %ld / %ld  --> %d\n", getBreakStartPos(rememberedLastProcessingRestartedIterator),
                                                                u.beginChangedPos, 
                                                                getLastBreakStackByte(rememberedLastProcessingRestartedIterator));
#endif

    if (!processingEndBeforeRestartFlag) {
        processingEndBeforeRestartFlag = true;
        // remember current position as highest interruption
        copyToIteratorFromIterator(processingEndBeforeRestartIterator, rememberedLastProcessingRestartedIterator);
    } else if (getBreakIndex(processingEndBeforeRestartIterator) <= getBreakIndex(rememberedLastProcessingRestartedIterator)) {
        // remember current position as highest interruption
        copyToIteratorFromIterator(processingEndBeforeRestartIterator, rememberedLastProcessingRestartedIterator);
    }
    while (!isLastBreak(processingEndBeforeRestartIterator)
            && getBreakStartPos(processingEndBeforeRestartIterator) <= u.beginChangedPos) {
        incIterator(processingEndBeforeRestartIterator);
    }

    if (needsProcessingFlag && getBreakIndex(startNextProcessIterator) > getBreakIndex(rememberedLastProcessingRestartedIterator))
    {
        // restart because current hiliting process is below
        
        // interrupt current processing

        if (getBreakIndex(processingEndBeforeRestartIterator) <= getBreakIndex(startNextProcessIterator)) {
            // remember current processing position as highest interruption
            copyToIteratorFromIterator(processingEndBeforeRestartIterator, startNextProcessIterator);
        }
        copyToIteratorFromIterator(startNextProcessIterator, rememberedLastProcessingRestartedIterator);
    }
    else if (!needsProcessingFlag) {
        copyToIteratorFromIterator(startNextProcessIterator, rememberedLastProcessingRestartedIterator);
        needsProcessingFlag = true;
    }
    ASSERT(!isEndOfBreaks(startNextProcessIterator));
}

void HilitedText::registerUpdateListener(Callback<UpdateInfo>::Ptr updateCallback)
{
    updateListeners.registerCallback(updateCallback);
}

void HilitedText::flushPendingUpdates()
{
    if (this->endChangedPos == 0) {
        return;
    }
    
    // todo: überdenken!!
    ASSERT(this->beginChangedPos <= this->endChangedPos);
    if (this->beginChangedPos < this->endChangedPos) {
        updateListeners.invokeAllCallbacks(UpdateInfo(this->beginChangedPos, this->endChangedPos));
    }
    this->beginChangedPos = textData->getLength();
    this->endChangedPos   = 0;
}

int HilitedText::pcreCalloutFunction(void* voidPtr, pcre_callout_block* calloutBlock)
{
    HilitedText* self = static_cast<HilitedText*>(voidPtr);

    ASSERT(calloutBlock->callout_number == 1);

    bool didMatch = false;

    if (   calloutBlock->capture_last != -1 
        && self->ovector[calloutBlock->capture_last * 2 + 1] == calloutBlock->current_position)
    {
        int i1 = self->ovector[calloutBlock->capture_last * 2 + 0];
        int i2 = self->ovector[calloutBlock->capture_last * 2 + 1];
        
        didMatch = self->pushedSubstr.equals(calloutBlock->subject + i1, i2 - i1);
    }

    return didMatch ? 0 : 1;
}



int HilitedText::process(TimeVal endTime)
{
    if (!syntaxPatterns->hasPatterns()) {
        needsProcessingFlag =  false;
        return 0;
    }
    ASSERT(!isEndOfBreaks(startNextProcessIterator));
    long pos = getBreakEndPos(startNextProcessIterator);

    long wasStartPos = pos;
    long lastSetBreakEnd = pos;
    SyntaxPattern* sp = syntaxPatterns->get(getLastBreakStackByte(startNextProcessIterator));

    long processAmountUnit = 10 * breakPointDistance;

    util::maximize(&processAmountUnit, (long)3000);

    long searchEndPos = pos + processAmountUnit;

#if 0
TimeVal startTime = TimeVal::now();
printf("------HilitedText::process start = %ld / %ld .. %ld --> %d\n", getBreakStartPos(startNextProcessIterator),
                                                                pos, searchEndPos, 
                                                                getLastBreakStackByte(startNextProcessIterator));
#endif

    long oldBreakPos = pos;
    long foundStartPos = pos;
    long foundEndPos = pos;
    BreakType foundType = Break_INTER;
    util::minimize(&this->beginChangedPos, pos);
    bool canBeStopped = false;
    copyBreakStackTo(startNextProcessIterator, patternStack);
    pushedSubstr = patternStack.getAdditionalDataAsString();
    
    bool wasZeroLengthMatch = false;
    bool loopFinished = false;
    const long textDataLength = textData->getLength();
    do
    {
        util::minimize(&searchEndPos, textDataLength);
        
        while (!canBeStopped && pos < searchEndPos)
        {
            ASSERT(sp == syntaxPatterns->get(patternStack.getLast()));
            long extendedSearchEndPos = searchEndPos + sp->maxREBytesExtend;
            BasicRegex::MatchOptions additionalOptions;
            
            util::minimize(&extendedSearchEndPos, textDataLength);
            if (!textData->isBeginOfLine(pos)) {
                additionalOptions |= BasicRegex::NOTBOL;
            }
            if (!textData->isEndOfLine(extendedSearchEndPos)) {
                additionalOptions |= BasicRegex::NOTEOL;
            }
            
            bool matched = sp->re.findMatch(this, &HilitedText::pcreCalloutFunction,
                                            (const char*) textData->getAmount(pos, extendedSearchEndPos - pos), 
                        extendedSearchEndPos - pos, 0,
                        additionalOptions /*| BasicRegex::NOTEMPTY*/, ovector);
            
            if (matched)
            {
                // something matched
    
                if (ovector[1] == 0)
                {
                    if (wasZeroLengthMatch) {
                        ovector[1] = 1; // prevent endless loop for second zero length match
                        wasZeroLengthMatch = false;
                    }
                    else {
                        wasZeroLengthMatch = true;
                    }
                }
                else {
                    wasZeroLengthMatch = false;
                }
    
                canBeStopped = fillWithBreaks(startNextProcessIterator, pos, pos + ovector[0], &lastSetBreakEnd, patternStack);
                if (canBeStopped) break;
                
                int cid = sp->getMatchedChild(ovector);
                if (cid == -1)
                {
                    // EndPattern matched
     
                    foundStartPos = pos + ovector[0];
                    foundEndPos = pos + ovector[1];
                    foundType = Break_END;
    
                    //memset(hb->buffer + pos + ovector[0], sp->style, ovector[1] - ovector[0]);
                    pos += ovector[1];
                    patternStack.removeLast();
                    sp = syntaxPatterns->get(patternStack.getLast());
                    pushedSubstr = patternStack.getAdditionalDataAsString();
                }
                else
                {
                    // normal Child matched
                    
                    SyntaxPattern* cpat = syntaxPatterns->getChildPattern(sp, cid);
                        
                    if (patternStack.getLength() >= STACK_SIZE
                            && cpat->hasEndPattern) {
    
                        // New Begin-Child, but Stack is too big
    
                        foundStartPos = pos + ovector[1];
                        foundEndPos   = foundStartPos;
                        pos += ovector[1];
                        foundType = Break_INTER;
                        
                    } else {
                        
                        // Stack is ok or doesn't need to grow
    
                        foundStartPos = pos + ovector[0];
                        foundEndPos   = pos + ovector[1];
    
                        if (cpat->hasEndPattern) {
                            if (!cpat->hasPushedSubstr) {
                                pushedSubstr = String();
                                patternStack.append(syntaxPatterns->getChildPatternId(sp, cid));
                            } else {
                                
                                int pushedSubstrNo = syntaxPatterns->getPushedSubstrNo(sp, cid);
                            
                                pushedSubstr = textData->getSubstringBetween(pos + ovector[pushedSubstrNo * 2 + 0],
                                                                             pos + ovector[pushedSubstrNo * 2 + 1]);
    
                                patternStack.append(syntaxPatterns->getChildPatternId(sp, cid),
                                                    pushedSubstr);
                            }
                            sp = cpat;
                            foundType = Break_BEGIN;
                        } else {
                            foundType = Break_INTER;
                        }
    
                        pos += ovector[1];
                    }
                }
            }
            else
            {
                // nothing matched
    
                canBeStopped = fillWithBreaks(startNextProcessIterator, pos, searchEndPos, &lastSetBreakEnd, patternStack);
                if (canBeStopped) break;
                //memset(hb->buffer + pos, sp->style, searchEndPos - pos);
                pos = searchEndPos;
                foundStartPos = foundEndPos = pos;
                foundType = Break_INTER;
            }
            if (foundEndPos >= getBreakEndPos(startNextProcessIterator) + breakPointDistance)
            {
                incIterator(startNextProcessIterator);
                canBeStopped = setBreak(startNextProcessIterator, foundStartPos, foundStartPos, foundEndPos, 
                        foundType, patternStack);
                lastSetBreakEnd = getBreakEndPos(startNextProcessIterator);
                if (canBeStopped) break;
            }
        }
        if (canBeStopped || pos >= textDataLength || TimeVal::now() >= endTime) {
            loopFinished = true;
        }
        else
        {
            searchEndPos = pos + processAmountUnit;
        }
    } while (!loopFinished);
    
    int processedAmount = (pos - wasStartPos);
    
    util::maximize(&this->endChangedPos, getBreakEndPos(startNextProcessIterator));
    util::maximize(&this->endChangedPos, lastSetBreakEnd);

    ASSERT(!isEndOfBreaks(startNextProcessIterator));

    if (canBeStopped)
    {
        this->processingEndBeforeRestartFlag = false;
        while (!isEndOfBreaks(tryToBeLastBreakIterator)) {
            incIterator(tryToBeLastBreakIterator);
        }
        if (textDataLength - getBreakStartPos(tryToBeLastBreakIterator) < 2 * breakPointDistance) { 
            this->needsProcessingFlag = false;
            this->endChangedPos = textDataLength;
        } else {
            this->needsProcessingFlag = true;

            incIterator(startNextProcessIterator);
            util::maximize(&this->endChangedPos, getBreakStartPos(startNextProcessIterator));

            copyToIteratorFromIterator(startNextProcessIterator, tryToBeLastBreakIterator);
            decIterator(startNextProcessIterator);
        }
#if 0
printf("------HilitedText::process can be stoppend = %ld / %ld (%d) --> %d  <%ld>\n", getBreakStartPos(startNextProcessIterator),
                                                                   getBreakEndPos(startNextProcessIterator), processedAmount, 
                                                                   getLastBreakStackByte(startNextProcessIterator),
                                                                   (long)startTime.getMicroSecsBefore(TimeVal::now())/1000);
#endif
    }
    else
    {
        if (searchEndPos >= textDataLength)
        {
            this->processingEndBeforeRestartFlag = false;
            this->needsProcessingFlag = false;
            
            if (!isLastBreak(startNextProcessIterator))
            {
                // delete trailing Breaks

                incIterator(startNextProcessIterator);
                Iterator tempIterator = createNewIterator();
                copyToIteratorFromIterator(tempIterator, startNextProcessIterator);
                while (!isEndOfBreaks(tempIterator)) {
                    incIterator(tempIterator);
                }
                deleteBreaks(startNextProcessIterator, tempIterator);
            }
            util::maximize(&this->endChangedPos, searchEndPos);
        }
#if 0
long overTime = 0;
if (TimeVal::now() > endTime) {
    overTime = (long)endTime.getMicroSecsBefore(TimeVal::now())/1000;
}
printf("------HilitedText::process cannot be stoppend = %ld / %ld  (%d) --> %d  <%ld,  %ld>\n", getBreakStartPos(startNextProcessIterator),
                                                                   getBreakEndPos(startNextProcessIterator), processedAmount, 
                                                                   getLastBreakStackByte(startNextProcessIterator),
                                                                   (long)startTime.getMicroSecsBefore(TimeVal::now())/1000,
                                                                   overTime);
#endif
    }

    ASSERT(!needsProcessingFlag || !isEndOfBreaks(startNextProcessIterator));
    return processedAmount;
}

