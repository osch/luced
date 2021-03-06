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

#include "LuaVar.hpp"
#include "LuaException.hpp"
#include "LuaVarList.hpp"
#include "LuaInterpreter.hpp"
#include "LuaException.hpp"

using namespace LucED;

/*
LuaVar LuaVar::call()
{
    ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
    lua_pushvalue(L, stackIndex);
    int error = lua_pcall(L, 0, 1, 0);

    if (error != 0)
    {
        LuaException ex(lua_tostring(L, -1));
        lua_pop(L, 1);
        throw ex;
    } else {    
        return LuaVar(lua_gettop(L));
    }
}
*/
/*LuaVar LuaVar::call(const LuaVar& arg) const
{
    ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
    lua_pushvalue(L, stackIndex);
    lua_pushvalue(L, arg.stackIndex);
    
    int error = lua_pcall(L, 1, 1, 0);

    if (error != 0)
    {
        LuaException ex(lua_tostring(L, -1));
        lua_pop(L, 1);
        throw ex;
    } else {    
        return LuaVar(lua_gettop(L));
    }
}

LuaVar LuaVar::call(const LuaVarList& list) const
{
    ASSERT(stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(stackGeneration));
    lua_pushvalue(L, stackIndex);
    for (int i = 0; i < list.getLength(); ++i) {
        lua_pushvalue(L, list[i].stackIndex);
    }
    int error = lua_pcall(L, list.getLength(), 1, 0);

    if (error != 0)
    {
        LuaException ex(lua_tostring(L, -1));
        lua_pop(L, 1);
        throw ex;
    } else {    
        return LuaVar(lua_gettop(L));
    }
}

*/

void LuaVarRef::throwLuaException(const LuaVar& errorObject)
{
    throw LuaException(errorObject);
}
