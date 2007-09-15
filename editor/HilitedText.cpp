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
    textData                      ->registerUpdateListener(Callback1<TextData::UpdateInfo>(this, &HilitedText::treatTextDataUpdate));
    EventDispatcher::getInstance()->registerUpdateSource  (Callback0                      (this, &HilitedText::flushPendingUpdates));

    this->beginChangedPos = 0;
    this->endChangedPos = 0;
    this->processingEndBeforeRestartFlag = false;
    this->needsProcessingFlag = false;

    this->syntaxPatternUpdateCallback = Callback1<SyntaxPatterns::Ptr>(this, &HilitedText::treatSyntaxPatternsUpdate);
    this->syntaxPatterns = GlobalConfig::getInstance()->getSyntaxPatternsForLanguageMode(this->languageMode,
                                                                                         this->syntaxPatternUpdateCallback);
    if (languageMode.isValid()) {
        this->breakPointDistance = languageMode->getHilitingBreakPointDistance();
    } else {
        this->breakPointDistance = 50;
    }

    if (syntaxPatterns.isValid()) {
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
        this->syntaxPatternUpdateCallback.disable();
        this->syntaxPatternUpdateCallback = Callback1<SyntaxPatterns::Ptr>(this, &HilitedText::treatSyntaxPatternsUpdate);

        treatSyntaxPatternsUpdate(GlobalConfig::getInstance()->getSyntaxPatternsForLanguageMode(this->languageMode,
                                                                                                this->syntaxPatternUpdateCallback));
    }
}

void HilitedText::treatSyntaxPatternsUpdate(SyntaxPatterns::Ptr newSyntaxPatterns)
{
    this->syntaxPatterns = newSyntaxPatterns;
    if (syntaxPatterns.isValid()) {
        this->ovector.increaseTo(syntaxPatterns->getMaxOvecSize());
    }
    HilitingBase::clear();

    this->beginChangedPos = 0;
    this->endChangedPos = 0;
    this->processingEndBeforeRestartFlag = false;
    this->needsProcessingFlag = newSyntaxPatterns.isValid();
    
    hilitingChangedCallbacks.invokeAllCallbacks(this);
}


bool HilitedText::setBreak(IteratorHandle iterator, 
                           long startPos1, long startPos, long endPos, BreakType type, 
                           const ByteArray& parsingStack)
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
        long *lastFillEnd, ByteArray& patternStack)
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


void HilitedText::gotoReparseStart(long textPos, IteratorHandle iterator)
{
    moveIteratorToNextBefore(iterator, textPos);
    while (true)
    {
        if (isFirstBreak(iterator)) {
            return;
        }
        BreakType type = getBreakType(iterator);
        if (type == Break_BEGIN) {
            byte parentPatternId = getBreakStackByte(iterator, getBreakStackLength(iterator) - 2);  
            SyntaxPattern* parentPattern = syntaxPatterns->get(parentPatternId);
            if (textPos - getBreakStartPos(iterator) <= parentPattern->maxREBytesExtend) {
                // noch weiter vorne Reparsen
                decIterator(iterator);
                continue;
            } else {
                return;
            }
        } else if (type == Break_INTER) {
            byte patternId = getLastBreakStackByte(iterator);
            SyntaxPattern *pattern = syntaxPatterns->get(patternId);
            if (textPos - getBreakStartPos(iterator) <= pattern->maxREBytesExtend) {
                // noch weiter vorne Reparsen
                decIterator(iterator);
                continue;
            } else {
                return;
            }
        } else if (type == Break_END) {
            byte patternId = getLastBreakStackByte(iterator);
            SyntaxPattern *pattern = syntaxPatterns->get(patternId);
            if (textPos - getBreakStartPos(iterator) <= pattern->maxREBytesExtend) {
                // noch weiter vorne Reparsen
                decIterator(iterator);
                continue;
            } else {
                return;
            }
        }
    }
}

bool HilitedText::needsProcessing()
{
    return needsProcessingFlag;
}

