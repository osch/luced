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

#include "FindUtil.hpp"
#include "MemArray.hpp"
#include "GlobalLuaInterpreter.hpp"
#include "RegexException.hpp"
#include "GlobalConfig.hpp"

using namespace LucED;


FindUtil::FindUtil(RawPtr<TextData> textData)
    : wasFoundFlag(false),
      textPosition(0),
      maximalEndOfMatchPosition(-1),
      textData(textData),
      wasError(false),
      luaException(""),
      wasInitializedFlag(false),
      maxForwardAssertionLength(GlobalConfig::getInstance()->getMaxRegexAssertionLength()),
      maxBackwardAssertionLength(GlobalConfig::getInstance()->getMaxRegexAssertionLength()),
      noMatchBeforePosition(-1)
{}


LuaVar FindUtil::evaluateCallout(CalloutObject::Ptr callout, const char* subject, 
                                    long startMatch, long pos, int lastCaptured, int topCaptured)
{
    RawPtr<LuaInterpreter> luaInterpreter = GlobalLuaInterpreter::getInstance();
    LuaAccess luaAccess = luaInterpreter->getCurrentLuaAccess();
    
    LuaVar rslt(luaAccess);
    {
        LuaVar      o    = luaAccess.retrieve(callout->storeReference);
        MemArray<int>& args = callout->args;

        LuaFunctionArguments luaArgs(luaAccess);
        String firstArg;

        if (args.getLength() == 0) {
            if (lastCaptured != -1 && ovector[lastCaptured * 2 + 1] == pos) {
                firstArg = String(subject + ovector[lastCaptured * 2 + 0], ovector[lastCaptured * 2 + 1] - ovector[lastCaptured * 2 + 0]);
            } else {
                firstArg = String(subject + pos - 1, 1);
            }
            luaArgs << firstArg;
        } else {
            String k;
            for (int j = 0, m = args.getLength(); j < m; ++j)
            {
                if (args[j] == 0) {
                    k = String(subject + startMatch, pos - startMatch);
                } else if (args[j] <= topCaptured) {
                    k = String(subject + ovector[args[j] * 2 + 0], ovector[args[j] * 2 + 1] - ovector[args[j] * 2 + 0]);
                }
                luaArgs << k;
                if (j == 0) {
                    firstArg = k;
                }
            }
        }


        if (o.isTable())
        {
            rslt = o[firstArg];
        }
        else if (o.isFunction())
        {
            rslt = o.call(luaArgs);
        }
    }
    return rslt;
}

int FindUtil::pcreCalloutFunction(void* selfVoidPtr, pcre_callout_block* calloutBlock)
{
    FindUtil* self = static_cast<FindUtil*>(selfVoidPtr);

    self->wasError = false;
    try
    {
        if (calloutBlock->current_position > 0)
        {
            CalloutObject::Ptr callout = self->calloutObjects[calloutBlock->callout_number - 1];
            
            LuaVar rslt = self->evaluateCallout(callout, 
                                                   calloutBlock->subject, 
                                                   calloutBlock->start_match, 
                                                   calloutBlock->current_position,
                                                   calloutBlock->capture_last,
                                                   calloutBlock->capture_top);

            return rslt.isTrue() ? 0 : 1;
        }
        return 1;
    } catch (LuaException& ex) {
        self->wasError = true;
        self->luaException = ex;
        return -1;
    }
}



namespace // anonymous namespace
{
    class MyRegexException : public BaseException
    {
    public:
        MyRegexException(const String& message, int position)
            : BaseException(message),
              position(position)
        {}
        virtual const char* what() const throw() {
            return "FindUtil::MyRegexException";
        }
        int getPosition() const {
            return position;
        }
    private:
        int position;
    };
} // anonymous namespace


String FindUtil::quoteRegexCharacters(const String& s)
{
    String rslt;
    for (int i = 0, n = s.getLength(); i < n; ++i)
    {
        char c = s[i];
        switch (c) {
            case '\\': case '^': case '$': case '.': case '[': case ']':
            case '(':  case ')': case '?': case '*': case '+': case '{':
            case '}':  case '-': case '|': { rslt << '\\' << c; break; }
            
            case '\n':           { rslt << '\\' << 'n'; break; }
            
            default:             { rslt << c; }
        }
    }
    return rslt;
}

