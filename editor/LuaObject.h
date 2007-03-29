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
#include "LuaStackChecker.h"

namespace LucED 
{

using std::string;

class LuaInterpreter;
class LuaObjectList;

class LuaObject
{
public:
    static const LuaObject none;
    
    LuaObject() {
        lua_pushnil(L);
        stackIndex = lua_gettop(L);
    #ifdef DEBUG
        stackGeneration = LuaStackChecker::getInstance()->registerAndGetGeneration(stackIndex);
    #endif
        lua_checkstack(L, 10);
    }
        
    LuaObject(const LuaObject& rhs) {
        lua_pushvalue(L, rhs.stackIndex);
        stackIndex = lua_gettop(L);
    #ifdef DEBUG
        stackGeneration = LuaStackChecker::getInstance()->registerAndGetGeneration(stackIndex);
    #endif
        lua_checkstack(L, 10);
    }

    LuaObject& operator=(const LuaObject& rhs) {
        lua_pushvalue(L, rhs.stackIndex);
        lua_replace(L, stackIndex);
        return *this;
    }

    ~LuaObject() {
    #ifdef DEBUG
        LuaStackChecker::getInstance()->truncateGenerationAtStackIndex(stackGeneration, stackIndex);
    #endif
        lua_remove(L, stackIndex);
    }
    bool isValid() const {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        return stackIndex != -1 && !isNil();
    }
    bool isNil() const {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        return lua_isnil(L, stackIndex);
    }
    
    bool isBoolean() const {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        return lua_isboolean(L, stackIndex);
    }
    bool isNumber() const {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        return lua_isnumber(L, stackIndex);
    }
    bool isString() const {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        return lua_isstring(L, stackIndex);
    }
    bool isTable() const {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        return lua_istable(L, stackIndex);
    }
    bool isFunction() const {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        return lua_isfunction(L, stackIndex);
    }
    void setNil() const {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        lua_pushnil(L);
        lua_replace(L, stackIndex);
    }
    LuaObject call() const;
    LuaObject call(const LuaObject& arg) const;
    LuaObject call(const LuaObjectList& args) const;
    
    const char* getTypeName() const {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        return lua_typename(L, lua_type(L, stackIndex));
    }
    
    bool toBoolean() const {
        return lua_toboolean(L, stackIndex);
    }
    double toNumber() const {
        return lua_tonumber(L, stackIndex);
    }
    string toString() const {
        return string(lua_tostring(L, stackIndex),
                      lua_strlen(  L, stackIndex));
    }
    
    bool operator==(const char* rhs) const {
        int rhsLength = strlen(rhs);
        return rhsLength == lua_strlen(L, stackIndex)
            && memcmp(lua_tostring(L, stackIndex), rhs, rhsLength) == 0;
    }
    bool operator!=(const char* rhs) const {
        return !(*this == rhs);
    }
    
    bool operator==(const string& rhs) const {
        return rhs.length() == lua_strlen(L, stackIndex)
            && memcmp(lua_tostring(L, stackIndex), rhs.c_str(), rhs.length()) == 0;
    }
    bool operator!=(const string& rhs) const {
        return !(*this == rhs);
    }
    
    LuaObject operator[](const char* fieldName) {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        lua_pushstring(L, fieldName);
        lua_gettable(L, stackIndex);
        return LuaObject(lua_gettop(L));
    }

    LuaObject operator[](const string& fieldName) {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        lua_pushlstring(L, fieldName.c_str(), fieldName.length());
        lua_gettable(L, stackIndex);
        return LuaObject(lua_gettop(L));
    }

    LuaObject operator[](int index) {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        lua_pushnumber(L, index);
        lua_gettable(L, stackIndex);
        return LuaObject(lua_gettop(L));
    }

    
private:
    friend class LuaInterpreter;
    friend class LuaIterator;
    
    LuaObject(int stackIndex) : stackIndex(stackIndex)
    {
    #ifdef DEBUG
        stackGeneration = LuaStackChecker::getInstance()->registerAndGetGeneration(stackIndex);
    #endif
        lua_checkstack(L, 10);
    }
    
    static lua_State* L;
    
    int stackIndex;
#ifdef DEBUG
    int stackGeneration;
#endif
};

inline bool operator==(const char* lhs, const LuaObject& rhs)
{
    return rhs == lhs;
}

inline bool operator!=(const char* lhs, const LuaObject& rhs)
{
    return rhs != lhs;
}

inline bool operator==(const string& lhs, const LuaObject& rhs)
{
   return rhs == lhs;
}

inline bool operator!=(const string& lhs, const LuaObject& rhs)
{
   return rhs != lhs;
}


} // namespace LucED

#endif // LUAOBJECT_H
