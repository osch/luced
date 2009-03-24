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
#include "HilitingBuffer.hpp"

using namespace LucED;

static long calculateMaxDistance(HilitedText::Ptr hilitedText)
{
    if (hilitedText->getSyntaxPatterns().isValid()) {
        return 3 * util::maximum(
            hilitedText->getLanguageMode()->getHilitingBreakPointDistance(),
            hilitedText->getSyntaxPatterns()->getTotalMaxExtend());
    } else {
        return 3 * hilitedText->getLanguageMode()->getHilitingBreakPointDistance();
    }
}

HilitingBuffer::HilitingBuffer(HilitedText::Ptr hilitedText)
  : startPos(0),
    hilitedText(hilitedText),
    syntaxPatterns(hilitedText->getSyntaxPatterns()),
    languageMode(hilitedText->getLanguageMode()),
    iterator(hilitedText->createNewIterator()),
    maxDistance(calculateMaxDistance(hilitedText))
    
{
    hilitedText->registerHilitingChangedCallback(newCallback(this, &HilitingBuffer::treatChangedHiliting));

    textData = hilitedText->getTextData();
    if (syntaxPatterns->hasPatterns()) {
        ovector.increaseTo(syntaxPatterns->getMaxOvecSize());
    }
    hilitedText->registerUpdateListener(newCallback(this, &HilitingBuffer::treatHilitingUpdate));
    textData->registerUpdateListener(newCallback(this, &HilitingBuffer::treatTextDataUpdate));

    syntaxPatterns->registerTextStylesChangedCallback(newCallback(this, &HilitingBuffer::treatTextStylesChanged));
}

void HilitingBuffer::treatTextStylesChanged()
{
    textStylesChangedListeners.invokeAllCallbacks(syntaxPatterns->getTextStylesArray());
}

void HilitingBuffer::treatChangedHiliting(HilitedText* changedHiliting)
{
    ASSERT(changedHiliting == hilitedText);
    ASSERT(hilitedText->getTextData() == textData);
    
    syntaxPatterns = hilitedText->getSyntaxPatterns();
    languageMode   = hilitedText->getLanguageMode();

    syntaxPatterns->registerTextStylesChangedCallback(newCallback(this, &HilitingBuffer::treatTextStylesChanged));

    maxDistance = calculateMaxDistance(hilitedText);
    if (syntaxPatterns->hasPatterns()) {
        ovector.increaseTo(syntaxPatterns->getMaxOvecSize());
    }
    styleBuffer.clear();
    patternStack.clear();
    textStylesChangedListeners.invokeAllCallbacks(syntaxPatterns->getTextStylesArray());
    updateListeners           .invokeAllCallbacks(UpdateInfo(0, textData->getLength()));
}


void HilitingBuffer::treatHilitingUpdate(HilitedText::UpdateInfo update)
{
    ASSERT(update.beginPos < update.endPos);
    
    if (update.beginPos < this->startPos + styleBuffer.getLength() && update.endPos > this->startPos)
    {
        styleBuffer.clear();
        patternStack.clear();
    }
    updateListeners.invokeAllCallbacks(UpdateInfo(update.beginPos, update.endPos));
}


void HilitingBuffer::treatTextDataUpdate(TextData::UpdateInfo update)
{
    if (update.beginChangedPos  < this->startPos + styleBuffer.getLength()
     && update.oldEndChangedPos > this->startPos)
    {
        styleBuffer.clear();
        patternStack.clear();
    }
    else if (update.oldEndChangedPos <= this->startPos)
    {
        this->startPos                   += update.changedAmount;
        this->rememberedSearchRestartPos += update.changedAmount;
    }
}


static inline void fillSubs(ObjectArray<CombinedSubPatternStyle>& sps, ByteArray& array, int pos, MemArray<int>& ovector)
{
    for (int i = 0; i < sps.getLength(); ++i) {
        int subNo = sps[i].substrNo * 2;
        if (ovector[subNo] != -1) {
            array.fillAmountWith(pos + ovector[subNo], ovector[subNo + 1] - ovector[subNo], sps[i].style);
        }
    }
}


