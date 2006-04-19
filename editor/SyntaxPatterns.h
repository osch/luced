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

#ifndef SYNTAXPATTERNS_H
#define SYNTAXPATTERNS_H

#include "TextStyle.h"
#include "HeapObject.h"
#include "LuaObject.h"
#include "HeapHashMap.h"
#include "Regex.h"
#include "MemArray.h"

namespace LucED {


struct SubPatternDescriptor
{
    SubPatternDescriptor(string subPatternName, int style) : subPatternName(subPatternName), style(style)
    {}
    string subPatternName;
    int style;
};

struct CombinedSubPatternStyle
{
    CombinedSubPatternStyle(int substrNo, int style) : substrNo(substrNo), style(style)
    {}
    int substrNo;
    int style;
};

struct ChildPatternDescriptor
{
    ChildPatternDescriptor(int id) : id(id), substrNo(-1)
    {}
    int id;
    int substrNo;
};

struct SyntaxPattern
{
    SyntaxPattern() : style(0), hasEndPattern(false), maxBeginBytesExtend(0), maxEndBytesExtend(0),
            endSubstrNo(0), maxREBytesExtend(0)
    {}

    int getMatchedChild(const MemArray<int>& ovector);

    string name;
    int style;
    string beginPattern;
    string endPattern;
    bool hasEndPattern;
    int  maxBeginBytesExtend;
    int  maxEndBytesExtend;
    ObjectArray<ChildPatternDescriptor> childList;
    ObjectArray<SubPatternDescriptor>   beginSubStyles;
    ObjectArray<SubPatternDescriptor>   endSubStyles;

    int  endSubstrNo;
    int  maxREBytesExtend;
    
    Regex re;
    ObjectArray<CombinedSubPatternStyle> combinedSubs;
};


class SyntaxPatterns : public HeapObject
{
public:
    typedef HeapObjectPtr<SyntaxPatterns> Ptr;
    typedef HeapHashMap<string,int> NameToIndexMap;
    
    static SyntaxPatterns::Ptr create(LuaObject config, NameToIndexMap::ConstPtr textStyleToIndexMap) {
        return Ptr(new SyntaxPatterns(config, textStyleToIndexMap));
    }

    static TextStyles::Ptr getPreliminaryStaticTextStyles();
    
    SyntaxPattern* get(int i) {
        return allPatterns.getPtr(i);
    }
    SyntaxPattern *getChildPattern(SyntaxPattern *parent, int childId)
    {
        ChildPatternDescriptor *cdescr = parent->childList.getPtr(childId);
        return get(cdescr->id);
    }
    int getChildPatternId(SyntaxPattern *parent, int childId)
    {
        ChildPatternDescriptor *cdescr = parent->childList.getPtr(childId);
        return cdescr->id;
    }
    int getMaxOvecSize() const {
        return maxOvecSize;
    }
    int getTotalMaxExtend() const {
        return totalMaxREBytesExtend;
    }
private:
    
    SyntaxPatterns(LuaObject config, NameToIndexMap::ConstPtr textStyleToIndexMap);

    void compile(int i);
    void compileAll();
    
    ObjectArray<SyntaxPattern> allPatterns;
    
    int maxOvecSize;
    int totalMaxREBytesExtend;
    
    static TextStyles::Ptr preliminaryStaticTextStyles;
};




} // namespace LucED

#endif // SYNTAXPATTERNS_H
