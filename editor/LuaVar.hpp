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

#ifndef LUA_VAR_HPP
#define LUA_VAR_HPP

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

#include "StackTrace.hpp"
#undef PRINT_STACK_TRACE

namespace LucED 
{

class LuaVar;
class LuaCFunctionResult;
class LuaCFunctionArguments;

class LuaInterpreter;
class LuaVarList;
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


template<class KeyType>
class LuaObjectTableElementRef : private LuaAccess
{
public:
    
    template<class T
            >
    void operator=(const T& newValue) {
        ASSERT(isCorrect());
        push(key);
        push(newValue);
        lua_settable(L, tableStackIndex);
    }

    void setNil() {
        ASSERT(isCorrect());
        push(key);
        lua_pushnil(L);
        lua_settable(L, tableStackIndex);
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
    long toLong() const {
        ASSERT(isCorrect());
        push(key);
        lua_gettable(L, tableStackIndex);
        long rslt = lua_tointeger(L, -1);
        lua_pop(L, 1);
        return rslt;
    }
    int toInt() const {
        ASSERT(isCorrect());
        push(key);
        lua_gettable(L, tableStackIndex);
        int rslt = lua_tointeger(L, -1);
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
    bool operator==(const LuaVarRef& rhs) const;
    bool operator!=(const LuaVarRef& rhs) const;

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
    
    LuaAccess getLuaAccess() const {
        return *this;
    }
    
    LuaStoredObjectReference store() const;
    
#ifdef DEBUG
    bool isCorrect() const {
        ASSERT(LuaAccess::isCorrect());
        ASSERT(tableStackIndex <= getLuaStackChecker()->getHighestStackIndexForGeneration(stackGeneration));
        ASSERT(isCorrect(key));
        return true;
    }
#endif

        LuaObjectTableElementRef(const LuaObjectTableElementRef&);

private:
    friend class LuaVarRef;
    friend class LuaVar;
    friend class LuaAccess;

#ifdef DEBUG
    static bool isCorrect(const KeyType& rhs) {
        return true;
    }
#endif

#ifdef DEBUG
    LuaObjectTableElementRef(const LuaAccess& luaAccess, int tableStackIndex, int stackGeneration, KeyType key)
        : LuaAccess(luaAccess),
          tableStackIndex(tableStackIndex),
          key(key),
          stackGeneration(stackGeneration)
    {
        ASSERT(isCorrect());
    }
#else
    LuaObjectTableElementRef(const LuaAccess& luaAccess, int tableStackIndex, KeyType key)
        : LuaAccess(luaAccess),
          tableStackIndex(tableStackIndex),
          key(key)
    {
        ASSERT(isCorrect());
    }
#endif
    
    int     tableStackIndex;
    KeyType key;
#ifdef DEBUG
    int     stackGeneration;
#endif
};

class LuaVarRef : protected LuaAccess
{
public:
    void setMetaTable(const LuaVarRef& rhs)
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
    long toLong() const {
        ASSERT(isCorrect());
        return (long)(lua_tointeger(L, stackIndex));
    }
    int toInt() const {
        ASSERT(isCorrect());
        return (long)(lua_tointeger(L, stackIndex));
    }
    template<class T
            >
    bool is() const;
    
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
    const char* getStringPtr() const {
        ASSERT(isCorrect());
        ASSERT(isString());
        size_t len;
        const char* ptr = lua_tolstring(L, stackIndex, &len);
        return ptr;
    }
    long getStringLength() const {
        ASSERT(isCorrect());
        ASSERT(isString());
        size_t len;
        const char* ptr = lua_tolstring(L, stackIndex, &len);
        return len;
    }
    bool isPtr() const {
        ASSERT(isCorrect());
        LuaAccess::UserData* userDataPtr = static_cast<UserData*>(lua_touserdata(L, stackIndex));
        return (   userDataPtr != NULL 
                && userDataPtr->magic == LuaAccess::MAGIC);
    }
    bool isOwningPtr() const {
        ASSERT(isCorrect());
        LuaAccess::UserData* userDataPtr = static_cast<UserData*>(lua_touserdata(L, stackIndex));
        return (   userDataPtr != NULL 
                && userDataPtr->magic == LuaAccess::MAGIC 
                && userDataPtr->isOwningPtr);
    }
    void* getRawPtrPtr() const {
        ASSERT(isCorrect());
        LuaAccess::UserData* userDataPtr = static_cast<UserData*>(lua_touserdata(L, stackIndex));
        if (    userDataPtr != NULL 
            &&  userDataPtr->magic == LuaAccess::MAGIC)
        {
            return userDataPtr + 1;
        }
        else {
            return NULL;
        }
    }
    bool isWeakPtr() const {
        ASSERT(isCorrect());
        LuaAccess::UserData* userDataPtr = static_cast<UserData*>(lua_touserdata(L, stackIndex));
        return (    userDataPtr != NULL 
                &&  userDataPtr->magic == LuaAccess::MAGIC 
                && !userDataPtr->isOwningPtr);
    }
    WeakPtr<HeapObject> toWeakPtr() const {
        ASSERT(isCorrect());
        LuaAccess::UserData* userDataPtr = static_cast<UserData*>(lua_touserdata(L, stackIndex));
        if (userDataPtr != NULL && userDataPtr->magic == LuaAccess::MAGIC) {
            if (userDataPtr->isOwningPtr) {
                return *static_cast<OwningPtr<HeapObject>*>(static_cast<void*>(userDataPtr + 1));
            } else {
                return *static_cast<WeakPtr<HeapObject>*>(static_cast<void*>(userDataPtr + 1));
            }
        } else {
            return WeakPtr<HeapObject>();
        }
    }
    OwningPtr<HeapObject> toOwningPtr() const {
        ASSERT(isCorrect());
        LuaAccess::UserData* userDataPtr = static_cast<UserData*>(lua_touserdata(L, stackIndex));
        if (userDataPtr != NULL && userDataPtr->magic == LuaAccess::MAGIC) {
            if (userDataPtr->isOwningPtr) {
                return *static_cast<OwningPtr<HeapObject>*>(static_cast<void*>(userDataPtr + 1));
            } else {
                return OwningPtr<HeapObject>();
            }
        } else {
            return OwningPtr<HeapObject>();
        }
    }
    
    bool operator==(const LuaVarRef& rhs) const {
        ASSERT(isCorrect());
        ASSERT(rhs.isCorrect());
        ASSERT(isSameLuaAccess(rhs));
        return lua_equal(L, stackIndex, rhs.stackIndex);
    }
    bool operator!=(const LuaVarRef& rhs) const {
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
    
    LuaObjectTableElementRef<void*> operator[](void* key) const {
        ASSERT(isCorrect());
    #ifdef DEBUG
        return LuaObjectTableElementRef<void*>(*this, stackIndex, stackGeneration, key);
    #else
        return LuaObjectTableElementRef<void*>(*this, stackIndex, key);
    #endif
    }

    LuaObjectTableElementRef<const char*> operator[](const char* fieldName) const {
        ASSERT(isCorrect());
    #ifdef DEBUG
        return LuaObjectTableElementRef<const char*>(*this, stackIndex, stackGeneration, fieldName);
    #else
        return LuaObjectTableElementRef<const char*>(*this, stackIndex, fieldName);
    #endif
    }

    LuaObjectTableElementRef<const String> operator[](const String& fieldName) const {
        ASSERT(isCorrect());
    #ifdef DEBUG
        return LuaObjectTableElementRef<const String>(*this, stackIndex, stackGeneration, fieldName);
    #else
        return LuaObjectTableElementRef<const String>(*this, stackIndex, fieldName);
    #endif
    }

    LuaObjectTableElementRef<int> operator[](int index) const {
        ASSERT(isCorrect());
    #ifdef DEBUG
        return LuaObjectTableElementRef<int>(*this, stackIndex, stackGeneration, index);
    #else
        return LuaObjectTableElementRef<int>(*this, stackIndex, index);
    #endif
    }

    LuaObjectTableElementRef<LuaVarRef> operator[](const LuaVarRef& key) const {
        ASSERT(isCorrect());
    #ifdef DEBUG
        return LuaObjectTableElementRef<LuaVarRef>(*this, stackIndex, stackGeneration, key);
    #else
        return LuaObjectTableElementRef<LuaVarRef>(*this, stackIndex, key);
    #endif
    }
    
    template< class T
            >
    LuaVar call(const T& argument);

    LuaVar call(LuaFunctionArguments& args);

    LuaVar call();

    
    RawPtr<LuaInterpreter> getLuaInterpreter() const;

    LuaAccess getLuaAccess() const {
        ASSERT(isCorrect());
        return *this;
    }

    LuaStoredObjectReference store() const;

#ifdef DEBUG
    bool isCorrect() const {
        ASSERT(LuaAccess::isCorrect());
        ASSERT(stackIndex <= getLuaStackChecker()->getHighestStackIndexForGeneration(stackGeneration));
        return true;
    }
#endif

protected:
    friend class LuaInterpreter;
    friend class LuaAccess;
    friend class LuaIterator;
    friend class LuaStoredObject;
    friend class LuaCFunctionArguments;
    friend class LuaCFunctionResult;
    friend class LuaFunctionArguments;
    friend class LuaVarList;
    friend class ObjectArray<LuaVar>;
    friend class LuaVar;
    
    explicit LuaVarRef(const LuaAccess& luaAccess)
        : LuaAccess(luaAccess)
    {
    #ifdef DEBUG
        stackIndex = 0;
        stackGeneration = -1;
    #endif
    }
        
#ifdef DEBUG
    explicit LuaVarRef(const LuaAccess& luaAccess, int stackIndex, int stackGeneration)
        : LuaAccess(luaAccess),
          stackIndex(stackIndex),
          stackGeneration(stackGeneration)
    {
        ASSERT(isCorrect());
    }
#else
    explicit LuaVarRef(const LuaAccess& luaAccess, int stackIndex)
        : LuaAccess(luaAccess),
          stackIndex(stackIndex)
    {
        ASSERT(isCorrect());
    }
#endif
    
    
    int stackIndex;
#ifdef DEBUG
    int        stackGeneration;
#endif
};

template<>
inline bool LuaVarRef::is<String>() const
{
    return isString();
}

template<>
inline bool LuaVarRef::is<int>() const
{
    return isNumber();
}

template<>
inline bool LuaVarRef::is<long>() const
{
    return isNumber();
}


template<class KeyType
        >
bool LuaObjectTableElementRef<KeyType>::operator==(const LuaVarRef& rhs) const
{
    ASSERT(isCorrect());
    ASSERT(rhs.isCorrect());
    ASSERT(isSameLuaAccess(rhs));
    push(key);
    lua_gettable(L, tableStackIndex);
    bool rslt = lua_equal(L, -1, rhs.stackIndex);
    lua_pop(L, 1);
    return rslt;
}
template<class KeyType
        >
bool LuaObjectTableElementRef<KeyType>::operator!=(const LuaVarRef& rhs) const 
{
    return !(*this == rhs);
}


class LuaVar : public LuaVarRef
{
public:
    explicit LuaVar(const LuaAccess& luaAccess)
        : LuaVarRef(luaAccess)
    {
        lua_pushnil(L);
        stackIndex = lua_gettop(L);
    #ifdef DEBUG
        stackGeneration = getLuaStackChecker()->registerAndGetGeneration(stackIndex);
    #endif
#ifdef PRINT_STACK_TRACE
StackTrace::print();
printf("%p %d (%d)\n", this, stackIndex, stackGeneration);
#endif
    }
        
    LuaVar(const LuaVar& rhs)
        : LuaVarRef(rhs.getLuaAccess())
    {
        ASSERT(rhs.isCorrect());
        lua_pushvalue(L, rhs.stackIndex);
        stackIndex = lua_gettop(L);
    #ifdef DEBUG
        stackGeneration = getLuaStackChecker()->registerAndGetGeneration(stackIndex);
    #endif
#ifdef PRINT_STACK_TRACE
StackTrace::print();
printf("%p %d (%d)\n", this, stackIndex, stackGeneration);
#endif
    }

    LuaVar(const LuaVarRef& rhs)
        : LuaVarRef(rhs.getLuaAccess())
    {
        ASSERT(rhs.isCorrect());
        lua_pushvalue(L, rhs.stackIndex);
        stackIndex = lua_gettop(L);
    #ifdef DEBUG
        stackGeneration = getLuaStackChecker()->registerAndGetGeneration(stackIndex);
    #endif
#ifdef PRINT_STACK_TRACE
StackTrace::print();
printf("%p %d (%d)\n", this, stackIndex, stackGeneration);
#endif
    }

    template<class KeyType
            >
    LuaVar(const LuaObjectTableElementRef<KeyType>& rhs)
        : LuaVarRef(rhs.getLuaAccess())
    {
        ASSERT(rhs.isCorrect());
        push(rhs);
        stackIndex = lua_gettop(L);
    #ifdef DEBUG
        stackGeneration = getLuaStackChecker()->registerAndGetGeneration(stackIndex);
    #endif
#ifdef PRINT_STACK_TRACE
StackTrace::print();
printf("%p %d (%d)\n", this, stackIndex, stackGeneration);
#endif
    }
    
    LuaVar& operator=(const LuaVar& rhs)
    {
        ASSERT(isCorrect());
        ASSERT(isSameLuaAccess(rhs));
        ASSERT(rhs.isCorrect());
        
        lua_pushvalue(L, rhs.stackIndex);
        lua_replace(L, stackIndex);

#ifdef PRINT_STACK_TRACE
StackTrace::print();
printf("%p %d (%d)\n", this, stackIndex, stackGeneration);
#endif
        return *this;
    }


    template<class T
            >
    LuaVar& assign(const T& rhs) {
        ASSERT(isCorrect());

        push(rhs);
        lua_replace(L, stackIndex);
        return *this;
    }
    
    LuaVar& assign(const char* ptr, long length) {
        ASSERT(isCorrect());

        push(ptr, length);
        lua_replace(L, stackIndex);
        return *this;
    }

    template<class T
            >
    LuaVar& operator=(const T& rhs) {
        return assign(rhs);
    }

    LuaVar& setNil() {
        ASSERT(isCorrect());
        lua_pushnil(L);
        lua_replace(L, stackIndex);
        return *this;
    }

#if 0
    LuaVar& operator=(const LuaVarRef& rhs)
    {
        ASSERT(isCorrect());
        ASSERT(isSameLuaAccess(rhs));
        ASSERT(rhs.isCorrect());
        
        push(rhs);
        lua_replace(L, stackIndex);

#ifdef PRINT_STACK_TRACE
StackTrace::print();
printf("%p %d (%d)\n", this, stackIndex, stackGeneration);
#endif
        return *this;
    }

    template<class KeyType
            >
    LuaVar& operator=(const LuaObjectTableElementRef<KeyType>& rhs)
    {
        ASSERT(isCorrect());
        ASSERT(isSameLuaAccess(rhs));
        ASSERT(rhs.isCorrect());

        push(rhs);
        lua_replace(L, stackIndex);

#ifdef PRINT_STACK_TRACE
StackTrace::print();
printf("%p %d (%d)\n", this, stackIndex, stackGeneration);
#endif
        return *this;
    }
#endif

    ~LuaVar()
    {
#ifdef PRINT_STACK_TRACE
StackTrace::print();
printf("%p %d (%d)\n", this, stackIndex, stackGeneration);
#endif
        ASSERT(isCorrect());
        lua_unuse(L, stackIndex);
    #ifdef DEBUG
        getLuaStackChecker()->truncateGenerationAtStackIndex(stackGeneration, lua_gettop(L) + 1);
    #endif
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
    
#ifdef DEBUG
    bool isCorrect() const {
        return (LuaVarRef::isCorrect());
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
    friend class LuaVarList;
    friend class ObjectArray<LuaVar>;
    friend class LuaVarRef;

    template<class KeyType
            >
    friend class LuaObjectTableElementRef;
    
    explicit LuaVar(const LuaAccess& luaAccess, int stackIndex)
        : LuaVarRef(luaAccess)
    {
        this->stackIndex = stackIndex;
    #ifdef DEBUG
        stackGeneration = getLuaStackChecker()->registerAndGetGeneration(stackIndex);
    #endif
        ASSERT(isCorrect());
#ifdef PRINT_STACK_TRACE
StackTrace::print();
printf("%p %d (%d)\n", this, stackIndex, stackGeneration);
#endif
    }
    
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool operator==(const char* lhs, const LuaVarRef& rhs)
{
    return rhs == lhs;
}

inline bool operator!=(const char* lhs, const LuaVarRef& rhs)
{
    return rhs != lhs;
}

inline bool operator==(const String& lhs, const LuaVarRef& rhs)
{
   return rhs == lhs;
}

inline bool operator!=(const String& lhs, const LuaVarRef& rhs)
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

inline LuaVar LuaVarRef::call(LuaFunctionArguments& args)
{
    ASSERT(isCorrect());
    ASSERT(args.isCorrect());

    lua_pushvalue(L, stackIndex);
    lua_replace  (L, -args.getLength() - 2);
    
    const int numberOfResults = 1;
    int error = lua_pcall(L, args.getLength(), numberOfResults, 0);

    if (error != 0)
    {
        LuaException ex(lua_tostring(L, -1));
        lua_pop(L, 1);
        LuaFunctionArguments::LuaObjectAccess::clearAfterCall(args, 0);
        throw ex;
    }
    else {
        LuaFunctionArguments::LuaObjectAccess::clearAfterCall(args, numberOfResults);
        return LuaVar(getLuaAccess(), lua_gettop(L));
    }
}

template<>
inline LuaVar LuaVarRef::call(const LuaVarRef& arg)
{
    ASSERT(isCorrect());
    ASSERT(arg.isCorrect());
    ASSERT(isSameLuaAccess(arg));
    
    return call(LuaFunctionArguments(getLuaAccess()) << arg);
}

template<>
inline LuaVar LuaVarRef::call(const LuaVar& arg)
{
    return call<LuaVarRef>(arg);
}


template< class T
        >
inline LuaVar LuaVarRef::call(const T& arg)
{
    ASSERT(isCorrect());
    
    return call(LuaFunctionArguments(getLuaAccess()) << arg);
}


inline LuaVar LuaVarRef::call()
{
    ASSERT(isCorrect());

    lua_pushvalue(L, stackIndex);

    int error = lua_pcall(L, 0, 1, 0);

    if (error != 0)
    {
        LuaException ex(lua_tostring(L, -1));
        lua_pop(L, 1);
        throw ex;
    } else {    
        return LuaVar(getLuaAccess(), lua_gettop(L));
    }
}


inline LuaStoredObjectReference LuaVarRef::store() const
{
    ASSERT(isCorrect());
    return LuaAccess::store(*this);
}

template
<
    class KeyType
>
inline LuaStoredObjectReference LuaObjectTableElementRef<KeyType>::store() const
{
    ASSERT(isCorrect());
    return LuaAccess::store(*this);
}


inline RawPtr<LuaInterpreter> LuaVarRef::getLuaInterpreter() const
{
    ASSERT(isCorrect());
    return LuaStateAccess::getLuaInterpreter(L);
}

#ifdef DEBUG
template<
        >
inline bool LuaObjectTableElementRef<LuaVarRef>::isCorrect(const LuaVarRef& rhs)
{
    return rhs.isCorrect();
}
#endif

} // namespace LucED

#endif // LUA_VAR_HPP
