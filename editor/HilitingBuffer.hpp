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

#ifndef HILITINGBUFFER_H
#define HILITINGBUFFER_H

#include "HeapObject.hpp"
#include "HilitedText.hpp"
#include "CallbackContainer.hpp"
#include "MemArray.hpp"
#include "OwningPtr.hpp"

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

    static HilitingBuffer::Ptr create(HilitedText::Ptr hilitedText) {
        return HilitingBuffer::Ptr(new HilitingBuffer(hilitedText));
    }
    
    int getTextStyle(long textPos) {
        if (textPos >= startPos && textPos - startPos < styleBuffer.getLength()) {
            return styleBuffer[textPos - startPos];
        } else {
            long textLength = textData->getLength();
            if (textLength > 0 && textPos >= textLength) {
                return getTextStyle(textData->getPrevWCharPos(textLength));
            }
            long numberStyles = 150;
            if (textPos + numberStyles > textLength) {
                numberStyles = textLength - textPos;
            }
            byte* rslt = getNonBufferedTextStyles(textPos, numberStyles);
            return rslt == NULL ? 0 : *rslt;
        }
    }
    
    byte* getTextStyles(long textPos, long numberStyles) {
        ASSERT(textPos + numberStyles <= textData->getLength());
        if (textPos >= startPos && numberStyles <= styleBuffer.getLength()) {
            return styleBuffer.getPtr(textPos - startPos);
        } else {
            return getNonBufferedTextStyles(textPos, numberStyles);
        }
    }
    
    void registerTextStylesChangedListeners(Callback<const ObjectArray<TextStyle::Ptr>&>::Ptr callback);
    void registerUpdateListener(Callback<UpdateInfo>::Ptr updateCallback);
    void registerLanguageModeChangedCallback(Callback<LanguageMode::Ptr>::Ptr callback) {
        languageModeChangedCallbacks.registerCallback(callback);
    }
    
    LanguageMode::Ptr getLanguageMode() const {
        return languageMode;
    }
    
    HilitedText::Ptr getHilitedText() const {
        return hilitedText;
    }

private:
    
    HilitingBuffer(HilitedText::Ptr hilitedText);
    
    static int pcreCalloutFunction(void*, pcre_callout_block*);

    byte* getNonBufferedTextStyles(long textPos, long numberStyles);


    void treatHilitingUpdate(HilitedText::UpdateInfo update);

    void treatTextDataUpdate(TextData::UpdateInfo update);

    void treatTextStylesChanged();
    void treatChangedHiliting(HilitedText* changedHiliting);
    void treatLanguageModeChange(LanguageMode::Ptr newLanguageMode);

    HilitedText::Ptr hilitedText;
    HilitedText::Iterator iterator;
    TextData::Ptr textData;
    SyntaxPatterns::Ptr syntaxPatterns;
    LanguageMode::Ptr languageMode;

    long startPos;
    long rememberedSearchRestartPos;
    PatternStack patternStack;
    ByteArray styleBuffer;
    CallbackContainer<UpdateInfo> updateListeners;
    CallbackContainer<const ObjectArray<TextStyle::Ptr>&> textStylesChangedListeners;
    MemArray<int> ovector;

    int maxDistance;

    String pushedSubstr;

    CallbackContainer<LanguageMode::Ptr> languageModeChangedCallbacks;
};

} // namespace LucED

#endif // HILITINGBUFFER_H
