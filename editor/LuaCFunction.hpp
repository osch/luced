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

#ifndef LUA_C_FUNCTION_HPP
#define LUA_C_FUNCTION_HPP

#include <exception>

#include "LuaCFunctionArguments.hpp"
#include "LuaCFunctionResult.hpp"
#include "BaseException.hpp"

namespace LucED
{

template
<
    LuaCFunctionResult F(const LuaCFunctionArguments& args)
>
class LuaCFunction
{
public:
    static LuaCFunction createWrapper()
    {
        return LuaCFunction();
    }

private:
    friend class LuaAccess;
    friend class LuaVar;
    
    template<class KeyType
            >
    friend class LuaObjectTableElementRef;
    
    LuaCFunction()
    {}
    
    static int invokeFunction(lua_State* L)
    {
        LuaAccess              luaAccess(L);
        RawPtr<LuaInterpreter> luaInterpreter = luaAccess.getLuaInterpreter();
        LuaAccess              oldLuaAccess   = luaInterpreter->getCurrentLuaAccess();
        
        LuaInterpreter::LuaCFunctionAccess::setCurrentLuaAccess(luaInterpreter, luaAccess);
        #ifdef DEBUG
            OwningPtr<LuaStackChecker> oldChecker = LuaStateAccess::replaceLuaStackChecker(L, LuaStackChecker::create());
        #endif
        
        int numberOfResults = 0;
        bool wasError = false;
        {
            LuaCFunctionArguments args(luaAccess);

            try
            {
                numberOfResults = F(args).numberOfResults;
            }
            catch (BaseException& ex)
            {
                lua_pushstring(L, ex.getMessage().toCString());
                wasError = true;
            }
            catch (std::exception& ex)
            {
                lua_pushstring(L, ex.what());
                wasError = true;
            }
            catch (...)
            {
                lua_pushstring(L, "unknown error");
                wasError = true;
            }
        }
        #ifdef DEBUG
            LuaStateAccess::replaceLuaStackChecker(L, oldChecker);
        #endif
        LuaInterpreter::LuaCFunctionAccess::setCurrentLuaAccess(luaInterpreter, oldLuaAccess);

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

#endif // LUA_C_FUNCTION_HPP
