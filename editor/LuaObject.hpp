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

#ifndef LUA_OBJECT_HPP
#define LUA_OBJECT_HPP

#include <new>

#include <stddef.h>
#include "String.hpp"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "HeapObject.hpp"
#include "RawPtr.hpp"
#include "HeapObjectArray.hpp"
#include "LuaStackChecker.hpp"
#include "LuaException.hpp"
#include "NonCopyable.hpp"
#include "LuaStateAccess.hpp"
#include "WeakPtr.hpp"

namespace LucED 
{

class LuaCFunctionResult;
class LuaCFunctionArguments;

class LuaInterpreter;
class LuaObjectList;
class LuaFunctionArguments;

template
<
    LuaCFunctionResult F(const LuaCFunctionArguments& args)
>
class LuaCFunction;
template
<
    class C,
    LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
>
class LuaCMethod;


class LuaObject : private LuaAccess
{
public:
    LuaObject(const LuaAccess& luaAccess)
        : LuaAccess(luaAccess)
    {
        lua_pushnil(L);
        stackIndex = lua_gettop(L);
    #ifdef DEBUG
        stackGeneration = luaStackChecker->registerAndGetGeneration(stackIndex);
    #endif
        lua_checkstack(L, 10);
    }
        
    LuaObject(const LuaObject& rhs)
        : LuaAccess(rhs)
    {
        ASSERT(rhs.stackIndex <= luaStackChecker->getHighestStackIndexForGeneration(rhs.stackGeneration));
        lua_pushvalue(L, rhs.stackIndex);
        stackIndex = lua_gettop(L);
    #ifdef DEBUG
        stackGeneration = luaStackChecker->registerAndGetGeneration(stackIndex);
    #endif
        lua_checkstack(L, 10);
    }

    LuaObject& operator=(const LuaObject& rhs)
    {
        ASSERT(isSameLuaAccess(rhs));
        ASSERT(stackIndex <= luaStackChecker->getHighestStackIndexForGeneration(stackGeneration));
        ASSERT(rhs.stackIndex <= luaStackChecker->getHighestStackIndexForGeneration(rhs.stackGeneration));
        
        lua_pushvalue(L, rhs.stackIndex);
        lua_replace(L, stackIndex);

        return *this;
    }

    ~LuaObject()
    {
        ASSERT(isCorrect());
    #ifdef DEBUG
        luaStackChecker->truncateGenerationAtStackIndex(stackGeneration, stackIndex);
    #endif
        lua_remove(L, stackIndex);
    }
    void setMetaTable(const LuaObject& rhs)
    {
        ASSERT(isCorrect());
        ASSERT(rhs.isCorrect());
        ASSERT(isSameLuaAccess(rhs));

        lua_pushvalue(L, rhs.stackIndex);
        lua_setmetatable(L, stackIndex);
    }
    
    
    bool isNil() const {
        ASSERT(isCorrect());
        return lua_isnil(L, stackIndex);
    }
    
    bool isValid() const {
        return !isNil();
    }
    
    bool isBoolean() const {
        ASSERT(isCorrect());
        return lua_isboolean(L, stackIndex);
    }
    bool isNumber() const {
        ASSERT(isCorrect());
        return lua_isnumber(L, stackIndex);
    }
    bool isString() const {
        ASSERT(isCorrect());
        return lua_isstring(L, stackIndex);
    }
    bool isTable() const {
        ASSERT(isCorrect());
        return lua_istable(L, stackIndex);
    }
    bool isFunction() const {
        ASSERT(isCorrect());
        return lua_isfunction(L, stackIndex);
    }
    bool isUserData() const {
        ASSERT(isCorrect());
        return lua_isuserdata(L, stackIndex);
    }
    void setToNil() const {
        ASSERT(isCorrect());
        lua_pushnil(L);
        lua_replace(L, stackIndex);
    }
    void setToEmptyTable() const {
        ASSERT(isCorrect());
        lua_newtable(L);
        lua_replace(L, stackIndex);
    }
    
    const char* getTypeName() const {
        ASSERT(isCorrect());
        return lua_typename(L, lua_type(L, stackIndex));
    }
    
