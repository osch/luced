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

#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "String.hpp"

#include "HeapObject.hpp"
#include "TextStyle.hpp"
#include "HeapHashMap.hpp"
#include "SyntaxPatterns.hpp"
#include "LanguageModes.hpp"
#include "SingletonInstance.hpp"

namespace LucED {



class GlobalConfig : public HeapObject
{
public:
    typedef HeapHashMap<String,int> NameToIndexMap;
    
    static GlobalConfig* getInstance();
    
    void readConfig(const String& configPath);
    
    bool getUseKeyPressRepeater() {
        return useKeyPressRepeater;
    }
    long getKeyPressRepeatFirstMicroSecs() {
        return keyPressRepeatFirstMicroSecs;
    }
    long getKeyPressRepeatNextMicroSecs() {
        return keyPressRepeatNextMicroSecs;
    }
    int getScrollBarWidth() {
        return scrollBarWidth;
    }
    long getScrollBarRepeatFirstMicroSecs() {
        return scrollBarRepeatFirstMicroSecs;
    }
    long getScrollBarRepeatNextMicroSecs() {
        return scrollBarRepeatNextMicroSecs;
    }
    long getDoubleClickMilliSecs() {
        return doubleClickMilliSecs;
    }
    String getGuiColor01() {
        return guiColor01;
    }
    String getGuiColor02() {
        return guiColor02;
    }
    String getGuiColor03() {
        return guiColor03;
    }
    String getGuiColor04() {
        return guiColor04;
    }
    String getGuiColor05() {
        return guiColor05;
    }
    String getGuiFont() {
        return guiFont;
    }
    String getGuiFontColor() {
        return guiFontColor;
    }
    int getInitialWindowWidth() {
        return initialWindowWidth;
    }
    int getInitialWindowHeight() {
        return initialWindowHeight;
    }
    long getX11SelectionChunkLength() {
        return x11SelectionChunkLength;
    }
    int getButtonInnerSpacing() {
        return buttonInnerSpacing;
    }
    int getGuiSpacing() {
        return guiSpacing;
    }
    bool isEditorPanelOnTop() const {
        return editorPanelOnTop;
    }
    bool shouldKeepRunningIfOwningClipboard() const {
        return keepRunningIfOwningClipboard;
    }

    TextStyles::Ptr getTextStyles() {
        return textStyles;
    }
    
    NameToIndexMap::ConstPtr getTextStyleNameToIndexMap() {
        return textStyleNameToIndexMap;
    }
    
    bool hasSyntaxPatternsFor(const String& languageMode) {
        NameToIndexMap::Value foundIndex = languageModeToSyntaxIndex->get(languageMode);
        return foundIndex.isValid();
    }
    
    SyntaxPatterns::Ptr getSyntaxPatternsForLanguageMode(const String& languageMode);
    SyntaxPatterns::Ptr getSyntaxPatternsForLanguageMode(LanguageMode::Ptr languageMode);
    SyntaxPatterns::Ptr getSyntaxPatternsForFileName(const String& fileName);
    LanguageMode::Ptr   getLanguageModeForFileName(const String& fileName);
    LanguageMode::Ptr   getDefaultLanguageMode();

private:
    friend class SingletonInstance<GlobalConfig>;
    static SingletonInstance<GlobalConfig> instance;
    
    GlobalConfig();
    
    bool useKeyPressRepeater;
    long keyPressRepeatFirstMicroSecs;
    long keyPressRepeatNextMicroSecs;
    
    int  scrollBarWidth;
    long scrollBarRepeatFirstMicroSecs;
    long scrollBarRepeatNextMicroSecs;
    long doubleClickMilliSecs;
    
    String guiColor01;
    String guiColor02;
    String guiColor03;
    String guiColor04;
    String guiColor05;
    String guiFont;
    String guiFontColor;
    
    int initialWindowWidth;
    int initialWindowHeight;

    TextStyles::Ptr textStyles;
    NameToIndexMap::Ptr textStyleNameToIndexMap;
    
    ObjectArray<SyntaxPatterns::Ptr> allSyntaxPatterns;
    NameToIndexMap::Ptr languageModeToSyntaxIndex;
    
    LanguageModes::Ptr languageModes;
    
    long x11SelectionChunkLength;
    int buttonInnerSpacing;
    int guiSpacing;
    bool editorPanelOnTop;
    bool keepRunningIfOwningClipboard;
};

} // namespace LucED

#endif // GLOBALCONFIG_H
