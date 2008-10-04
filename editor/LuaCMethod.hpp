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
#include "LuaObject.hpp"
#include "LuaInterpreter.hpp"

#ifndef LUA_C_METHOD_HPP
#define LUA_C_METHOD_HPP

#include <typeinfo>

#include "LuaException.hpp"
#include "HeapObject.hpp"
#include "WeakPtr.hpp"
#include "LuaCFunctionArguments.hpp"
#include "LuaCFunctionResult.hpp"

namespace LucED
{


class LuaCMethodBase
{
protected:
    static HeapObject* getCheckedObjectPtr(LuaObject methodLuaObject, const std::type_info& typeInfo);
    static void handleException(lua_State* L);
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
    friend class LuaObject;
    
    LuaCMethod()
    {}
    
    static int invokeFunction(lua_State* L)
    {
        LuaAccess              luaAccess(L);
        RawPtr<LuaInterpreter> luaInterpreter = luaAccess.getLuaInterpreter();
        LuaAccess              oldLuaAccess   = luaInterpreter->getCurrentLuaAccess();
        
        LuaInterpreter::LuaCFunctionAccess::setCurrentLuaAccess(luaInterpreter, luaAccess);
        
        int numberOfResults = 0;
        bool wasError = false;
        {
            try
            {
                LuaCFunctionArguments args(luaAccess);
            
                LuaObject object = args[0];

                HeapObject* rawObjectPtr = LuaCMethodBase::getCheckedObjectPtr(object, typeid(C));
                
                numberOfResults = (static_cast<C*>(rawObjectPtr)->*M)(args).numberOfResults;
            }
            catch (...)
            {
                LuaCMethodBase::handleException(L);
                wasError = true;
            }
        }
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

#endif // LUA_C_METHOD_HPP
