/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch at luced dot de
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

#include "LuaInterpreter.h"
#include "File.h"
#include "ByteBuffer.h"
#include "LuaException.h"

using namespace LucED;


LuaInterpreter::LuaInterpreter()
{
    L = lua_open();
}

LuaInterpreter::~LuaInterpreter()
{
    lua_close(L);
}

void LuaInterpreter::executeFile(string name)
{
    ByteBuffer buffer;
    File(name).loadInto(buffer);
    int error = luaL_loadbuffer(L, (const char*) buffer.getTotalAmount(), buffer.getLength(), name.c_str())
            || lua_pcall(L, 0, 0, 0);
    if (error) {
        LuaException ex(lua_tostring(L, -1));
        lua_pop(L, 1);
        throw ex;
    }
}


int LuaInterpreter::newLuaObject(int stackIndex)
{
    int found = -1;
    for (int i = 0; i < tables.getLength(); ++i) {
        if (tables[i].usageCounter == 0) {
            found = i;
            break;
        }
    }
    if (found == -1) {
        tables.append(TableInfo(stackIndex));
        found = tables.getLength() - 1;
    } else {
        tables[found].stackIndex = stackIndex;
    }
    return found;
}

LuaObject LuaInterpreter::getGlobal(const char* name)
{
    lua_getglobal(L, name);
    int stackIndex = lua_gettop(L);
    return constructLuaObject(this, newLuaObject(stackIndex));
}

void LuaInterpreter::releaseTable(int index)
{
    int stackIndex = tables[index].stackIndex;
    lua_remove(L, stackIndex);
    for (int i = 0; i < tables.getLength(); ++i) {
        if (tables[i].usageCounter > 0 && tables[i].stackIndex > stackIndex) {
            tables[i].stackIndex -= 1;
        }
    }
}

void LuaInterpreter::setGlobal(const char* name, LuaObject value)
{
    lua_pushvalue(L, getStackIndex(value));
    lua_setglobal(L, name);
}


void LuaInterpreter::clearGlobal(const char* name)
{
    lua_pushnil(L);
    lua_setglobal(L, name);
}
