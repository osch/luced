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
    return languageModes->getLanguageModeForFile(fileName);
}


LanguageMode::Ptr GlobalConfig::getDefaultLanguageMode() const
{
    return languageModes->getDefaultLanguageMode();
}

static const char* THIS_PREFIX = "this.";

typedef ConfigData::ActionKeyBindings::Element::ActionKeyBinding ConfigDataActionKeyBinding;

void GlobalConfig::appendActionKeyBindingTo(ActionKeyConfig::Ptr            actionKeyConfig, 
                                            ConfigDataActionKeyBinding::Ptr configData,
                                            String                          thisPackageName)
{
    String actionName = configData->getActionName();
    
    if (actionName.startsWith(THIS_PREFIX) && thisPackageName != "") {
        actionName = String() << thisPackageName << "." 
                              << actionName.getTail(strlen(THIS_PREFIX));
    }
    QualifiedName qualifiedActionName(actionName);
    if (   qualifiedActionName.getQualifier() == ""
        || qualifiedActionName.getQualifier() == "this"
        || qualifiedActionName.getName() == "")
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


void GlobalConfig::readConfig()
{
    packagesMap.clear();
    
    configDirectory = DefaultConfig::getCreatedConfigDirectory();

    ConfigException::ErrorList::Ptr errorList = ConfigException::ErrorList::create();
                                    errorList->setFallbackFileName(String() << configDirectory 
                                                                            << "/config.lua");

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

            // textStyles
    
            this->textStyleDefinitions = TextStyleDefinitions::create(configData->getFonts(),
                                                                      configData->getTextStyles());

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
                    else if (e->isReferer())
                    {
                        String packageName = e->getReferer()->getReferToPackage();

                        packagesMap.set(packageName, true);
                        LuaVar package = GlobalLuaInterpreter::getInstance()->requireConfigPackage(packageName);
                        if (package.isTable()) {
                            LuaVar getLanguageModes = package["getLanguageModes"];
                            if (getLanguageModes.isValid())
                            {
                                if (!getLanguageModes.isFunction()) {
                                    throw ConfigException(String() << "'"<< packageName 
                                                                   << ".getLanguageModes' must be function");
                                }
                                LuaVar languageModes = getLanguageModes.call();
                                if (!languageModes.isTable()) {
                                    throw ConfigException(String() << "'"<< packageName 
                                                                   << ".getLanguageModes()' must return table");
                                }
                                
                                LuaVar v(luaAccess);
                                for (int i = 0; v = languageModes[i+1], v.isValid(); ++i)
                                {
                                    LanguageMode::Ptr newMode = LanguageMode::create();
                                    newMode->readConfig(v);
                                    newLanguageModes->append(newMode);
                                }
                            }
                        }
                    }
                }
            }

            // ActionKeyBinding
    
            ActionKeyConfig::Ptr newActionKeyConfig = ActionKeyConfig::create();
            {
                ConfigData::ActionKeyBindings::Ptr actionKeyBindings = configData->getActionKeyBindings();
                for (int i = 0; i < actionKeyBindings->getLength(); ++i)
                {
                    ConfigData::ActionKeyBindings::Element::Ptr e = actionKeyBindings->get(i);
                    if (e->isActionKeyBinding()) 
                    {   
                        appendActionKeyBindingTo(newActionKeyConfig,
                                                 e->getActionKeyBinding(),
                                                 "");
                    }
                    else if (e->isReferer())
                    {
                        String packageName = e->getReferer()->getReferToPackage();

                        packagesMap.set(packageName, true);
                        LuaVar package = GlobalLuaInterpreter::getInstance()->requireConfigPackage(packageName);
                        if (package.isTable()) {
                            LuaVar getActionKeyBindings = package["getActionKeyBindings"];
                            if (getActionKeyBindings.isValid())
                            {
                                if (!getActionKeyBindings.isFunction()) {
                                    throw ConfigException(String() << "'"<< packageName 
                                                                   << ".getActionKeyBindings' must be function");
                                }
                                LuaVar languageModes = getActionKeyBindings.call();
                                if (!languageModes.isTable()) {
                                    throw ConfigException(String() << "'"<< packageName 
                                                                   << ".getActionKeyBindings()' must return table");
                                }
                                
                                LuaVar v(luaAccess);
                                for (int i = 0; v = languageModes[i+1], v.isValid(); ++i)
                                {
                                    ConfigDataActionKeyBinding::Ptr newBinding = ConfigDataActionKeyBinding::create();
                                    newBinding->readConfig(v);
                                    appendActionKeyBindingTo(newActionKeyConfig,
                                                             newBinding,
                                                             packageName);
                                }
                            }
                        }
                    }
                }
            }
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

void GlobalConfig::notifyAboutNewFileContent(String absoluteFileName)
{
    if (File(absoluteFileName).getDirName().startsWith(configDirectory))
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
