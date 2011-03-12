/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2011 Oliver Schmidt, oliver at luced dot de
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

#include "MatchLuaInterface.hpp"
#include "ActionIdRegistry.hpp"
#include "LuaCMethodArgChecker.hpp"
#include "LuaArgException.hpp"
#include "RegexException.hpp"

using namespace LucED;

int MatchLuaInterface::getCaptureNumber(const LuaVar& luaVar)
{
    int captureNumber = 0;

    if (luaVar.isNumber()) {
        captureNumber = luaVar.toInt();
    }
    else if (luaVar.isString()) {
        String captureName = luaVar.toString();
        try {
            captureNumber = regex.getCaptureNumberByName(captureName);
        } catch (RegexException& ex) {
            throw LuaArgException(String() << "unknown capture name: " << captureName);
        }
    }
    else {
        throw LuaArgException("argument must be number or name of capture");
    }

    if (captureNumber >= captureCount) {
        throw LuaArgException(String() << "invalid capture number: " << captureNumber);
    }
    
    return captureNumber;
}

LuaCFunctionResult MatchLuaInterface::internGetPos(const LuaCFunctionArguments& args, int offs)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    int captureNumber = 0;

    if (args.getLength() > 0)
    {
        captureNumber = getCaptureNumber(args[0]);
    }
    
    int rslt = ovector[captureNumber * 2 + offs];

    if (rslt < 0) {
        return LuaCFunctionResult(luaAccess);
    } else {
        return LuaCFunctionResult(luaAccess) << rslt;
    }
}

LuaCFunctionResult MatchLuaInterface::getPositions(const LuaCFunctionArguments& args)
{
    LuaCFunctionResult rslt(args.getLuaAccess());
    
    for (int i = 0; i < 2 * captureCount; ++i) {
        int pos = ovector[i];
        if (pos < 0) {
            rslt << Null;
        } else {
            rslt << pos;
        }
    }
    return rslt;
}

LuaCFunctionResult MatchLuaInterface::remove(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();

    if (!textData.isValid() || !e.isValid()) {
        return LuaCFunctionResult(luaAccess);
    }

    int captureNumber = 0;

    if (args.getLength() > 0)
    {
        captureNumber = getCaptureNumber(args[0]);
    }
    
    int spos = ovector[2*captureNumber];
    int epos = ovector[2*captureNumber+1];
    
    if (spos < 0 || epos < 0) {
        return LuaCFunctionResult(luaAccess);
    }
    
    if (epos > textData->getLength()) {
        epos = textData->getLength();
    }
    
    if (epos > spos)
    {
        int removeLength = epos - spos;
        
        TextData::TextMark m = e->createNewMarkFromCursor();
        m.moveToPos(spos);
        textData->removeAtMark(m, removeLength);

        for (int i = 0; i < 2 * captureCount + 1; ++i)
        {
            int p = ovector[i];
            
            if (p >= 0) {
                if (p >= epos) {
                    ovector[i] = p - removeLength;
                }
                else if (spos <= p && p < epos) {
                    ovector[i] = spos;
                }
            }
        }
    }
    return LuaCFunctionResult(luaAccess);
}


LuaCFunctionResult MatchLuaInterface::replace(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();

    if (!textData.isValid() || !e.isValid()) {
        return LuaCFunctionResult(luaAccess);
    }

    int captureNumber = 0;
    String replaceString;

    if (args.getLength() == 1 && args[0].isString()) 
    {
        replaceString = args[0].toString();
    }
    else if (args.getLength() == 2 && args[1].isString())
    {
        captureNumber = getCaptureNumber(args[0]);
        replaceString = args[1].toString();
    }
    else {
        throw LuaArgException(String() << "first argument is optional and must by match number/name, second argument must be replace string");
    }
    
    int spos = ovector[2*captureNumber];
    int epos = ovector[2*captureNumber+1];
    
    if (spos < 0 || epos < 0) {
        return LuaCFunctionResult(luaAccess);
    }
    
    if (epos > textData->getLength()) {
        epos = textData->getLength();
    }
    
    if (epos >= spos)
    {
        int removeLength = epos - spos;
        
        TextData::TextMark m = e->createNewMarkFromCursor();
        m.moveToPos(spos);
        
        if (removeLength > 0) {
            textData->removeAtMark(m, removeLength);
        }
        long insertedLength = textData->insertAtMark(m, replaceString);
        
        long diff = insertedLength - removeLength;

        for (int i = 0; i < 2 * captureCount + 1; ++i)
        {
            int p = ovector[i];
            
            if (p >= 0) {
                if (i != 2 * captureNumber) { // leave begin of capture unchanged
                    if (p >= epos) {
                        ovector[i] = p + diff;
                    }
                    else if (spos <= p && p < epos) {
                        ovector[i] = spos;
                    }
                }
            }
        }
    }
    return LuaCFunctionResult(luaAccess);
}

LuaCFunctionResult MatchLuaInterface::getBeginPos(const LuaCFunctionArguments& args)
{
    return internGetPos(args, 0);
}


LuaCFunctionResult MatchLuaInterface::getEndPos(const LuaCFunctionArguments& args)
{
    return internGetPos(args, 1);
}


LuaCFunctionResult MatchLuaInterface::getMatchedBytes(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();

    if (!textData.isValid() || !e.isValid()) {
        return LuaCFunctionResult(luaAccess);
    }

    int captureNumber = 0;
    
    if (args.getLength() > 0) {
        captureNumber = getCaptureNumber(args[0]);
    }
    
    long beginPos = ovector[captureNumber * 2];
    long endPos   = ovector[captureNumber * 2 + 1];
    
    if (beginPos < 0 || endPos < 0) {
        return LuaCFunctionResult(luaAccess);
    }

    const long length = textData->getLength();

    if (beginPos < 0) {
        beginPos = 0;
    }
    if (beginPos > length) {
        return LuaCFunctionResult(luaAccess);
    }
    
    long amount = endPos - beginPos;
    
    if (beginPos + amount > length) {
        amount = length - beginPos;
    }
    
    LuaVar rslt(luaAccess);
    
    if (amount > 0)
    {
        rslt = luaAccess.toLua((const char*)(textData->getAmount(beginPos, amount)),
                               amount);
    }
    else {
        rslt = "";
    }
    return LuaCFunctionResult(luaAccess) << rslt;
}

