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


LuaCFunctionResult MatchLuaInterface::getBeginPos(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    if (args.getLength() == 0 && ovector.getLength() >= 2) {
        return LuaCFunctionResult(luaAccess) << ovector[0];
    }
    LuaVar arg = args[0];
    if (arg.isNumber()) {
        int i = arg.toInt();
        if (0 <= i && 2*i < ovector.getLength()) {
            return LuaCFunctionResult(luaAccess) << ovector[2*i];
        }
    }
    
    return LuaCFunctionResult(luaAccess);
}


LuaCFunctionResult MatchLuaInterface::getEndPos(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    if (args.getLength() == 0 && ovector.getLength() >= 2) {
        return LuaCFunctionResult(luaAccess) << ovector[1];
    }
    LuaVar arg = args[0];
    if (arg.isNumber()) {
        int i = arg.toInt();
        if (0 <= i && 2*i + 1 < ovector.getLength()) {
            return LuaCFunctionResult(luaAccess) << ovector[2*i + 1];
        }
    }
    return LuaCFunctionResult(luaAccess);
}

LuaCFunctionResult MatchLuaInterface::getMatchedBytes(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();

    if (!textData.isValid() || !e.isValid()) {
        return LuaCFunctionResult(luaAccess);
    }

    int captureNumber = 0;
    
    if (args.getLength() > 0) {
        if (args[0].isNumber()) {
            captureNumber = args[0].toInt();
        }
        else if (args[0].isString()) {
            String captureName = args[0].toString();
            try {
                captureNumber = regex.getCaptureNumberByName(captureName);
            } catch (RegexException& ex) {
                throw LuaArgException(String() << "unknown capture name: " << captureName);
            }
        }
        else {
            throw LuaArgException("first argument must be number or name of capture");
        }
    }
    if (captureNumber * 2 + 1 >= ovector.getLength()) {
        throw LuaArgException(String() << "invalid capture number: " << captureNumber);
    }
    
    long beginPos = ovector[captureNumber * 2];
    long endPos   = ovector[captureNumber * 2 + 1];

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

