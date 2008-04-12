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

#ifndef FINDUTIL_HPP
#define FINDUTIL_HPP

#include "String.hpp"
#include <pcre.h>

#include "TextData.hpp"
#include "WeakPtr.hpp"
#include "MemArray.hpp"
#include "ObjectArray.hpp"
#include "NonCopyable.hpp"
#include "LuaStoredObject.hpp"
#include "LuaException.hpp"
#include "BasicRegex.hpp"
#include "Flags.hpp"
#include "RawPtr.hpp"

namespace LucED
{



class FindUtil : public NonCopyable
{
public:

    enum Option
    {
        REGEX,
        WHOLE_WORD,
        IGNORE_CASE,
        BACKWARD,
        NOT_AT_START
    };
    
    typedef Flags<Option> Options;


    FindUtil(RawPtr<TextData> textData);
    
    void setOptions(Options options) {
        setRegexFlag                     ( options.contains(REGEX));
        setWholeWordFlag                 ( options.contains(WHOLE_WORD));
        setIgnoreCaseFlag                ( options.contains(IGNORE_CASE));
        setSearchForwardFlag             (!options.contains(BACKWARD));
        setAllowMatchAtStartOfSearchFlag (!options.contains(NOT_AT_START));
    }

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
    void setMaximalEndOfMatchPosition(long epos) {
        maximalEndOfMatchPosition = epos;
    }
    void setTextData(RawPtr<TextData> textData) {
        this->textData = textData;
    }
    void setAllowMatchAtStartOfSearchFlag(bool flag) {
        allowMatchAtStartOfSearchFlag = flag;
    }

    bool doesMatch();
    
    bool doesMatch(const String& searchString, long textPosition, Options options) {
        setOptions(options);
        setTextPosition(textPosition);
        setSearchString(searchString);
        return doesMatch();
    }
        
    void findNext();
    
    long find(const String& searchString, long textPosition, Options options) {
        setOptions(options);
        setTextPosition(textPosition);
        setSearchString(searchString);
        findNext();
        if (wasFound()) {
            return getMatchBeginPos();
        } else {
            return -1;
        }
        return wasFound();
    }
    
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
    long getMatchLength() const {
        ASSERT(wasFoundFlag);
        return ovector[1] - ovector[0];
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
    RawPtr<TextData> getTextData() {
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
    long maximalEndOfMatchPosition;
    RawPtr<TextData> textData;
    
    String searchString;
    MemArray<int> ovector;
    
    ObjectArray<CalloutObject::Ptr> calloutObjects;
    bool wasError;
    LuaException luaException;
    
    bool wasInitializedFlag;

    MemArray<int> expressionPositions;
    BasicRegex regex;

    long maxRegexAssertionLength;
    
    bool allowMatchAtStartOfSearchFlag;
};

} // namespace LucED

#endif // FINDUTIL_HPP

