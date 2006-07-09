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

#include "LanguageModes.h"
#include "ConfigException.h"
#include "RegexException.h"

using namespace LucED;


LanguageMode::LanguageMode(const string& name, Regex regex)
        : name(name), regex(regex)
{}

LanguageMode::LanguageMode(LuaObject config)
    : approximateUnknownHilitingFlag(true),
      approximateUnknownHilitingReparseRange(2000),
      hilitingBreakPointDistance(50)
{
    LuaObject o = config["name"];
    if (!o.isString()) {
        throw ConfigException("invalid or missing element 'name' in languageMode");
    }
    name = o.toString();

    o = config["fileNameRegex"];
    if (o.isValid()) {
        if (!o.isString()) {
            throw ConfigException("languageMode '" + name + "' has invalid element 'fileNameRegex'");
        }
        try {
            regex = Regex(o.toString());
        } catch (RegexException& ex) {
            throw ConfigException("languageMode '" + name + "' has invalid element 'fileNameRegex': "
                    + ex.getMessage());
        }
    }

    o = config["approximateUnknownHiliting"];
    if (o.isValid()) {
        if (!o.isBoolean()) {
            throw ConfigException("languageMode '" + name + "' has invalid element 'approximateUnknownHiliting'");
        }
        approximateUnknownHilitingFlag = o.toBoolean();
    }

    o = config["approximateUnknownHilitingReparseRange"];
    if (o.isValid()) {
        if (!o.isNumber()) {
            throw ConfigException("languageMode '" + name + "' has invalid element 'approximateUnknownHilitingReparseRange'");
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
}

LanguageModes::LanguageModes()
    : defaultLanguageMode(LanguageMode::create("default"))
{
}

void LanguageModes::append(const string& name)
{
    modes.append(LanguageMode::create(name));
    nameToIndexMap.set(name, modes.getLength() - 1);
}

void LanguageModes::append(LuaObject config)
{
    modes.append(LanguageMode::create(config));
    nameToIndexMap.set(modes[modes.getLength() - 1]->getName(), modes.getLength() - 1);
    Regex regex = modes[modes.getLength() - 1]->getRegex();
    if (regex.isValid()) {
        ovector.increaseTo(regex.getOvecSize());
    }
}

LanguageMode::Ptr LanguageModes::getLanguageModeForFile(const string& fileName)
{
    for (int i = 0; i < modes.getLength(); ++i)
    {
        Regex re = modes[i]->getRegex();
        if (re.isValid()) {
            bool matched = re.findMatch(fileName.c_str(), fileName.length(), 0, Regex::MatchOptions(), ovector);
            if (matched && ovector[0] == 0 && ovector[1] == fileName.length()) {
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


