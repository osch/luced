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
#include "LuaInterpreter.hpp"
#include "LuaObject.hpp"
#include "System.hpp"
#include "File.hpp"

using namespace LucED;

SingletonInstance<GlobalConfig> GlobalConfig::instance;;

static const char* const luaFileExtension = ".lua";

GlobalConfig* GlobalConfig::getInstance()
{
    return instance.getPtr();
}

GlobalConfig::GlobalConfig()
        : useKeyPressRepeater(true),
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
          syntaxPatternsConfig(SyntaxPatternsConfig::create())
{
    setlocale(LC_CTYPE, "");
}


SyntaxPatterns::Ptr GlobalConfig::getSyntaxPatternsForLanguageMode(const String& languageModeName,
                                                                   Callback<SyntaxPatterns::Ptr>::Ptr changeCallback)
{
    return this->syntaxPatternsConfig->getSyntaxPatternsForLanguageMode(languageModeName, changeCallback);
}


SyntaxPatterns::Ptr GlobalConfig::getSyntaxPatternsForLanguageMode(LanguageMode::Ptr languageMode,
                                                                   Callback<SyntaxPatterns::Ptr>::Ptr changeCallback)
{
    if (languageMode.isValid()) {
        return getSyntaxPatternsForLanguageMode(languageMode->getName(), changeCallback);
    } else {
        return SyntaxPatterns::Ptr();
    }
}
    

SyntaxPatterns::Ptr GlobalConfig::getSyntaxPatternsForFileName(const String& fileName,
                                                               Callback<SyntaxPatterns::Ptr>::Ptr changeCallback)
{
    return getSyntaxPatternsForLanguageMode(languageModes->getLanguageModeForFile(fileName)->getName(),
                                            changeCallback);
}


LanguageMode::Ptr GlobalConfig::getLanguageModeForFileName(const String& fileName)
{
    return languageModes->getLanguageModeForFile(fileName);
}


LanguageMode::Ptr GlobalConfig::getDefaultLanguageMode()
{
    return languageModes->getDefaultLanguageMode();
}


