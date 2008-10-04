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

#ifndef LUA_STATE_ACCESS_HPP
#define LUA_STATE_ACCESS_HPP

#include <lua.h>

#include "debug.hpp"
#include "LuaStackChecker.hpp"
#include "RawPtr.hpp"

namespace LucED
{

class LuaInterpreter;

class LuaStateAccess
{
public:
    static void setLuaInterpreter(lua_State* L, RawPtr<LuaInterpreter> luaInterpreter);
    
    static RawPtr<LuaInterpreter> getLuaInterpreter(lua_State* L);

#ifdef DEBUG
    static LuaStackChecker* getLuaStackChecker(lua_State* L);
#endif

private:
    static lua_State* currentState;
};

} // namespace LucED

#endif // LUA_STATE_ACCESS_HPP
