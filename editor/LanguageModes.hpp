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

#ifndef LANGUAGEMODES_H
#define LANGUAGEMODES_H

#include "String.hpp"

#include "HeapObject.hpp"
#include "ObjectArray.hpp"
#include "BasicRegex.hpp"
#include "MemArray.hpp"
#include "HashMap.hpp"
#include "SyntaxPatterns.hpp"
#include "OwningPtr.hpp"

namespace LucED {



class LuaObject;

class LanguageMode : public HeapObject
{
public:
    typedef OwningPtr<LanguageMode> Ptr;

    static Ptr create(const String& name, BasicRegex regex = BasicRegex()) {
        return Ptr(new LanguageMode(name, regex));
    }
    static Ptr create(LuaObject config);
        
    String getName() const {
        return name;
    }
    BasicRegex getRegex() const {
        return regex;
    }
    bool hasApproximateUnknownHilitingFlag() const {
        return approximateUnknownHilitingFlag;
    }
    long getApproximateUnknownHilitingReparseRange() const {
        return approximateUnknownHilitingReparseRange;
    }
    int getHilitingBreakPointDistance() {
        return hilitingBreakPointDistance;
    }
    int getHardTabWidth() const {
        return hardTabWidth;
    }
    int getSoftTabWidth() const {
        return softTabWidth;
    }
    
private:
    friend class LanguageModes;

    LanguageMode(const String& name, BasicRegex regex = BasicRegex());
    LanguageMode(LuaObject config);
    
    String name;
    BasicRegex regex;
    bool approximateUnknownHilitingFlag;
    long approximateUnknownHilitingReparseRange;
    int hilitingBreakPointDistance;
    int hardTabWidth;
    int softTabWidth;
};

class LanguageModes : public HeapObject
{
public:
    typedef OwningPtr<LanguageModes> Ptr;
    static Ptr create() {
        return Ptr(new LanguageModes());
    }
    
    void append(const String& name);
    void append(LuaObject config);
    
    LanguageMode::Ptr getLanguageModeForFile(const String& fileName);
    LanguageMode::Ptr getDefaultLanguageMode();

    LanguageMode::Ptr getLanguageMode(const String& name) {
        HashMap<String,int>::Value foundValue = nameToIndexMap.get(name);
        if (foundValue.isValid()) {
            return modes[foundValue.get()];
        } else {
            return LanguageMode::Ptr();
        }
    }
    
    bool hasLanguageMode(const String& name) const {
        return nameToIndexMap.hasKey(name);
    }
        
    int getLength() const {
        return modes.getLength();
    }
    
    LanguageMode::Ptr get(int i) const {
        return modes[i];
    }
    
private:
    LanguageModes();

    LanguageMode::Ptr defaultLanguageMode;
    ObjectArray<LanguageMode::Ptr> modes;
    MemArray<int> ovector;
    HashMap<String,int> nameToIndexMap;
};

} // namespace LucED

#endif // LANGUAGEMODES_H
