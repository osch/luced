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

#ifndef GLOBALCONFIG_H
#define GLOBALCONFIG_H

#include "String.hpp"

#include "HeapObject.hpp"
#include "TextStyle.hpp"
#include "HeapHashMap.hpp"
#include "SyntaxPatterns.hpp"
#include "LanguageModes.hpp"
#include "SingletonInstance.hpp"
#include "Callback.hpp"
#include "CallbackContainer.hpp"
#include "MicroSeconds.hpp"

namespace LucED {



class GlobalConfig : public HeapObject
{
public:
    typedef HeapHashMap<String,int> NameToIndexMap;
    
    static GlobalConfig* getInstance();
    
    void readConfig();
    
    bool getUseKeyPressRepeater() {
        return useKeyPressRepeater;
    }
    MicroSeconds getKeyPressRepeatFirstMicroSecs() {
        return keyPressRepeatFirstMicroSecs;
    }
    MicroSeconds getKeyPressRepeatNextMicroSecs() {
        return keyPressRepeatNextMicroSecs;
    }
    int getScrollBarWidth() {
        return scrollBarWidth;
    }
    MicroSeconds getScrollBarRepeatFirstMicroSecs() {
        return scrollBarRepeatFirstMicroSecs;
    }
    MicroSeconds getScrollBarRepeatNextMicroSecs() {
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
    String getPrimarySelectionColor() {
        return primarySelectionColor;
    }
    String getPseudoSelectionColor() {
        return pseudoSelectionColor;
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
    
    long getMaxRegexAssertionLength() const {
        return maxRegexAssertionLength;
    }
    
    SyntaxPatterns::Ptr getSyntaxPatternsForLanguageMode(const String& languageMode,
                                                         Callback1<SyntaxPatterns::Ptr> changeCallback);

    SyntaxPatterns::Ptr getSyntaxPatternsForLanguageMode(LanguageMode::Ptr languageMode, 
                                                         Callback1<SyntaxPatterns::Ptr> changeCallback);

    SyntaxPatterns::Ptr getSyntaxPatternsForFileName    (const String& fileName,
                                                         Callback1<SyntaxPatterns::Ptr> changeCallback);

    LanguageMode::Ptr   getLanguageModeForFileName(const String& fileName);
    LanguageMode::Ptr   getDefaultLanguageMode();

    void notifyAboutNewFileContent(String absoluteFileName);

    void registerConfigChangedCallback(const Callback0& callback) {
        configChangedCallbackContainer.registerCallback(callback);
    }

private:
    friend class SingletonInstance<GlobalConfig>;
    static SingletonInstance<GlobalConfig> instance;
    
    GlobalConfig();

    SyntaxPatterns::Ptr loadSyntaxPatterns(const String& absoluteFileName);

    bool         useKeyPressRepeater;
    MicroSeconds keyPressRepeatFirstMicroSecs;
    MicroSeconds keyPressRepeatNextMicroSecs;
    
    int          scrollBarWidth;
    MicroSeconds scrollBarRepeatFirstMicroSecs;
    MicroSeconds scrollBarRepeatNextMicroSecs;
    long         doubleClickMilliSecs;
    
    String guiColor01;
    String guiColor02;
    String guiColor03;
    String guiColor04;
    String guiColor05;
    String guiFont;
    String guiFontColor;
    String primarySelectionColor;
    String pseudoSelectionColor;
    
    int initialWindowWidth;
    int initialWindowHeight;

    TextStyles::Ptr textStyles;
    NameToIndexMap::Ptr textStyleNameToIndexMap;
    
    ObjectArray<SyntaxPatterns::Ptr> allSyntaxPatterns;
    NameToIndexMap::Ptr languageModeToSyntaxIndex;
    
    NameToIndexMap::Ptr configFileNameToSyntaxIndex;
    
    LanguageModes::Ptr languageModes;
    
    ObjectArray< Callback1Container<SyntaxPatterns::Ptr> > syntaxPatternCallbackContainers;
    Callback0Container configChangedCallbackContainer;
    
    long x11SelectionChunkLength;
    int buttonInnerSpacing;
    int guiSpacing;
    bool editorPanelOnTop;
    bool keepRunningIfOwningClipboard;
    long maxRegexAssertionLength;
    
    String generalConfigFileName;
    String syntaxPatternDirectory;
    NameToIndexMap::Ptr languageModeToIndex;
};

} // namespace LucED

#endif // GLOBALCONFIG_H
