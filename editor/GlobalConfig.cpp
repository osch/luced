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

#include <limits.h>

#include "GlobalConfig.hpp"
#include "ConfigException.hpp"
#include "DirectoryReader.hpp"
#include "RegexException.hpp"
#include "GlobalLuaInterpreter.hpp"
#include "LuaVar.hpp"
#include "System.hpp"
#include "File.hpp"
#include "ActionIdRegistry.hpp"
#include "QualifiedName.hpp"
#include "DefaultConfig.hpp"
                            
using namespace LucED;

SingletonInstance<GlobalConfig> GlobalConfig::instance;;

static const char* const luaFileExtension = ".lua";

GlobalConfig* GlobalConfig::getInstance()
{
    return instance.getPtr();
}

ConfigData::Ptr GlobalConfig::getConfigData()
{
    if (!getInstance()->configData.isValid()) {
        getInstance()->configData = ConfigData::create();
    }
    return getInstance()->configData;
}

GlobalConfig::GlobalConfig()
        : syntaxPatternsConfig(SyntaxPatternsConfig::create()),
          textStyleDefinitions(TextStyleDefinitions::create())
{}



SyntaxPatterns::Ptr GlobalConfig::getSyntaxPatternsForLanguageMode(LanguageMode::Ptr languageMode,
                                                                   Callback<SyntaxPatterns::Ptr>::Ptr changeCallback) const
{
    if (languageMode.isValid() && languageMode->getSyntaxName().getLength() > 0) 
    {
        return this->syntaxPatternsConfig->getSyntaxPatterns(languageMode->getSyntaxName(),
                                                             changeCallback);
    } else {
        return this->syntaxPatternsConfig->getDefaultSyntaxPatterns(changeCallback);
    }
}
    

LanguageMode::Ptr GlobalConfig::getLanguageModeForFileName(const String& fileName) const
{
    return languageModes->getLanguageMode(languageModeSelectors->getLanguageModeNameForFileName(fileName));
}

GlobalConfig::LanguageModeAndEncoding GlobalConfig::getLanguageModeAndEncodingForFileNameAndContent(const String& fileName, 
                                                                                                    RawPtr<const ByteBuffer> fileContent) const
{
    LanguageModeSelectors::Result result = languageModeSelectors->getResultForFileNameAndContent(fileName, fileContent);

    LanguageMode::Ptr languageMode = languageModes->getLanguageMode(result.languageModeName);
    if (!languageMode.isValid()) {
        languageMode = getDefaultLanguageMode();
    }
    if (isConfigFile(fileName)) {
        result.encodingName = "UTF-8";
    }
    return LanguageModeAndEncoding(languageMode,
                                   result.encodingName);
}


LanguageMode::Ptr GlobalConfig::getDefaultLanguageMode() const
{
    return languageModes->getDefaultLanguageMode();
}

static const char* THIS_PREFIX = "this.";

typedef ConfigData::Fonts            ::Element::Font             ConfigDataFont;
typedef ConfigData::TextStyles       ::Element::TextStyle        ConfigDataTextStyle;
typedef ConfigData::ActionKeyBindings::Element::ActionKeyBinding ConfigDataActionKeyBinding;

typedef HeapObjectArray<ConfigDataFont::Ptr>      ConfigDataFontList;
typedef HeapObjectArray<ConfigDataTextStyle::Ptr> ConfigDataTextStyleList;


void GlobalConfig::appendFontTo(ConfigDataFont::Ptr      font,
                                ConfigDataFontList::Ptr  fonts, 
                                const String&            thisPackageName)
{
    fonts->append(font);
}


void GlobalConfig::appendTextStyleTo(ConfigDataTextStyle::Ptr     textStyle,
                                     ConfigDataTextStyleList::Ptr textStyles, 
                                     const String&                thisPackageName)
{
    textStyles->append(textStyle);
}


void GlobalConfig::appendLanguageModeTo(LanguageMode::Ptr   languageMode,
                                        LanguageModes::Ptr  languageModes, 
                                        const String&       thisPackageName)
{
    String syntaxName = languageMode->getSyntaxName();
    
    if (syntaxName.startsWith(THIS_PREFIX) && thisPackageName != "") {
        syntaxName = String() << thisPackageName << "." 
                              << syntaxName.getTail(strlen(THIS_PREFIX));
    
        languageMode->setSyntaxName(syntaxName);
    }
    languageModes->append(languageMode);
}

