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

#ifndef FINDUTIL_H
#define FINDUTIL_H

#include <string>
#include <pcre.h>

#include "TextData.h"
#include "WeakPtr.h"
#include "MemArray.h"
#include "ObjectArray.h"
#include "NonCopyable.h"
#include "LuaStoredObject.h"
#include "LuaException.h"

namespace LucED
{

using std::string;

class FindUtil : public NonCopyable
{
public:

    FindUtil()
        : searchForwardFlag(true),
          ignoreCaseFlag(false),
          regexFlag(false),
          wholeWordFlag(false),
          wasFoundFlag(false),
          textPosition(0),
          textData(NULL),
          wasError(false),
          luaException("")
    {}

    void setSearchForwardFlag(bool flag) {
        searchForwardFlag = flag;
    }
    bool isSearchingForward() const {
        return searchForwardFlag;
    }
    void setIgnoreCaseFlag(bool flag) {
        ignoreCaseFlag = flag;
    }
    void setRegexFlag(bool flag) {
        regexFlag = flag;
    }
    void setWholeWordFlag(bool flag) {
        wholeWordFlag = flag;
    }
    void setSearchString(const string& searchString) {
        this->searchString = searchString;
    }
    void setTextPosition(long pos) {
        textPosition = pos;
    }
    void setTextData(TextData* textData) {
        this->textData = textData;
    }
    
    void findNext();
    
    bool wasFound() const {
        return wasFoundFlag;
    }
    long getTextPosition() const {
        return textPosition;
    } 
    long getMatchBeginPos() const {
        ASSERT(wasFoundFlag);
        return ovector[0];
    }
    long getMatchEndPos() const {
        ASSERT(wasFoundFlag);
        return ovector[1];
    }
    
private:
    static int pcreCalloutFunction(void* self, pcre_callout_block*);
    
    bool searchForwardFlag;
    bool ignoreCaseFlag;
    bool regexFlag;
    bool wholeWordFlag;  
    bool wasFoundFlag;
    long textPosition;
    WeakPtr<TextData> textData;
    
    string searchString;
    MemArray<int> ovector;
    
    struct CalloutObject
    {
        CalloutObject(LuaStoredObject::Ptr luaObject) : luaObject(luaObject)
        {}
        LuaStoredObject::Ptr luaObject;
        MemArray<int>        args;
    };
    ObjectArray<CalloutObject> calloutObjects;
    bool wasError;
    LuaException luaException;
};

} // namespace LucED

#endif // FINDUTIL_H