void HilitedText::treatTextDataUpdate(TextData::UpdateInfo u)
{
    if (!syntaxPatterns.isValid()) {
        return;
    }
    HilitingBase::treatTextDataUpdate(rememberedLastProcessingRestartedIterator, 
            u.beginChangedPos, u.oldEndChangedPos, u.changedAmount);

    gotoReparseStart(u.beginChangedPos, rememberedLastProcessingRestartedIterator);

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

void HilitedText::registerUpdateListener(const Callback1<UpdateInfo>& updateCallback)
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

int HilitedText::process(int requestedProcessingAmount)
{
    if (!syntaxPatterns.isValid()) {
        needsProcessingFlag =  false;
        return 0;
    }
    ASSERT(!isEndOfBreaks(startNextProcessIterator));
    long pos = getBreakEndPos(startNextProcessIterator);
    long wasStartPos = pos;
    long lastSetBreakEnd = pos;
    SyntaxPattern* sp = syntaxPatterns->get(getLastBreakStackByte(startNextProcessIterator));

    const long processAmountUnit = 10 * breakPointDistance;

    long searchEndPos = pos + processAmountUnit * requestedProcessingAmount;
    util::minimize(&searchEndPos, textData->getLength());
    long oldBreakPos = pos;
    long foundStartPos = pos;
    long foundEndPos = pos;
    BreakType foundType = Break_INTER;
    util::minimize(&this->beginChangedPos, pos);
    bool canBeStopped = false;
    copyBreakStackTo(startNextProcessIterator, this->patternStack);
    
    while (!canBeStopped && pos < searchEndPos)
    {
        ASSERT(sp == syntaxPatterns->get(patternStack.getLast()));
        long extendedSearchEndPos = searchEndPos + sp->maxREBytesExtend;
        Regex::MatchOptions additionalOptions;
        
        util::minimize(&extendedSearchEndPos, textData->getLength());
        if (!textData->isBeginOfLine(pos)) {
            additionalOptions |= Regex::NOTBOL;
        }
        if (!textData->isEndOfLine(extendedSearchEndPos)) {
            additionalOptions |= Regex::NOTEOL;
        }
        bool matched = sp->re.findMatch( (const char*) textData->getAmount(pos, extendedSearchEndPos - pos), 
                extendedSearchEndPos - pos, 0,
                additionalOptions /*| Regex::NOTEMPTY*/, ovector);

        if (matched)
        {
            // something matched

            canBeStopped = fillWithBreaks(startNextProcessIterator, pos, pos + ovector[0], &lastSetBreakEnd, this->patternStack);
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
                    pos += ovector[1];
                    if (cpat->hasEndPattern) {
                        patternStack.append(syntaxPatterns->getChildPatternId(sp, cid));
                        sp = cpat;
                        foundType = Break_BEGIN;
                    } else {
                        foundType = Break_INTER;
                    }
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
    int processedAmount = (pos - wasStartPos)/processAmountUnit;
    
    util::maximize(&this->endChangedPos, searchEndPos);

    ASSERT(!isEndOfBreaks(startNextProcessIterator));

    if (canBeStopped)
    {
        this->processingEndBeforeRestartFlag = false;
        while (!isEndOfBreaks(tryToBeLastBreakIterator)) {
            incIterator(tryToBeLastBreakIterator);
        }
        if (textData->getLength() - getBreakStartPos(tryToBeLastBreakIterator) < 2 * breakPointDistance) { 
            this->needsProcessingFlag = false;
        } else {
            this->needsProcessingFlag = true;
            copyToIteratorFromIterator(startNextProcessIterator, tryToBeLastBreakIterator);
            decIterator(startNextProcessIterator);
        }
    }
    else
    {
        if (searchEndPos >= textData->getLength())
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
        }
    }
    ASSERT(!needsProcessingFlag || !isEndOfBreaks(startNextProcessIterator));
    return processedAmount;
}