    bool toBoolean() const {
        ASSERT(isCorrect());
        return lua_toboolean(L, stackIndex);
    }
    bool isTrue() const {
        ASSERT(isCorrect());
        return lua_toboolean(L, stackIndex);
    }
    bool isFalse() const {
        ASSERT(isCorrect());
        return !lua_toboolean(L, stackIndex);
    }

    double toNumber() const {
        ASSERT(isCorrect());
        return lua_tonumber(L, stackIndex);
    }
    String toString() const {
        ASSERT(isCorrect());
        size_t len;
        const char* ptr = lua_tolstring(L, stackIndex, &len);
        if (ptr != NULL) {
            return String(ptr, len);
        } else {
            return String();
        }
    }
    WeakPtr<HeapObject> toUserDataPtr() const {
        ASSERT(isCorrect());
        return *static_cast<WeakPtr<HeapObject>*>(lua_touserdata(L, stackIndex));
    }
    
    bool operator==(const LuaObject& rhs) const {
        ASSERT(isCorrect());
        ASSERT(rhs.isCorrect());
        ASSERT(isSameLuaAccess(rhs));
        return lua_equal(L, stackIndex, rhs.stackIndex);
    }
    bool operator!=(const LuaObject& rhs) const {
        return !(*this == rhs);
    }
    bool operator==(const char* rhs) const {
        ASSERT(isCorrect());
        int rhsLength = strlen(rhs);
        return isString()
            && rhsLength == lua_strlen(L, stackIndex)
            && memcmp(lua_tostring(L, stackIndex), rhs, rhsLength) == 0;
    }
    bool operator!=(const char* rhs) const {
        return !(*this == rhs);
    }
    
    bool operator==(const String& rhs) const {
        ASSERT(isCorrect());
        return isString() 
            && rhs.getLength() == lua_strlen(L, stackIndex)
            && memcmp(lua_tostring(L, stackIndex), rhs.toCString(), rhs.getLength()) == 0;
    }
    bool operator!=(const String& rhs) const {
        return !(*this == rhs);
    }
    
    template<class KeyType>
    class LuaObjectTableElementRef : private LuaAccess
    {
    public:

        void operator=(const LuaObject& newValue) {
            ASSERT(isCorrect());
            ASSERT(newValue.isCorrect());
            ASSERT(isSameLuaAccess(newValue));
            push(key);
            lua_pushvalue(L, newValue.stackIndex);
            lua_settable( L, tableStackIndex);
        }
        template
        <
            class C,
            LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
        >
        void operator=(const LuaCMethod<C,M>& newValue);
        
        template
        <
            LuaCFunctionResult F(const LuaCFunctionArguments& args)
        >
        void operator=(const LuaCFunction<F>& newValue);

        operator LuaObject() {
            ASSERT(isCorrect());
            push(key);
            lua_gettable( L, tableStackIndex);
            return LuaObject(*this, lua_gettop(L));
        }
        bool isNil() const {
            ASSERT(isCorrect());
            push(key);
            lua_gettable(L, tableStackIndex);
            bool rslt = lua_isnil(L, -1);
            lua_pop(L, 1);
            return rslt;
        }
        bool isValid() const {
            return !isNil();
        }

        bool isBoolean() const {
            ASSERT(isCorrect());
            push(key);
            lua_gettable(L, tableStackIndex);
            bool rslt = lua_isboolean(L, -1);
            lua_pop(L, 1);
            return rslt;
        }
        bool isNumber() const {
            ASSERT(isCorrect());
            push(key);
            lua_gettable(L, tableStackIndex);
            bool rslt = lua_isnumber(L, -1);
            lua_pop(L, 1);
            return rslt;
        }
        bool isString() const {
            ASSERT(isCorrect());
            push(key);
            lua_gettable(L, tableStackIndex);
            bool rslt = lua_isstring(L, -1);
            lua_pop(L, 1);
            return rslt;
        }
        bool isTable() const {
            ASSERT(isCorrect());
            push(key);
            lua_gettable(L, tableStackIndex);
            bool rslt = lua_istable(L, -1);
            lua_pop(L, 1);
            return rslt;
        }
        bool isFunction() const {
            ASSERT(isCorrect());
            push(key);
            lua_gettable(L, tableStackIndex);
            bool rslt = lua_isfunction(L, -1);
            lua_pop(L, 1);
            return rslt;
        }

