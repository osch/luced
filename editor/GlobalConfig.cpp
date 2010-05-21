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
#include "CurrentDirectoryKeeper.hpp"
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

void GlobalConfig::readActionKeyBinding(ActionKeyConfig::Ptr actionKeyConfig, 
                                        LuaVarRef actionKeyBinding,
                                        String    thisPackageName)
{
    LuaAccess luaAccess = actionKeyBinding.getLuaAccess();
    LuaVar o(luaAccess);
    
    for (int i = 0; o = actionKeyBinding[i + 1], o.isValid(); ++i)
    {
        if (!o.isTable()) {
            throw ConfigException("actionKeyBinding entry must be table");
        }
        LuaVar n = o["actionName"];
        if (n.isString())
        {
            String actionName = n.toString();
            if (actionName.startsWith(THIS_PREFIX) && thisPackageName != "") {
                actionName = String() << thisPackageName << "." 
                                      << actionName.getTail(strlen(THIS_PREFIX));
            }
            QualifiedName qualifiedActionName(actionName);
            if (   qualifiedActionName.getQualifier() == ""
                || qualifiedActionName.getQualifier() == "this"
                || qualifiedActionName.getName() == "")
            {
                throw ConfigException(String() << "Invalid actionName '" << n.toString() << "'");
            }
            ActionId actionId = ActionIdRegistry::getInstance()->getActionId(actionName);
            LuaVar keys = o["keys"];
            if (!keys.isValid() || (!keys.isString() && !keys.isTable())) {
                throw ConfigException(String() << "Invalid element 'keys' for action = '" << actionName << "'");
            }
            if (keys.isString())
            {
                actionKeyConfig->append(KeyCombination(keys.toString()), 
                                        actionId);
            }
            else
            {
                LuaVar k(luaAccess);
                
                for (int i = 0; k = keys[i + 1], k.isValid(); ++i)
                {
                    if (!k.isString()) {
                        throw ConfigException(String() << "Invalid element 'keys' for action = '" << actionName << "'");
                    }
                    actionKeyConfig->append(KeyCombination(k.toString()), 
                                            actionId);
                }
            }
            
        }
        else
        {
            n = o["referToPackage"];
            if (!n.isString()) {
                throw ConfigException("actionKeyBinding entry must have 'actionName' or 'referToPackage' field");
            }
            String packageName = n.toString();
            packagesMap.set(packageName, true);
            LuaVar package = GlobalLuaInterpreter::getInstance()->requireConfigPackage(packageName);
            if (package.isTable()) {
                LuaVar getActionKeyBinding = package["getActionKeyBinding"];
                if (!getActionKeyBinding.isFunction()) {
                    throw ConfigException(String() << "'"<< packageName 
                                                   << ".getActionKeyBinding' must be function");
                }
                LuaVar packageBinding = getActionKeyBinding.call();
                if (!packageBinding.isTable()) {
                    throw ConfigException(String() << "'"<< packageName 
                                                   << ".getActionKeyBinding()' must return table");
                }
                readActionKeyBinding(actionKeyConfig, packageBinding, packageName);
            }
        }
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

    LanguageModes::Ptr newLanguageModes = LanguageModes::create();
    
    luaInterpreter->setConfigDir(configDirectory);
    luaInterpreter->setMode(ConfigPackageLoader::MODE_NORMAL);

    bool tryItAgain = false;
    int  tryCounter = 0;
    do
    {
        try
        {
            ++tryCounter;
            
            CurrentDirectoryKeeper currentDirectoryKeeper(configDirectory);
            
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
    
            LuaVar o(luaAccess);
            LuaVar lm = configTable["languageModes"];
            if (lm.isValid())
            {
                if (!lm.isTable()) {
                    throw ConfigException("invalid languageModes");
                }
                for (int i = 0; o = lm[i+1], o.isValid(); ++i)
                {
                    LuaVar n = o["name"];
                    if (!n.isString()) {
                        throw ConfigException("invalid or missing element 'name' in languageMode");
                    }
                    newLanguageModes->append(o);
                }
            }
            ActionKeyConfig::Ptr newActionKeyConfig = ActionKeyConfig::create();
            LuaVar actionKeyBinding = configTable["actionKeyBinding"];
            if (actionKeyBinding.isValid())
            {
                if (!actionKeyBinding.isTable()) {
                    throw ConfigException("invalid actionKeyBinding");
                }
                readActionKeyBinding(newActionKeyConfig, actionKeyBinding, "");
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
