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
#include "LuaInterpreter.hpp"

#ifndef LUA_SINGLETON_C_METHOD_HPP
#define LUA_SINGLETON_C_METHOD_HPP

#include <typeinfo>

#include "LuaCMethod.hpp"
#include "HeapObject.hpp"
#include "WeakPtr.hpp"
#include "LuaCFunctionArguments.hpp"
#include "LuaCFunctionResult.hpp"
#include "LuaClassRegistry.hpp"
#include "LuaCClosure.hpp"

namespace LucED
{


template
<
    class C,
    LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
>
class LuaSingletonCMethod : public LuaCMethodBase
{
public:
    static LuaSingletonCMethod createWrapper()
    {
        return LuaSingletonCMethod();
    }

private:
    friend class LuaAccess;
    
    LuaSingletonCMethod()
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
                
                numberOfResults = (C::getInstance()->*M)(args).numberOfResults;
            }
            catch (...)
            {
                LuaCMethodBase::handleException(luaAccess,
                                                LuaClassRegistry::ClassAttributes<C>::getLuaClassName(),
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

#endif // LUA_SINGLETON_C_METHOD_HPP
