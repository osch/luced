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
#include "String.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "HeapObject.h"
#include "HeapObjectArray.h"
#include "LuaStackChecker.h"
#include "LuaException.h"
#include "NonCopyable.h"

namespace LucED 
{



template<class ImplFunction> class LuaCFunction;

class LuaInterpreter;
class LuaObjectList;
class LuaFunctionArguments;



class LuaObject
{
public:

    static LuaObject Table() {
        lua_newtable(L);
        return LuaObject(lua_gettop(L));
    }

    static LuaObject Nil() {
        lua_pushnil(L);
        return LuaObject(lua_gettop(L));
    }

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

    LuaObject(const String& rhs) {
        lua_pushlstring(L, rhs.toCString(), rhs.getLength());
        stackIndex = lua_gettop(L);
    #ifdef DEBUG
        stackGeneration = LuaStackChecker::getInstance()->registerAndGetGeneration(stackIndex);
    #endif
        lua_checkstack(L, 10);
    }

    template<class ImplFunction> 
    LuaObject(LuaCFunction<ImplFunction> f);

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
    void setToNil() const {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        lua_pushnil(L);
        lua_replace(L, stackIndex);
    }
    void setToEmptyTable() const {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        lua_newtable(L);
        lua_replace(L, stackIndex);
    }
    
    const char* getTypeName() const {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        return lua_typename(L, lua_type(L, stackIndex));
    }
    
    bool toBoolean() const {
        return lua_toboolean(L, stackIndex);
    }
    bool isTrue() const {
        return lua_toboolean(L, stackIndex);
    }
    bool isFalse() const {
        return !lua_toboolean(L, stackIndex);
    }

    double toNumber() const {
        return lua_tonumber(L, stackIndex);
    }
    String toString() const {
        size_t len;
        const char* ptr = lua_tolstring(L, stackIndex, &len);
        if (ptr != NULL) {
            return String(ptr, len);
        } else {
            return String();
        }
    }
    
    bool operator==(const LuaObject& rhs) const {
        ASSERT(    stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(    stackGeneration));
        ASSERT(rhs.stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(rhs.stackGeneration));
        return lua_equal(L, stackIndex, rhs.stackIndex);
    }
    bool operator!=(const LuaObject& rhs) const {
        return !(*this == rhs);
    }
    bool operator==(const char* rhs) const {
        int rhsLength = strlen(rhs);
        return isString()
            && rhsLength == lua_strlen(L, stackIndex)
            && memcmp(lua_tostring(L, stackIndex), rhs, rhsLength) == 0;
    }
    bool operator!=(const char* rhs) const {
        return !(*this == rhs);
    }
    
    bool operator==(const String& rhs) const {
        return isString() 
            && rhs.getLength() == lua_strlen(L, stackIndex)
            && memcmp(lua_tostring(L, stackIndex), rhs.toCString(), rhs.getLength()) == 0;
    }
    bool operator!=(const String& rhs) const {
        return !(*this == rhs);
    }
    
    template<class KeyType>
    class LuaObjectTableElementRef
    {
    public:

        void operator=(const LuaObject& newValue) {
            push(key);
            lua_pushvalue(LuaObject::L, newValue.stackIndex);
            lua_settable( LuaObject::L, tableStackIndex);
        }
        operator LuaObject() {
            push(key);
            lua_gettable( LuaObject::L, tableStackIndex);
            return LuaObject(lua_gettop(L));
        }
        bool isValid() const {
            push(key);
            lua_gettable(LuaObject::L, tableStackIndex);
            bool rslt = !lua_isnil(L, -1);
            lua_pop(LuaObject::L, 1);
            return rslt;
        }
        bool isNil() const {
            push(key);
            lua_gettable(LuaObject::L, tableStackIndex);
            bool rslt = lua_isnil(L, -1);
            lua_pop(LuaObject::L, 1);
            return rslt;
        }

        bool isBoolean() const {
            push(key);
            lua_gettable(LuaObject::L, tableStackIndex);
            bool rslt = lua_isboolean(L, -1);
            lua_pop(LuaObject::L, 1);
            return rslt;
        }
        bool isNumber() const {
            push(key);
            lua_gettable(LuaObject::L, tableStackIndex);
            bool rslt = lua_isnumber(L, -1);
            lua_pop(LuaObject::L, 1);
            return rslt;
        }
        bool isString() const {
            push(key);
            lua_gettable(LuaObject::L, tableStackIndex);
            bool rslt = lua_isstring(L, -1);
            lua_pop(LuaObject::L, 1);
            return rslt;
        }
        bool isTable() const {
            push(key);
            lua_gettable(LuaObject::L, tableStackIndex);
            bool rslt = lua_istable(L, -1);
            lua_pop(LuaObject::L, 1);
            return rslt;
        }
        bool isFunction() const {
            push(key);
            lua_gettable(LuaObject::L, tableStackIndex);
            bool rslt = lua_isfunction(L, -1);
            lua_pop(LuaObject::L, 1);
            return rslt;
        }

