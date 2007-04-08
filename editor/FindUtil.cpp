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

#include <ctype.h>

#include "FindUtil.h"
#include "MemArray.h"
#include "Regex.h"
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
            string firstArg;
            
            if (args.getLength() == 0) {
                if (capt != -1 && ovector[capt * 2 + 1] == pos) {
                    firstArg = string(ptr + ovector[capt * 2 + 0], ovector[capt * 2 + 1] - ovector[capt * 2 + 0]);
                } else {
                    firstArg = string(ptr + pos - 1, 1);
                }
                luaArgs << firstArg;
            } else {
                string k;
                for (int j = 0, m = args.getLength(); j < m; ++j)
                {
                    if (args[j] == 0) {
                        int start = calloutBlock->start_match; 
                        k = string(ptr + start, pos - start);
                    } else if (args[j] <= top) {
                        k = string(ptr + ovector[args[j] * 2 + 0], ovector[args[j] * 2 + 1] - ovector[args[j] * 2 + 0]);
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
        PreParam(const string& name, int pos) : name(name), pos(pos)
        {}
        string name;
        int    pos;
    };
    
    bool isNumber(const string& s) {
        for (int i = 0, n = s.length(); i < n; ++i) {
            if (!isdigit(s[i])) {
                return false;
            }
        }
        return true;
    }

    class MyRegexException : public BaseException
    {
    public:
        MyRegexException(const string& message, int position)
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



void FindUtil::findNext()
{
    ObjectArray< ObjectArray<PreParam> > preParams;
    
    calloutObjects.clear();

    if (searchString.length() <= 0) {
        return;
    }

    Regex::CreateOptions opts = Regex::MULTILINE | Regex::ANCHORED;
    if (ignoreCaseFlag) {
        opts |= Regex::IGNORE_CASE;
    }

    MemArray<int> expressionPositions;

    string searchPattern;
    if (wholeWordFlag) {
        searchPattern.append("\\b");             expressionPositions.append(0);
    }

    if (regexFlag)
    {
        int calloutCounter = 1;
        for (int i = 0, n = searchString.length(); i < n; ++i) {
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
                    string varName = searchString.substr(i+2, varEnd - (i+2));

                    LuaInterpreter::Result exprResult = lua->executeExpression(varName);
                    
                    LuaObject g;
                    
                    if (exprResult.objects.getLength() > 0) {
                        g = exprResult.objects[0];
                    }

                    if (!g.isTable() && !g.isFunction()) {                    
                        throw RegexException("Lua expression '" + varName + "' must evaluate to table or function.", i+2);
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
                                string paramString = searchString.substr(p, p2 - p);
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
                searchPattern.push_back(searchString[i]);           expressionPositions.append(i);
            }
        }
    }
    else {
        for (int i = 0, n = searchString.length(); i < n; ++i) {
            switch (searchString[i]) {
                case '\\': case '^': case '$': case '.': case '[': case ']':
                case '(':  case ')': case '?': case '*': case '+': case '{':
                case '}':  case '-': searchPattern.push_back('\\'); expressionPositions.append(i);
            }
            searchPattern.push_back(searchString[i]);               expressionPositions.append(i);
        }
    }
    if (wholeWordFlag) {
        searchString.append("\\b");                                expressionPositions.append(searchString.length());
    }
    
    expressionPositions.append(searchString.length());

    try
    {
        Regex regex = Regex(searchPattern, opts);
        int   textLength     = textData->getLength();
        byte* textStart      = textData->getAmount(0, textLength);
        ovector.clear();
        ovector.appendAmount(regex.getOvecSize());
        
        for (int i = 0, n = preParams.getLength(); i < n; ++i)
        {
            ObjectArray<PreParam>& p = preParams[i];
            for (int j = 0, m = p.getLength(); j < m; ++j)
            {
                int parenNumber;
                if (isNumber(p[j].name)) {
                    parenNumber = atoi(p[j].name.c_str());
                    if (parenNumber < 0 || parenNumber > regex.getNumberOfCapturingSubpatterns()) {
                        throw MyRegexException("Invalid number '" + p[j].name + "' for capturing substring", p[j].pos);
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