FindUtil::PreparsedCallout::PreparsedCallout(const String& findString, const int startPosParameter)
    : startPosition(startPosParameter)
{
    try
    {
        int pos = startPosition;

        ASSERT(findString[pos] == '(' && findString[pos+1] == '*');

        const int n = findString.getLength();

        int parenCounter = 1;
        int j;
        int lastParen = -1;
        for (j = pos+2; parenCounter > 0 && j < n; ++j) {
            if (findString[j] == '(') {
                if (parenCounter == 1) {
                    lastParen = j;
                } else {
                    lastParen = -1;
                }
                ++parenCounter;
            } else if (findString[j] == ')') {
                --parenCounter;
            }
        }
        RawPtr<LuaInterpreter> luaInterpreter = GlobalLuaInterpreter::getInstance();
        LuaAccess              luaAccess      = luaInterpreter->getCurrentLuaAccess();
        
        int varEnd;
        if (lastParen != -1) {
            varEnd = lastParen;
        } else {
            varEnd = j-1;
        }
        if (varEnd < pos+2) {
            throw RegexException(String() << "Missing ')' for Lua callout expression.", 
                                 pos+2);
        } else if (varEnd == pos+2) {
            throw RegexException(String() << "Missing Lua callout expression within '(*...)'.", 
                                 pos+2);
        }
        String varName = findString.getSubstring(pos+2, varEnd - (pos+2));

        LuaAccess::Result exprResult = luaAccess.executeExpression(varName);

        LuaVar g(luaAccess);

        if (exprResult.objects.getLength() > 0) {
            g = exprResult.objects[0];
        }

        if (!g.isTable() && !g.isFunction()) {                    
            throw RegexException(String() << "Lua expression '" << varName << "' must evaluate to table or function.", 
                                 pos+2);
        }
        storeReference = g.store();

        if (lastParen != -1) {
            for (int p = lastParen + 1; p < j; ++p) {
                int p2 = p;
                while (p2 < n && findString[p2] != ',' && findString[p2] != ')') {
                    ++p2;
                }
                if (p2 > p)
                {
                    String paramString = findString.getSubstring(p, p2 - p);
                    arguments.append(paramString);
                    positions.append(p);
                }
                p = p2;
            }
        }

        lastPosition = j - 1;
    }
    catch (LuaException& ex) {
        throw RegexException(ex.getMessage(), startPosition+2);
    }
}

FindUtil::CalloutObject::Ptr FindUtil::buildCalloutObject(FindUtil::PreparsedCallout::Ptr preparsed)
{
    CalloutObject::Ptr rslt = CalloutObject::create(preparsed->getStoredLuaObjectReference());
        
    for (int j = 0, m = preparsed->getNumberOfArguments(); j < m; ++j)
    {
        String argument = preparsed->getArgument(j);
        int parenNumber;
        if (argument.consistsOfDigits()) {
            parenNumber = argument.toInt();
            if (parenNumber < 0 || parenNumber > regex.getNumberOfCapturingSubpatterns()) {
                throw RegexException(String() << "Invalid number '" << argument << "' for capturing substring",
                                     preparsed->getArgumentPosition(j));
            }
        } else {
            try {
                parenNumber = regex.getStringNumber(argument);
            } catch (RegexException& ex) {
                throw RegexException(ex.getMessage(),
                                     preparsed->getArgumentPosition(j));
            }
        }
        rslt->args.append(parenNumber);
    }
    return rslt;
}


void FindUtil::initialize()
{
    this->maxForwardAssertionLength  = GlobalConfig::getInstance()->getMaxRegexAssertionLength();
    this->maxBackwardAssertionLength = GlobalConfig::getInstance()->getMaxRegexAssertionLength();

    ObjectArray<PreparsedCallout::Ptr> preparsedCallouts;
    
    calloutObjects.clear();

    String findString = p.getFindString();

    if (findString.getLength() <= 0) {
        regex = BasicRegex();
        wasInitializedFlag = true;
        return;
    }

    BasicRegex::CreateOptions opts = BasicRegex::MULTILINE;
    if (p.hasIgnoreCaseFlag()) {
        opts |= BasicRegex::IGNORE_CASE;
    }

    String searchPattern;
    if (p.hasWholeWordFlag()) {
        searchPattern.append("\\b");             expressionPositions.append(0);
    }

    if (p.hasRegexFlag())
    {
        int calloutCounter = 1;
        for (int i = 0, n = findString.getLength(); i < n; ++i)
        {
            if ((i == 0 || findString[i-1] != '\\')
             && findString[i] == '(' && i+1 < n && findString[i+1] == '*')
            {
                {
                    char counterString[20];
                    sprintf(counterString, "%d", calloutCounter++);
                    searchPattern.append("(?C"); expressionPositions.append(i);
                                                 expressionPositions.append(i);
                                                 expressionPositions.append(i);
                    searchPattern.append(counterString);

                    for (int i2 = 0, n2 = strlen(counterString); i2 < n2; ++i2) {
                                                 expressionPositions.append(i);
                    }
                    searchPattern.append(")");   expressionPositions.append(i);
                }
                try
                {
	            PreparsedCallout::Ptr preparsedCallout = PreparsedCallout::parse(findString, i);
                    i = preparsedCallout->getLastPosition();
                    preparsedCallouts.append(preparsedCallout);
                }
                catch (RegexException& ex)
                {
                    throw MyRegexException(ex.getMessage(), ex.getPosition());
                }
            }
            else if ((i == 0 || findString[i-1] != '\\')
                    && findString[i] == '(' && i+3 < n && findString[i+1] == '?' && findString[i+2] == 'C')
            {
                throw MyRegexException("Callout pattern '(?C...)' not allowed.", i);
            }
            else
            {
                searchPattern.append(findString[i]);           expressionPositions.append(i);
            }
        }
    }
    else {
        for (int i = 0, n = findString.getLength(); i < n; ++i) {
            switch (findString[i]) {
                case '\\': case '^': case '$': case '.': case '[': case ']': case '|':
                case '(':  case ')': case '?': case '*': case '+': case '{':
                case '}':  case '-': searchPattern.append('\\'); expressionPositions.append(i);
            }
            searchPattern.append(findString[i]);               expressionPositions.append(i);
        }
    }
    if (p.hasWholeWordFlag()) {
        searchPattern.append("\\b");                             expressionPositions.append(findString.getLength());
    }
    
    expressionPositions.append(findString.getLength());

    regex = BasicRegex(searchPattern, opts);

    ovector.clear();
    ovector.appendAmount(regex.getOvecSize());

    for (int i = 0, n = preparsedCallouts.getLength(); i < n; ++i)
    {
        try {
            calloutObjects.append(buildCalloutObject(preparsedCallouts[i]));
        }
        catch (RegexException& ex)
        {
            throw MyRegexException(ex.getMessage(), ex.getPosition());
        }
    }
    wasInitializedFlag = true;
}

