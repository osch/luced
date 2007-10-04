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

#include "ReplaceUtil.hpp"
#include "SubstitutionException.hpp"
#include "RegexException.hpp"

using namespace LucED;

static unsigned char convertHexCharToNumber(char hexChar)
{
    switch (hexChar) {
        case '0': return  0;
        case '1': return  1;
        case '2': return  2;
        case '3': return  3;
        case '4': return  4;
        case '5': return  5;
        case '6': return  6;
        case '7': return  7;
        case '8': return  8;
        case '9': return  9;

        case 'a':
        case 'A': return 10;

        case 'b':
        case 'B': return 11;

        case 'c':
        case 'C': return 12;

        case 'd':
        case 'D': return 13;

        case 'e':
        case 'E': return 14;

        case 'f':
        case 'F': return 15;
    }
    ASSERT(false);
    return 0;
}


String ReplaceUtil::getSubstitutedString()
{
    if (!FindUtil::wasInitialized()) {
        FindUtil::initialize();
    }

    if (getRegexFlag() == false)
    {
        return getReplaceString();
    }
    else
    {
        const String replaceString = getReplaceString();
        String rslt;
        for (int i = 0, n = replaceString.getLength(); i < n; ++i)
        {
            if (replaceString[i] == '\\' && i + 1 < n)
            {
                switch (replaceString[i+1])
                {
                    case 'n': rslt.append('\n'); break;
                    case 'r': rslt.append('\r'); break;
                    case 't': rslt.append('\t'); break;
                    case 'f': rslt.append('\f'); break;
                    case 'v': rslt.append('\v'); break;
                    case 'x': {
                        String hexCode = replaceString.getSubstring(i+2, 2);
                        if (!hexCode.isHex()) {
                            throw SubstitutionException("\\x needs two hex digits", i);
                        }
                        int code = (convertHexCharToNumber(hexCode[0]) << 4)
                                  + convertHexCharToNumber(hexCode[1]);
                        rslt.append((char)code);
                        i += 2;
                        break;
                    }
                    default:  {
                        if (isdigit(replaceString[i+1])) {
                            int capturedIndex  = replaceString.getSubstring(i+1, 1).toInt(); 
                            int capturedPos    = getCapturedSubpatternBeginPos(capturedIndex);
                            int capturedLength = getCapturedSubpatternLength  (capturedIndex);
                            if (capturedPos == -1) {
                                throw SubstitutionException(
                                    String() << "illegal index '" << replaceString[i+1] << "' for captured pattern", i);
                            }
                            String capturedText = getTextData()->getSubstring(capturedPos, capturedLength);
                            rslt.append(capturedText);
                        } else {
                            rslt.append(replaceString[i+1]);
                        }
                    }
                }
                ++i;
            }
            else if (replaceString[i] == '(' && i + 2 < n && replaceString[i+1] == '*')
            {
                // lua callout
                try {
	            PreparsedCallout::Ptr preparsedCallout = PreparsedCallout::parse(replaceString, i);
                    CalloutObject::Ptr callout = FindUtil::buildCalloutObject(preparsedCallout);

                    LuaObject objectToCall = callout->getCallableObject();
                    LuaObject callResult;
                    
                    if (objectToCall.isTable())
                    {
                        int capturedIndex = callout->getCaptureIndexForArgument(0);
                        int spos = getCapturedSubpatternBeginPos(capturedIndex);
                        int len  = getCapturedSubpatternLength(capturedIndex);

                        callResult = objectToCall[getTextData()->getSubstring(spos, len)];
                    }
                    else if (objectToCall.isFunction())
                    {
                        LuaFunctionArguments args;

                        for (int i = 0; i < callout->getNumberOfArguments(); ++i)
                        {
                            int capturedIndex = callout->getCaptureIndexForArgument(i);
                            int spos = getCapturedSubpatternBeginPos(capturedIndex);
                            int len  = getCapturedSubpatternLength(capturedIndex);
                            args << getTextData()->getSubstring(spos, len);
                        }
                        callResult = objectToCall.call(args);
                    }
                    if (callResult.isString()) {
                        rslt.append(callResult.toString());
                    }
                    i = preparsedCallout->getLastPosition();
                }
                catch (RegexException& ex) {
                    throw SubstitutionException(ex.getMessage(), ex.getPosition());
                }
            } else {
                rslt.append(replaceString[i]);
            }
        }
        return rslt;
    }
}

bool ReplaceUtil::replaceAllBetween(long spos, long epos)
{
    TextData* textData = getTextData();
    TextData::TextMark textMark = textData->createNewMark();
    textMark.moveToPos(spos);

    FindUtil::setTextPosition(textMark.getPos());
    FindUtil::setMaximalEndOfMatchPosition(epos);
    
    bool wasAnythingReplaced = false;
    
    try
    {
        FindUtil::setAllowMatchAtStartOfSearchFlag(true);
        
        while (textMark.getPos() < epos)
        {
            FindUtil::findNext();
            if (FindUtil::wasFound())
            {
                textMark.moveForwardToPos(FindUtil::getTextPosition());
                
                if (FindUtil::getTextPosition() < epos)
                {
                    wasAnythingReplaced = true;

                    String substitutedString = getSubstitutedString();
                    textData->insertAtMark(textMark, substitutedString);

                    textMark.moveForwardToPos(textMark.getPos() + substitutedString.getLength());

                    textData->removeAtMark(textMark, FindUtil::getMatchLength());

                    if (FindUtil::getMatchLength() == 0) {
                        textMark.inc();
                    }

                    epos += substitutedString.getLength() - FindUtil::getMatchLength();

                    FindUtil::setTextPosition(textMark.getPos());
                    FindUtil::setMaximalEndOfMatchPosition(epos);
                }
            }
            else
            {
                break;
            }
        }

        FindUtil::setMaximalEndOfMatchPosition(-1);
    } catch (...) {
        FindUtil::setMaximalEndOfMatchPosition(-1);
        throw;
    }
    
    return wasAnythingReplaced;
}


