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

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "LuaInterpreter.h"
#include "File.h"
#include "ByteBuffer.h"
#include "LuaException.h"
#include "LuaObject.h"
#include "LuaStackChecker.h"

using namespace LucED;

namespace LucED
{

class LuaFunction
{
public:

    static int function_print(lua_State* L)
    {
        bool wasError = false;
        int numberOfArguments = lua_gettop(L);
        int numberOfResults = 0;
        
        LuaInterpreter* lua = static_cast<LuaInterpreter*>(lua_touserdata(L, lua_upvalueindex(1)));
        ASSERT(lua != 0);

        try
        {
            for (int i = 1; i <= numberOfArguments; ++i)
            {
                string value = string(lua_tostring(L, i),
                                      lua_strlen(L, i));
                printf("print <%s>\n", value.c_str());
            }
        }
        catch (...)
        {
            lua_pushstring(L, "unknown error");
            wasError = true;
        }

        if (wasError) {
            lua_error(L);
        } else {
            return numberOfResults;
        }
    }
};

} // namespace LucED

SingletonInstance<LuaInterpreter> LuaInterpreter::instance;;


LuaInterpreter::LuaInterpreter()
{
    LuaStackChecker::getInstance(); // assure that StackChecker exists for all LuaObjects
    
    L = lua_open();

    luaopen_base(L);
    luaopen_table(L);
    luaopen_io(L);
    luaopen_string(L);
    luaopen_math(L);
    luaopen_debug(L);
    luaopen_loadlib(L);

    lua_pushlightuserdata(L, this);
    lua_pushcclosure(L, LuaFunction::function_print, 1);    
    lua_setglobal(L, "print");
    
    LuaObject::L = L;
}

LuaInterpreter::~LuaInterpreter()
{
    lua_close(L);
    
    LuaObject::L = NULL;
}



string LuaInterpreter::executeScript(const char* scriptBegin, long scriptLength, string name)
{
    int error = luaL_loadbuffer(L, scriptBegin, scriptLength, name.c_str())
            || lua_pcall(L, 0, 0, 0);

    if (error) {
        LuaException ex(lua_tostring(L, -1));
        lua_pop(L, 1);
        throw ex;
    }
    return string();
}

string LuaInterpreter::executeFile(string name)
{
    ByteBuffer buffer;
    File(name).loadInto(buffer);
    return executeScript((const char*) buffer.getTotalAmount(), buffer.getLength(), name);
}


LuaObject LuaInterpreter::getGlobal(const char* name)
{
    lua_getglobal(L, name);
    return LuaObject(lua_gettop(L));
}


void LuaInterpreter::setGlobal(const char* name, LuaObject value)
{
    lua_pushvalue(L, value.stackIndex);
    lua_setglobal(L, name);
}


void LuaInterpreter::clearGlobal(const char* name)
{
    lua_pushnil(L);
    lua_setglobal(L, name);
}

