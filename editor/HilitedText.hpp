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

#ifndef HILITED_TEXT_HPP
#define HILITED_TEXT_HPP

#include "HilitingBase.hpp"
#include "TextData.hpp"
#include "SyntaxPatterns.hpp"
#include "CallbackContainer.hpp"
#include "ProcessHandler.hpp"
#include "MemArray.hpp"
#include "LanguageModes.hpp"
#include "OwningPtr.hpp"
#include "RawPtr.hpp"

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

    int process(TimeVal endTime);
    bool needsProcessing();
    
    void registerUpdateListener(Callback<UpdateInfo>::Ptr updateCallback);

    SyntaxPatterns::Ptr getSyntaxPatterns() {
        return syntaxPatterns;
    }

    void registerHilitingChangedCallback(Callback<HilitedText*>::Ptr hilitingChangeCallback) {
        hilitingChangedCallbacks.registerCallback(hilitingChangeCallback);
    }

    LanguageMode::Ptr getLanguageMode() {
        return languageMode;
    }

    void setLanguageMode(LanguageMode::Ptr languageMode);
    
    TextData::Ptr getTextData() {
        return textData;
    }

private:
    
    HilitedText(TextData::Ptr textData, LanguageMode::Ptr languageMode);

    static int pcreCalloutFunction(void*, pcre_callout_block*);

    bool setBreak(IteratorHandle iterator, 
            long startPos1, long startPos, long endPos, BreakType type, 
            const PatternStack& stack);

    void treatTextDataUpdate(TextData::UpdateInfo);
    
    void treatSyntaxPatternsUpdate(SyntaxPatterns::Ptr newSyntaxPatterns);

    int getReparseDistance(IteratorHandle iterator);

    void gotoReparseStart(long textPos, IteratorHandle iterator);
    bool fillWithBreaks(IteratorHandle iterator, 
            long fillStart, long fillEnd,
            long *lastFillEnd, PatternStack& patternStack);
            
    void flushPendingUpdates();
    
    Iterator rememberedLastProcessingRestartedIterator;
    Iterator processingEndBeforeRestartIterator;
    Iterator startNextProcessIterator;
    Iterator tryToBeLastBreakIterator;
    bool     processingEndBeforeRestartFlag;
    bool     needsProcessingFlag;
    

    TextData::Ptr textData;
    LanguageMode::Ptr languageMode;
    SyntaxPatterns::Ptr syntaxPatterns;
    
    long beginChangedPos;
    long   endChangedPos;
    
    PatternStack patternStack;
    CallbackContainer<UpdateInfo> updateListeners;
    
    ProcessHandler::Ptr processHandler;
    
    MemArray<int> ovector;
    
    int breakPointDistance;

    CallbackContainer<HilitedText*> hilitingChangedCallbacks;    

    Callback<SyntaxPatterns::Ptr>::Ptr syntaxPatternUpdateCallback;

    String pushedSubstr;
};

} // namespace LucED

#endif // HILITED_TEXT_HPP
