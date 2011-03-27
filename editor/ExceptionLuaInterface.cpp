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

#include "ExceptionLuaInterface.hpp"
#include "LuaCMethodArgChecker.hpp"
#include "LuaArgException.hpp"
#include "Regex.hpp"

using namespace LucED;

ExceptionLuaInterface::Ptr ExceptionLuaInterface::create(const LuaVar& errorObject) 
{
    ExceptionLuaInterface::Ptr rslt = errorObject.toOwningPtr<ExceptionLuaInterface>();

    if (rslt.isValid()) {
        return rslt;
    } else {
        return Ptr(new ExceptionLuaInterface(errorObject));
    }
}

ExceptionLuaInterface::ExceptionLuaInterface(LuaAccess luaAccess,
                                             String    message)
    : message(message),
      lineNumber(-1),
      isBuiltinFileFlag(false)
{
    evaluateLuaStackTrace(luaAccess);
}

static String unquoteSourceString(const String& source)
{
    String rslt;

    for (int i = 0, len = source.getLength(); i < len; ++i)
    {
        char c = source[i];
        if (c != '\\' || i + 1 >= len) {
            rslt << c;
        } else {
            ++i;
            c = source[i];
            switch (c) {
                case 'n': rslt << "\n"; break;
                case 'r': rslt << "\r"; break;
                default:  rslt << c;    break;
            }
        }
    }
    return rslt;
}

ExceptionLuaInterface::ExceptionLuaInterface(const LuaVar& errorObject)
    : lineNumber(-1),
      isBuiltinFileFlag(false)
{
    LuaAccess  luaAccess = errorObject.getLuaAccess();

    if (errorObject.isString()) {
        String errorText = errorObject.toString();

        Regex  r("^\\[\\s*([^\"\\]]*?)\\s*(?>\"((?>\\\\.|[^\\\\])*)\")?\\]:(\\d+)\\: \\s*(.*)");

        if (r.matches(errorText)) {
            String type  = errorText.getSubstring(Pos(r.getCaptureBegin(1)), 
                                                   Pos(r.getCaptureEnd(1)));
            String source = errorText.getSubstring(Pos(r.getCaptureBegin(2)), 
                                                   Pos(r.getCaptureEnd(2)));
            lineNumber    = errorText.getSubstring(Pos(r.getCaptureBegin(3)), 
                                                   Pos(r.getCaptureEnd(3))).toInt() - 1;
            message       = errorText.getTail(r.getCaptureBegin(4));

            source = unquoteSourceString(source);

            if (type == "file") {
                fileName = source;
                isBuiltinFileFlag = false;
            }
            else if (source[0] == '$') {
                fileName = source.getTail(1);
                isBuiltinFileFlag = true;
            }
        } else {
            lineNumber   = -1;
            message = errorText;
        }
    }
    if (lineNumber == -1)
    {
        evaluateLuaStackTrace(luaAccess);
    }
}

inline void ExceptionLuaInterface::evaluateLuaStackTrace(const LuaAccess& luaAccess)
{
    int callLevel = 1;

retry:
    lua_Debug debugInfo;   memset(&debugInfo, 0, sizeof(debugInfo));
    
    int rc = lua_getstack(luaAccess.L, callLevel, &debugInfo);
    if (rc == 1)
    {
        lua_getinfo (luaAccess.L, "Sln", &debugInfo);

        if (strcmp(debugInfo.what, "C") == 0) {
            callLevel += 1;
            goto retry;
        }
    
        if (debugInfo.source != NULL) {
            if (debugInfo.source[0] == '@' || debugInfo.source[0] == '$') {
                fileName   = debugInfo.source + 1;
                lineNumber = debugInfo.currentline - 1;
            }
            if (debugInfo.source[0] == '$') {
                isBuiltinFileFlag = true;
            }
        }
    }
}


void ExceptionLuaInterface::prependMessage(const String& message)
{
    if (this->message.getLength() > 0) {
        this->message = String() << message << ": " << this->message;
    } else {
        this->message = message;
    }
}


LuaCFunctionResult ExceptionLuaInterface::toString(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    return LuaCFunctionResult(luaAccess) << message;
}

