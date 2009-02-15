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

#include "SyntaxPatternsConfig.hpp"
#include "QualifiedName.hpp"
#include "GlobalLuaInterpreter.hpp"
#include "ConfigException.hpp"

using namespace LucED;

SyntaxPatterns::Ptr SyntaxPatternsConfig::loadSyntaxPatterns(const String& syntaxNameString,
                                                             HeapHashMap<String,TextStyleDefinition>::Ptr textStyleDefinitions)
{
    if (syntaxNameString.getLength() > 0)
    {
        QualifiedName syntaxName(syntaxNameString);
        String        packageName = syntaxName.getQualifier();
        LuaVar package = GlobalLuaInterpreter::getInstance()->require(packageName);
        if (package.isTable()) {
            LuaVar syntaxDefinitionGetter = package["getSyntaxDefinition"];
            if (!syntaxDefinitionGetter.isFunction()) {
                throw ConfigException(String() << "'"<< packageName 
                                               << ".getSyntaxDefinition' must be function");
            }
            LuaVar syntaxDefinition = syntaxDefinitionGetter.call(syntaxName.getName());
            if (!syntaxDefinition.isTable()) {
                throw ConfigException(String() << "Syntax definition '" << syntaxNameString << "' not available");
            }
            return SyntaxPatterns::create(syntaxDefinition, textStyleDefinitions);
        }
        else {
            throw ConfigException(String() << "Package '" << packageName << "' does not provide table object");
        }
    }
    else
    {
        return SyntaxPatterns::createWithoutPatterns(textStyleDefinitions);
    }
}


SyntaxPatterns::Ptr SyntaxPatternsConfig::getSyntaxPatterns(const String&                      syntaxNameString,
                                                            Callback<SyntaxPatterns::Ptr>::Ptr changedCallback)
{
    Entry::Ptr entry = patterns.get(syntaxNameString);

    if (!entry.isValid())
    {
        SyntaxPatterns::Ptr newPatterns = loadSyntaxPatterns(syntaxNameString, textStyleDefinitions);
        
        entry = Entry::create(newPatterns);

        patterns.set(syntaxNameString, entry);
    }
    if (changedCallback.isValid()) {
        entry->addChangedCallback(changedCallback);
    }
    return entry->getSyntaxPatterns();
}


void SyntaxPatternsConfig::refresh(HeapHashMap<String,TextStyleDefinition>::Ptr newTextStyleDefinitions)
{
    HashMap<String,Entry::Ptr>::Iterator patternIterator = patterns.getIterator();

    while (!patternIterator.isAtEnd())
    {
        String     syntaxName = patternIterator.getKey();
        Entry::Ptr entry      = patternIterator.getValue();

        SyntaxPatterns::Ptr oldPatterns = entry->getSyntaxPatterns();
        SyntaxPatterns::Ptr newPatterns = loadSyntaxPatterns(syntaxName, newTextStyleDefinitions);

        if (!oldPatterns->hasSamePatternStructureThan(newPatterns))
        {
            entry->refreshWithNewSyntaxPatterns(newPatterns);
        }
        else {
            oldPatterns->updateTextStyles(newTextStyleDefinitions);
        }
        patternIterator.gotoNext();
    }
    textStyleDefinitions = newTextStyleDefinitions;
}
