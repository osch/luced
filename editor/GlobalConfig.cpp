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

#include <locale.h>
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
          actionKeyConfig(ActionKeyConfig::create())
{
    setlocale(LC_CTYPE, "");
}



SyntaxPatterns::Ptr GlobalConfig::getSyntaxPatternsForLanguageMode(const String& languageModeName,
                                                                   Callback<SyntaxPatterns::Ptr>::Ptr changeCallback) const
{
    return this->syntaxPatternsConfig->getSyntaxPatternsForLanguageMode(languageModeName, changeCallback);
}


SyntaxPatterns::Ptr GlobalConfig::getSyntaxPatternsForLanguageMode(LanguageMode::Ptr languageMode,
                                                                   Callback<SyntaxPatterns::Ptr>::Ptr changeCallback) const
{
    if (languageMode.isValid()) {
        return getSyntaxPatternsForLanguageMode(languageMode->getName(), changeCallback);
    } else {
        return SyntaxPatterns::Ptr();
    }
}
    

SyntaxPatterns::Ptr GlobalConfig::getSyntaxPatternsForFileName(const String& fileName,
                                                               Callback<SyntaxPatterns::Ptr>::Ptr changeCallback) const
{
    return getSyntaxPatternsForLanguageMode(languageModes->getLanguageModeForFile(fileName)->getName(),
                                            changeCallback);
}


LanguageMode::Ptr GlobalConfig::getLanguageModeForFileName(const String& fileName) const
{
    return languageModes->getLanguageModeForFile(fileName);
}


LanguageMode::Ptr GlobalConfig::getDefaultLanguageMode() const
{
    return languageModes->getDefaultLanguageMode();
}

void GlobalConfig::readActionKeyBinding(ActionKeyConfig::Ptr actionKeyConfig, LuaVarRef actionKeyBinding)
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
            LuaVar package = GlobalLuaInterpreter::getInstance()->require(packageName);
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
                readActionKeyBinding(actionKeyConfig, packageBinding);
            }
        }
    }
}

void GlobalConfig::readConfig()
{
    packagesMap.clear();
    
    ConfigException::ErrorList::Ptr errorList = ConfigException::ErrorList::create();
    
    configDirectory = File(".luced").getAbsoluteName();
    
    if (!File(configDirectory).exists()) {
        String homeDirectory = System::getInstance()->getHomeDirectory();
        configDirectory = File(homeDirectory, ".luced").getAbsoluteName();
    }

    LuaAccess luaAccess = GlobalLuaInterpreter::getInstance()->getCurrentLuaAccess();

    this->generalConfigFileName = File(configDirectory, "config.lua").getAbsoluteName();
    String configFileName = generalConfigFileName;

    languageModes           = LanguageModes::create();
    textStyles              = TextStyles::create();
    textStyleNameToIndexMap = NameToIndexMap::create();

    try
    {
        CurrentDirectoryKeeper currentDirectoryKeeper(configDirectory);
        
        LuaAccess::Result luaRslt = luaAccess.executeFile(configFileName);
        
        if (luaRslt.objects.getLength() < 1) {
            throw ConfigException("config file does not return table");
        }
        
        LuaVar configTable = luaRslt.objects[0];

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
        
        for (int i = 0; o = ts[i + 1], o.isValid(); ++i) {
            LuaVar n = o["name"];
            if (!n.isString()) {
                throw ConfigException("textstyle has invalid name");
            }
            String name = n.toString();
            if (i == 0 && name != "default") {
                throw ConfigException("first textstyle must be named 'default'");
            }
            if (textStyleNameToIndexMap->hasKey(name)) {
                throw ConfigException(String() << "duplicate textstyle '" << name << "'");
            }

            textStyleNameToIndexMap->set(name, i);

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
            textStyles->appendNewStyle(fontname, colorname);
        }

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
                languageModes->append(o);
            }
        }
        ActionKeyConfig::Ptr newActionKeyConfig = ActionKeyConfig::create();
        LuaVar actionKeyBinding = configTable["actionKeyBinding"];
        if (actionKeyBinding.isValid())
        {
            if (!actionKeyBinding.isTable()) {
                throw ConfigException("invalid actionKeyBinding");
            }
            readActionKeyBinding(newActionKeyConfig, actionKeyBinding);
        }
        actionKeyConfig = newActionKeyConfig;
    }
    catch (BaseException& ex) {
        errorList->appendNew(configFileName, ex.getMessage());
        if (textStyles->getLength() == 0) {
            textStyles->appendNewStyle("-*-courier-medium-r-*-*-*-120-75-75-*-*-*-*", "black");
        }
    }

    if (textStyles->getLength() == 0) {
        textStyles->appendNewStyle("-*-courier-medium-r-*-*-*-120-75-75-*-*-*-*", "black");
        errorList->appendNew(configFileName, "missing textstyles");
    }

    // SyntaxPatterns
    
    this->syntaxPatternsConfig->clear();
    this->syntaxPatternDirectory = File(configDirectory, "syntaxpatterns").getAbsoluteName();
    
    
    for (DirectoryReader dirReader(syntaxPatternDirectory); dirReader.next();)
    {
        if (dirReader.isFile()) {
            String fileName = dirReader.getName();
            if (fileName.endsWith(luaFileExtension))
            {
                String languageModeName = fileName.getSubstring(0, fileName.getLength() - strlen(luaFileExtension));
                configFileName = File(syntaxPatternDirectory, String() << languageModeName << luaFileExtension).getAbsoluteName();

                if (!languageModes->hasLanguageMode(languageModeName)) {
                    languageModes->append(languageModeName);
                }

                try
                {
                    LuaAccess::Result luaRslt = luaAccess.executeFile(configFileName);

                    if (luaRslt.objects.getLength() <= 0 || !luaRslt.objects[0].isTable()) {
                        throw ConfigException(String() << "Syntaxpattern '" << languageModeName << "' returns invalid element");
                    }
                    LuaVar sp = luaRslt.objects[0];
                    
                    try
                    {
                        this->syntaxPatternsConfig->append(languageModeName, 
                                                           configFileName, 
                                                           SyntaxPatterns::create(sp, textStyleNameToIndexMap));
                    }
                    catch (RegexException& ex) {
                        throw ConfigException(String() << "syntaxpatterns for language mode '" << languageModeName << "' have invalid regex: "
                                << ex.getMessage());
                    }
                }
                catch (BaseException& ex)
                {
                    errorList->appendNew(configFileName, ex.getMessage());
                    this->syntaxPatternsConfig->append(languageModeName, 
                                                       configFileName, 
                                                       SyntaxPatterns::Ptr());
                }
            }
        }
    }
    for (int i = 0; i < languageModes->getLength(); ++i)
    {
        LanguageMode::Ptr languageMode      = languageModes->get(i);
        const String      languageModeName  = languageMode->getName();
        const String      configFileName    = String() << syntaxPatternDirectory << "/" << languageModeName << luaFileExtension;

        if (!this->syntaxPatternsConfig->hasEntryForConfigFileName(configFileName))
        {
            this->syntaxPatternsConfig->append(languageModeName, 
                                               configFileName, 
                                               SyntaxPatterns::Ptr());
        }
    }
    if (errorList->getLength() > 0)
    {
        throw ConfigException(errorList);
    }

    configChangedCallbackContainer.invokeAllCallbacks();
}


