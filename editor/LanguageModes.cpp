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

LanguageModes::LanguageModes()
    : defaultLanguageMode(LanguageMode::create())
{
    defaultLanguageMode->setName("default");
}

void LanguageModes::append(const String& name)
{
    LanguageMode::Ptr languageMode = LanguageMode::create();
    languageMode->setName(name);
    modes.append(languageMode);
    nameToIndexMap.set(name, modes.getLength() - 1);
}

void LanguageModes::append(LanguageMode::Ptr languageMode)
{
    modes.append(languageMode);
    nameToIndexMap.set(languageMode->getName(), modes.getLength() - 1);
    BasicRegex regex = languageMode->getFileNameRegex();
    if (regex.isValid()) {
        ovector.increaseTo(regex.getOvecSize());
    }
}

LanguageMode::Ptr LanguageModes::getLanguageModeForFile(const String& fileName)
{
    for (int i = 0; i < modes.getLength(); ++i)
    {
        BasicRegex re = modes[i]->getFileNameRegex();
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