        bool toBoolean() const {
            ASSERT(isCorrect());
            push(key);
            lua_gettable(L, tableStackIndex);
            bool rslt = lua_toboolean(L, -1);
            lua_pop(L, 1);
            return rslt;
        }
        bool isTrue() const {
            return toBoolean();
        }
        bool isFalse() const {
            return !toBoolean();
        }
        double toNumber() const {
            ASSERT(isCorrect());
            push(key);
            lua_gettable(L, tableStackIndex);
            double rslt = lua_tonumber(L, -1);
            lua_pop(L, 1);
            return rslt;
        }
        String toString() const {
            ASSERT(isCorrect());
            push(key);
            lua_gettable(L, tableStackIndex);
            String rslt = String(lua_tostring(L, -1),
                                 lua_strlen(  L, -1));
            lua_pop(L, 1);
            return rslt;
        }
        bool operator==(const LuaObject& rhs) const {
            ASSERT(isCorrect());
            ASSERT(rhs.isCorrect());
            ASSERT(isSameLuaAccess(rhs));
            push(key);
            lua_gettable(L, tableStackIndex);
            bool rslt = lua_equal(L, -1, rhs.stackIndex);
            lua_pop(L, 1);
            return rslt;
        }
        bool operator!=(const LuaObject& rhs) const {
            return !(*this == rhs);
        }
        bool operator==(const char* rhs) const {
            ASSERT(isCorrect());
            push(key);
            lua_gettable(L, tableStackIndex);
            int rhsLength = strlen(rhs);
            bool rslt = isString()
                     && rhsLength == lua_strlen(L, -1)
                     && memcmp(lua_tostring(L, -1), rhs, rhsLength) == 0;
            lua_pop(L, 1);
            return rslt;
        }
        bool operator!=(const char* rhs) const {
            return !(*this == rhs);
        }

        bool operator==(const String& rhs) const {
            ASSERT(isCorrect());
            push(key);
            lua_gettable(L, tableStackIndex);
            bool rslt = isString() 
                && rhs.getLength() == lua_strlen(L, -1)
                && memcmp(lua_tostring(L, -1), rhs.toCString(), rhs.getLength()) == 0;
            lua_pop(L, 1);
            return rslt;
        }
        bool operator!=(const String& rhs) const {
            return !(*this == rhs);
        }
        
        LuaStoredObjectReference store() const;

    #ifdef DEBUG
        bool isCorrect() const {
            return LuaAccess::isCorrect();
        }
    #endif
    private:
        friend class LuaObject;

        LuaObjectTableElementRef(const LuaAccess& luaAccess, int tableStackIndex, KeyType key)
            : LuaAccess(luaAccess),
              tableStackIndex(tableStackIndex),
              key(key)
        {
            ASSERT(isCorrect());
        }
        LuaObjectTableElementRef(const LuaObjectTableElementRef&);
        
        int     tableStackIndex;
        KeyType key;
    };
    
    LuaObjectTableElementRef<const char*> operator[](const char* fieldName) {
        ASSERT(isCorrect());
        return LuaObjectTableElementRef<const char*>(*this, stackIndex, fieldName);
    }

    LuaObjectTableElementRef<const String&> operator[](const String& fieldName) {
        ASSERT(isCorrect());
        return LuaObjectTableElementRef<const String&>(*this, stackIndex, fieldName);
    }

    LuaObjectTableElementRef<int> operator[](int index) {
        ASSERT(isCorrect());
        return LuaObjectTableElementRef<int>(*this, stackIndex, index);
    }

    LuaObjectTableElementRef<const LuaObject&> operator[](const LuaObject& key) {
        ASSERT(isCorrect());
        return LuaObjectTableElementRef<const LuaObject&>(*this, stackIndex, key);
    }
    
    LuaObject call(LuaFunctionArguments& args);
    LuaObject call(const LuaObject& arg);
    LuaObject call();


    RawPtr<LuaInterpreter> getLuaInterpreter() const;

    LuaAccess getLuaAccess() const {
        ASSERT(isCorrect());
        return *this;
    }

