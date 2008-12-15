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

#ifndef FINDUTIL_HPP
#define FINDUTIL_HPP

#include "String.hpp"
#include <pcre.h>

#include "TextData.hpp"
#include "WeakPtr.hpp"
#include "MemArray.hpp"
#include "ObjectArray.hpp"
#include "NonCopyable.hpp"
#include "LuaStoredObjectReference.hpp"
#include "LuaException.hpp"
#include "BasicRegex.hpp"
#include "Flags.hpp"
#include "RawPtr.hpp"
#include "SearchParameter.hpp"
#include "SearchParameterTypes.hpp"
#include "LuaVar.hpp"

namespace LucED
{

class FindUtil : public NonCopyable,
                 public SearchParameterTypes
{
public:
    FindUtil(RawPtr<TextData> textData);
    
    void setParameter(const SearchParameter& p) {
        wasInitializedFlag = false;
        this->p = p;
    }
    
    void setOptions(Options options) {
        wasInitializedFlag = false;
        p.setOptions(options);
    }

    void setSearchForwardFlag(bool flag) {
        wasInitializedFlag = false;
        p.setSearchForwardFlag(flag);
    }
    bool isSearchingForward() const {
        return p.hasSearchForwardFlag();
    }
    void setIgnoreCaseFlag(bool flag) {
        wasInitializedFlag = false;
        p.setIgnoreCaseFlag(flag);
    }
    void setRegexFlag(bool flag) {
        wasInitializedFlag = false;
        p.setRegexFlag(flag);
    }
    void setWholeWordFlag(bool flag) {
        wasInitializedFlag = false;
        p.setWholeWordFlag(flag);
    }
    void setFindString(const String& findString) {
        wasInitializedFlag = false;
        p.setFindString(findString);
    }
    void setTextPosition(long pos) {
        textPosition = pos;
    }
    void setMaximalEndOfMatchPosition(long epos) {
        maximalEndOfMatchPosition = epos;
    }
    void setNoMatchBeforePosition(long pos) {
        noMatchBeforePosition = pos;
    }
    void setMaxForwardAssertionLength(long maxForwardAssertionLength) {
        this->maxForwardAssertionLength = maxForwardAssertionLength;
    }
    void setMaxBackwardAssertionLength(long maxBackwardAssertionLength) {
        this->maxBackwardAssertionLength = maxBackwardAssertionLength;
    }
    void setTextData(RawPtr<TextData> textData) {
        this->textData = textData;
    }
    void setAllowMatchAtStartOfSearchFlag(bool flag) {
        p.setAllowMatchAtStartOfSearchFlag(flag);
    }

    bool doesMatch();
    
    bool doesMatch(const String& findString, long textPosition, Options options) {
        setOptions(options);
        setTextPosition(textPosition);
        setFindString(findString);
        return doesMatch();
    }
        
    void findNext();
    
    long find(const String& findString, long textPosition, Options options) {
        setOptions(options);
        setTextPosition(textPosition);
        setFindString(findString);
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
    String getFindString() const {
        return p.getFindString();
    }
    bool getWholeWordFlag() const {
        return p.hasWholeWordFlag();
    }
    bool getRegexFlag() const {
        return p.hasRegexFlag();
    }
    bool getIgnoreCaseFlag() const {
        return p.hasIgnoreCaseFlag();
    }
    bool getSearchForwardFlag() const {
        return p.hasSearchForwardFlag();
    }

    static String quoteRegexCharacters(const String& s);
    
    int getNumberOfCapturingSubpatterns() const {
        return regex.getNumberOfCapturingSubpatterns();
    }
    
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
        
        LuaStoredObjectReference getStoredLuaObjectReference() const {
            return storeReference;
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
        
        LuaStoredObjectReference storeReference;
        ObjectArray<String>  arguments;
        MemArray<int>        positions;
        int                  startPosition;
        int                  lastPosition;
    };
    
    class CalloutObject : public HeapObject
    {
    public:
        typedef OwningPtr<CalloutObject> Ptr;

        LuaStoredObjectReference getCallableObjectReference() const {
            return storeReference;
        }
        
        int getNumberOfArguments() const {
            return args.getLength();
        }
        
        int getCaptureIndexForArgument(int i) const {
            return args[i];
        }
        
    private:
        friend class FindUtil;
        
        static Ptr create(const LuaStoredObjectReference& storeReference) {
            return Ptr(new CalloutObject(storeReference));
        }
        
        CalloutObject(const LuaStoredObjectReference& storeReference) : storeReference(storeReference)
        {}
        LuaStoredObjectReference storeReference;
        MemArray<int>            args;
    };
    
    void initialize();

    bool wasInitialized() const {
        return wasInitializedFlag;
    }
    
    CalloutObject::Ptr buildCalloutObject(PreparsedCallout::Ptr preparsed);
    
private:
    static int pcreCalloutFunction(void* self, pcre_callout_block*);

    LuaVar evaluateCallout(CalloutObject::Ptr callout, const char* subject, 
                           long startMatch, long pos, int lastCaptured, int topCaptured);

protected:
    SearchParameter p;

private:        
    bool wasFoundFlag;
    long textPosition;
    long maximalEndOfMatchPosition;
    RawPtr<TextData> textData;
    
    MemArray<int> ovector;
    
    ObjectArray<CalloutObject::Ptr> calloutObjects;
    bool wasError;
    LuaException luaException;
    
    bool wasInitializedFlag;

    MemArray<int> expressionPositions;
    BasicRegex regex;

    long maxForwardAssertionLength;
    long maxBackwardAssertionLength;
    
    long noMatchBeforePosition;
};

} // namespace LucED

#endif // FINDUTIL_HPP

