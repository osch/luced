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
#include "TextStyleDefinition.hpp"
#include "TextStyle.hpp"
#include "TextStyleCache.hpp"
#include "ObjectArray.hpp"
#include "Nullable.hpp"
#include "CallbackContainer.hpp"
#include "Callback.hpp"
          
namespace LucED
{

class LuaVar;

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
    
    static Ptr create(LuaVar config, 
                      HeapHashMap<String,TextStyleDefinition>::ConstPtr textStyleDefinitions);

    static Ptr createWithoutPatterns(HeapHashMap<String,TextStyleDefinition>::ConstPtr textStyleDefinitions);

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
    bool hasSamePatternStructureThan (RawPtr<const SyntaxPatterns> rhs) const;

    const ObjectArray<TextStyle::Ptr>& getTextStylesArray() const {
        return textStyles;
    }
    TextStyle::Ptr getDefaultTextStyle() const {
        return defaultTextStyle;
    }
    
    bool hasPatterns() const {
        return allPatterns.getLength() > 0;
    }

    void updateTextStyles(HeapHashMap<String,TextStyleDefinition>::ConstPtr newTextStyleDefinitions);
    
    void registerTextStylesChangedCallback(Callback<>::Ptr callback) {
        textStylesChangedCallback.registerCallback(callback);
    }
    
private:
    SyntaxPatterns(Nullable<LuaVar> config, 
                   HeapHashMap<String,TextStyleDefinition>::ConstPtr textStyleDefinitions);

    void compile(int i);
    void compileAll();
    
    Nullable<int> getTextStyleIndex(const String& textStyleName);
    
    ObjectArray<SyntaxPattern> allPatterns;
    
    int maxOvecSize;
    int totalMaxREBytesExtend;
    
    bool hasSerializedString;
    String serializedString;

    HeapHashMap<String,TextStyleDefinition>::ConstPtr textStyleDefinitions;
    TextStyle::Ptr              defaultTextStyle;
    ObjectArray<TextStyle::Ptr> textStyles;
    HashMap<String,int>         textStyleToIndexMap;
    
    CallbackContainer<> textStylesChangedCallback;
};




} // namespace LucED

#endif // SYNTAX_PATTERNS_HPP
