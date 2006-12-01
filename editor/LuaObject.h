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

#ifndef LUAOBJECT_H
#define LUAOBJECT_H

#include <stddef.h>
#include <string>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "HeapObject.h"
#include "HeapObjectArray.h"

namespace LucED {

using std::string;

class LuaInterpreter;
class LuaObjectList;

class LuaObjectAccessToLuaInterpreter : private HeapObjectRefManipulator
{
protected:
    static void incTableRefCounter(LuaInterpreter *lua, int index);
    static void decTableRefCounter(LuaInterpreter *lua, int index);
    static lua_State* getL(LuaInterpreter *lua);
    static int getStackIndex(LuaInterpreter *lua, int index);
    static int newLuaObject(LuaInterpreter *lua, int stackIndex);
};


class LuaObject : private LuaObjectAccessToLuaInterpreter
{
public:
    static const LuaObject none;
    
    LuaObject() : lua(NULL), index(-1) {}
        
    LuaObject(const LuaObject& src) {
        lua   = src.lua;
        index = src.index;
        incTableRefCounter(lua, index);
    }
    LuaObject& operator=(const LuaObject& src) {
        LuaInterpreter *oldLua = lua;
        int oldIndex = index;
        lua = src.lua;
        index = src.index;
        incTableRefCounter(lua, index);
        decTableRefCounter(oldLua, oldIndex);
    }
    ~LuaObject() {
        decTableRefCounter(lua, index);
    }
    bool isValid() const {
        return lua != NULL && !isNil();
    }
    bool isNil() const {
        return lua_isnil(getL(), getStackIndex());
    }
    
    bool isBoolean() const {
        return lua_isboolean(getL(), getStackIndex());
    }
    bool isNumber() const {
        return lua_isnumber(getL(), getStackIndex());
    }
    bool isString() const {
        return lua_isstring(getL(), getStackIndex());
    }
    bool isTable() const {
        return lua_istable(getL(), getStackIndex());
    }
    bool isFunction() const {
        return lua_isfunction(getL(), getStackIndex());
    }
    LuaObject call() const;
    LuaObject call(const LuaObject& arg) const;
    LuaObject call(const LuaObjectList& args) const;
    
    const char* getTypeName() const;
    
    bool toBoolean() const {
        return lua_toboolean(getL(), getStackIndex());
    }
    double toNumber() const {
        return lua_tonumber(getL(), getStackIndex());
    }
    string toString() const {
        return string(lua_tostring(getL(), getStackIndex()),
                      lua_strlen(  getL(), getStackIndex()));
    }
    LuaObject operator[](const char* fieldName);
    LuaObject operator[](const string& fieldName);
    LuaObject operator[](int index);
    
    typedef HeapObjectArray<string> KeyList;
    KeyList::Ptr getTableKeys() const;
    
private:
    friend class LuaInterpreterAccessToLuaObject;
    LuaObject(LuaInterpreter *lua, int index) : lua(lua), index(index) {
        incTableRefCounter(lua, index);
    }
    int getStackIndex() const {
        return LuaObjectAccessToLuaInterpreter::getStackIndex(lua, index);
    }
    lua_State* getL() const {
        return LuaObjectAccessToLuaInterpreter::getL(lua);
    }
    LuaObject newLuaObject(int stackIndex) const {
        return LuaObject(lua, LuaObjectAccessToLuaInterpreter::newLuaObject(lua, stackIndex));
    }
    LuaInterpreter *lua;
    int index;
};

class LuaInterpreterAccessToLuaObject
{
protected:
    static LuaObject constructLuaObject(LuaInterpreter *lua, int index) {
        return LuaObject(lua, index);
    }
    static int getStackIndex(const LuaObject& object) {
        return object.getStackIndex();
    }
};

};

#include "LuaInterpreter.h"

#endif // LUAOBJECT_H
