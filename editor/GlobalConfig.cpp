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
                            
using namespace LucED;

SingletonInstance<GlobalConfig> GlobalConfig::instance;;

static const char* const luaFileExtension = ".lua";

GlobalConfig* GlobalConfig::getInstance()
{
    return instance.getPtr();
}

GlobalConfig::GlobalConfig()
        : useOwnKeyPressRepeater(false),
          doNotUseX11XkbExtension(false),
          keyPressRepeatFirstMicroSecs(300 * 1000),
          keyPressRepeatNextMicroSecs(  15 * 1000),
          scrollBarWidth(12+2+1),
          scrollBarRepeatFirstMicroSecs(300 * 1000),
          scrollBarRepeatNextMicroSecs(  20 * 1000),
          doubleClickMilliSecs(400),
          guiColor01("grey30"),
          guiColor02("grey70"),
          guiColor03("grey80"),
          guiColor04("grey85"),
          guiColor05("grey90"),
          guiFont("-*-helvetica-medium-r-*-*-*-120-75-75-*-*-*-*"),
          guiFontColor("black"),
          primarySelectionColor("grey"),
          pseudoSelectionColor("rgb:f0/f0/ff"), // was lavender
          initialWindowWidth(80),
          initialWindowHeight(25),
          x11SelectionChunkLength(20000),
          buttonInnerSpacing(2),
          guiSpacing(2),
          editorPanelOnTop(false),
          keepRunningIfOwningClipboard(false),
          maxRegexAssertionLength(3000),
          syntaxPatternsConfig(SyntaxPatternsConfig::create()),
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
    
    ConfigException::ErrorList::Ptr errorList = ConfigException::ErrorList::create();
                                    errorList->setFallbackFileName(String() << configDirectory 
                                                                            << "/config.lua");
    
    configDirectory = File(".luced").getAbsoluteName();
    
    if (!File(configDirectory).exists()) {
        String homeDirectory = System::getInstance()->getHomeDirectory();
        configDirectory = File(homeDirectory, ".luced").getAbsoluteName();
    }
    RawPtr<GlobalLuaInterpreter> luaInterpreter = GlobalLuaInterpreter::getInstance();
    LuaAccess luaAccess = luaInterpreter->getCurrentLuaAccess();

    LanguageModes::Ptr newLanguageModes = LanguageModes::create();
    
    luaInterpreter->setConfigDir(configDirectory);
    luaInterpreter->setMode(ConfigPackageLoader::MODE_NORMAL);

    bool tryItAgain = false;
    do
    {
        try
        {
            CurrentDirectoryKeeper currentDirectoryKeeper(configDirectory);
            
            LuaVar configTable = luaInterpreter->getGeneralConfigModule("config");
            
            if (!configTable.isTable()) {
                throw ConfigException("config file does not return table");
            }
            
            // generalConfig
    
            LuaVar generalConfig = configTable["generalConfig"];
    
            if (generalConfig.isValid())
            {
                if (!generalConfig.isTable()) {
                    throw ConfigException("invalid generalConfig");
                }
    
                LuaVar o = generalConfig["useOwnKeyPressRepeater"];
                if (o.isValid()) {
                    if (!o.isBoolean()) {
                        throw ConfigException("invalid useOwnKeyPressRepeater");
                    }
                    this->useOwnKeyPressRepeater = o.toBoolean();
                }
    
                o = generalConfig["doNotUseX11XkbExtension"];
                if (o.isValid()) {
                    if (!o.isBoolean()) {
                        throw ConfigException("invalid doNotUseX11XkbExtension");
                    }
                    this->doNotUseX11XkbExtension = o.toBoolean();
                }
    
                o = generalConfig["keyPressRepeatFirstMilliSecs"];
                if (o.isValid()) {
                    if (!o.isNumber()) {
                        throw ConfigException("invalid keyPressRepeatFirstMilliSecs");
                    }
                    this->keyPressRepeatFirstMicroSecs = MicroSeconds((long) (o.toNumber() * 1000));
                }
    
                o = generalConfig["keyPressRepeatNextMilliSecs"];
                if (o.isValid()) {
                    if (!o.isNumber()) {
                        throw ConfigException("invalid keyPressRepeatNextMilliSecs");
                    }
                    this->keyPressRepeatNextMicroSecs = MicroSeconds((long) (o.toNumber() * 1000));
                }
    
                o = generalConfig["scrollBarWidth"];
                if (o.isValid()) {
                    if (!o.isNumber()) {
                        throw ConfigException("invalid scrollBarWidth");
                    }
                    this->scrollBarWidth = (int) o.toNumber();
                }
    
                o = generalConfig["scrollBarRepeatFirstMilliSecs"];
                if (o.isValid()) {
                    if (!o.isNumber()) {
                        throw ConfigException("invalid scrollBarRepeatFirstMilliSecs");
                    }
                    this->scrollBarRepeatFirstMicroSecs = MicroSeconds((long) (o.toNumber() * 1000));
                }
    
                o = generalConfig["scrollBarRepeatNextMilliSecs"];
                if (o.isValid()) {
                    if (!o.isNumber()) {
                        throw ConfigException("invalid scrollBarRepeatNextMilliSecs");
                    }
                    this->scrollBarRepeatNextMicroSecs = MicroSeconds((long) (o.toNumber() * 1000));
                }
    
                o = generalConfig["doubleClickMilliSecs"];
                if (o.isValid()) {
                    if (!o.isNumber()) {
                        throw ConfigException("invalid doubleClickMilliSecs");
                    }
                    this->doubleClickMilliSecs = (long) o.toNumber();
                }
    
                o = generalConfig["guiColor01"];
                if (o.isValid()) {
                    if (!o.isString()) {
                        throw ConfigException("invalid guiColor01");
                    }
                    this->guiColor01 = o.toString();
                }
                o = generalConfig["guiColor02"];
                if (o.isValid()) {
                    if (!o.isString()) {
                        throw ConfigException("invalid guiColor02");
                    }
                    this->guiColor02 = o.toString();
                }
                o = generalConfig["guiColor03"];
                if (o.isValid()) {
                    if (!o.isString()) {
                        throw ConfigException("invalid guiColor03");
                    }
                    this->guiColor03 = o.toString();
                }
                o = generalConfig["guiColor04"];
                if (o.isValid()) {
                    if (!o.isString()) {
                        throw ConfigException("invalid guiColor04");
                    }
                    this->guiColor04 = o.toString();
                }
                o = generalConfig["guiColor05"];
                if (o.isValid()) {
                    if (!o.isString()) {
                        throw ConfigException("invalid guiColor05");
                    }
                    this->guiColor05 = o.toString();
                }
                o = generalConfig["guiFont"];
                if (o.isValid()) {
                    if (!o.isString()) {
                        throw ConfigException("invalid guiFont");
                    }
                    this->guiFont = o.toString();
                }
                o = generalConfig["guiFontColor"];
                if (o.isValid()) {
                    if (!o.isString()) {
                        throw ConfigException("invalid guiFontColor");
                    }
                    this->guiFontColor = o.toString();
                }
    
                o = generalConfig["primarySelectionColor"];
                if (o.isValid()) {
                    if (!o.isString()) {
                        throw ConfigException("invalid primarySelectionColor");
                    }
                    this->primarySelectionColor = o.toString();
                }
                o = generalConfig["pseudoSelectionColor"];
                if (o.isValid()) {
                    if (!o.isString()) {
                        throw ConfigException("invalid pseudoSelectionColor");
                    }
                    this->pseudoSelectionColor = o.toString();
                }
    
                o = generalConfig["initialWindowWidth"];
                if (o.isValid()) {
                    if (!o.isNumber()) {
                        throw ConfigException("invalid initialWindowWidth");
                    }
                    this->initialWindowWidth = (int) o.toNumber();
                }
    
                o = generalConfig["initialWindowHeight"];
                if (o.isValid()) {
                    if (!o.isNumber()) {
                        throw ConfigException("invalid initialWindowHeight");
                    }
                    this->initialWindowHeight = (int) o.toNumber();
                }
    
                o = generalConfig["x11SelectionChunkLength"];
                if (o.isValid()) {
                    if (!o.isNumber()) {
                        throw ConfigException("invalid x11SelectionChunkLength");
                    }
                    this->x11SelectionChunkLength = (int) o.toNumber();
                    if (x11SelectionChunkLength <= 0) {
                        x11SelectionChunkLength = LONG_MAX;
                    }
                }
    
                o = generalConfig["buttonInnerSpacing"];
                if (o.isValid()) {
                    if (!o.isNumber()) {
                        throw ConfigException("invalid buttonInnerSpacing");
                    }
                    this->buttonInnerSpacing = (int) o.toNumber();
                }
    
                o = generalConfig["guiSpacing"];
                if (o.isValid()) {
                    if (!o.isNumber()) {
                        throw ConfigException("invalid guiSpacing");
                    }
                    this->guiSpacing = (int) o.toNumber();
                }
    
                o = generalConfig["editorPanelOnTop"];
                if (o.isValid()) {
                    if (!o.isBoolean()) {
                        throw ConfigException("invalid editorPanelOnTop");
                    }
                    this->editorPanelOnTop = o.toBoolean();
                }
    
                o = generalConfig["keepRunningIfOwningClipboard"];
                if (o.isValid()) {
                    if (!o.isBoolean()) {
                        throw ConfigException("invalid keepRunningIfOwningClipboard");
                    }
                    this->keepRunningIfOwningClipboard = o.toBoolean();
                }
    
                o = generalConfig["maxRegexAssertionLength"];
                if (o.isValid()) {
                    if (!o.isNumber()) {
                        throw ConfigException("invalid maxRegexAssertionLength");
                    }
                    this->maxRegexAssertionLength = (long) o.toNumber();
                }
    
            }
    
            // textStyles
    
            LuaVar ts = configTable["textStyles"];
            if (!ts.isTable()) {
                throw ConfigException("invalid textstyles");
            }
    
            LuaVar o(luaAccess);
            
            TextStyleDefinitions::Ptr newTextStyleDefinitions = TextStyleDefinitions::create();
                    
            for (int i = 0; o = ts[i + 1], o.isValid(); ++i) {
                LuaVar n = o["name"];
                if (!n.isString()) {
                    throw ConfigException("textstyle has invalid name");
                }
                String name = n.toString();
                if (i == 0 && name != "default") {
                    throw ConfigException("first textstyle must be named 'default'");
                }
    
                LuaVar f = o["font"];
                if (!f.isString()) {
                    throw ConfigException(String() << "invalid font in textstyle '" << name << "'");
                }
                String fontname = f.toString();
                LuaVar c = o["color"];
                if (!c.isString()) {
                    throw ConfigException(String() << "invalid color in textstyle '" << name << "'");
                }
                String colorname = c.toString();
    
                newTextStyleDefinitions->append(TextStyleDefinition(name, fontname, colorname));
            }
            this->textStyleDefinitions = newTextStyleDefinitions;
    
            // LanguageModes
    
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
            if (tryItAgain) {
                printf("%s\n", ex.toString().toCString());
            }
            ASSERT(!tryItAgain); // MODE_FALLBACK should not throw exception

            if (!tryItAgain)
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
        if (!actionKeyConfig.isValid())
        {
            tryItAgain = true;
            luaInterpreter->setMode(ConfigPackageLoader::MODE_FALLBACK);
        }
        else if (tryItAgain) {
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
