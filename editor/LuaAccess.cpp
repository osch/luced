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

#include "LuaAccess.hpp"
#include "LuaStateAccess.hpp"
#include "LuaAccess.hpp"
#include "ByteBuffer.hpp"
#include "File.hpp"
#include "LuaVar.hpp"
#include "Regex.hpp"

using namespace LucED;


LuaAccess::Result LuaAccess::executeScript(const char* scriptBegin, long scriptLength, String name) const
{
    ASSERT(isCorrect());

    RawPtr<LuaInterpreter> luaInterpreter = getLuaInterpreter();
    
    String oldPrintBuffer = LuaInterpreter::PrintBufferAccess::getPrintBuffer(luaInterpreter);
                            LuaInterpreter::PrintBufferAccess::setPrintBuffer(luaInterpreter, String());

    int oldTop = lua_gettop(L);

    int error = luaL_loadbuffer(L, scriptBegin, scriptLength, name.toCString())
            || lua_pcall(L, 0, LUA_MULTRET, 0);

    String printBuffer = LuaInterpreter::PrintBufferAccess::getPrintBuffer(luaInterpreter);
                         LuaInterpreter::PrintBufferAccess::setPrintBuffer(luaInterpreter, oldPrintBuffer);

    if (error) {
        LuaException ex(lua_tostring(L, -1));
        lua_pop(L, 1);
        throw ex;
    }
    Result rslt;
           rslt.output = printBuffer;
    for (int i = oldTop + 1, n = lua_gettop(L); i <= n; ++i) {
        rslt.objects.appendObjectWithStackIndex(*this, i);
    }

    return rslt;
}

LuaAccess::Result LuaAccess::executeExpression(const char* scriptBegin, long scriptLength, String name) const
{
    ASSERT(isCorrect());

    RawPtr<LuaInterpreter> luaInterpreter = getLuaInterpreter();
    
    String oldPrintBuffer = LuaInterpreter::PrintBufferAccess::getPrintBuffer(luaInterpreter);
                            LuaInterpreter::PrintBufferAccess::setPrintBuffer(luaInterpreter, String());

    
    String script = "return ";
    script.append(scriptBegin, scriptLength);
    
    int oldTop = lua_gettop(L);

    int error = luaL_loadbuffer(L, script.toCString(), script.getLength(), name.toCString())
            || lua_pcall(L, 0, LUA_MULTRET, 0);

    String printBuffer = LuaInterpreter::PrintBufferAccess::getPrintBuffer(luaInterpreter);
                         LuaInterpreter::PrintBufferAccess::setPrintBuffer(luaInterpreter, oldPrintBuffer);
    if (error) {
        LuaException ex(lua_tostring(L, -1));
        lua_pop(L, 1);
        throw ex;
    }

    Result rslt;
           rslt.output = printBuffer;
    for (int i = oldTop + 1, n = lua_gettop(L); i <= n; ++i) {
        rslt.objects.appendObjectWithStackIndex(*this, i);
    }

    return rslt;
}


LuaAccess::Result LuaAccess::executeFile(String name) const
{
    ByteBuffer buffer;
    File(name).loadInto(buffer);
    return executeScript((const char*) buffer.getTotalAmount(), buffer.getLength(), name);
}


LuaVar LuaAccess::loadString(const char* script, const String& pseudoFileName) const
{
    int rc = luaL_loadstring(L, script);
    
    if (rc != 0) {
        if (rc == LUA_ERRMEM) {
            throw LuaException("out of memory");
        } else {
            String errorText = lua_tostring(L, -1);
            lua_pop(L, 1);
    
            Regex  r("^line (\\d+)\\: \\s*");
            int    lineNumber;
            String errorMessage;
    
            if (r.matches(errorText)) {
                lineNumber   = errorText.getSubstringBetween(r.getCaptureBegin(1), 
                                                             r.getCaptureEnd(1)).toInt() - 1;
                errorMessage = errorText.getTail(r.getCaptureEnd(0));
            } else {
                lineNumber   = -1;
                errorMessage = errorText;
            }
            throw LuaException(errorMessage,
                               lineNumber,
                               pseudoFileName);
        }
    }
    return LuaVar(*this, lua_gettop(L));
}



LuaVar LuaAccess::loadFile(const String& fileName) const
{
    int rc = luaL_loadfile(L, fileName.toCString());
    
    if (rc != 0) {
        if (rc == LUA_ERRFILE) {
            lua_pop(L, 1);
            lua_pushnil(L);
        } else if (rc == LUA_ERRMEM) {
            throw LuaException("out of memory");
        } else {
            String errorText = lua_tostring(L, -1);
            lua_pop(L, 1);

            Regex  r("^line (\\d+)\\: \\s*");
            int    lineNumber;
            String errorMessage;

            if (r.findMatch(errorText)) {
                lineNumber   = errorText.getSubstringBetween(r.getCaptureBegin(1), 
                                                             r.getCaptureEnd(1)).toInt() - 1;
                errorMessage = errorText.getTail(r.getCaptureEnd(0));
            } else {
                lineNumber   = -1;
                errorMessage = errorText;
            }
            throw LuaException(errorMessage,
                               lineNumber,
                               fileName);
        }
    }    
    return LuaVar(*this, lua_gettop(L));
}

