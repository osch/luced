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

#ifndef LUA_ACCESS_HPP
#define LUA_ACCESS_HPP

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "HeapObject.hpp"
#include "ObjectArray.hpp"
#include "OwningPtr.hpp"
#include "SingletonInstance.hpp"
#include "String.hpp"
#include "RawPtr.hpp"

namespace LucED
{

class LuaCFunctionArguments;
class LuaCFunctionResult;
class LuaObject;
class LuaStoredObjectReference;
class LuaInterpreter;
class LuaObjectList;

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

#ifdef DEBUG
class LuaStackChecker;
#endif


class LuaAccess
{
public:
    explicit LuaAccess(lua_State* luaState);
    
    LuaStoredObjectReference store(const LuaObject& object) const;
    LuaObject retrieve(const LuaStoredObjectReference& storeReference) const;
    
    LuaObject getGlobal(const char* name) const;
    LuaObject getGlobal(const String& name) const;

    LuaObject newNil() const;    
    LuaObject newObject() const;    
    LuaObject newTable() const;

    template
    <
        class T
    >
    LuaObject newObject(const T& rhs) const;
    
    void clearGlobal(const char* name) const {
        ASSERT(isCorrect());
        lua_pushnil(L);
        lua_setglobal(L, name);
    }
    
    template
    <
        class T
    >
    void setGlobal(const char* name, const T& value) const;

    RawPtr<LuaInterpreter> getLuaInterpreter() const;
    
    bool isSameLuaAccess(const LuaAccess& rhs) const {
        ASSERT(isCorrect());
        return (L == rhs.L);
    }
    
    class Result;
    Result executeFile(String name) const;
    Result executeScript(const char* beginScript, long scriptLength, String name = String()) const;
    Result executeExpression(const char* beginScript, long scriptLength, String name = String()) const;

    Result executeExpression(const String& expr, String name = String()) const;
    Result executeScript(String script, String name = String()) const;
    
#ifdef DEBUG
    bool isCorrect() const;
#endif
    
protected:
    friend class LuaCFunctionArguments;
    friend class LuaCFunctionResult;
    friend class LuaStoredObjectReference;
    friend class LuaInterpreter;
    friend class LuaIterator;
    
    void push(const char* arg) const {
        ASSERT(isCorrect());
        lua_pushstring(L, arg);
    }
    void push(const String& arg) const {
        ASSERT(isCorrect());
        lua_pushlstring(L, arg.toCString(), arg.getLength());
    }
    void push(int arg) const {
        ASSERT(isCorrect());
        lua_pushnumber(L, arg);
    }
    void push(const LuaObject& arg) const;

    void push(const WeakPtr<HeapObject>& rhs) const;
    
    template
    <
        LuaCFunctionResult F(const LuaCFunctionArguments& args)
    >
    void push(LuaCFunction<F> wrapper) const;

    template
    <
        class C,
        LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
    >
    void push(LuaCMethod<C,M> wrapper) const;

    lua_State* L;
#ifdef DEBUG
    RawPtr<LuaStackChecker> luaStackChecker;
#endif
};

} // namespace LucED

#include "LuaObject.hpp"
#include "LuaCFunctionArguments.hpp"
#include "LuaCFunctionResult.hpp"
#include "LuaCMethod.hpp"
#include "LuaCFunction.hpp"
#include "LuaStackChecker.hpp"
#include "LuaStateAccess.hpp"
#include "LuaStoredObjectReference.hpp"
#include "LuaInterpreter.hpp"

