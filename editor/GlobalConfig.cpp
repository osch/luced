/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch@luced.de
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

using namespace LucED;

GlobalConfig::Ptr GlobalConfig::instance = GlobalConfig::create();


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
          guiColor04("grey90"),
          guiFont("-*-helvetica-medium-r-*-*-*-120-75-75-*-*-*-*"),
          guiFontColor("black"),
          initialWindowWidth(600),
          initialWindowHeight(350),
          x11SelectionChunkLength(20000)
{
    setlocale(LC_CTYPE, "");
}


SyntaxPatterns::Ptr GlobalConfig::getSyntaxPatternsForLanguageMode(const string& languageMode) {
    NameToIndexMap::Value foundIndex = languageModeToSyntaxIndex->get(languageMode);
    if (foundIndex.isValid()) {
        return allSyntaxPatterns.get(foundIndex.get());
    } else {
        return SyntaxPatterns::Ptr();
    }
}
    
SyntaxPatterns::Ptr GlobalConfig::getSyntaxPatternsForFileName(const string& fileName)
{
    return getSyntaxPatternsForLanguageMode(languageModes->getModeForFile(fileName)->getName());
}

LanguageMode::Ptr GlobalConfig::getLanguageModeForFileName(const string& fileName)
{
    return languageModes->getModeForFile(fileName);
}


void GlobalConfig::readConfig(const string& configPath)
{
    LuaInterpreter::Ptr lua = LuaInterpreter::create();
    lua->executeFile(configPath + "/general.lua");
    
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
        string name = n.toString();
        if (i == 0 && name != "default") {
            throw ConfigException("first textstyle must be named 'default'");
        }
        if (textStyleNameToIndexMap->hasKey(name)) {
            throw ConfigException("duplicate textstyle '" + name + "'");
        }
        
        textStyleNameToIndexMap->set(name, i);
        
        LuaObject f = o["font"];
        if (!f.isString()) {
            throw ConfigException("invalid font in textstyle '" + name + "'");
        }
        string fontname = f.toString();
        LuaObject c = o["color"];
        if (!c.isString()) {
            throw ConfigException("invalid color in textstyle '" + name + "'");
        }
        string colorname = c.toString();
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
    
    DirectoryReader dirReader(configPath + "/syntaxpatterns");
    for (int i = 0; dirReader.next();) {
        if (dirReader.isFile()) {
            string name = dirReader.getName();
            if (name.length() > 4 && name.substr(name.length() - 4, 4) == ".lua") {
                name = name.substr(0, name.length() - 4);
                lua->clearGlobal("syntaxpatterns");
                lua->executeFile(configPath + "/syntaxpatterns/" + name + ".lua");
                languageModeToSyntaxIndex->set(name, i);
                LuaObject sp = lua->getGlobal("syntaxPatterns");
                if (!sp.isTable()) {
                    throw ConfigException("Syntaxpattern '" + name + "' has invalid 'syntaxpatterns' element");
                }
                try {
                    allSyntaxPatterns.append(
                            SyntaxPatterns::create(sp, textStyleNameToIndexMap));
                } catch (RegexException& ex) {
                    throw ConfigException("syntaxpatterns for language mode '" + name + "' have invalid regex: "
                            + ex.getMessage());
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

