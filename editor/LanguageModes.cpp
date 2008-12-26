/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2007 Oliver Schmidt, oliver at luced dot de
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

#include "LanguageModes.hpp"
#include "ConfigException.hpp"
#include "RegexException.hpp"
#include "LuaVar.hpp"

using namespace LucED;

LanguageMode::Ptr LanguageMode::create(LuaVar config)
{
    return Ptr(new LanguageMode(config));
}


LanguageMode::LanguageMode(const String& name, BasicRegex regex)
    : name(name), regex(regex),
      approximateUnknownHilitingFlag(true),
      approximateUnknownHilitingReparseRange(2000),
      hilitingBreakPointDistance(50),
      hardTabWidth(8),
      softTabWidth(-1)
{}

LanguageMode::LanguageMode(LuaVar config)
    : approximateUnknownHilitingFlag(true),
      approximateUnknownHilitingReparseRange(2000),
      hilitingBreakPointDistance(50),
      hardTabWidth(8),
      softTabWidth(-1)
{
    LuaVar o = config["name"];
    if (!o.isString()) {
        throw ConfigException("invalid or missing element 'name' in languageMode");
    }
    name = o.toString();

    o = config["syntaxName"];
    if (o.isValid()) {
        if (!o.isString()) {
            throw ConfigException(String() << "languageMode '" << name << "' has invalid element 'syntaxName'");
        }
        syntaxName = o.toString();
    }

    o = config["fileNameRegex"];
    if (o.isValid()) {
        if (!o.isString()) {
            throw ConfigException(String() << "languageMode '" << name << "' has invalid element 'fileNameRegex'");
        }
        try {
            regex = BasicRegex(o.toString());
        } catch (RegexException& ex) {
            throw ConfigException(String() << "languageMode '" << name << "' has invalid element 'fileNameRegex': "
                    << ex.getMessage());
        }
    }

    o = config["approximateUnknownHiliting"];
    if (o.isValid()) {
        if (!o.isBoolean()) {
            throw ConfigException(String() << "languageMode '" << name << "' has invalid element 'approximateUnknownHiliting'");
        }
        approximateUnknownHilitingFlag = o.toBoolean();
    }

    o = config["approximateUnknownHilitingReparseRange"];
    if (o.isValid()) {
        if (!o.isNumber()) {
            throw ConfigException(String() << "languageMode '" << name << "' has invalid element 'approximateUnknownHilitingReparseRange'");
        }
        approximateUnknownHilitingReparseRange = (long) o.toNumber();
    }

    o = config["hilitingBreakPointDistance"];
    if (o.isValid()) {
        if (!o.isNumber()) {
            throw ConfigException("invalid hilitingBreakPointDistance");
        }
        this->hilitingBreakPointDistance = (int) o.toNumber();
    }
    
    o = config["hardTabWidth"];
    if (o.isValid()) {
        if (!o.isNumber()) {
            throw ConfigException("invalid hardTabWidth");
        }
        this->hardTabWidth = (int) o.toNumber();
        if (this->hardTabWidth < 1) {
            throw ConfigException("invalid hardTabWidth");
        }
    }
    
    o = config["softTabWidth"];
    if (o.isValid()) {
        if (!o.isNumber()) {
            throw ConfigException("invalid softTabWidth");
        }
        this->softTabWidth = (int) o.toNumber();
    }
    
    
}

LanguageModes::LanguageModes()
    : defaultLanguageMode(LanguageMode::create("default"))
{
}

void LanguageModes::append(const String& name)
{
    modes.append(LanguageMode::create(name));
    nameToIndexMap.set(name, modes.getLength() - 1);
}

void LanguageModes::append(LuaVar config)
{
    LanguageMode::Ptr newMode = LanguageMode::create(config);
    modes.append(newMode);
    nameToIndexMap.set(newMode->getName(), modes.getLength() - 1);
    BasicRegex regex = newMode->getRegex();
    if (regex.isValid()) {
        ovector.increaseTo(regex.getOvecSize());
    }
}

LanguageMode::Ptr LanguageModes::getLanguageModeForFile(const String& fileName)
{
    for (int i = 0; i < modes.getLength(); ++i)
    {
        BasicRegex re = modes[i]->getRegex();
        if (re.isValid()) {
            bool matched = re.findMatch(fileName.toCString(), fileName.getLength(), 0, BasicRegex::MatchOptions(), ovector);
            if (matched && ovector[0] == 0 && ovector[1] == fileName.getLength()) {
                return modes[i];
            }
        }
    }
    return defaultLanguageMode;
}

LanguageMode::Ptr LanguageModes::getDefaultLanguageMode()
{
    return defaultLanguageMode;
}