void GlobalConfig::appendLanguageModeSelectorTo(LanguageModeSelector::Ptr   languageModeSelector,
                                                LanguageModeSelectors::Ptr  languageModeSelectors, 
                                                const String&               thisPackageName)
{
    languageModeSelectors->append(languageModeSelector);
}


void GlobalConfig::appendActionKeyBindingTo(ConfigDataActionKeyBinding::Ptr configData,
                                            ActionKeyConfig::Ptr            actionKeyConfig, 
                                            const String&                   thisPackageName)
{
    String actionName = configData->getActionName();
    
    if (actionName.startsWith(THIS_PREFIX) && thisPackageName != "") {
        actionName = String() << thisPackageName << "." 
                              << actionName.getTail(strlen(THIS_PREFIX));
    }
    QualifiedName qualifiedActionName(actionName);
    if (   qualifiedActionName.getQualifier() == "this"
        || qualifiedActionName.getName() == ""
        || qualifiedActionName.getName() == "this")
    {
        throw ConfigException(String() << "Invalid actionName '" << actionName << "'");
    }
    ActionId actionId = ActionIdRegistry::getInstance()->getActionId(actionName);
    ConfigDataActionKeyBinding::Keys::Ptr keys = configData->getKeys();
    
    for (int i = 0; i < keys->getLength(); ++i)
    {
        actionKeyConfig->append(KeyCombination(keys->get(i)), 
                                actionId);
    }
}



template<class ConfigType,
         class ListType,
         void appendConfigTo(typename ConfigType::Ptr,
                             typename ListType::Ptr, 
                             const String&)
        >
void GlobalConfig::appendConfigFromPackageTo(const String&           packageName, 
                                             const String&           getterFunctionName,
                                             typename ListType::Ptr  list)
{
    packagesMap.set(packageName, true);

    RawPtr<GlobalLuaInterpreter> luaInterpreter = GlobalLuaInterpreter::getInstance();
    LuaAccess luaAccess = luaInterpreter->getCurrentLuaAccess();

    LuaVar package = luaInterpreter->requireConfigPackage(packageName);
    if (package.isTable()) {
        LuaVar getterFunction = package[getterFunctionName];
        if (getterFunction.isValid())
        {
            if (!getterFunction.isFunction()) {
                throw ConfigException(String() << "'"<< packageName 
                                               << "." << getterFunctionName << "' must be function");
            }
            LuaVar configInLua = getterFunction.call();
            if (!configInLua.isTable()) {
                throw ConfigException(String() << "'"<< packageName 
                                               << "." << getterFunctionName << "()' must return table");
            }
            
            LuaVar v(luaAccess);
            for (int i = 0; v = configInLua[i+1], v.isValid(); ++i)
            {
                typename ConfigType::Ptr newConfigData = ConfigType::create();
                newConfigData->readConfig(v);
                appendConfigTo(newConfigData, list, packageName);
            }
        }
    }
}

void GlobalConfig::registerUserDefinedAction(const KeyCombination& keyCombination, const LuaVar& action)
{
    LuaAccess luaAccess = action.getLuaAccess();

    int i;
    for (i = 0; i < userDefinedActions.getLength(); ++i) {
        if (userDefinedActions[i].keyCombination == keyCombination) {
            break;
        }
    }
    if (i == userDefinedActions.getLength()) {
        for (i = 0; i < userDefinedActions.getLength(); ++i) {
            if (userDefinedActions[i].keyCombination == Null) {
                break;
            }
        }
    }
    if (i == userDefinedActions.getLength()) {
        if (action.isFunction()) {
            ActionId id = ActionIdRegistry::getInstance()->getActionId(String() << "userDefinedAction" << i);
            userDefinedActions.append(id);
        } else {
            i = -1;
        }
    }
    if (i >= 0) {
        if (action.isFunction()) {
            userDefinedActions[i].keyCombination = keyCombination;
            userDefinedActions[i].action = luaAccess.store(action);
        } else {
            userDefinedActions[i].keyCombination = Null;
            userDefinedActions[i].action         = Null;
        }
    }
    actionKeyConfig = buildActionKeyConfig();
}

LuaVar GlobalConfig::getUserDefinedAction(const LuaAccess& luaAccess, ActionId actionId)
{
    for (int i = 0; i  < userDefinedActions.getLength(); ++i) {
        if (   userDefinedActions[i].actionId == actionId
            && userDefinedActions[i].action.isValid())
        {
            return luaAccess.retrieve(userDefinedActions[i].action.get());
        }
    }
    return LuaVar(luaAccess);
}

