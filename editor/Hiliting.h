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

#ifndef HILITING_H
#define HILITING_H

#include "HilitingBase.h"
#include "TextData.h"
#include "Slot.h"
#include "SyntaxPatterns.h"
#include "CallbackContainer.h"
#include "ProcessHandler.h"
#include "MemArray.h"
#include "LanguageModes.h"

// TODO: Konstanten
//
#define STACK_SIZE 100

namespace LucED {

class Hiliting : public HilitingBase
{
public:
    typedef HeapObjectPtr<Hiliting> Ptr;
    
    struct UpdateInfo {
        long beginPos;
        long endPos;
        UpdateInfo(long beginPos, long endPos)
            : beginPos(beginPos), endPos(endPos) {}
    };

    static Hiliting::Ptr create(TextData::Ptr textData, LanguageMode::Ptr languageMode) {
        return Hiliting::Ptr(new Hiliting(textData, languageMode));
    }

    int process(int requestedProcessingAmount);
    bool needsProcessing();
    
    void registerUpdateListener(const Callback1<UpdateInfo>& updateCallback);

    void flushPendingUpdates() {
        if (this->endChangedPos != 0) {
            flushPendingUpdatesIntern();
        }
    }
    
    SyntaxPatterns::Ptr getSyntaxPatterns() {
        return syntaxPatterns;
    }

    LanguageMode::Ptr getLanguageMode() {
        return languageMode;
    }

    TextData::Ptr getTextData() {
        return textData;
    }

private:
    
    Hiliting(TextData::Ptr textData, LanguageMode::Ptr languageMode);

    bool setBreak(IteratorHandle iterator, 
            long startPos1, long startPos, long endPos, BreakType type, 
            const ByteArray& stack);

    void treatTextDataUpdate(TextData::UpdateInfo);
    Slot1<TextData::UpdateInfo> slotForTextDataUpdateTreatment;

    void gotoReparseStart(long textPos, IteratorHandle iterator);
    bool fillWithBreaks(IteratorHandle iterator, 
            long fillStart, long fillEnd,
            long *lastFillEnd, ByteArray& patternStack);
            
    void flushPendingUpdatesIntern();
    Slot0 slotForFlushPendingUpdates;
    
    Iterator rememberedLastProcessingRestartedIterator;
    Iterator processingEndBeforeRestartIterator;
    bool     processingEndBeforeRestartFlag;
    bool     needsProcessingFlag;
    
    Iterator startNextProcessIterator;
    Iterator tryToBeLastBreakIterator;

    TextData::Ptr textData;
    LanguageMode::Ptr languageMode;
    SyntaxPatterns::Ptr syntaxPatterns;
    
    long beginChangedPos;
    long   endChangedPos;
    
    ByteArray patternStack;
    Callback1Container<UpdateInfo> updateListeners;
    
    ProcessHandlerSlot processHandlerSlot;
    
    MemArray<int> ovector;
    
    const int breakPointDistance;
};

} // namespace LucED

#endif // HILITING_H