        bool toBoolean() const {
            push(key);
            lua_gettable(LuaObject::L, tableStackIndex);
            bool rslt = lua_toboolean(L, -1);
            lua_pop(LuaObject::L, 1);
            return rslt;
        }
        bool isTrue() const {
            return toBoolean();
        }
        bool isFalse() const {
            return !toBoolean();
        }
        double toNumber() const {
            push(key);
            lua_gettable(LuaObject::L, tableStackIndex);
            double rslt = lua_tonumber(L, -1);
            lua_pop(LuaObject::L, 1);
            return rslt;
        }
        String toString() const {
            push(key);
            lua_gettable(LuaObject::L, tableStackIndex);
            String rslt = String(lua_tostring(L, -1),
                                 lua_strlen(  L, -1));
            lua_pop(LuaObject::L, 1);
            return rslt;
        }
        bool operator==(const LuaObject& rhs) const {
            push(key);
            lua_gettable(LuaObject::L, tableStackIndex);
            bool rslt = lua_equal(L, -1, rhs.stackIndex);
            lua_pop(LuaObject::L, 1);
            return rslt;
        }
        bool operator!=(const LuaObject& rhs) const {
            return !(*this == rhs);
        }
        bool operator==(const char* rhs) const {
            push(key);
            lua_gettable(LuaObject::L, tableStackIndex);
            int rhsLength = strlen(rhs);
            bool rslt = isString()
                     && rhsLength == lua_strlen(L, stackIndex)
                     && memcmp(lua_tostring(L, stackIndex), rhs, rhsLength) == 0;
            lua_pop(LuaObject::L, 1);
            return rslt;
        }
        bool operator!=(const char* rhs) const {
            return !(*this == rhs);
        }

        bool operator==(const String& rhs) const {
            push(key);
            lua_gettable(LuaObject::L, tableStackIndex);
            bool rslt = isString() 
                && rhs.getLength() == lua_strlen(L, stackIndex)
                && memcmp(lua_tostring(L, stackIndex), rhs.toCString(), rhs.getLength()) == 0;
            lua_pop(LuaObject::L, 1);
            return rslt;
        }
        bool operator!=(const String& rhs) const {
            return !(*this == rhs);
        }

    private:
        friend class LuaObject;

        LuaObjectTableElementRef(int tableStackIndex, KeyType key)
            : tableStackIndex(tableStackIndex),
              key(key)
        {}
        LuaObjectTableElementRef(const LuaObjectTableElementRef&);
        
        int     tableStackIndex;
        KeyType key;
    };
    
    LuaObjectTableElementRef<const char*> operator[](const char* fieldName) {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        return LuaObjectTableElementRef<const char*>(stackIndex, fieldName);
    }

    LuaObjectTableElementRef<const String&> operator[](const String& fieldName) {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        return LuaObjectTableElementRef<const String&>(stackIndex, fieldName);
    }

    LuaObjectTableElementRef<int> operator[](int index) {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        return LuaObjectTableElementRef<int>(stackIndex, index);
    }

    LuaObjectTableElementRef<const LuaObject&> operator[](const LuaObject& key) {
        ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
        return LuaObjectTableElementRef<const LuaObject&>(stackIndex, key);
    }
    
    LuaObject call(const LuaFunctionArguments& args);
    LuaObject call();

    operator LuaFunctionArguments() const;
    
    LuaFunctionArguments operator<<(const LuaObject& rhs) const;

private:
    friend class LuaInterpreter;
    friend class LuaIterator;
    friend class LuaStoredObject;
    friend class LuaCFunctionArguments;
    friend class LuaCFunctionResult;
    friend class LuaFunctionArguments;
    friend class LuaObjectList;
    friend class ObjectArray<LuaObject>;
    
    explicit LuaObject(int stackIndex) : stackIndex(stackIndex)
    {
    #ifdef DEBUG
        stackGeneration = LuaStackChecker::getInstance()->registerAndGetGeneration(stackIndex);
    #endif
        lua_checkstack(L, 10);
    }
    
    static void push(const char* arg) {
        lua_pushstring(L, arg);
    }
    static void push(const String& arg) {
        lua_pushlstring(L, arg.toCString(), arg.getLength());
    }
    static void push(int arg) {
        lua_pushnumber(L, arg);
    }
    static void push(const LuaObject& arg) {
        lua_pushvalue(L, arg.stackIndex);
    }


    static lua_State* L;
    
    int stackIndex;
#ifdef DEBUG
    int stackGeneration;
#endif
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool operator==(const char* lhs, const LuaObject& rhs)
{
    return rhs == lhs;
}

inline bool operator!=(const char* lhs, const LuaObject& rhs)
{
    return rhs != lhs;
}

inline bool operator==(const String& lhs, const LuaObject& rhs)
{
   return rhs == lhs;
}

inline bool operator!=(const String& lhs, const LuaObject& rhs)
{
   return rhs != lhs;
}

} // namespace LucED

#include "LuaCFunction.h"
#include "LuaFunctionArguments.h"

namespace LucED
{

/**
 * Constructs LuaObject for CFunction.
 */
template<class ImplFunction>
LuaObject::LuaObject(LuaCFunction<ImplFunction>)
{
    lua_pushcfunction(L, &LuaCFunction<ImplFunction>::invokeFunction);
    stackIndex = lua_gettop(L);
#ifdef DEBUG
    stackGeneration = LuaStackChecker::getInstance()->registerAndGetGeneration(stackIndex);
#endif
    lua_checkstack(L, 10);
}


inline LuaObject LuaObject::call(const LuaFunctionArguments& args)
{
#ifdef DEBGUG
    args.checkStack();
    ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
#endif
    lua_checkstack(L, 10);
    lua_pushvalue(L, stackIndex);

    lua_replace(L, -args.getLength() - 2);

    int error = lua_pcall(L, args.getLength(), 1, 0);
    args.numberArguments = 0;

    if (error != 0)
    {
        LuaException ex(lua_tostring(L, -1));
        lua_pop(L, 1);
        throw ex;
    } else {    
        return LuaObject(lua_gettop(L));
    }
}

inline LuaObject LuaObject::call()
{
    return call(LuaFunctionArguments());
}

inline LuaObject::operator LuaFunctionArguments() const 
{
    return LuaFunctionArguments() << *this;
}

inline LuaFunctionArguments LuaObject::operator<<(const LuaObject& rhs) const
{
    return LuaFunctionArguments() << *this << rhs;
}


} // namespace LucED

#endif // LUAOBJECT_H
