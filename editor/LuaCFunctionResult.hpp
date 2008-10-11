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

#ifndef LUA_C_FUNCTION_RESULT_HPP
#define LUA_C_FUNCTION_RESULT_HPP

#include "LuaVar.hpp"

namespace LucED
{

class LuaCFunctionArguments;

class LuaCFunctionResult
{
public:
    LuaCFunctionResult(const LuaAccess& luaAccess)
        : numberOfResults(0),
          luaAccess(luaAccess)
    {}
    
    template<class T
            >
    LuaCFunctionResult& operator<<(const T& rsltObject) {
        luaAccess.push(rsltObject);
        ++numberOfResults;
        return *this;
    }

private:
    friend class LuaCFunctionArguments;
    
    template
    <
        LuaCFunctionResult F(const LuaCFunctionArguments& args)
    >
    friend class LuaCFunction;
    
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

    int numberOfResults;
    LuaAccess luaAccess;
};

} // namespace LucED

#endif // LUA_C_FUNCTION_RESULT_HPP
