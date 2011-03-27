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
#include "Null.hpp"
#include "StaticAssertion.hpp"

namespace LucED
{

class LuaCFunctionArguments;
class LuaCFunctionResult;
class LuaVar;
class LuaVarRef;
class LuaStoredObjectReference;
class LuaInterpreter;
class LuaVarList;

template<class KeyType
        >
class LuaObjectTableElementRef;


template
<
    class LuaCClosureT
>
class LuaCClosureImpl;

template
<
    class C,
    LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
>
class LuaCMethod;

template
<
    class C,
    LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
>
class LuaSingletonCMethod;

#ifdef DEBUG
class LuaStackChecker;
#endif


class LuaAccess
{
public:
    explicit LuaAccess(lua_State* luaState);
    
    static const long MAGIC = 547394;
    
    struct UserData
    {
        long magic;
        bool isOwningPtr;
    };
    
    LuaStoredObjectReference store(const LuaVar& object) const;
    LuaVar retrieve(const LuaStoredObjectReference& storeReference) const;
    
    LuaVar getGlobal(const char* name) const;
    LuaVar getGlobal(const String& name) const;
    LuaVar getGlobalVariables() const;

    LuaVar getTrue() const;    
    LuaVar getFalse() const;    
    LuaVar getNil() const;    

    LuaVar newTable() const;

    template
    <
        class T
    >
    LuaVar toLua(const T& rhs) const;
    
    LuaVar toLua(const char* ptr, long length) const;

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
    
    LuaVar loadString(const char*   script, const String& pseudoFileName = "") const;
    LuaVar loadString(const String& script, const String& pseudoFileName = "") const;

    LuaVar loadFile(const String& fileName) const;
    
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
    friend class LuaCMethodBase;

    template<class T
            >
    friend class LuaCClosureImpl;

    template<class C,
             LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
            >
    friend class LuaCMethod;
    
    void push(const char* arg) const {
        ASSERT(isCorrect());
        lua_pushstring(L, arg);
    }
    void push(const char* arg, long length) const {
        ASSERT(isCorrect());
        lua_pushlstring(L, arg, length);
    }
    void push(const String& arg) const {
        ASSERT(isCorrect());
        lua_pushlstring(L, arg.toCString(), arg.getLength());
    }
    void push(int arg) const {
        ASSERT(isCorrect());
        lua_pushnumber(L, arg);
    }
    void push(long arg) const {
        ASSERT(isCorrect());
        lua_pushnumber(L, arg);
    }
    void push(bool arg) const {
        ASSERT(isCorrect());
        lua_pushboolean(L, arg);
    }
    void push(NullType n) const {
        ASSERT(isCorrect());
        lua_pushnil(L);
    }
    void push(const LuaVarRef& arg) const;

    template<class T
            >
    void push(const WeakPtr<T>& rhs) const;
   
    template<class T
            >
    void push(const OwningPtr<T>& rhs) const;

    template<class T
            >
    void push(T* rhs) const;
    
    template<class T
            >
    void pushOwningPtr(const OwningPtr<T>& rhs) const;
    
    template<class T
            >
    void pushWeakPtr(T* rhs) const;
    

    template<bool isOwning
            >
    struct IsOwning
    {
        template<class T
                >
        static void pushInternal(const LuaAccess& luaAccess, const OwningPtr<T>& rhs);
    };
    
    template<bool isInClassRegistry
            >
    struct IsInClassRegistry
    {
        template<class T
                >
        static void setMetatable(const LuaAccess& luaAccess, LuaVarRef newObject, T* dummy);
    };

    void push(void* voidPtr) const;

    template<class KeyType
            >
    void push(const LuaObjectTableElementRef<KeyType>& rhs) const;
    
    template
    <
        class LuaCClosureT
    >
    void push(const LuaCClosureImpl<LuaCClosureT>& wrapper) const;

    template
    <
        class C,
        LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
    >
    void push(LuaCMethod<C,M> wrapper) const;

