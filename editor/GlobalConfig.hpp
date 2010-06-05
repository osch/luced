/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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
#include "LuaVar.hpp"
#include "TextStyleDefinition.hpp"
#include "TextStyleDefinitions.hpp"
#include "ConfigData.hpp"

namespace LucED
{

class GlobalConfig : public HeapObject
{
public:
    typedef HeapHashMap<String,int> NameToIndexMap;
    
    static GlobalConfig* getInstance();
    
    void readConfig();
    
    static ConfigData::Ptr getConfigData();
    
    ActionKeyConfig::Ptr getActionKeyConfig() const {
        return actionKeyConfig;
    }
    
    SyntaxPatterns::Ptr getSyntaxPatternsForLanguageMode(LanguageMode::Ptr languageMode, 
                                                         Callback<SyntaxPatterns::Ptr>::Ptr changeCallback) const;

    LanguageMode::Ptr   getLanguageModeForFileName(const String& fileName) const;
    LanguageMode::Ptr   getDefaultLanguageMode() const;

    void notifyAboutNewFileContent(String absoluteFileName);

    void registerConfigChangedCallback(Callback<>::Ptr callback) {
        configChangedCallbackContainer.registerCallback(callback);
    }
    
    String getConfigDirectory() const {
        return configDirectory;
    }
    
    TextStyle::Ptr getDefaultTextStyle() const {
        return defaultTextStyle;
    }
    
    bool dependsOnPackage(const String& packageName) const;

private:
    friend class SingletonInstance<GlobalConfig>;
    static SingletonInstance<GlobalConfig> instance;
    
    GlobalConfig();

    typedef ConfigData::Fonts            ::Element::Font             ConfigDataFont;
    typedef ConfigData::TextStyles       ::Element::TextStyle        ConfigDataTextStyle;
    typedef ConfigData::ActionKeyBindings::Element::ActionKeyBinding ConfigDataActionKeyBinding;

    typedef HeapObjectArray<ConfigDataFont::Ptr>      ConfigDataFontList;
    typedef HeapObjectArray<ConfigDataTextStyle::Ptr> ConfigDataTextStyleList;

    static void appendFontTo(ConfigDataFont::Ptr      font,
                             ConfigDataFontList::Ptr  fonts, 
                             const String&            thisPackageName);
                              
    static void appendTextStyleTo(ConfigDataTextStyle::Ptr     textStyle,
                                  ConfigDataTextStyleList::Ptr textStyles, 
                                  const String&                thisPackageName);
                              
    static void appendLanguageModeTo(LanguageMode::Ptr   languageMode,
                                     LanguageModes::Ptr  languageModes, 
                                     const String&       thisPackageName);
                              
    static void appendActionKeyBindingTo(ConfigDataActionKeyBinding::Ptr configData,
                                         ActionKeyConfig::Ptr            actionKeyConfig, 
                                         const String&                   thisPackageName);

    template<class ConfigType,
             class ListType,
             void (*append)(typename ConfigType::Ptr,
                            typename ListType::Ptr, 
                            const String&)
            >
    void appendConfigFromPackageTo(const String&           packageName, 
                                   const String&           getterFunctionName,
                                   typename ListType::Ptr  list);

    LanguageModes::Ptr languageModes;
    
    SyntaxPatternsConfig::Ptr syntaxPatternsConfig;
    
    CallbackContainer<> configChangedCallbackContainer;
    
    ActionKeyConfig::Ptr        actionKeyConfig;
    
    String configDirectory;
    
    HashMap<String,bool> packagesMap;
    
    TextStyleDefinitions::Ptr textStyleDefinitions;
    TextStyle::Ptr defaultTextStyle;
    
    ConfigData::Ptr configData;
};

} // namespace LucED

#endif // GLOBAL_CONFIG_HPP
