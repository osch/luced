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

#include "LuaObject.h"
#include "LuaException.h"
#include "LuaObjectList.h"

using namespace LucED;


const LuaObject LuaObject::none;

LuaObject LuaObject::operator[](const char* fieldName)
{
    lua_pushstring(getL(), fieldName);
    lua_gettable(getL(), getStackIndex());
    return newLuaObject(lua_gettop(getL()));
}

LuaObject LuaObject::operator[](const string& fieldName)
{
    lua_pushlstring(getL(), fieldName.c_str(), fieldName.length());
    lua_gettable(getL(), getStackIndex());
    return newLuaObject(lua_gettop(getL()));
}

LuaObject LuaObject::operator[](int index)
{
    lua_pushnumber(getL(), index);
    lua_gettable(getL(), getStackIndex());
    return newLuaObject(lua_gettop(getL()));
}

LuaObject LuaObject::call() const
{
    lua_pushvalue(getL(), getStackIndex());
    int error = lua_pcall(getL(), 0, 1, 0);

    if (error != 0)
    {
        LuaException ex(lua_tostring(getL(), -1));
        lua_pop(getL(), 1);
        throw ex;
    } else {    
        return newLuaObject(lua_gettop(getL()));
    }
}

LuaObject LuaObject::call(const LuaObject& arg) const
{
    lua_pushvalue(getL(), getStackIndex());
    lua_pushvalue(getL(), arg.getStackIndex());
    
    int error = lua_pcall(getL(), 1, 1, 0);

    if (error != 0)
    {
        LuaException ex(lua_tostring(getL(), -1));
        lua_pop(getL(), 1);
        throw ex;
    } else {    
        return newLuaObject(lua_gettop(getL()));
    }
}

LuaObject LuaObject::call(const LuaObjectList& list) const
{
    lua_pushvalue(getL(), getStackIndex());
    for (int i = 0; i < list.getLength(); ++i) {
        lua_pushvalue(getL(), list[i].getStackIndex());
    }
    int error = lua_pcall(getL(), list.getLength(), 1, 0);

    if (error != 0)
    {
        LuaException ex(lua_tostring(getL(), -1));
        lua_pop(getL(), 1);
        throw ex;
    } else {    
        return newLuaObject(lua_gettop(getL()));
    }
}


const char* LuaObject::getTypeName() const
{
    return lua_typename(getL(), lua_type(getL(), getStackIndex()));
}

LuaObject::KeyList::Ptr LuaObject::getTableKeys() const
{
    HeapObjectArray<string>::Ptr rslt = HeapObjectArray<string>::create();
    
    lua_pushnil(getL());
    
    while (lua_next(getL(), getStackIndex()) != 0) {
        if (lua_type(getL(), -2) == LUA_TSTRING) {
            rslt->append(lua_tostring(getL(), -2));
        }
        lua_pop(getL(), 1);
    }
    
    return rslt;
}
