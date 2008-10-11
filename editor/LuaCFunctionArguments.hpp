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

#ifndef LUA_C_FUNCTION_ARGUMENTS_HPP
#define LUA_C_FUNCTION_ARGUMENTS_HPP

#include "NonCopyable.hpp"
#include "LuaVar.hpp"

namespace LucED
{

class LuaCFunctionArguments;
class LuaCFunctionResult;
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

template
<
    class C,
    LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
>
class LuaSingletonCMethod;


class LuaCFunctionArguments : public NonCopyable
{
public:

    LuaVarRef operator[](int i) const
    {
        ASSERT(0 <= i && i < numberArgs);
    #ifdef DEBUG
        return LuaVarRef(luaAccess, i + 1, stackGeneration);
    #else
        return LuaVarRef(luaAccess, i + 1);
    #endif
    }
    
    int getLength() const {
        ASSERT(isCorrect());
        return numberArgs;
    }
    
    LuaAccess getLuaAccess() const {
        ASSERT(isCorrect());
        return luaAccess;
    }

#ifdef DEBUG
    bool isCorrect() const {
        ASSERT(luaAccess.isCorrect());
        if (numberArgs > 0) {
            ASSERT(numberArgs <= luaAccess.luaStackChecker->getHighestStackIndexForGeneration(stackGeneration));
        }
        return true;
    }
#endif
    
private:
    
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
    

    LuaCFunctionArguments(const LuaAccess& luaAccess)
        : luaAccess(luaAccess),
          numberArgs(lua_gettop(luaAccess.L))
    {
    #ifdef DEBUG
        if (numberArgs > 1) {
            int s1          = luaAccess.luaStackChecker->registerAndGetGeneration(1);
            stackGeneration = luaAccess.luaStackChecker->registerAndGetGeneration(numberArgs);
            ASSERT(s1 = stackGeneration);
        } else if (numberArgs == 1) {
            stackGeneration = luaAccess.luaStackChecker->registerAndGetGeneration(1);
        }
    #endif
        ASSERT(isCorrect());
    }
    
    ~LuaCFunctionArguments()
    {
        ASSERT(isCorrect());
    #ifdef DEBUG
        if (numberArgs > 0) {
            luaAccess.luaStackChecker->truncateGenerationAtStackIndex(stackGeneration, 1);
        }
    #endif
    }
    
    void remove(int i) {
        ASSERT(0 <= i && i < numberArgs);
        lua_remove(luaAccess.L, i + 1);
    #ifdef DEBUG
        luaAccess.luaStackChecker->truncateGenerationAtStackIndex(stackGeneration, numberArgs);
    #endif
        numberArgs -= 1;
    }

    LuaAccess luaAccess;
    int       numberArgs;
    
#ifdef DEBUG
    int stackGeneration;
#endif
};

} // namespace LucED


#endif // LUA_C_FUNCTION_ARGUMENTS_HPP