namespace LucED
{

inline LuaAccess::LuaAccess(lua_State* luaState)
    : L(luaState)
#ifdef DEBUG
    , luaStackChecker(LuaStateAccess::getLuaStackChecker(L))
#endif
{}

inline void LuaAccess::push(const LuaObject& arg) const
{
    ASSERT(isCorrect());
    ASSERT(isSameLuaAccess(arg));

    lua_pushvalue(L, arg.stackIndex);
}

inline RawPtr<LuaInterpreter> LuaAccess::getLuaInterpreter() const
{
    ASSERT(isCorrect());

    return  LuaStateAccess::getLuaInterpreter(L);
}

inline LuaStoredObjectReference LuaAccess::store(const LuaObject& object) const
{
    ASSERT(isCorrect());

    lua_pushvalue(L, object.stackIndex);
    int registryReference = luaL_ref(L, LUA_REGISTRYINDEX);
    return LuaStoredObjectReference(registryReference, LuaStateAccess::getLuaInterpreter(L));
}

inline LuaObject LuaAccess::retrieve(const LuaStoredObjectReference& storeReference) const
{
    ASSERT(isCorrect());
    ASSERT(storeReference.ptr->getLuaInterpreter() == getLuaInterpreter());
    lua_rawgeti(L, LUA_REGISTRYINDEX, storeReference.ptr->getRegistryReference());
    return LuaObject(*this, lua_gettop(L));
}

inline LuaObject LuaAccess::newNil() const
{
    ASSERT(isCorrect());
    lua_pushnil(L);
    return LuaObject(*this, lua_gettop(L));
}

inline LuaObject LuaAccess::newObject() const
{
    ASSERT(isCorrect());
    lua_pushnil(L);
    return LuaObject(*this, lua_gettop(L));
}

inline LuaObject LuaAccess::newTable() const
{
    ASSERT(isCorrect());
    lua_newtable(L);
    return LuaObject(*this, lua_gettop(L));
}

template
<
    class T
>
inline LuaObject LuaAccess::newObject(const T& rhs) const
{
    ASSERT(isCorrect());
    push(rhs);
    return LuaObject(*this, lua_gettop(L));
}

inline void LuaAccess::push(const WeakPtr<HeapObject>& rhs) const
{
    ASSERT(isCorrect());
    WeakPtr<HeapObject>* userDataPtr = static_cast< WeakPtr<HeapObject>*
                                                  >
                                                  (lua_newuserdata(L, sizeof(WeakPtr<HeapObject>)));
    if (userDataPtr != NULL) {
        new (userDataPtr) WeakPtr<HeapObject>(rhs);
    }
}

inline LuaObject LuaAccess::getGlobal(const char* name) const
{
    ASSERT(isCorrect());

    lua_getglobal(L, name);
    return LuaObject(*this, lua_gettop(L));
}

inline LuaObject LuaAccess::getGlobal(const String& name) const
{
    ASSERT(isCorrect());

    return getGlobal(name.toCString());
}

template
<
    class T
>
inline void LuaAccess::setGlobal(const char* name, const T& value) const
{
    ASSERT(isCorrect());

    push(value);
    lua_setglobal(L, name);
}

template
<
    LuaCFunctionResult F(const LuaCFunctionArguments& args)
>
inline void LuaAccess::push(LuaCFunction<F> wrapper) const
{
    ASSERT(isCorrect());

    lua_pushcfunction(L, LuaCFunction<F>::invokeFunction);
}

template
<
    class C,
    LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
>
inline void LuaAccess::push(LuaCMethod<C,M> wrapper) const
{
    ASSERT(isCorrect());

    lua_pushcfunction(L, &(LuaCMethod<C,M>::invokeFunction));
}

class LuaAccess::Result
{
public:
    String        output;
    LuaObjectList objects;
};    

inline LuaAccess::Result LuaAccess::executeExpression(const String& expr, String name) const
{
    return executeExpression(expr.toCString(), expr.getLength(), name);
}
inline LuaAccess::Result LuaAccess::executeScript(String script, String name) const
{
    return executeScript(script.toCString(), script.getLength(), name);
}

#ifdef DEBUG
inline bool LuaAccess::isCorrect() const
{
    return    (L != NULL)
           && (luaStackChecker.isValid())
           && (L == LuaStateAccess::getLuaInterpreter(L)->getCurrentLuaAccess().L);
}
#endif

} // namespace LucED

#endif // LUA_ACCESS_HPP
