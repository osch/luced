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

#ifndef SYNTAX_PATTERNS_HPP
#define SYNTAX_PATTERNS_HPP

#include "TextStyle.hpp"
#include "HeapObject.hpp"
#include "HeapHashMap.hpp"
#include "BasicRegex.hpp"
#include "MemArray.hpp"
#include "OwningPtr.hpp"

namespace LucED {


class LuaObject;

struct SubPatternDescriptor
{
    SubPatternDescriptor(String subPatternName, int style) : subPatternName(subPatternName), style(style)
    {}
    String subPatternName;
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
    int pushedSubstrNo;
    bool hasPushedSubstr;
};

struct SyntaxPattern
{
    SyntaxPattern() : style(0), hasEndPattern(false), maxBeginBytesExtend(0), maxEndBytesExtend(0),
            endSubstrNo(0), maxREBytesExtend(0)
    {}

    int getMatchedChild(const MemArray<int>& ovector);

    String name;
    int style;
    String beginPattern;
    String endPattern;
    bool hasEndPattern;
    int  maxBeginBytesExtend;
    int  maxEndBytesExtend;
    ObjectArray<ChildPatternDescriptor> childList;
    ObjectArray<SubPatternDescriptor>   beginSubStyles;
    ObjectArray<SubPatternDescriptor>   endSubStyles;
    String pushedSubPatternName;

    int  endSubstrNo;
    int  maxREBytesExtend;
    
    BasicRegex re;
    ObjectArray<CombinedSubPatternStyle> combinedSubs;
    bool hasPushedSubstr;
};


class SyntaxPatterns : public HeapObject
{
public:
    typedef OwningPtr<SyntaxPatterns> Ptr;
    typedef HeapHashMap<String,int> NameToIndexMap;
    
    static Ptr create(LuaObject config, NameToIndexMap::ConstPtr textStyleToIndexMap);

    SyntaxPattern* get(int i) {
        return allPatterns.getPtr(i);
    }
    SyntaxPattern* getChildPattern(SyntaxPattern* parent, int childId)
    {
        ChildPatternDescriptor* cdescr = parent->childList.getPtr(childId);
        return get(cdescr->id);
    }
    bool hasPushedSubstr(SyntaxPattern* parent, int childId)
    {
        ChildPatternDescriptor* cdescr = parent->childList.getPtr(childId);
        return cdescr->hasPushedSubstr;
    }
    int getPushedSubstrNo(SyntaxPattern* parent, int childId)
    {
        ChildPatternDescriptor* cdescr = parent->childList.getPtr(childId);
        return cdescr->pushedSubstrNo;
    }
    int getChildPatternId(SyntaxPattern* parent, int childId)
    {
        ChildPatternDescriptor* cdescr = parent->childList.getPtr(childId);
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

#endif // SYNTAX_PATTERNS_HPP
