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

#include "SyntaxPatterns.hpp"
#include "ByteArray.hpp"
#include "ConfigException.hpp"
#include "HeapHashMap.hpp"
#include "util.hpp"
#include "LuaObject.hpp"
#include "LuaIterator.hpp"

using namespace LucED;


SyntaxPatterns::Ptr SyntaxPatterns::create(LuaObject config, NameToIndexMap::ConstPtr textStyleToIndexMap)
{
    return Ptr(new SyntaxPatterns(config, textStyleToIndexMap));
}


typedef HeapHashMap<String,int> NameToIndexMap;

static void fillChildPatterns(SyntaxPattern *sp, LuaObject actPattern, NameToIndexMap::Ptr nameToIndexMap)
{
    LuaObject o = actPattern["childPatterns"];
    if (o.isValid()) {
        if (!o.isTable()) {
            throw ConfigException(String() << "pattern '" << sp->name << "': invalid childPatterns");
        }
        for (int j = 1; o[j].isValid(); ++j) {
            LuaObject n = o[j];
            if (!n.isString()) {
                throw ConfigException(String() << "pattern '" << sp->name << "': invalid childPatterns");
            }
            NameToIndexMap::Value foundIndex = nameToIndexMap->get(n.toString());
            if (!foundIndex.isValid()) {
                throw ConfigException(String() << "pattern '" << sp->name << "': unknown childpattern '" << n.toString() << "'");
            }
            sp->childList.appendNew(foundIndex.get());
        }
    }
}


