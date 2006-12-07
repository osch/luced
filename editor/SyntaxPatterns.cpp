/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#include "SyntaxPatterns.h"
#include "ByteArray.h"
#include "ConfigException.h"
#include "HeapHashMap.h"
#include "util.h"

using namespace LucED;


//TextStyles::Ptr     SyntaxPatterns::preliminaryStaticTextStyles;
//
//#define TEXTSTYLE_NORMAL 0
//#define TEXTSTYLE_COMMENT 1
//#define TEXTSTYLE_PREPROC 2
//#define TEXTSTYLE_KEYWORD 3
//#define TEXTSTYLE_TYPE    4
//
//TextStyles::Ptr SyntaxPatterns::getPreliminaryStaticTextStyles()
//{
//    if (!preliminaryStaticTextStyles.isValid()) 
//    {
//        static const char *normal_font = "-*-courier-medium-r-*-*-*-120-75-75-*-*-*-*";
//        static const char *italic_font = "-*-courier-medium-o-*-*-*-120-75-75-*-*-*-*";
//        static const char *bold_font   = "-*-courier-bold-r-*-*-*-120-75-75-*-*-*-*";
//
//        preliminaryStaticTextStyles = TextStyles::create();
//
//        preliminaryStaticTextStyles->appendNewStyle(normal_font, "black");
//        preliminaryStaticTextStyles->appendNewStyle(italic_font, "grey20");
//        preliminaryStaticTextStyles->appendNewStyle(normal_font, "RoyalBlue4");
//        preliminaryStaticTextStyles->appendNewStyle(bold_font,   "black");
//        preliminaryStaticTextStyles->appendNewStyle(bold_font,   "brown");
//        preliminaryStaticTextStyles->appendNewStyle(normal_font, "darkGreen");
//        preliminaryStaticTextStyles->appendNewStyle(bold_font,   "darkGreen");
//        
//    }
//    return preliminaryStaticTextStyles;
//}
//
//

typedef HeapHashMap<string,int> NameToIndexMap;