    template
    <
        class C,
        LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
    >
    void push(LuaSingletonCMethod<C,M> wrapper) const;

#ifdef DEBUG
    RawPtr<LuaStackChecker> getLuaStackChecker() const;
#endif

public:
    lua_State* L;

private:
    template<class T
            >
    static LuaStoredObjectReference getMetaTableStoreReference(RawPtr<LuaInterpreter> luaInterpreter);
};

} // namespace LucED

#include "LuaClassRegistry.hpp"
#include "LuaVar.hpp"
#include "LuaCFunctionArguments.hpp"
#include "LuaCFunctionResult.hpp"
#include "LuaCMethod.hpp"
#include "LuaSingletonCMethod.hpp"
#include "LuaCClosure.hpp"
#include "LuaStackChecker.hpp"
#include "LuaStateAccess.hpp"
#include "LuaStoredObjectReference.hpp"
#include "LuaInterpreter.hpp"

namespace LucED
{

inline LuaAccess::LuaAccess(lua_State* luaState)
    : L(luaState)
{}

inline RawPtr<LuaInterpreter> LuaAccess::getLuaInterpreter() const
{
    ASSERT(isCorrect());

    return  LuaStateAccess::getLuaInterpreter(L);
}

inline LuaStoredObjectReference LuaAccess::store(const LuaVar& object) const
{
    ASSERT(isCorrect());

    lua_pushvalue(L, object.stackIndex);
    int registryReference = luaL_ref(L, LUA_REGISTRYINDEX);
    return LuaStoredObjectReference(registryReference, LuaStateAccess::getLuaInterpreter(L));
}

inline LuaVar LuaAccess::retrieve(const LuaStoredObjectReference& storeReference) const
{
    ASSERT(isCorrect());
    ASSERT(storeReference.ptr->getLuaInterpreter() == getLuaInterpreter());
    lua_rawgeti(L, LUA_REGISTRYINDEX, storeReference.ptr->getRegistryReference());
    return LuaVar(*this, lua_gettop(L));
}

inline LuaVar LuaAccess::getNil() const
{
    ASSERT(isCorrect());
    lua_pushnil(L);
    return LuaVar(*this, lua_gettop(L));
}
inline LuaVar LuaAccess::getTrue() const
{
    ASSERT(isCorrect());
    lua_pushboolean(L, true);
    return LuaVar(*this, lua_gettop(L));
}
inline LuaVar LuaAccess::getFalse() const
{
    ASSERT(isCorrect());
    lua_pushboolean(L, false);
    return LuaVar(*this, lua_gettop(L));
}


inline LuaVar LuaAccess::newTable() const
{
    ASSERT(isCorrect());
    lua_newtable(L);
    return LuaVar(*this, lua_gettop(L));
}

template
<
    class T
>
inline LuaVar LuaAccess::toLua(const T& rhs) const
{
    ASSERT(isCorrect());
    push(rhs);
    return LuaVar(*this, lua_gettop(L));
}

inline LuaVar LuaAccess::toLua(const char* ptr, long length) const
{
    ASSERT(isCorrect());
    push(ptr, length);
    return LuaVar(*this, lua_gettop(L));
}

inline void LuaAccess::push(const LuaVarRef& arg) const
{
    ASSERT(isCorrect());
    ASSERT(arg.isCorrect());
    ASSERT(isSameLuaAccess(arg));

    lua_pushvalue(L, arg.stackIndex);
}

inline void LuaAccess::push(void* arg) const
{
    ASSERT(isCorrect());

    lua_pushlightuserdata(L, arg);
}

template<
        >
template<class T
        >
inline void LuaAccess::IsOwning<true>::pushInternal(const LuaAccess& luaAccess, const OwningPtr<T>& rhs)
{
    luaAccess.pushOwningPtr(rhs);
}
template<
        >
template<class T
        >
inline void LuaAccess::IsOwning<false>::pushInternal(const LuaAccess& luaAccess, const OwningPtr<T>& rhs)
{
    luaAccess.pushWeakPtr(rhs.getRawPtr());
}

template<
        >
template<class T
        >
inline void LuaAccess::IsInClassRegistry<true>::setMetatable(const LuaAccess& luaAccess, LuaVarRef newObject, T* dummy)
{
    RawPtr<LuaInterpreter>   luaInterpreter = luaAccess.getLuaInterpreter();
    LuaStoredObjectReference storeReference = getMetaTableStoreReference<T>(luaInterpreter);
    
    ASSERT(storeReference.hasLuaInterpreter(luaInterpreter));
    
    LuaVar metaTable = luaAccess.retrieve(storeReference);
    newObject.setMetaTable(metaTable);
}
template<
        >
template<class T
        >
inline void LuaAccess::IsInClassRegistry<false>::setMetatable(const LuaAccess& luaAccess, LuaVarRef newObject, T* dummy)
{
}

template<class T
        >
inline void LuaAccess::pushOwningPtr(const OwningPtr<T>& rhs) const
{
    {
        ASSERT(isCorrect());
    
        StaticAssertion<LuaClassRegistry::ClassAttributes<T>::isOwningPtrType>();
    
        {
            RawPtr<LuaInterpreter> luaInterpreter = getLuaInterpreter();
        
            void*  voidPtr      = rhs.getRawPtr();
            LuaVar owningPtrMap = retrieve(LuaInterpreter::ClassRegistryAccess::getOwningPtrMapStoreReference(luaInterpreter));
            
            LuaVar existingObject = owningPtrMap[voidPtr];
            
            if (existingObject.isValid())
            {
                push(existingObject);
            }
            else
            {
                UserData* userDataPtr = static_cast< UserData*
                                                   >
                                                   (lua_newuserdata(L, sizeof(UserData) + sizeof(OwningPtr<HeapObject>)));
                if (userDataPtr != NULL)
                {
                    LuaVarRef newObject(*this, lua_gettop(L)); // newObject ist pushed on the Stack by lua_newuserdata
        
                    userDataPtr->magic       = MAGIC;
                    userDataPtr->isOwningPtr = true;
                
                    OwningPtr<HeapObject>* owningPtrPtr = static_cast< OwningPtr<HeapObject>*
                                                                     >
                                                                     (static_cast<void*>(userDataPtr + 1));
                    new (owningPtrPtr) OwningPtr<HeapObject>(rhs);
                
                    IsInClassRegistry<LuaClassRegistry::ClassAttributes<T>::isKnown>::setMetatable(*this, newObject, (T*)NULL);
                        
                    owningPtrMap[voidPtr] = newObject;
                }
                else {
                    lua_pushnil(L);
                }
            }
        }
    }
    lua_removeunusedbefore(L, lua_gettop(L));
}

template<class T
        >
inline void LuaAccess::pushWeakPtr(T* rhs) const
{
    {
        ASSERT(isCorrect());
        
        StaticAssertion<!LuaClassRegistry::ClassAttributes<T>::isOwningPtrType>();
    
        RawPtr<LuaInterpreter> luaInterpreter = getLuaInterpreter();
    
        void*  voidPtr    = static_cast<void*>(rhs);
        LuaVar weakPtrMap = retrieve(LuaInterpreter::ClassRegistryAccess::getWeakPtrMapStoreReference(luaInterpreter));
        
        LuaVar existingObject = weakPtrMap[voidPtr];
        
        if (existingObject.isValid())
        {
            push(existingObject);
        }
        else
        {
            UserData* userDataPtr = static_cast< UserData*
                                               >
                                               (lua_newuserdata(L, sizeof(UserData) + sizeof(WeakPtr<HeapObject>)));
            if (userDataPtr != NULL)
            {
                LuaVarRef newObject(*this, lua_gettop(L)); // newObject ist pushed on the Stack by lua_newuserdata
    
                userDataPtr->magic       = MAGIC;
                userDataPtr->isOwningPtr = false;
            
                WeakPtr<HeapObject>* weakPtrPtr = static_cast< WeakPtr<HeapObject>*
                                                             >
                                                             (static_cast<void*>(userDataPtr + 1));
                new (weakPtrPtr) WeakPtr<HeapObject>(rhs);
            
                IsInClassRegistry<LuaClassRegistry::ClassAttributes<T>::isKnown>::setMetatable(*this, newObject, (T*)NULL);

                weakPtrMap[voidPtr] = newObject;
            }
            else {
                lua_pushnil(L);
            }
        }
    }
    lua_removeunusedbefore(L, lua_gettop(L));
}


template<class T
        >
inline void LuaAccess::push(const OwningPtr<T>& rhs) const
{
    IsOwning<LuaClassRegistry::ClassAttributes<T>::isOwningPtrType>::pushInternal(*this, rhs);
}

template<class T
        >
inline void LuaAccess::push(T* rhs) const
{
    pushWeakPtr(rhs);
}

template<class T
        >
inline void LuaAccess::push(const WeakPtr<T>& rhs) const
{
    pushWeakPtr(rhs.getRawPtr());
}

template<class KeyType
        >
inline void LuaAccess::push(const LuaObjectTableElementRef<KeyType>& rhs) const
{
    ASSERT(isCorrect());
    ASSERT(rhs.isCorrect());
    ASSERT(isSameLuaAccess(rhs));

    push(rhs.key);
    lua_gettable(L, rhs.tableStackIndex);
}


inline LuaVar LuaAccess::getGlobal(const char* name) const
{
    ASSERT(isCorrect());

    lua_getglobal(L, name);
    return LuaVar(*this, lua_gettop(L));
}

inline LuaVar LuaAccess::getGlobal(const String& name) const
{
    ASSERT(isCorrect());

    return getGlobal(name.toCString());
}

inline LuaVar LuaAccess::getGlobalVariables() const
{
    ASSERT(isCorrect());
    
    lua_pushvalue(L, LUA_GLOBALSINDEX);

    return LuaVar(*this, lua_gettop(L));
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
    class LuaCClosureType
>
inline void LuaAccess::push(const LuaCClosureImpl<LuaCClosureType>& wrapper) const
{
    ASSERT(isCorrect());

    wrapper.pushUpvaluesTo(*this);

    lua_pushcclosure(L, LuaCClosureImpl<LuaCClosureType>::invokeFunction, 
                        LuaCClosureImpl<LuaCClosureType>::NUMBER_UPVALUES);
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

template
<
    class C,
    LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
>
inline void LuaAccess::push(LuaSingletonCMethod<C,M> wrapper) const
{
    ASSERT(isCorrect());

    lua_pushcfunction(L, &(LuaSingletonCMethod<C,M>::invokeFunction));
}

inline LuaVar LuaAccess::loadString(const String& script, const String& pseudoFileName) const {
    return loadString(script.toCString(), pseudoFileName);
}

class LuaAccess::Result
{
public:
    String        output;
    LuaVarList objects;
};    

inline LuaAccess::Result LuaAccess::executeExpression(const String& expr, String name) const
{
    return executeExpression(expr.toCString(), expr.getLength(), name);
}
inline LuaAccess::Result LuaAccess::executeScript(String script, String name) const
{
    return executeScript(script.toCString(), script.getLength(), name);
}

template<class T
        >
inline LuaStoredObjectReference LuaAccess::getMetaTableStoreReference(RawPtr<LuaInterpreter> luaInterpreter)
{
    return LuaInterpreter::ClassRegistryAccess
                         ::getMetaTableStoreReference<T>(luaInterpreter);
}

#ifdef DEBUG
inline RawPtr<LuaStackChecker> LuaAccess::getLuaStackChecker() const
{
    return LuaStateAccess::getLuaStackChecker(L);
}
inline bool LuaAccess::isCorrect() const
{
    ASSERT(L != NULL);
    ASSERT(getLuaStackChecker().isValid());
    ASSERT(L == LuaStateAccess::getLuaInterpreter(L)->getCurrentLuaAccess().L);
    return true;
}
#endif

} // namespace LucED

#endif // LUA_ACCESS_HPP
