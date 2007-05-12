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

#ifndef HILITING_H
#define HILITING_H

#include "HilitingBase.hpp"
#include "TextData.hpp"
#include "SyntaxPatterns.hpp"
#include "CallbackContainer.hpp"
#include "ProcessHandler.hpp"
#include "MemArray.hpp"
#include "LanguageModes.hpp"
#include "OwningPtr.hpp"

// TODO: Konstanten
//
#define STACK_SIZE 100

namespace LucED {

class HilitedText : public HilitingBase
{
public:
    typedef OwningPtr<HilitedText> Ptr;
    
    struct UpdateInfo {
        long beginPos;
        long endPos;
        UpdateInfo(long beginPos, long endPos)
            : beginPos(beginPos), endPos(endPos) {}
    };

    static HilitedText::Ptr create(TextData::Ptr textData, LanguageMode::Ptr languageMode) {
        return HilitedText::Ptr(new HilitedText(textData, languageMode));
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
    
    HilitedText(TextData::Ptr textData, LanguageMode::Ptr languageMode);

    bool setBreak(IteratorHandle iterator, 
            long startPos1, long startPos, long endPos, BreakType type, 
            const ByteArray& stack);

    void treatTextDataUpdate(TextData::UpdateInfo);

    void gotoReparseStart(long textPos, IteratorHandle iterator);
    bool fillWithBreaks(IteratorHandle iterator, 
            long fillStart, long fillEnd,
            long *lastFillEnd, ByteArray& patternStack);
            
    void flushPendingUpdatesIntern();
    
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
    
    ProcessHandler::Ptr processHandler;
    
    MemArray<int> ovector;
    
    int breakPointDistance;
};

} // namespace LucED

#endif // HILITING_H