SyntaxPatterns::SyntaxPatterns(LuaObject config, NameToIndexMap::ConstPtr textStyleToIndexMap)
    : maxOvecSize(0),
      totalMaxREBytesExtend(0)
{
    ASSERT(config.isTable());
    
    LuaObject root = config["root"];
    if (!root.isTable()) {
        throw ConfigException("pattern 'root' not properly defined");
    }
    NameToIndexMap::Ptr nameToIndexMap = NameToIndexMap::create();
    
    ObjectArray<String> patternNames;

    nameToIndexMap->set("root", 0);
    for (LuaIterator i = 0; i.in(config);) {
        String patternName = i.key().toString();
        if (patternName != "root") {
            nameToIndexMap->set(patternName, i + 1);  // i+1, because 0 is index for "root"
            patternNames.append(patternName);
            ++i;
        }
    }
    allPatterns.appendAmount(patternNames.getLength() + 1);
    
    {
        SyntaxPattern* sp = allPatterns.getPtr(0);
        sp->name = "root";
        LuaObject o = root["style"];
        if (!o.isString()) {
            throw ConfigException(String() << "pattern '" << sp->name << "': invalid style");
        }
        NameToIndexMap::Value foundIndex = textStyleToIndexMap->get(o.toString());
        if (!foundIndex.isValid()) {
            throw ConfigException(String() << "pattern '" << sp->name << "': invalid style '" << o.toString() << "'");
        }
        sp->style = foundIndex.get();
        fillChildPatterns(sp, root, nameToIndexMap);
    }
    
    for (int i = 0; i < patternNames.getLength(); ++i) {
        String name = patternNames[i];
        SyntaxPattern* sp = allPatterns.getPtr(i + 1);
        sp->name = name;
        LuaObject p = config[name];
        if (!p.isTable()) {
            throw ConfigException(String() << "pattern '" << name << "' not properly defined");
        }
        LuaObject o = p["style"];
        if (!o.isString()) {
            throw ConfigException(String() << "pattern '" << sp->name << "': invalid style");
        }
        NameToIndexMap::Value foundIndex = textStyleToIndexMap->get(o.toString());
        if (!foundIndex.isValid()) {
            throw ConfigException(String() << "pattern '" << sp->name << "': invalid style '" << o.toString() << "'");
        }
        sp->style = foundIndex.get();

        o = p["beginPattern"];
        if (o.isString())
        {
            sp->beginPattern = o.toString().getTrimmedSubstring();
            
            if (sp->beginPattern.getLength() == 0) {
                throw ConfigException(String() << "pattern '" << sp->name << "' has zero pattern length");
            }
 
            o = p["endPattern"];
            if (!o.isString()) {
                throw ConfigException(String() << "pattern '" << sp->name << "': missing 'endPattern' element");
            }
            sp->endPattern = o.toString();
            sp->hasEndPattern = true;
            
            o = p["maxBeginExtend"];
            if (!o.isNumber()) {
                throw ConfigException(String() << "pattern '" << sp->name << "': invalid 'maxBeginExtend' element");
            }
            sp->maxBeginBytesExtend = (int) o.toNumber();
            
            o = p["maxEndExtend"];
            if (!o.isNumber()) {
                throw ConfigException(String() << "pattern '" << sp->name << "': invalid 'maxEndExtend'");
            }
            sp->maxEndBytesExtend = (int) o.toNumber();

            fillChildPatterns(sp, p, nameToIndexMap);
            
            o = p["beginSubstyles"];
            if (o.isValid()) {
                if (!o.isTable()) {
                    throw ConfigException(String() << "pattern '" << sp->name << "': invalid 'beginSubstyles' element");
                }
                for (LuaIterator j; j.in(o);) {
                    String k = j.key().toString();
                    if (!j.value().isString()) {
                        throw ConfigException(String() << "pattern '" << sp->name << "': invalid 'beginSubstyles' element '" << k << "'");
                    }
                    NameToIndexMap::Value foundIndex = textStyleToIndexMap->get(j.value().toString());
                    if (!foundIndex.isValid()) {
                        throw ConfigException(String() << "pattern '" << sp->name << "': invalid 'beginSubstyles' element '" << k << "'");
                    }
                    sp->beginSubStyles.appendNew(k, foundIndex.get());
                }
            }

            o = p["endSubstyles"];
            if (o.isValid()) {
                if (!o.isTable()) {
                    throw ConfigException(String() << "pattern '" << sp->name << "': invalid 'endSubstyles' element");
                }
                for (LuaIterator j; j.in(o);) {
                    String k = j.key().toString();
                    if (!j.value().isString()) {
                        throw ConfigException(String() << "pattern '" << sp->name << "': invalid 'endSubstyles' element '" << k << "'");
                    }
                    NameToIndexMap::Value foundIndex = textStyleToIndexMap->get(j.value().toString());
                    if (!foundIndex.isValid()) {
                        throw ConfigException(String() << "pattern '" << sp->name << "': invalid 'endSubstyles' element '" << k << "'");
                    }
                    sp->endSubStyles.appendNew(k, foundIndex.get());
                }
            }
            
            o = p["pushSubpattern"];
            if (o.isValid()) {
                if (!o.isString()) {
                    throw ConfigException(String() << "pattern '" << sp->name << "': invalid 'pushSubpattern' element");
                }
                sp->pushedSubPatternName = o.toString();
            }
        }
        else {
            o = p["pattern"];
            if (!o.isString()) {
                throw ConfigException(String() << "pattern '" << sp->name << "' must have 'beginPattern' or 'pattern' element");
            }
            sp->beginPattern = o.toString().getTrimmedSubstring();
            if (sp->beginPattern.getLength() == 0) {
                throw ConfigException(String() << "pattern '" << sp->name << "' has zero pattern length");
            }
            sp->hasEndPattern = false;
            
            o = p["maxExtend"];
            if (!o.isNumber()) {
                throw ConfigException(String() << "pattern '" << sp->name << "': invalid 'maxExtend' element");
            }
            sp->maxBeginBytesExtend = (int) o.toNumber();

            o = p["substyles"];
            if (o.isValid()) {
                if (!o.isTable()) {
                    throw ConfigException(String() << "pattern '" << sp->name << "': invalid 'substyles' element");
                }
                for (LuaIterator j; j.in(o);) {
                    String k = j.key().toString();
                    if (!j.value().isString()) {
                        throw ConfigException(String() << "pattern '" << sp->name << "': invalid 'substyles' element '" << k << "'");
                    }
                    NameToIndexMap::Value foundIndex = textStyleToIndexMap->get(j.value().toString());
                    if (!foundIndex.isValid()) {
                        throw ConfigException(String() << "pattern '" << sp->name << "': invalid 'substyles' element '" << k << "'");
                    }
                    sp->beginSubStyles.appendNew(k, foundIndex.get());
                }
            }

        }
        
    }
    compileAll();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

static inline void maximize(int* ptr, int i)
{
    if (*ptr < i) {
        *ptr = i;
    }
}

void SyntaxPatterns::compile(int i)
{
    String patStr;
    bool first = true;
    int errorpos;
    SyntaxPattern* sp = get(i);
    
    sp->maxREBytesExtend = 0;
    
    for (int i = 0; i < sp->childList.getLength(); ++i) {
        ChildPatternDescriptor* cdescr = sp->childList.getPtr(i);
        SyntaxPattern* cpat = get(cdescr->id);
        if (!first) {
            patStr.append("|");
        }
        patStr.append("(")
                  .append("?P<").append(cpat->name).append(">")
                  .append(cpat->beginPattern)
              .append(")");

        maximize(&sp->maxREBytesExtend, cpat->maxBeginBytesExtend);

        first = false;
    }
    if (sp->hasEndPattern) {
        if (!first) {
            patStr.append("|");
        }
        patStr.append("(")
              .append("?P<").append(sp->name).append("_END>");

        if (sp->pushedSubPatternName.getLength() > 0)
        {
            sp->hasPushedSubstr = true;

            int a = 0;

            for (int i = 0, n = sp->endPattern.getLength(); i < n; ++i) {
                if (sp->endPattern[i] == '\\') {
                    ++i;
                } else if (sp->endPattern[i] == '[') {
                    while (i < n && sp->endPattern[i] != ']') {
                        if (sp->endPattern[i] == '\\') {
                            ++i;
                        }
                        ++i;
                    }
                } else {
                    if (sp->endPattern.equalsSubstringAt(i, "(*")) {
                        int j1 = i + 2;
                        while (j1 < n && (sp->endPattern[j1] == ' ' || sp->endPattern[j1] == '\t')) {
                            ++j1;
                        }
                        if (j1 < n)
                        {
                            int j = j1;
                            while (j < n && sp->endPattern[j] != ')' && sp->endPattern[j] != ' ' && sp->endPattern[j] != '\t') {
                                ++j;
                            }
                            int j2 = j;
                            while (j < n && sp->endPattern[j] != ')') {
                                ++j;
                            }
                            String s = sp->endPattern.getSubstringBetween(j1, j2);
                            if (s == sp->pushedSubPatternName) {
                                patStr.appendSubstring(sp->endPattern, a, i - a);
                                patStr.append("(?C1)");
                                i = j;
                                a = j + 1;
                            }
                            else
                            {
                                throw ConfigException(String() << "pattern '" << sp->name << "': invalid 'endPattern' element '(*" << s << ")'");
                            }
                        }
                    }
                }
            }
            patStr.appendSubstring(sp->endPattern, a, sp->endPattern.getLength() - a);
        }
        else
        {
            patStr.append(sp->endPattern);
        }
        patStr.append(")");
        
        maximize(&sp->maxREBytesExtend, sp->maxEndBytesExtend);
        
        first = false;
        
    }
    if (!first) {
        sp->re = BasicRegex(patStr, BasicRegex::CreateOptions() | BasicRegex::MULTILINE 
                                                                | BasicRegex::EXTENDED);
        util::maximize(&maxOvecSize, sp->re.getOvecSize());

        for (int ci = 0; ci < sp->childList.getLength(); ++ci)
        {
            ChildPatternDescriptor* cdescr = sp->childList.getPtr(ci);
            SyntaxPattern* cpat = get(cdescr->id);

            cdescr->substrNo = sp->re.getStringNumber(cpat->name);
            
            if (cpat->pushedSubPatternName.getLength() > 0)
            {
                cdescr->pushedSubstrNo = sp->re.getStringNumber(cpat->pushedSubPatternName);
                cdescr->hasPushedSubstr = true;
                cpat->hasPushedSubstr = true;
            }
            else
            {
                cdescr->pushedSubstrNo = -1;
                cdescr->hasPushedSubstr = false;
                cpat->hasPushedSubstr = false;
            }

            for (int i = 0; i < cpat->beginSubStyles.getLength(); ++i) {
                int substrNo = sp->re.getStringNumber(cpat->beginSubStyles[i].subPatternName);
                sp->combinedSubs.appendNew(substrNo, cpat->beginSubStyles[i].style);
            }
        }
        for (int i = 0; i < sp->endSubStyles.getLength(); ++i) {
            int substrNo = sp->re.getStringNumber(sp->endSubStyles[i].subPatternName);
            sp->combinedSubs.appendNew(substrNo, sp->endSubStyles[i].style);
        }
        
        if (sp->hasEndPattern) {
            sp->endSubstrNo = sp->re.getStringNumber(String() << sp->name << "_END");
        }
    } else {
        sp->re = BasicRegex();
    }

    maximize(&this->totalMaxREBytesExtend, sp->maxREBytesExtend);
}

void SyntaxPatterns::compileAll()
{
    int i;
    for (i=0; i < allPatterns.getLength(); ++i) {
        compile(i);
    }
}

int SyntaxPattern::getMatchedChild(const MemArray<int>& ovector)
{
    int rslt = 0;
    for (int i = 0; i < this->childList.getLength(); ++i) {
        ChildPatternDescriptor* cdescr = this->childList.getPtr(i);
        if (ovector[2 * cdescr->substrNo] != -1) {
            return rslt;
        }
        rslt += 1;
    }
    if (this->hasEndPattern) {
        if (ovector[2 * this->endSubstrNo] != -1) {
            return -1;
        }
    }
    return -2;
}
