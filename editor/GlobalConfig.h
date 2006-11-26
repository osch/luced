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

#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include <string>

#include "HeapObject.h"
#include "LuaObject.h"
#include "TextStyle.h"
#include "HeapHashMap.h"
#include "SyntaxPatterns.h"
#include "LanguageModes.h"
#include "SingletonInstance.h"

namespace LucED {

using std::string;

class GlobalConfig : public HeapObject
{
public:
    typedef HeapHashMap<string,int> NameToIndexMap;
    
    static GlobalConfig* getInstance();
    
    void readConfig(const string& configPath);
    
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
    string getGuiColor01() {
        return guiColor01;
    }
    string getGuiColor02() {
        return guiColor02;
    }
    string getGuiColor03() {
        return guiColor03;
    }
    string getGuiColor04() {
        return guiColor04;
    }
    string getGuiColor05() {
        return guiColor05;
    }
    string getGuiFont() {
        return guiFont;
    }
    string getGuiFontColor() {
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
    
    bool hasSyntaxPatternsFor(const string& languageMode) {
        NameToIndexMap::Value foundIndex = languageModeToSyntaxIndex->get(languageMode);
        return foundIndex.isValid();
    }
    
    SyntaxPatterns::Ptr getSyntaxPatternsForLanguageMode(const string& languageMode);
    SyntaxPatterns::Ptr getSyntaxPatternsForLanguageMode(LanguageMode::Ptr languageMode);
    SyntaxPatterns::Ptr getSyntaxPatternsForFileName(const string& fileName);
    LanguageMode::Ptr   getLanguageModeForFileName(const string& fileName);
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
    
    string guiColor01;
    string guiColor02;
    string guiColor03;
    string guiColor04;
    string guiColor05;
    string guiFont;
    string guiFontColor;
    
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
