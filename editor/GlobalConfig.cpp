/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#include "GlobalConfig.h"
#include "ConfigException.h"
#include "DirectoryReader.h"
#include "RegexException.h"
#include "LuaInterpreter.h"
#include "LuaObject.h"

using namespace LucED;

SingletonInstance<GlobalConfig> GlobalConfig::instance;;

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
          initialWindowWidth(80),
          initialWindowHeight(25),
          x11SelectionChunkLength(20000),
          buttonInnerSpacing(2),
          guiSpacing(2),
          editorPanelOnTop(false),
          keepRunningIfOwningClipboard(false)
{
    setlocale(LC_CTYPE, "");
}


SyntaxPatterns::Ptr GlobalConfig::getSyntaxPatternsForLanguageMode(const String& languageMode) {
    NameToIndexMap::Value foundIndex = languageModeToSyntaxIndex->get(languageMode);
    if (foundIndex.isValid()) {
        return allSyntaxPatterns.get(foundIndex.get());
    } else {
        return SyntaxPatterns::Ptr();
    }
}

SyntaxPatterns::Ptr GlobalConfig::getSyntaxPatternsForLanguageMode(LanguageMode::Ptr languageMode) {
    if (languageMode.isValid()) {
        return getSyntaxPatternsForLanguageMode(languageMode->getName());
    } else {
        return SyntaxPatterns::Ptr();
    }
}
    
SyntaxPatterns::Ptr GlobalConfig::getSyntaxPatternsForFileName(const String& fileName)
{
    return getSyntaxPatternsForLanguageMode(languageModes->getLanguageModeForFile(fileName)->getName());
}

LanguageMode::Ptr GlobalConfig::getLanguageModeForFileName(const String& fileName)
{
    return languageModes->getLanguageModeForFile(fileName);
}

LanguageMode::Ptr GlobalConfig::getDefaultLanguageMode()
{
    return languageModes->getDefaultLanguageMode();
}

void GlobalConfig::readConfig(const String& configPath)
{
    LuaInterpreter* lua = LuaInterpreter::getInstance();
    lua->executeFile(String() << configPath << "/general.lua");
    
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
            this->keyPressRepeatFirstMicroSecs = (long) (o.toNumber() * 1000);
        }

        o = globalConfig["keyPressRepeatNextMilliSecs"];
        if (o.isValid()) {
            if (!o.isNumber()) {
                throw ConfigException("invalid keyPressRepeatNextMilliSecs");
            }
            this->keyPressRepeatNextMicroSecs = (long) (o.toNumber() * 1000);
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
            this->scrollBarRepeatFirstMicroSecs = (long) (o.toNumber() * 1000);
        }

        o = globalConfig["scrollBarRepeatNextMilliSecs"];
        if (o.isValid()) {
            if (!o.isNumber()) {
                throw ConfigException("invalid scrollBarRepeatNextMilliSecs");
            }
            this->scrollBarRepeatNextMicroSecs = (long) (o.toNumber() * 1000);
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

    }


    // textStyles

    LuaObject ts = lua->getGlobal("textStyles");
    if (!ts.isTable()) {
        throw ConfigException("invalid textstyles");
    }
    textStyles = TextStyles::create();
    
    textStyleNameToIndexMap = NameToIndexMap::create();

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
    if (textStyles->getLength() == 0) {
        throw ConfigException("missing textstyles");
    }
    
    // LanguageModes
    
    languageModes = LanguageModes::create();
    NameToIndexMap::Ptr languageModeToIndex = NameToIndexMap::create();
    
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
            languageModeToIndex->set(n.toString(), i);
            languageModes->append(o);
        }
    }

    // SyntaxPatterns
    
    languageModeToSyntaxIndex = NameToIndexMap::create();
    
    DirectoryReader dirReader(String() << configPath << "/syntaxpatterns");
    for (int i = 0; dirReader.next();) {
        if (dirReader.isFile()) {
            String name = dirReader.getName();
            if (name.getLength() > 4 && name.getSubstring(name.getLength() - 4, 4) == ".lua") {
                name = name.getSubstring(0, name.getLength() - 4);
                lua->clearGlobal("syntaxpatterns");
                lua->executeFile(String() << configPath << "/syntaxpatterns/" << name << ".lua");
                languageModeToSyntaxIndex->set(name, i);
                LuaObject sp = lua->getGlobal("syntaxPatterns");
                if (!sp.isTable()) {
                    throw ConfigException(String() << "Syntaxpattern '" << name << "' has invalid 'syntaxpatterns' element");
                }
                try {
                    allSyntaxPatterns.append(
                            SyntaxPatterns::create(sp, textStyleNameToIndexMap));
                } catch (RegexException& ex) {
                    throw ConfigException(String() << "syntaxpatterns for language mode '" << name << "' have invalid regex: "
                            << ex.getMessage());
                }
                if (!languageModeToIndex->hasKey(name)) {
                    languageModes->append(name);
                }
                ++i;
            }
        }
    }
    lua->clearGlobal("syntaxPatterns");
    
}

