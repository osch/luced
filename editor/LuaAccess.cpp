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
        LuaVar errorObject(*this, lua_gettop(L));
        throw LuaException(errorObject);
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
        LuaVar errorObject(*this, lua_gettop(L));
        throw LuaException(errorObject);
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
    File(name).loadInto(&buffer);
    return executeScript((const char*) buffer.getTotalAmount(), buffer.getLength(), name);
}


LuaVar LuaAccess::loadString(const char* script, const String& pseudoFileName) const
{
    int rc = luaL_loadbuffer(L, script, strlen(script), pseudoFileName.toCString());
    
    if (rc != 0) {
        if (rc == LUA_ERRMEM) {
            throw LuaException(*this, "out of memory");
        } else {
            LuaVar errorObject(*this, lua_gettop(L));
            throw LuaException(errorObject);
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
            throw LuaException(*this, "out of memory");
        } else {
            LuaVar errorObject(*this, lua_gettop(L));
            throw LuaException(errorObject);
        }
    }    
    return LuaVar(*this, lua_gettop(L));
}

