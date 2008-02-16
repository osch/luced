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

#ifndef SYNTAX_PATTERS_CONFIG_HPP
#define SYNTAX_PATTERS_CONFIG_HPP

#include "HeapObject.hpp"
#include "SyntaxPatterns.hpp"
#include "ObjectArray.hpp"
#include "HashMap.hpp"
#include "String.hpp"

namespace LucED
{

class SyntaxPatternsConfig : public HeapObject
{
public:
    typedef OwningPtr<SyntaxPatternsConfig> Ptr;

    static SyntaxPatternsConfig::Ptr create() {
        return Ptr(new SyntaxPatternsConfig());
    }

    void clear() {
        this->languageModeToSyntaxIndex   = NameToIndexMap::create();
        this->configFileNameToSyntaxIndex = NameToIndexMap::create();
        this->allSyntaxPatterns.clear();
        this->syntaxPatternCallbackContainers.clear();
    }

    SyntaxPatterns::Ptr getSyntaxPatternsForLanguageMode(const String& languageModeName,
                                                         Callback<SyntaxPatterns::Ptr>::Ptr changeCallback)
    {
        NameToIndexMap::Value foundIndex = languageModeToSyntaxIndex->get(languageModeName);
        if (foundIndex.isValid())
        {
            if (changeCallback.isValid()) {
                syntaxPatternCallbackContainers[foundIndex.get()].registerCallback(changeCallback);
            }
            return allSyntaxPatterns.get(foundIndex.get());
        } else {
            return SyntaxPatterns::Ptr();
        }
    }
    
    void append(const String& languageModeName, const String& configFileName, SyntaxPatterns::Ptr syntaxPatterns)
    {
        allSyntaxPatterns.append(syntaxPatterns);
        syntaxPatternCallbackContainers.appendNew();     ASSERT(allSyntaxPatterns.getLength() == syntaxPatternCallbackContainers.getLength());
        
        int i = allSyntaxPatterns.getLength() - 1;
        
        languageModeToSyntaxIndex  ->set(languageModeName, i);
        configFileNameToSyntaxIndex->set(configFileName, i);
        
    }
    
    bool hasEntryForConfigFileName(const String& configFileName) const
    {
        return configFileNameToSyntaxIndex->hasKey(configFileName);
    }
    
    bool hasEntryForLanguageModeName(const String& languageModeName) const
    {
        return languageModeToSyntaxIndex->hasKey(languageModeName);
    }
    
    void updateForConfigFileName(const String& configFileName, SyntaxPatterns::Ptr newSyntaxPatterns)
    {
        NameToIndexMap::Value foundIndex = configFileNameToSyntaxIndex->get(configFileName);
        
        ASSERT(foundIndex.isValid());
        
        allSyntaxPatterns              [foundIndex.get()] = newSyntaxPatterns;
        syntaxPatternCallbackContainers[foundIndex.get()].invokeAllCallbacks(newSyntaxPatterns);
    }
    
    
private:
    typedef HeapHashMap<String,int> NameToIndexMap;

    SyntaxPatternsConfig() {
        clear();
    }

    ObjectArray<SyntaxPatterns::Ptr>                      allSyntaxPatterns;
    ObjectArray< CallbackContainer<SyntaxPatterns::Ptr> > syntaxPatternCallbackContainers;

    NameToIndexMap::Ptr languageModeToSyntaxIndex;
    NameToIndexMap::Ptr configFileNameToSyntaxIndex;

};

} // namespace LucED

#endif // SYNTAX_PATTERS_CONFIG_HPP