    LuaStoredObjectReference store() const;

#ifdef DEBUG
    bool isCorrect() const {
        return    (LuaAccess::isCorrect())
               && (stackIndex <= luaStackChecker->getHighestStackIndexForGeneration(stackGeneration));
    }
#endif

private:
    friend class LuaInterpreter;
    friend class LuaAccess;
    friend class LuaIterator;
    friend class LuaStoredObject;
    friend class LuaCFunctionArguments;
    friend class LuaCFunctionResult;
    friend class LuaFunctionArguments;
    friend class LuaObjectList;
    friend class ObjectArray<LuaObject>;
    
    explicit LuaObject(const LuaAccess& luaAccess, int stackIndex)
        : LuaAccess(luaAccess),
          stackIndex(stackIndex)
    {
    #ifdef DEBUG
        stackGeneration = luaStackChecker->registerAndGetGeneration(stackIndex);
    #endif
        lua_checkstack(L, 10);
        ASSERT(isCorrect());
    }
    
    
    int stackIndex;
#ifdef DEBUG
    int        stackGeneration;
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

#include "LuaFunctionArguments.hpp"
#include "LuaCMethod.hpp"
#include "LuaCFunction.hpp"
#include "LuaCFunctionArguments.hpp"
#include "LuaCFunctionResult.hpp"
#include "LuaStoredObjectReference.hpp"
#include "LuaInterpreter.hpp"

namespace LucED
{

inline LuaObject LuaObject::call(LuaFunctionArguments& args)
{
    ASSERT(isCorrect());
    ASSERT(args.isCorrect());

    lua_checkstack(L, 10);
    lua_pushvalue(L, stackIndex);

    lua_replace(L, -args.getLength() - 2);

    int error = lua_pcall(L, args.getLength(), 1, 0);

    LuaFunctionArguments::LuaObjectAccess::clearAfterCall(args);

    if (error != 0)
    {
        LuaException ex(lua_tostring(L, -1));
        lua_pop(L, 1);
        throw ex;
    } else {    
        return LuaObject(*this, lua_gettop(L));
    }
}

inline LuaObject LuaObject::call(const LuaObject& arg)
{
    ASSERT(isCorrect());
    ASSERT(arg.isCorrect());
    ASSERT(isSameLuaAccess(arg));
    
    LuaFunctionArguments args(getLuaAccess());
    args << arg;
    return call(args);
}

inline LuaObject LuaObject::call()
{
    ASSERT(isCorrect());

    lua_checkstack(L, 10);
    lua_pushvalue(L, stackIndex);

    int error = lua_pcall(L, 0, 1, 0);

    if (error != 0)
    {
        LuaException ex(lua_tostring(L, -1));
        lua_pop(L, 1);
        throw ex;
    } else {    
        return LuaObject(*this, lua_gettop(L));
    }
}

template
<
    class KeyType
>
template
<
    LuaCFunctionResult F(const LuaCFunctionArguments& args)
>
inline void LuaObject::LuaObjectTableElementRef<KeyType>::operator=(const LuaCFunction<F>& newValue)
{
    ASSERT(isCorrect());
    push(key);
    lua_pushcfunction(L, LuaCFunction<F>::invokeFunction);
    lua_settable( L, tableStackIndex);
}

template
<
    class KeyType
>
template
<
    class C,
    LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
>
inline void LuaObject::LuaObjectTableElementRef<KeyType>::operator=(const LuaCMethod<C,M>& newValue)
{
    ASSERT(isCorrect());
    push(key);
    lua_pushcfunction(L, &(LuaCMethod<C,M>::invokeFunction));
    lua_settable( L, tableStackIndex);
}

inline LuaStoredObjectReference LuaObject::store() const
{
    ASSERT(isCorrect());
    return LuaAccess::store(*this);
}

template
<
    class KeyType
>
inline LuaStoredObjectReference LuaObject::LuaObjectTableElementRef<KeyType>::store() const
{
    ASSERT(isCorrect());
    return LuaAccess::store(*this);
}

inline RawPtr<LuaInterpreter> LuaObject::getLuaInterpreter() const
{
    ASSERT(isCorrect());
    return LuaStateAccess::getLuaInterpreter(L);
}

} // namespace LucED

#endif // LUA_OBJECT_HPP
