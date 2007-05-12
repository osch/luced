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

#include "String.hpp"
#include <pcre.h>

#include "TextData.hpp"
#include "WeakPtr.hpp"
#include "MemArray.hpp"
#include "ObjectArray.hpp"
#include "NonCopyable.hpp"
#include "LuaStoredObject.hpp"
#include "LuaException.hpp"
#include "Regex.hpp"

namespace LucED
{



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
          luaException(""),
          wasInitializedFlag(false)
    {}

    void setSearchForwardFlag(bool flag) {
        wasInitializedFlag = false;
        searchForwardFlag = flag;
    }
    bool isSearchingForward() const {
        return searchForwardFlag;
    }
    void setIgnoreCaseFlag(bool flag) {
        wasInitializedFlag = false;
        ignoreCaseFlag = flag;
    }
    void setRegexFlag(bool flag) {
        wasInitializedFlag = false;
        regexFlag = flag;
    }
    void setWholeWordFlag(bool flag) {
        wasInitializedFlag = false;
        wholeWordFlag = flag;
    }
    void setSearchString(const String& searchString) {
        wasInitializedFlag = false;
        this->searchString = searchString;
    }
    void setTextPosition(long pos) {
        textPosition = pos;
    }
    void setTextData(TextData* textData) {
        this->textData = textData;
    }

    bool doesMatch();
        
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
    String getSearchString() const {
        return searchString;
    }
    bool getWholeWordFlag() const {
        return wholeWordFlag;
    }
    bool getRegexFlag() const {
        return regexFlag;
    }
    bool getIgnoreCaseFlag() const {
        return ignoreCaseFlag;
    }
    bool getSearchForwardFlag() const {
        return searchForwardFlag;
    }

    static String quoteRegexCharacters(const String& s);
    
    int getCapturedSubpatternBeginPos(int patternNumber) {
        if (patternNumber > regex.getNumberOfCapturingSubpatterns()) {
            return -1;
        } else {
            return ovector[patternNumber * 2 + 0];
        }
    }

    int getCapturedSubpatternLength(int patternNumber) {
        if (patternNumber > regex.getNumberOfCapturingSubpatterns()) {
            return -1;
        } else {
            return ovector[patternNumber * 2 + 1] - ovector[patternNumber * 2 + 0];
        }
    }
    
protected:
    TextData* getTextData() {
        return textData;
    }
    
    class PreparsedCallout : public HeapObject
    {
    public:
        typedef OwningPtr<PreparsedCallout> Ptr;
        
        static Ptr parse(const String& expression, int startPosition) {
            return Ptr(new PreparsedCallout(expression, startPosition));
        }
        
        LuaObject getLuaObject() const {
            return luaObject->retrieve();
        }
        
        LuaStoredObject::Ptr getStoredLuaObject() const {
            return luaObject;
        }
        
        int getNumberOfArguments() const {
            return arguments.getLength();
        }
        
        String getArgument(int i) const {
            return arguments[i];
        }
        
        int getArgumentPosition(int i) {
            return positions[i];
        }
        
        int getStartPosition() const {
            return startPosition;
        }
        
        int getLastPosition() const {
            return lastPosition;
        }
        
    private:
        PreparsedCallout(const String& expression, int startPosition);
        
        LuaStoredObject::Ptr luaObject;
        ObjectArray<String>  arguments;
        MemArray<int>        positions;
        int                  startPosition;
        int                  lastPosition;
    };
    
    class CalloutObject : public HeapObject
    {
    public:
        typedef OwningPtr<CalloutObject> Ptr;

        LuaObject getCallableObject() const {
            return luaObject->retrieve();
        }
        
        int getNumberOfArguments() const {
            return args.getLength();
        }
        
        int getCaptureIndexForArgument(int i) const {
            return args[i];
        }
        
    private:
        friend class FindUtil;
        
        static Ptr create(LuaStoredObject::Ptr luaObject) {
            return Ptr(new CalloutObject(luaObject));
        }
        
        CalloutObject(LuaStoredObject::Ptr luaObject) : luaObject(luaObject)
        {}
        LuaStoredObject::Ptr luaObject;
        MemArray<int>        args;
    };
    
    void initialize();

    bool wasInitialized() const {
        return wasInitializedFlag;
    }
    
    CalloutObject::Ptr buildCalloutObject(PreparsedCallout::Ptr preparsed);
    
private:
    static int pcreCalloutFunction(void* self, pcre_callout_block*);

    LuaObject evaluateCallout(CalloutObject::Ptr callout, const char* subject, 
                              long startMatch, long pos, int lastCaptured, int topCaptured);

    
    bool searchForwardFlag;
    bool ignoreCaseFlag;
    bool regexFlag;
    bool wholeWordFlag;  
    bool wasFoundFlag;
    long textPosition;
    WeakPtr<TextData> textData;
    
    String searchString;
    MemArray<int> ovector;
    
    ObjectArray<CalloutObject::Ptr> calloutObjects;
    bool wasError;
    LuaException luaException;
    
    bool wasInitializedFlag;

    MemArray<int> expressionPositions;
    Regex regex;
};

} // namespace LucED

#endif // FINDUTIL_H

