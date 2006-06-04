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

#ifndef LANGUAGEMODES_H
#define LANGUAGEMODES_H

#include <string>

#include "HeapObject.h"
#include "ObjectArray.h"
#include "Regex.h"
#include "MemArray.h"
#include "LuaObject.h"
#include "HashMap.h"
#include "SyntaxPatterns.h"
#include "OwningPtr.h"

namespace LucED {

using std::string;

class LanguageMode : public HeapObject
{
public:
    typedef OwningPtr<LanguageMode> Ptr;

    static Ptr create(const string& name, Regex regex = Regex()) {
        return Ptr(new LanguageMode(name, regex));
    }
    static Ptr create(LuaObject config) {
        return Ptr(new LanguageMode(config));
    }
        
    string getName() const {
        return name;
    }
    Regex getRegex() const {
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
    
private:
    friend class LanguageModes;

    LanguageMode(const string& name, Regex regex = Regex());
    LanguageMode(LuaObject config);
    
    string name;
    Regex regex;
    bool approximateUnknownHilitingFlag;
    long approximateUnknownHilitingReparseRange;
    int hilitingBreakPointDistance;
};

class LanguageModes : public HeapObject
{
public:
    typedef OwningPtr<LanguageModes> Ptr;
    static Ptr create() {
        return Ptr(new LanguageModes());
    }
    
    void append(const string& name);
    void append(LuaObject config);
    
    LanguageMode::Ptr getModeForFile(const string& fileName);
    
private:
    LanguageModes();

    LanguageMode::Ptr defaultLanguageMode;
    ObjectArray<LanguageMode::Ptr> modes;
    MemArray<int> ovector;
    HashMap<string,int> nameToIndexMap;
};

} // namespace LucED

#endif // LANGUAGEMODES_H