ActionKeyConfig::Ptr GlobalConfig::buildActionKeyConfig()
{
    ActionKeyConfig::Ptr newActionKeyConfig = ActionKeyConfig::create();
    {
        for (int i = 0; i < userDefinedActions.getLength(); ++i) {
            if (userDefinedActions[i].keyCombination.isValid()) {
                newActionKeyConfig->append(userDefinedActions[i].keyCombination.get(), 
                                           userDefinedActions[i].actionId);
            }
        }
        ConfigData::ActionKeyBindings::Ptr actionKeyBindings = configData->getActionKeyBindings();
    
        for (int i = 0; i < actionKeyBindings->getLength(); ++i)
        {
            ConfigData::ActionKeyBindings::Element::Ptr e = actionKeyBindings->get(i);
            if (e->isActionKeyBinding()) 
            {   
                appendActionKeyBindingTo(e->getActionKeyBinding(),
                                         newActionKeyConfig,
                                         "");
            }
            else if (e->isReferer())
            {
                appendConfigFromPackageTo<ConfigDataActionKeyBinding,
                                          ActionKeyConfig,
                                          appendActionKeyBindingTo>(e->getReferer()->getReferToPackage(),
                                                                    "getActionKeyBindings",
                                                                    newActionKeyConfig);
            }
        }
    }
    return newActionKeyConfig;
}

