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
#include "LuaStoredObjectReference.hpp"
#include "LuaVar.hpp"

#ifndef LUA_INTERPRETER_HPP
#define LUA_INTERPRETER_HPP

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "HeapObject.hpp"
#include "ObjectArray.hpp"
#include "OwningPtr.hpp"
#include "LuaVarList.hpp"
#include "String.hpp"
#include "LuaAccess.hpp"

namespace LucED
{

class LuaCFunctionArguments;
class LuaCFunctionResult;

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



class LuaInterpreter : public HeapObject
{
public:
    typedef OwningPtr<LuaInterpreter> Ptr;
    
    static Ptr create() {
        return Ptr(new LuaInterpreter());
    }

    LuaAccess getCurrentLuaAccess() const {
        return currentAccess;
    }
    
    LuaVar require(const String& packageName) const;
    
    class LuaCFunctionAccess
    {
    private:
        template
        <
            class LuaCClosureT
        >
        friend class LuaCClosureImpl;

        template
        <
            class C,
            LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
        >
        friend class LuaCMethod;

        template
        <
            class C,
            LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
        >
        friend class LuaSingletonCMethod;
        
        static void setCurrentLuaAccess(LuaInterpreter* luaInterpreter, const LuaAccess& luaAccess)
        {
            luaInterpreter->currentAccess = luaAccess;
        }
        static void setErrorHandlerStackIndex(LuaInterpreter* luaInterpreter, int errorHandlerStackIndex)
        {
            luaInterpreter->errorHandlerStackIndex = errorHandlerStackIndex;
        }
    };
    
    class PrintBufferAccess
    {
    private:
        friend class LuaAccess;
        
        static String getPrintBuffer(LuaInterpreter* luaInterpreter) {
            return luaInterpreter->printBuffer;
        }
        static void setPrintBuffer(LuaInterpreter* luaInterpreter, const String& printBuffer) {
            luaInterpreter->printBuffer = printBuffer;
        }
    };
    
    class ClassRegistryAccess
    {
    private:
        friend class LuaAccess;
        
        template<class T
                >
        static LuaStoredObjectReference getMetaTableStoreReference(LuaInterpreter* luaInterpreter) {
            return luaInterpreter->storedObjects->getMetaTableStoreReference<T>();
        }
        
        static LuaStoredObjectReference getWeakPtrMapStoreReference(LuaInterpreter* luaInterpreter) {
            return luaInterpreter->storedObjects->weakPtrMapStoreReference;
        }
        static LuaStoredObjectReference getOwningPtrMapStoreReference(LuaInterpreter* luaInterpreter) {
            return luaInterpreter->storedObjects->owningPtrMapStoreReference;
        }
    };

#ifdef DEBUG
    bool isCorrect() const {
        return    (rootAccess.isCorrect())
               && (rootAccess.getLuaInterpreter() == this)
               && (currentAccess.isCorrect())
               && (currentAccess.getLuaInterpreter() == this);
    }
#endif
    
    static LuaCFunctionResult errorHandlerFunction(const LuaCFunctionArguments& args);

    int getErrorHandlerStackIndex() const {
        return errorHandlerStackIndex;
    }
    
protected:
    LuaInterpreter();
    ~LuaInterpreter();

private:
    static LuaCFunctionResult printFunction       (const LuaCFunctionArguments& args);
    static LuaCFunctionResult stdoutWriteFunction (const LuaCFunctionArguments& args);
    static LuaCFunctionResult ioWriteFunction     (const LuaCFunctionArguments& args);
    static LuaCFunctionResult ioOutputFunction    (const LuaCFunctionArguments& args);
    static LuaCFunctionResult testFunction        (const LuaCFunctionArguments& args);
    static LuaCFunctionResult doNothingFunction   (const LuaCFunctionArguments& args);

    class StoredObjects : public HeapObject
    {
    public:
        typedef OwningPtr<StoredObjects> Ptr;
        
        static Ptr create(const LuaAccess& luaAccess) {
            return Ptr(new StoredObjects(luaAccess));
        }
      
        LuaStoredObjectReference originalToStringFunction;
        LuaStoredObjectReference originalIoOutputFunction;
        LuaStoredObjectReference originalIoWriteFunction;
        LuaStoredObjectReference lucedStdout;
        LuaStoredObjectReference owningPtrMapStoreReference;
        LuaStoredObjectReference weakPtrMapStoreReference;

        LuaStoredObjectReference requireFunctionStoreReference;

        template<class T
                >
        LuaStoredObjectReference getMetaTableStoreReference() {
            return classRegistry.getMetaTableStoreReference<T>();
        }
    private:
        StoredObjects(const LuaAccess& luaAccess)
            : classRegistry(luaAccess)
        {}

        LuaClassRegistry classRegistry;
    };
    
    static lua_State* initState(LuaInterpreter* luaInterpreter, lua_State* L);
    
    StoredObjects::Ptr storedObjects;
    bool isLucedStdoutActive;
    
    String printBuffer;

    LuaAccess rootAccess;
    LuaAccess currentAccess;
    
    int errorHandlerStackIndex;
};



} // namespace LucED



#endif // LUA_INTERPRETER_HPP