static void fillChildPatterns(SyntaxPattern *sp, LuaObject actPattern, NameToIndexMap::Ptr nameToIndexMap)
{
    LuaObject o = actPattern["childPatterns"];
    if (o.isValid()) {
        if (!o.isTable()) {
            throw ConfigException("pattern '" + sp->name + "': invalid childPatterns");
        }
        for (int j = 1; o[j].isValid(); ++j) {
            LuaObject n = o[j];
            if (!n.isString()) {
                throw ConfigException("pattern '" + sp->name + "': invalid childPatterns");
            }
            NameToIndexMap::Value foundIndex = nameToIndexMap->get(n.toString());
            if (!foundIndex.isValid()) {
                throw ConfigException("pattern '" + sp->name + "': unknown childpattern '" + n.toString() + "'");
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
    LuaObject::KeyList::Ptr keys = config.getTableKeys();

    nameToIndexMap->set("root", 0);
    for (int i=0; i < keys->getLength();) {
        if (keys->get(i) != "root") {
            nameToIndexMap->set(keys->get(i), i + 1);
            ++i;
        } else {
            keys->remove(i);
        }
    }
    allPatterns.appendAmount(keys->getLength() + 1);
    
    {
        SyntaxPattern* sp = allPatterns.getPtr(0);
        sp->name = "root";
        LuaObject o = root["style"];
        if (!o.isString()) {
            throw ConfigException("pattern '" + sp->name + "': invalid style");
        }
        NameToIndexMap::Value foundIndex = textStyleToIndexMap->get(o.toString());
        if (!foundIndex.isValid()) {
            throw ConfigException("pattern '" + sp->name + "': invalid style '" + o.toString() + "'");
        }
        sp->style = foundIndex.get();
        fillChildPatterns(sp, root, nameToIndexMap);
    }
    
    for (int i = 0; i < keys->getLength(); ++i) {
        string name = keys->get(i);
        SyntaxPattern* sp = allPatterns.getPtr(i + 1);
        sp->name = name;
        LuaObject p = config[name];
        if (!p.isTable()) {
            throw ConfigException("pattern '" + name + "' not properly defined");
        }
        LuaObject o = p["style"];
        if (!o.isString()) {
            throw ConfigException("pattern '" + sp->name + "': invalid style");
        }
        NameToIndexMap::Value foundIndex = textStyleToIndexMap->get(o.toString());
        if (!foundIndex.isValid()) {
            throw ConfigException("pattern '" + sp->name + "': invalid style '" + o.toString() + "'");
        }
        sp->style = foundIndex.get();

        o = p["beginPattern"];
        if (o.isString()) {
            sp->beginPattern = o.toString();
            
            o = p["endPattern"];
            if (!o.isString()) {
                throw ConfigException("pattern '" + sp->name + "': missing 'endPattern' element");
            }
            sp->endPattern = o.toString();
            sp->hasEndPattern = true;
            
            o = p["maxBeginExtend"];
            if (!o.isNumber()) {
                throw ConfigException("pattern '" + sp->name + "': invalid 'maxBeginExtend' element");
            }
            sp->maxBeginBytesExtend = (int) o.toNumber();
            
            o = p["maxEndExtend"];
            if (!o.isNumber()) {
                throw ConfigException("pattern '" + sp->name + "': invalid 'maxEndExtend'");
            }
            sp->maxEndBytesExtend = (int) o.toNumber();

            fillChildPatterns(sp, p, nameToIndexMap);
            
            o = p["beginSubstyles"];
            if (o.isValid()) {
                if (!o.isTable()) {
                    throw ConfigException("pattern '" + sp->name + "': invalid 'beginSubstyles' element");
                }
                LuaObject::KeyList::Ptr keys = o.getTableKeys();
                for (int j = 0; j < keys->getLength(); ++j) {
                    string k = keys->get(j);
                    if (!o[k].isString()) {
                        throw ConfigException("pattern '" + sp->name + "': invalid 'beginSubstyles' element '" + k + "'");
                    }
                    NameToIndexMap::Value foundIndex = textStyleToIndexMap->get(o[k].toString());
                    if (!foundIndex.isValid()) {
                        throw ConfigException("pattern '" + sp->name + "': invalid 'beginSubstyles' element '" + k + "'");
                    }
                    sp->beginSubStyles.appendNew(k, foundIndex.get());
                }
            }

            o = p["endSubstyles"];
            if (o.isValid()) {
                if (!o.isTable()) {
                    throw ConfigException("pattern '" + sp->name + "': invalid 'endSubstyles' element");
                }
                LuaObject::KeyList::Ptr keys = o.getTableKeys();
                for (int j = 0; j < keys->getLength(); ++j) {
                    string k = keys->get(j);
                    if (!o[k].isString()) {
                        throw ConfigException("pattern '" + sp->name + "': invalid 'endSubstyles' element '" + k + "'");
                    }
                    NameToIndexMap::Value foundIndex = textStyleToIndexMap->get(o[k].toString());
                    if (!foundIndex.isValid()) {
                        throw ConfigException("pattern '" + sp->name + "': invalid 'endSubstyles' element '" + k + "'");
                    }
                    sp->endSubStyles.appendNew(k, foundIndex.get());
                }
            }
        } 
        else {
            o = p["pattern"];
            if (!o.isString()) {
                throw ConfigException("pattern '" + sp->name + "' must have 'beginPattern' or 'pattern' element");
            }
            sp->beginPattern = o.toString();
            sp->hasEndPattern = false;
            
            o = p["maxExtend"];
            if (!o.isNumber()) {
                throw ConfigException("pattern '" + sp->name + "': invalid 'maxExtend' element");
            }
            sp->maxBeginBytesExtend = (int) o.toNumber();

            o = p["substyles"];
            if (o.isValid()) {
                if (!o.isTable()) {
                    throw ConfigException("pattern '" + sp->name + "': invalid 'substyles' element");
                }
                LuaObject::KeyList::Ptr keys = o.getTableKeys();
                for (int j = 0; j < keys->getLength(); ++j) {
                    string k = keys->get(j);
                    if (!o[k].isString()) {
                        throw ConfigException("pattern '" + sp->name + "': invalid 'substyles' element '" + k + "'");
                    }
                    NameToIndexMap::Value foundIndex = textStyleToIndexMap->get(o[k].toString());
                    if (!foundIndex.isValid()) {
                        throw ConfigException("pattern '" + sp->name + "': invalid 'substyles' element '" + k + "'");
                    }
                    sp->beginSubStyles.appendNew(k, foundIndex.get());
                }
            }

        }
        
    }
    compileAll();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

static inline void maximize(int *ptr, int i) {
    if (*ptr < i) {
        *ptr = i;
    }
}

void SyntaxPatterns::compile(int i) {
    ByteArray patStr;
    bool first = true;
    const char *errortxt;
    int errorpos;
    SyntaxPattern *sp = get(i);
    
    sp->maxREBytesExtend = 0;
    
    for (int i = 0; i < sp->childList.getLength(); ++i) {
        ChildPatternDescriptor *cdescr = sp->childList.getPtr(i);
        SyntaxPattern *cpat = get(cdescr->id);
        if (!first) {
            patStr.appendCStr("|");
        }
        patStr.appendCStr("(")
                  .appendCStr("?P<").appendString(cpat->name).appendCStr(">")
                  .appendString(cpat->beginPattern)
              .appendCStr(")");

        maximize(&sp->maxREBytesExtend, cpat->maxBeginBytesExtend);

        first = false;
    }
    if (sp->hasEndPattern) {
        if (!first) {
            patStr.appendCStr("|");
        }
        patStr.appendCStr("(")
                  .appendCStr("?P<").appendString(sp->name).appendCStr("_END>")
                  .appendString(sp->endPattern)
              .appendCStr(")");
        
        maximize(&sp->maxREBytesExtend, sp->maxEndBytesExtend);
        
        first = false;
    }
    if (!first) {
        sp->re = Regex(patStr, Regex::MULTILINE | Regex::EXTENDED);
        util::maximize(&maxOvecSize, sp->re.getOvecSize());

        for (int ci = 0; ci < sp->childList.getLength(); ++ci) {
            ChildPatternDescriptor *cdescr = sp->childList.getPtr(ci);
            SyntaxPattern *cpat = get(cdescr->id);
            cdescr->substrNo = sp->re.getStringNumber(cpat->name);

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
            ByteArray temp;
            temp.appendString(sp->name).appendCStr("_END");
            sp->endSubstrNo = sp->re.getStringNumber(temp);
        }
    } else {
        sp->re = Regex();
    }

    maximize(&this->totalMaxREBytesExtend, sp->maxREBytesExtend);
}

void SyntaxPatterns::compileAll() {
    int i;
    for (i=0; i < allPatterns.getLength(); ++i) {
        compile(i);
    }
}

int SyntaxPattern::getMatchedChild(const MemArray<int>& ovector)
{
    int rslt = 0;
    for (int i = 0; i < this->childList.getLength(); ++i) {
        ChildPatternDescriptor *cdescr = this->childList.getPtr(i);
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
