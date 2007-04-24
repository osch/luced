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

#include "FindUtil.h"
#include "MemArray.h"
#include "LuaInterpreter.h"
#include "RegexException.h"

using namespace LucED;


int FindUtil::pcreCalloutFunction(void* selfVoidPtr, pcre_callout_block* calloutBlock)
{
    FindUtil* self = static_cast<FindUtil*>(selfVoidPtr);

    self->wasError = false;
    try
    {
        int* ovector = calloutBlock->offset_vector;
        const char* ptr = calloutBlock->subject;
        int pos = calloutBlock->current_position;
        int capt = calloutBlock->capture_last;
        int top  = calloutBlock->capture_top;

        if (pos > 0)
        {        
            LuaObject o         = self->calloutObjects[calloutBlock->callout_number - 1].luaObject->retrieve();
            MemArray<int>& args = self->calloutObjects[calloutBlock->callout_number - 1].args;
            
            LuaFunctionArguments luaArgs;
            String firstArg;
            
            if (args.getLength() == 0) {
                if (capt != -1 && ovector[capt * 2 + 1] == pos) {
                    firstArg = String(ptr + ovector[capt * 2 + 0], ovector[capt * 2 + 1] - ovector[capt * 2 + 0]);
                } else {
                    firstArg = String(ptr + pos - 1, 1);
                }
                luaArgs << firstArg;
            } else {
                String k;
                for (int j = 0, m = args.getLength(); j < m; ++j)
                {
                    if (args[j] == 0) {
                        int start = calloutBlock->start_match; 
                        k = String(ptr + start, pos - start);
                    } else if (args[j] <= top) {
                        k = String(ptr + ovector[args[j] * 2 + 0], ovector[args[j] * 2 + 1] - ovector[args[j] * 2 + 0]);
                    }
                    luaArgs << k;
                    if (j == 0) {
                        firstArg = k;
                    }
                }
            }
            

            if (o.isTable())
            {
                return o[firstArg].isTrue() ? 0 : 1;
            }
            else if (o.isFunction())
            {
                LuaObject r = o.call(luaArgs);
                return r.isTrue() ? 0 : 1;
            }
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
    class PreParam
    {
    public:
        PreParam(const String& name, int pos) : name(name), pos(pos)
        {}
        String name;
        int    pos;
    };
    
    class MyRegexException : public BaseException
    {
    public:
        MyRegexException(const String& message, int position)
            : BaseException(message),
              position(position)
        {}
        virtual const char *what() {
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
            case '}':  case '-': { rslt << '\\' << c; break; }
            
            case '\n':           { rslt << '\\' << 'n'; break; }
            
            default:             { rslt << c; }
        }
    }
    return rslt;
}

void FindUtil::initialize()
{
    ObjectArray< ObjectArray<PreParam> > preParams;
    
    calloutObjects.clear();

    if (searchString.getLength() <= 0) {
        return;
    }

    Regex::CreateOptions opts = Regex::MULTILINE | Regex::ANCHORED;
    if (ignoreCaseFlag) {
        opts |= Regex::IGNORE_CASE;
    }

    String searchPattern;
    if (wholeWordFlag) {
        searchPattern.append("\\b");             expressionPositions.append(0);
    }

    if (regexFlag)
    {
        int calloutCounter = 1;
        for (int i = 0, n = searchString.getLength(); i < n; ++i) {
            if ((i == 0 || searchString[i-1] != '\\')
             && searchString[i] == '(' && i+3 < n && searchString[i+1] == '*') {
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
                {
                    int parenCounter = 1;
                    int j;
                    int lastParen = -1;
                    for (j = i+2; parenCounter > 0 && j < n; ++j) {
                        if (searchString[j] == '(') {
                            if (parenCounter == 1) {
                                lastParen = j;
                            } else {
                                lastParen = -1;
                            }
                            ++parenCounter;
                        } else if (searchString[j] == ')') {
                            --parenCounter;
                        }
                    }
                    LuaInterpreter* lua = LuaInterpreter::getInstance();
                    int varEnd;
                    if (lastParen != -1) {
                        varEnd = lastParen;
                    } else {
                        varEnd = j-1;
                    }
                    String varName = searchString.getSubstring(i+2, varEnd - (i+2));

                    LuaInterpreter::Result exprResult = lua->executeExpression(varName);
                    
                    LuaObject g;
                    
                    if (exprResult.objects.getLength() > 0) {
                        g = exprResult.objects[0];
                    }

                    if (!g.isTable() && !g.isFunction()) {                    
                        throw RegexException(String() << "Lua expression '" << varName << "' must evaluate to table or function.", i+2);
                    }
                    calloutObjects.appendNew(LuaStoredObject::store(g));
                    preParams.appendAmount(1);

                    if (lastParen != -1) {
                        for (int p = lastParen + 1; p < n; ++p) {
                            int p2 = p;
                            while (p2 < n && searchString[p2] != ',' && searchString[p2] != ')') {
                                ++p2;
                            }
                            if (p2 > p)
                            {
                                String paramString = searchString.getSubstring(p, p2 - p);
                                preParams.getLast().append(PreParam(paramString, p));
                            }
                            p = p2;
                        }
                    }

                    i = j - 1;
                }
            } else if ((i == 0 || searchString[i-1] != '\\')
                    && searchString[i] == '(' && i+3 < n && searchString[i+1] == '?' && searchString[i+2] == 'C') {
                throw RegexException("Callout pattern '(?C...)' not allowed.", i);
            } else {
                searchPattern.append(searchString[i]);           expressionPositions.append(i);
            }
        }
    }
    else {
        for (int i = 0, n = searchString.getLength(); i < n; ++i) {
            switch (searchString[i]) {
                case '\\': case '^': case '$': case '.': case '[': case ']':
                case '(':  case ')': case '?': case '*': case '+': case '{':
                case '}':  case '-': searchPattern.append('\\'); expressionPositions.append(i);
            }
            searchPattern.append(searchString[i]);               expressionPositions.append(i);
        }
    }
    if (wholeWordFlag) {
        searchString.append("\\b");                                expressionPositions.append(searchString.getLength());
    }
    
    expressionPositions.append(searchString.getLength());

    regex = Regex(searchPattern, opts);

    ovector.clear();
    ovector.appendAmount(regex.getOvecSize());

    for (int i = 0, n = preParams.getLength(); i < n; ++i)
    {
        ObjectArray<PreParam>& p = preParams[i];
        for (int j = 0, m = p.getLength(); j < m; ++j)
        {
            int parenNumber;
            if (p[j].name.consistsOfDigits()) {
                parenNumber = p[j].name.toInt();
                if (parenNumber < 0 || parenNumber > regex.getNumberOfCapturingSubpatterns()) {
                    throw MyRegexException(String() << "Invalid number '" << p[j].name << "' for capturing substring", p[j].pos);
                }
            } else {
                try {
                    parenNumber = regex.getStringNumber(p[j].name);
                } catch (RegexException& ex) {
                    throw MyRegexException(ex.getMessage(), p[j].pos);
                }
            }
            calloutObjects[i].args.append(parenNumber);
        }
    }
}

bool FindUtil::doesMatch()
{
    try
    {
        if (!wasInitialized) {
            initialize();
            wasInitialized = true;
        }

        int   textLength     = textData->getLength();
        byte* textStart      = textData->getAmount(0, textLength);

        if (regex.findMatch(this, &FindUtil::pcreCalloutFunction,
                            (char*)textStart, textLength, textPosition, Regex::MatchOptions(), ovector)) {
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
        if (!wasInitialized) {
            initialize();
            wasInitialized = true;
        }
        
        int   textLength     = textData->getLength();
        byte* textStart      = textData->getAmount(0, textLength);

        wasFoundFlag = false;
        if (searchForwardFlag) 
        {
            while (!wasFoundFlag && textPosition <= textLength) {
                if (regex.findMatch(this, &FindUtil::pcreCalloutFunction,
                                    (char*)textStart, textLength, textPosition, Regex::MatchOptions(), ovector)) {
                    wasFoundFlag = true;
                }
                else {
                    ++textPosition;
                }
            }
        } else {
            --textPosition;
            while (!wasFoundFlag && textPosition >= 0) {
                if (regex.findMatch(this, &FindUtil::pcreCalloutFunction,
                                    (char*)textStart, textLength, textPosition, Regex::MatchOptions(), ovector)) {
                    wasFoundFlag = true;
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