SyntaxPatterns::Ptr GlobalConfig::loadSyntaxPatterns(const String& absoluteFileName)
{
    LuaAccess         luaAccess = GlobalLuaInterpreter::getInstance()->getCurrentLuaAccess();
    
    LuaAccess::Result luaRslt   = luaAccess.executeFile(absoluteFileName);
    
    if (luaRslt.objects.getLength() <= 0 || !luaRslt.objects[0].isTable()) {
        throw ConfigException(String() << "Syntaxpattern '" << absoluteFileName << "' returns invalid element");
    }
    LuaVar sp = luaRslt.objects[0];
    SyntaxPatterns::Ptr rslt;
    try {
        rslt = SyntaxPatterns::create(sp, textStyleNameToIndexMap);
    } catch (RegexException& ex) {
        throw ConfigException(String() << "syntaxpatterns for language mode '" << absoluteFileName << "' have invalid regex: "
                << ex.getMessage());
    }
    return rslt;
}


void GlobalConfig::notifyAboutNewFileContent(String absoluteFileName)
{
    if (absoluteFileName == generalConfigFileName)
    {
        readConfig();
    }
    else
    {
        if (this->syntaxPatternsConfig->hasEntryForConfigFileName(absoluteFileName))
        {
            this->syntaxPatternsConfig->updateForConfigFileName(absoluteFileName, 
                                                                loadSyntaxPatterns(absoluteFileName));
        }
        else if (File(absoluteFileName).getDirName() == syntaxPatternDirectory && absoluteFileName.endsWith(luaFileExtension))
        {
            const String baseName = File(absoluteFileName).getBaseName();
            ASSERT(baseName.endsWith(luaFileExtension));
            
            const String        languageModeName = baseName.getSubstring(0, baseName.getLength() - strlen(luaFileExtension));

            if (!languageModes->hasLanguageMode(languageModeName)) {
                languageModes->append(languageModeName);
            }
            
            this->syntaxPatternsConfig->append(languageModeName,
                                               absoluteFileName,
                                               loadSyntaxPatterns(absoluteFileName));
        }
    }
}

bool GlobalConfig::dependsOnPackage(const String& packageName) const
{
    Nullable<bool> flag = packagesMap.get(packageName);
    
    return flag.isValid() && flag.get();
}