void GlobalConfig::readConfig()
{
    ConfigException::ErrorList::Ptr errorList = ConfigException::ErrorList::create();
    
    String configDirectory = ".luced";
    
    if (!File(configDirectory).exists()) {
        String homeDirectory = System::getInstance()->getHomeDirectory();
        configDirectory = File(homeDirectory, ".luced").getAbsoluteName();
    }

    LuaInterpreter* lua = LuaInterpreter::getInstance();

    this->generalConfigFileName = File(configDirectory, "general.lua").getAbsoluteName();
    String configFileName = generalConfigFileName;

    languageModes           = LanguageModes::create();
    textStyles              = TextStyles::create();
    textStyleNameToIndexMap = NameToIndexMap::create();

    try
    {
        lua->executeFile(configFileName);

        // globalConfig

        LuaObject globalConfig = lua->getGlobal("globalConfig");

        if (globalConfig.isValid())
        {
            if (!globalConfig.isTable()) {
                throw ConfigException("invalid globalConfig");
            }

            LuaObject o = globalConfig["useKeyPressRepeater"];
            if (o.isValid()) {
                if (!o.isBoolean()) {
                    throw ConfigException("invalid useKeyPressRepeater");
                }
                this->useKeyPressRepeater = o.toBoolean();
            }

            o = globalConfig["keyPressRepeatFirstMilliSecs"];
            if (o.isValid()) {
                if (!o.isNumber()) {
                    throw ConfigException("invalid keyPressRepeatFirstMilliSecs");
                }
                this->keyPressRepeatFirstMicroSecs = MicroSeconds((long) (o.toNumber() * 1000));
            }

            o = globalConfig["keyPressRepeatNextMilliSecs"];
            if (o.isValid()) {
                if (!o.isNumber()) {
                    throw ConfigException("invalid keyPressRepeatNextMilliSecs");
                }
                this->keyPressRepeatNextMicroSecs = MicroSeconds((long) (o.toNumber() * 1000));
            }

            o = globalConfig["scrollBarWidth"];
            if (o.isValid()) {
                if (!o.isNumber()) {
                    throw ConfigException("invalid scrollBarWidth");
                }
                this->scrollBarWidth = (int) o.toNumber();
            }

            o = globalConfig["scrollBarRepeatFirstMilliSecs"];
            if (o.isValid()) {
                if (!o.isNumber()) {
                    throw ConfigException("invalid scrollBarRepeatFirstMilliSecs");
                }
                this->scrollBarRepeatFirstMicroSecs = MicroSeconds((long) (o.toNumber() * 1000));
            }

            o = globalConfig["scrollBarRepeatNextMilliSecs"];
            if (o.isValid()) {
                if (!o.isNumber()) {
                    throw ConfigException("invalid scrollBarRepeatNextMilliSecs");
                }
                this->scrollBarRepeatNextMicroSecs = MicroSeconds((long) (o.toNumber() * 1000));
            }

            o = globalConfig["doubleClickMilliSecs"];
            if (o.isValid()) {
                if (!o.isNumber()) {
                    throw ConfigException("invalid doubleClickMilliSecs");
                }
                this->doubleClickMilliSecs = (long) o.toNumber();
            }

            o = globalConfig["guiColor01"];
            if (o.isValid()) {
                if (!o.isString()) {
                    throw ConfigException("invalid guiColor01");
                }
                this->guiColor01 = o.toString();
            }
            o = globalConfig["guiColor02"];
            if (o.isValid()) {
                if (!o.isString()) {
                    throw ConfigException("invalid guiColor02");
                }
                this->guiColor02 = o.toString();
            }
            o = globalConfig["guiColor03"];
            if (o.isValid()) {
                if (!o.isString()) {
                    throw ConfigException("invalid guiColor03");
                }
                this->guiColor03 = o.toString();
            }
            o = globalConfig["guiColor04"];
            if (o.isValid()) {
                if (!o.isString()) {
                    throw ConfigException("invalid guiColor04");
                }
                this->guiColor04 = o.toString();
            }
            o = globalConfig["guiColor05"];
            if (o.isValid()) {
                if (!o.isString()) {
                    throw ConfigException("invalid guiColor05");
                }
                this->guiColor05 = o.toString();
            }
            o = globalConfig["guiFont"];
            if (o.isValid()) {
                if (!o.isString()) {
                    throw ConfigException("invalid guiFont");
                }
                this->guiFont = o.toString();
            }
            o = globalConfig["guiFontColor"];
            if (o.isValid()) {
                if (!o.isString()) {
                    throw ConfigException("invalid guiFontColor");
                }
                this->guiFontColor = o.toString();
            }

            o = globalConfig["primarySelectionColor"];
            if (o.isValid()) {
                if (!o.isString()) {
                    throw ConfigException("invalid primarySelectionColor");
                }
                this->primarySelectionColor = o.toString();
            }
            o = globalConfig["pseudoSelectionColor"];
            if (o.isValid()) {
                if (!o.isString()) {
                    throw ConfigException("invalid pseudoSelectionColor");
                }
                this->pseudoSelectionColor = o.toString();
            }

            o = globalConfig["initialWindowWidth"];
            if (o.isValid()) {
                if (!o.isNumber()) {
                    throw ConfigException("invalid initialWindowWidth");
                }
                this->initialWindowWidth = (int) o.toNumber();
            }

            o = globalConfig["initialWindowHeight"];
            if (o.isValid()) {
                if (!o.isNumber()) {
                    throw ConfigException("invalid initialWindowHeight");
                }
                this->initialWindowHeight = (int) o.toNumber();
            }

            o = globalConfig["x11SelectionChunkLength"];
            if (o.isValid()) {
                if (!o.isNumber()) {
                    throw ConfigException("invalid x11SelectionChunkLength");
                }
                this->x11SelectionChunkLength = (int) o.toNumber();
                if (x11SelectionChunkLength <= 0) {
                    x11SelectionChunkLength = LONG_MAX;
                }
            }

            o = globalConfig["buttonInnerSpacing"];
            if (o.isValid()) {
                if (!o.isNumber()) {
                    throw ConfigException("invalid buttonInnerSpacing");
                }
                this->buttonInnerSpacing = (int) o.toNumber();
            }

            o = globalConfig["guiSpacing"];
            if (o.isValid()) {
                if (!o.isNumber()) {
                    throw ConfigException("invalid guiSpacing");
                }
                this->guiSpacing = (int) o.toNumber();
            }

            o = globalConfig["editorPanelOnTop"];
            if (o.isValid()) {
                if (!o.isBoolean()) {
                    throw ConfigException("invalid editorPanelOnTop");
                }
                this->editorPanelOnTop = o.toBoolean();
            }

            o = globalConfig["keepRunningIfOwningClipboard"];
            if (o.isValid()) {
                if (!o.isBoolean()) {
                    throw ConfigException("invalid keepRunningIfOwningClipboard");
                }
                this->keepRunningIfOwningClipboard = o.toBoolean();
            }

            o = globalConfig["maxRegexAssertionLength"];
            if (o.isValid()) {
                if (!o.isNumber()) {
                    throw ConfigException("invalid maxRegexAssertionLength");
                }
                this->maxRegexAssertionLength = (long) o.toNumber();
            }

        }


        // textStyles

        LuaObject ts = lua->getGlobal("textStyles");
        if (!ts.isTable()) {
            throw ConfigException("invalid textstyles");
        }

        LuaObject o;
        for (int i = 0; o = ts[i + 1], o.isValid(); ++i) {
            LuaObject n = o["name"];
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

            LuaObject f = o["font"];
            if (!f.isString()) {
                throw ConfigException(String() << "invalid font in textstyle '" << name << "'");
            }
            String fontname = f.toString();
            LuaObject c = o["color"];
            if (!c.isString()) {
                throw ConfigException(String() << "invalid color in textstyle '" << name << "'");
            }
            String colorname = c.toString();
            textStyles->appendNewStyle(fontname, colorname);
        }

        // LanguageModes

        LuaObject lm = lua->getGlobal("languageModes");
        if (lm.isValid())
        {
            if (!lm.isTable()) {
                throw ConfigException("invalid languageModes");
            }
            for (int i = 0; o = lm[i+1], o.isValid(); ++i)
            {
                LuaObject n = o["name"];
                if (!n.isString()) {
                    throw ConfigException("invalid or missing element 'name' in languageMode");
                }
                languageModes->append(o);
            }
        }
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
                    LuaInterpreter::Result luaRslt = lua->executeFile(configFileName);

                    if (luaRslt.objects.getLength() <= 0 || !luaRslt.objects[0].isTable()) {
                        throw ConfigException(String() << "Syntaxpattern '" << languageModeName << "' returns invalid element");
                    }
                    LuaObject sp = luaRslt.objects[0];
                    
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
}


SyntaxPatterns::Ptr GlobalConfig::loadSyntaxPatterns(const String& absoluteFileName)
{
    LuaInterpreter::Result luaRslt = LuaInterpreter::getInstance()->executeFile(absoluteFileName);
    
    if (luaRslt.objects.getLength() <= 0 || !luaRslt.objects[0].isTable()) {
        throw ConfigException(String() << "Syntaxpattern '" << absoluteFileName << "' returns invalid element");
    }
    LuaObject sp = luaRslt.objects[0];
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
        configChangedCallbackContainer.invokeAllCallbacks();
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
