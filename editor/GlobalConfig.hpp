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

#ifndef GLOBAL_CONFIG_HPP
#define GLOBAL_CONFIG_HPP

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
#include "SyntaxPatternsConfig.hpp"
#include "ActionKeyConfig.hpp"

namespace LucED
{

class GlobalConfig : public HeapObject
{
public:
    typedef HeapHashMap<String,int> NameToIndexMap;
    
    static GlobalConfig* getInstance();
    
    void readConfig();
    
    ActionKeyConfig::Ptr getActionKeyConfig() const {
        return actionKeyConfig;
    }
    
    bool getUseOwnKeyPressRepeater() const {
        return useOwnKeyPressRepeater;
    }
    bool getDoNotUseX11XkbExtension() const {
        return doNotUseX11XkbExtension;
    }
    MicroSeconds getKeyPressRepeatFirstMicroSecs() const {
        return keyPressRepeatFirstMicroSecs;
    }
    MicroSeconds getKeyPressRepeatNextMicroSecs() const {
        return keyPressRepeatNextMicroSecs;
    }
    int getScrollBarWidth() const {
        return scrollBarWidth;
    }
    MicroSeconds getScrollBarRepeatFirstMicroSecs() const {
        return scrollBarRepeatFirstMicroSecs;
    }
    MicroSeconds getScrollBarRepeatNextMicroSecs() const {
        return scrollBarRepeatNextMicroSecs;
    }
    long getDoubleClickMilliSecs() const {
        return doubleClickMilliSecs;
    }
    String getGuiColor01() const {
        return guiColor01;
    }
    String getGuiColor02() const {
        return guiColor02;
    }
    String getGuiColor03() const {
        return guiColor03;
    }
    String getGuiColor04() const {
        return guiColor04;
    }
    String getGuiColor05() const {
        return guiColor05;
    }
    String getGuiFont() const {
        return guiFont;
    }
    String getGuiFontColor() const {
        return guiFontColor;
    }
    String getPrimarySelectionColor() const {
        return primarySelectionColor;
    }
    String getPseudoSelectionColor() const {
        return pseudoSelectionColor;
    }
    int getInitialWindowWidth() const {
        return initialWindowWidth;
    }
    int getInitialWindowHeight() const {
        return initialWindowHeight;
    }
    long getX11SelectionChunkLength() const {
        return x11SelectionChunkLength;
    }
    int getButtonInnerSpacing() const {
        return buttonInnerSpacing;
    }
    int getGuiSpacing() const {
        return guiSpacing;
    }
    bool isEditorPanelOnTop() const {
        return editorPanelOnTop;
    }
    bool shouldKeepRunningIfOwningClipboard() const {
        return keepRunningIfOwningClipboard;
    }

    TextStyles::Ptr getTextStyles() const {
        return textStyles;
    }
    
    NameToIndexMap::ConstPtr getTextStyleNameToIndexMap() const {
        return textStyleNameToIndexMap;
    }
    
    bool hasSyntaxPatternsForLanguageMode(const String& languageMode) const {
        return syntaxPatternsConfig->hasEntryForLanguageModeName(languageMode);
    }
    
    long getMaxRegexAssertionLength() const {
        return maxRegexAssertionLength;
    }
    
    SyntaxPatterns::Ptr getSyntaxPatternsForLanguageMode(const String& languageMode,
                                                         Callback<SyntaxPatterns::Ptr>::Ptr changeCallback) const;

    SyntaxPatterns::Ptr getSyntaxPatternsForLanguageMode(LanguageMode::Ptr languageMode, 
                                                         Callback<SyntaxPatterns::Ptr>::Ptr changeCallback) const;

    SyntaxPatterns::Ptr getSyntaxPatternsForFileName    (const String& fileName,
                                                         Callback<SyntaxPatterns::Ptr>::Ptr changeCallback) const;

    LanguageMode::Ptr   getLanguageModeForFileName(const String& fileName) const;
    LanguageMode::Ptr   getDefaultLanguageMode() const;

    void notifyAboutNewFileContent(String absoluteFileName);

    void registerConfigChangedCallback(Callback<>::Ptr callback) {
        configChangedCallbackContainer.registerCallback(callback);
    }

private:
    friend class SingletonInstance<GlobalConfig>;
    static SingletonInstance<GlobalConfig> instance;
    
    GlobalConfig();

    SyntaxPatterns::Ptr loadSyntaxPatterns(const String& absoluteFileName);

    bool         useOwnKeyPressRepeater;
    bool         doNotUseX11XkbExtension;
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
    
    
    LanguageModes::Ptr languageModes;
    
    SyntaxPatternsConfig::Ptr syntaxPatternsConfig;
    
    CallbackContainer<> configChangedCallbackContainer;
    
    long x11SelectionChunkLength;
    int buttonInnerSpacing;
    int guiSpacing;
    bool editorPanelOnTop;
    bool keepRunningIfOwningClipboard;
    long maxRegexAssertionLength;
    
    String generalConfigFileName;
    String syntaxPatternDirectory;
    
    ActionKeyConfig::Ptr        actionKeyConfig;
};

} // namespace LucED

#endif // GLOBAL_CONFIG_HPP