int HilitingBuffer::pcreCalloutFunction(void* voidPtr, pcre_callout_block* calloutBlock)
{
    HilitingBuffer* self = static_cast<HilitingBuffer*>(voidPtr);

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



byte* HilitingBuffer::getNonBufferedTextStyles(long pos, long numberStyles)
{
    if (numberStyles == 0) {
        return NULL;
    }
    
    const long desiredEndPos = pos + numberStyles;
    
    ASSERT(0 <= pos && pos < desiredEndPos);
    ASSERT(desiredEndPos <= textData->getLength());
    
    SyntaxPattern* sp = NULL;
    pushedSubstr = String();
    long searchStartPos = 0;
    
    if (!syntaxPatterns->hasPatterns()) {
        return NULL;
    }

    if (patternStack.getLength() > 0 && styleBuffer.getLength() > 0) {
        // try to reuse patternStack
        searchStartPos = this->rememberedSearchRestartPos; //this->startPos + styleBuffer.getLength();
        ASSERT(this->startPos <= searchStartPos);
        ASSERT(searchStartPos < this->startPos + styleBuffer.getLength());
        if (pos >= searchStartPos && pos - searchStartPos < maxDistance) {
            sp = syntaxPatterns->get(patternStack.getLast());
            pushedSubstr = patternStack.getAdditionalDataAsString();
        }
    }
    if (sp == NULL)
    {

        patternStack.clear();
    
        // find new BreakPoint that must end really before pos, because new chars
        // at pos could have lead to another break at this position, 
        // therefore take (pos - 1)
    
        hilitedText->moveIteratorToNextBefore(iterator, (pos > 0) ? (pos - 1) : pos);    
        
        searchStartPos = hilitedText->getBreakEndPos(iterator);
        ASSERT(0 <= searchStartPos && searchStartPos <= pos);

        // TODO: vielleicht besser PROCESS_AMOUNT, da das größte vorkommende Pattern
        //       größer als 2 * getHilitingBreakPointDistance() sein könnte.
        if (pos - searchStartPos >= maxDistance) {
            // HilitedText would be too expensive -> return default style or approximate
            if (languageMode->hasApproximateUnknownHilitingFlag()) {
                sp = syntaxPatterns->get(0);
                patternStack.clear();
                patternStack.append(0);
                searchStartPos = pos - languageMode->getApproximateUnknownHilitingReparseRange();
                if (searchStartPos < 0) {
                    searchStartPos = 0;
                }
            } else {
                return NULL;
            }
        }
        else
        {
            hilitedText->copyBreakStackTo(iterator, patternStack);
            sp = syntaxPatterns->get(patternStack.getLast());
            pushedSubstr = patternStack.getAdditionalDataAsString();
        }
    }
    if (searchStartPos > this->startPos && searchStartPos - this->startPos 
            < util::minimum(
                    2 * maxDistance + 2 * languageMode->getApproximateUnknownHilitingReparseRange(),
                    styleBuffer.getLength()))
    {
        styleBuffer.removeTail(searchStartPos - this->startPos);
    }
    else
    {
        styleBuffer.clear();
        this->startPos = searchStartPos;
    }
    
    // searchEndPos

    long searchEndPos = desiredEndPos + maxDistance;
    util::minimize(&searchEndPos, textData->getLength());
    
    bool wasZeroLengthMatch = false;
    
    while (true) 
    {
        BasicRegex::MatchOptions additionalOptions;
        
        long extendedSearchEndPos = searchEndPos + sp->maxREBytesExtend;
        
        util::minimize(&extendedSearchEndPos, textData->getLength());

        if (!textData->isBeginOfLine(searchStartPos)) {
            additionalOptions |= BasicRegex::NOTBOL;
        }
        if (!textData->isEndOfLine(extendedSearchEndPos)) {
            additionalOptions |= BasicRegex::NOTEOL;
        }

        // do searching        
        
        this->rememberedSearchRestartPos = searchStartPos;
        
        bool matched = sp->re.findMatch(this, &HilitingBuffer::pcreCalloutFunction,
                                        (const char*) textData->getAmount(searchStartPos, extendedSearchEndPos - searchStartPos), 
                extendedSearchEndPos - searchStartPos, 0,
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
            
            int cid;

            cid = sp->getMatchedChild(ovector);
            if (cid == -1) {
                
                // EndPattern matched

                long styleBufferPos = styleBuffer.getLength();
                styleBuffer.appendAndFillAmountWith(ovector[1], sp->style);
                fillSubs(sp->combinedSubs, styleBuffer, styleBufferPos, ovector);

                if (desiredEndPos <= searchStartPos + ovector[1]) {
                    return styleBuffer.getPtr(pos - this->startPos);
                }
                patternStack.removeLast();
                sp = syntaxPatterns->get(patternStack.getLast());
                pushedSubstr = patternStack.getAdditionalDataAsString();
                searchStartPos += ovector[1];

            } else {

                // normal Child matched
                
                SyntaxPattern *childPat = syntaxPatterns->getChildPattern(sp, cid);

                if (patternStack.getLength() >= STACK_SIZE 
                        && childPat->hasEndPattern) {

                    // New Begin-Child, but Stack is too big
                    
                    long styleBufferPos = styleBuffer.getLength();
                    styleBuffer.appendAndFillAmountWith(ovector[1], sp->style);

                    if (desiredEndPos <= searchStartPos + ovector[1]) {
                        return styleBuffer.getPtr(pos - this->startPos);
                    }
                    searchStartPos += ovector[1];
                    
                } else {
                    // Stack is ok or doesn't need to grow
                    
                    long styleBufferPos = styleBuffer.getLength();
                    styleBuffer.appendAmount(ovector[1]);
                    styleBuffer.fillAmountWith(styleBufferPos, ovector[0], sp->style);
                    styleBuffer.fillAmountWith(styleBufferPos + ovector[0], ovector[1] - ovector[0], childPat->style);
                    fillSubs(sp->combinedSubs, styleBuffer, styleBufferPos, ovector);
                    
                    if (desiredEndPos <= searchStartPos + ovector[1]) {
                        return styleBuffer.getPtr(pos - this->startPos);
                    }
                    if (childPat->hasEndPattern) {
                        if (!childPat->hasPushedSubstr) {
                            pushedSubstr = String();
                            patternStack.append(syntaxPatterns->getChildPatternId(sp, cid));
                        } else {
                            int pushedSubstrNo = syntaxPatterns->getPushedSubstrNo(sp, cid);
                            
                            pushedSubstr = textData->getSubstring(Pos(searchStartPos + ovector[pushedSubstrNo * 2 + 0]),
                                                                  Pos(searchStartPos + ovector[pushedSubstrNo * 2 + 1]));

                            patternStack.append(syntaxPatterns->getChildPatternId(sp, cid),
                                                pushedSubstr);
                        }
                        sp = childPat;
                    }
                    searchStartPos += ovector[1];
                }

            }

        } else {
            // nothing matched
            styleBuffer.appendAndFillAmountWith(searchEndPos - searchStartPos, sp->style);
            return styleBuffer.getPtr(pos - this->startPos);
        }

    }
}

void HilitingBuffer::registerUpdateListener(Callback<UpdateInfo>::Ptr callback)
{
    updateListeners.registerCallback(callback);
}

void HilitingBuffer::registerTextStylesChangedListeners(Callback<const ObjectArray<TextStyle::Ptr>&>::Ptr callback)
{
    textStylesChangedListeners.registerCallback(callback);
}