void GlobalConfig::readConfig()
{
    packagesMap.clear();
    
    configDirectory = DefaultConfig::getCreatedConfigDirectory().getAbsoluteNameWithResolvedLinks();

    ConfigException::ErrorList::Ptr errorList = ConfigException::ErrorList::create();

    RawPtr<GlobalLuaInterpreter> luaInterpreter = GlobalLuaInterpreter::getInstance();
    LuaAccess luaAccess = luaInterpreter->getCurrentLuaAccess();

    
    luaInterpreter->setConfigDir(configDirectory);
    luaInterpreter->setMode(ConfigPackageLoader::MODE_NORMAL);

    bool tryItAgain = false;
    int  tryCounter = 0;
    do
    {
        try
        {
            ++tryCounter;
            
            LuaVar configTable = luaInterpreter->getGeneralConfigModule("config");

            if (!configTable.isTable()) {
                throw ConfigException("config file does not return table");
            }
            
            configData = ConfigData::create();
            
            configData->readConfig(configTable);
            
            // fonts
            
            ConfigData::Fonts::Ptr   fontsConfig = configData->getFonts();
            ConfigDataFontList::Ptr  fontList    = ConfigDataFontList::create();
            
            for (int i = 0; i < fontsConfig->getLength(); ++i)
            {
                ConfigData::Fonts::Element::Ptr e = fontsConfig->get(i);
                
                if (e->isFont())
                {
                    fontList->append(e->getFont());
                }
                else
                {
                    ASSERT(e->isReferer());
                    
                    appendConfigFromPackageTo<ConfigDataFont,
                                              ConfigDataFontList,
                                              appendFontTo>(e->getReferer()->getReferToPackage(),
                                                            "getFonts",
                                                            fontList);
                }
            }
            

            // textStyles
            
            typedef HeapObjectArray<ConfigDataTextStyle::Ptr>   TextStyleList;
            
            ConfigData::TextStyles::Ptr  textStylesConfig = configData->getTextStyles();
            ConfigDataTextStyleList::Ptr textStyleList    = ConfigDataTextStyleList::create();

            for (int i = 0; i < textStylesConfig->getLength(); ++i)
            {
                ConfigData::TextStyles::Element::Ptr e = textStylesConfig->get(i);
                
                if (e->isTextStyle())
                {
                    textStyleList->append(e->getTextStyle());
                }
                else
                {
                    ASSERT(e->isReferer());
                    
                    appendConfigFromPackageTo<ConfigDataTextStyle,
                                              ConfigDataTextStyleList,
                                              appendTextStyleTo>(e->getReferer()->getReferToPackage(),
                                                                 "getTextStyles",
                                                                 textStyleList);
                }
            }
            
    
            this->textStyleDefinitions = TextStyleDefinitions::create(fontList,
                                                                      textStyleList);

            // LanguageModeSelectors
    
            LanguageModeSelectors::Ptr newLanguageModeSelectors = LanguageModeSelectors::create();
            {
                ConfigData::LanguageModeSelectors::Ptr languageModeSelectors = configData->getLanguageModeSelectors();
                for (int i = 0; i < languageModeSelectors->getLength(); ++i)
                {
                    ConfigData::LanguageModeSelectors::Element::Ptr e = languageModeSelectors->get(i);
                    if (e->isLanguageModeSelector()) 
                    {   
                        newLanguageModeSelectors->append(e->getLanguageModeSelector());
                    }
                    else
                    {
                        ASSERT(e->isReferer());
                        
                        appendConfigFromPackageTo<LanguageModeSelector,
                                                  LanguageModeSelectors,
                                                  appendLanguageModeSelectorTo>(e->getReferer()->getReferToPackage(),
                                                                                "getLanguageModeSelectors",
                                                                                newLanguageModeSelectors);
                    }
                }
            }

            // LanguageModes
    
            LanguageModes::Ptr newLanguageModes = LanguageModes::create();
            {
                ConfigData::LanguageModes::Ptr languageModes = configData->getLanguageModes();
                for (int i = 0; i < languageModes->getLength(); ++i)
                {
                    ConfigData::LanguageModes::Element::Ptr e = languageModes->get(i);
                    if (e->isLanguageMode()) 
                    {   
                        newLanguageModes->append(e->getLanguageMode());
                    }
                    else
                    {
                        ASSERT(e->isReferer());
                        
                        appendConfigFromPackageTo<LanguageMode,
                                                  LanguageModes,
                                                  appendLanguageModeTo>(e->getReferer()->getReferToPackage(),
                                                                        "getLanguageModes",
                                                                        newLanguageModes);
                    }
                }
            }

            // ActionKeyBinding
    
            ActionKeyConfig::Ptr newActionKeyConfig = buildActionKeyConfig();
            
            languageModeSelectors = newLanguageModeSelectors;
            languageModes = newLanguageModes;
            actionKeyConfig = newActionKeyConfig;
        }
        catch (BaseException& ex)
        {
            if (tryCounter == 2) {
                printf("Internal config error: %s\n", ex.toString().toCString());
            }
            ASSERT(tryCounter == 1); // MODE_FALLBACK should not throw exception

            if (tryCounter == 1)
            {
                errorList->appendCatchedException();
            }
            else {
                if (textStyleDefinitions->isEmpty())
                {
                    textStyleDefinitions->append(TextStyleDefinition("default",
                                                                     "-*-courier-medium-r-*-*-*-120-75-75-*-*-*-*", 
                                                                     "black"));
                }
            }
        }
        if (tryCounter == 1 && !actionKeyConfig.isValid())
        {
            tryItAgain = true;
            luaInterpreter->setMode(ConfigPackageLoader::MODE_FALLBACK);
        }
        else {
            tryItAgain = false;
        }
    }
    while (tryItAgain);

    if (textStyleDefinitions->isEmpty()) {
        textStyleDefinitions->append(TextStyleDefinition("default",
                                                         "-*-courier-medium-r-*-*-*-120-75-75-*-*-*-*", 
                                                         "black"));
        errorList->appendErrorMessage("missing textstyles");
    }
    Nullable<TextStyleDefinition> defaultTextStyleDefinition = textStyleDefinitions->get(0);
    if (!defaultTextStyleDefinition.isValid()) {
        throw ConfigException("missing text style \"default\"");
    }
    defaultTextStyle = TextStyleCache::getInstance()->getTextStyle(defaultTextStyleDefinition);

    // SyntaxPatterns
    
    this->syntaxPatternsConfig->refresh(textStyleDefinitions);
    
    if (errorList->getLength() > 0)
    {
        throw ConfigException(errorList);
    }

    configChangedCallbackContainer.invokeAllCallbacks();
}

bool GlobalConfig::isConfigFile(String fileName) const
{
    File realName = File(fileName).getAbsoluteNameWithResolvedLinks();
    return realName.getDirName().startsWith(configDirectory);
}


void GlobalConfig::notifyAboutNewFileContent(String fileName)
{
    if (isConfigFile(fileName))
    {
        GlobalLuaInterpreter::getInstance()->resetModules();
    
        readConfig();
    }
}

bool GlobalConfig::dependsOnPackage(const String& packageName) const
{
    Nullable<bool> flag = packagesMap.get(packageName);
    
    return flag.isValid() && flag.get();
}