bool FindUtil::doesMatch()
{
    try
    {
        if (!wasInitialized()) {
            initialize();
        }

        int   textLength     = textData->getLength();
        byte* textStart      = textData->getAmount(0, textLength);

        if (textPosition < 0 || textPosition > textLength) {
            wasFoundFlag = false;
            return false;
        }

        if (regex.findMatch(this, &FindUtil::pcreCalloutFunction,
                            (char*)textStart, textLength, textPosition, BasicRegex::ANCHORED, ovector)) {
            wasFoundFlag = true;
        } else {
            wasFoundFlag = false;
        }
        return wasFoundFlag;
    }
    catch (MyRegexException& ex) {
        throw RegexException(ex.getMessage(), ex.getPosition());
    }
    catch (RegexException& ex) {
        throw RegexException(ex.getMessage(), expressionPositions[ex.getPosition()]);
    }
}

void FindUtil::findNext()
{
    try
    {
        if (!wasInitialized()) {
            initialize();
        }
	
        if (textPosition < 0 || textPosition > textData->getLength()) {
            wasFoundFlag = false;
            return;
        }

	long startingTextPosition = textPosition;
	int doItCounter = 0;
	
    doItAgain:

        wasFoundFlag = false;

        ++doItCounter;

        if (p.hasSearchForwardFlag()) 
        {
            long epos;
            if (maximalEndOfMatchPosition == -1) {
                epos = textData->getLength();
            } else {
                epos = maximalEndOfMatchPosition;
            }
            long blockStartPos = (textPosition - maxBackwardAssertionLength > 0)
                               ? (textPosition - maxBackwardAssertionLength) 
                               : 0;

            long blockLength   = (epos + maxForwardAssertionLength < textData->getLength()) 
                               ? (epos + maxForwardAssertionLength - blockStartPos) 
                               : (textData->getLength() - blockStartPos);
            
            byte* blockStartPtr = textData->getAmount(blockStartPos, blockLength);

            if (regex.findMatch(this, &FindUtil::pcreCalloutFunction,
                                (char*)blockStartPtr, blockLength, textPosition - blockStartPos,
                                BasicRegex::MatchOptions(), ovector))
            {
                for (int i = 0, n = ovector.getLength(); i < n; ++i) {
                    ovector[i] += blockStartPos;
                }
                if (ovector[1] <= epos)
                {
                    if (p.hasAllowMatchAtStartOfSearchFlag() || doItCounter > 1 || startingTextPosition < ovector[1])
                    {
                        wasFoundFlag = true;
                        textPosition = ovector[0];
                    }
                    else if (doItCounter == 1 && textPosition < textData->getLength())
                    {
                        ++textPosition;
                        goto doItAgain;
                    }
                }
            }
        } else {
            int   textLength     = textData->getLength();
            byte* textStart      = textData->getAmount(0, textLength);
            
            long epos;
            if (maximalEndOfMatchPosition == -1) {
                epos = textLength;
            } else {
                epos = maximalEndOfMatchPosition;
            }
            long zpos = 0;
            if (noMatchBeforePosition != -1) {
                zpos = noMatchBeforePosition;
            }
            while (!wasFoundFlag && textPosition >= zpos) {
                if (regex.findMatch(this, &FindUtil::pcreCalloutFunction,
                                    (char*)textStart, textLength, textPosition, BasicRegex::ANCHORED, ovector))
                {
                    if (ovector[1] <= epos) 
                    {
                        if (p.hasAllowMatchAtStartOfSearchFlag() || doItCounter > 1 || ovector[0] < startingTextPosition)
                        {
                            wasFoundFlag = true;
                        }
                        else if (doItCounter == 1 && 0 < textPosition)
                        {
                            --textPosition;
                            goto doItAgain;
                        }
                        else {
                            --textPosition;
                        }
                    }
                }
                else {
                    --textPosition;
                }
            }
        }
        if (wasError) {
            throw luaException;
        }
    }
    catch (MyRegexException& ex) {
        throw RegexException(ex.getMessage(), ex.getPosition());
    }
    catch (RegexException& ex) {
        throw RegexException(ex.getMessage(), expressionPositions[ex.getPosition()]);
    }
}


