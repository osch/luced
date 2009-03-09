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
#include "LuaVar.hpp"
#include "LuaInterpreter.hpp"

#ifndef LUA_C_METHOD_HPP
#define LUA_C_METHOD_HPP

#include <typeinfo>

#include "LuaException.hpp"
#include "HeapObject.hpp"
#include "WeakPtr.hpp"
#include "LuaCFunctionArguments.hpp"
#include "LuaCFunctionResult.hpp"
#include "LuaClassRegistry.hpp"
#include "LuaCClosure.hpp"

namespace LucED
{


class LuaCMethodBase
{
protected:

    static void throwInvalidNumberArgsError(const char* luaClassName);

    template<class T
            >
    static T* castDynamicToValidPtr(const LuaVarRef& luaObject, const char* luaClassName)
    {
        T* rslt = luaObject.toWeakPtr<T>();
        if (rslt == NULL) {
            throwDynamicCastError(luaObject, luaClassName);
        }
        return rslt;
    }

    static void handleException(lua_State* L, const char* className,
                                              const char* methodName);

private:    
    static void throwInvalidArgumentError(const LuaVarRef& luaObject,
                                          const char*      luaClassName);

    static void throwDynamicCastError(const LuaVarRef& luaObject,
                                      const char*      luaClassName);
};

template
<
    class C,
    LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
>
class LuaCMethod : public LuaCMethodBase
{
public:
    static LuaCMethod createWrapper()
    {
        return LuaCMethod();
    }

private:
    friend class LuaAccess;
//    friend class LuaVar;
    
    LuaCMethod()
    {}

    static int invokeFunction(lua_State* L)
    {
        LuaAccess              luaAccess(L);
        RawPtr<LuaInterpreter> luaInterpreter            = luaAccess.getLuaInterpreter();
        LuaAccess              oldLuaAccess              = luaInterpreter->getCurrentLuaAccess();
        int                    oldErrorHandlerStackIndex = luaInterpreter->getErrorHandlerStackIndex();
        
        LuaInterpreter::LuaCFunctionAccess::setCurrentLuaAccess(luaInterpreter, luaAccess);
        #ifdef DEBUG
            OwningPtr<LuaStackChecker> oldChecker = LuaStateAccess::replaceLuaStackChecker(L, LuaStackChecker::create());
        #endif
        
        int numberOfResults = 0;
        bool wasError = false;
        {
            LuaCFunctionArguments args(luaAccess);

            LuaVar errorHandler(luaAccess, LuaCClosure::create<&LuaInterpreter::errorHandlerFunction>());
            LuaInterpreter::LuaCFunctionAccess::setErrorHandlerStackIndex(luaInterpreter, errorHandler.getStackIndex());

            try
            {
                const char* className = LuaClassRegistry::ClassAttributes<C>::getLuaClassName();
                
                if (args.getLength() < 1)
                {
                    LuaCMethodBase::throwInvalidNumberArgsError(className);
                }
                C* objectPtr = LuaCMethodBase::castDynamicToValidPtr<C>(args[0], 
                                                                        className);
                
                numberOfResults = (objectPtr->*M)(args).numberOfResults;
            }
            catch (...)
            {
                LuaCMethodBase::handleException(L, LuaClassRegistry::ClassAttributes<C>::getLuaClassName(),
                                                   LuaClassRegistry::getMethodName<C,M>());
                wasError = true;
            }
        }
        #ifdef DEBUG
            LuaStateAccess::replaceLuaStackChecker(L, oldChecker);
        #endif
        LuaInterpreter::LuaCFunctionAccess::setErrorHandlerStackIndex(luaInterpreter, oldErrorHandlerStackIndex);
        LuaInterpreter::LuaCFunctionAccess::setCurrentLuaAccess      (luaInterpreter, oldLuaAccess);

        if (wasError) {
            lua_error(L);
            ASSERT(false);
            return 0;
        } else {
            return numberOfResults;
        }
    }
};
 
} // namespace LucED

#endif // LUA_C_METHOD_HPP
