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
        ASSERT(0 <= i + offset && i + offset < numberArgs);
    #ifdef DEBUG
        return LuaVarRef(luaAccess, i + 1 + offset, stackGeneration);
    #else
        return LuaVarRef(luaAccess, i + 1 + offset);
    #endif
    }
    
    bool has(int i) const {
        i -= offset;
        return 0 <= i && i < numberArgs;
    }
    
    int getLength() const {
        ASSERT(isCorrect());
        return numberArgs - offset;
    }
    
    LuaAccess getLuaAccess() const {
        ASSERT(isCorrect());
        return luaAccess;
    }

#ifdef DEBUG
    bool isCorrect() const {
        ASSERT(luaAccess.isCorrect());
        if (numberArgs > 0) {
            ASSERT(numberArgs <= luaAccess.getLuaStackChecker()->getHighestStackIndexForGeneration(stackGeneration));
        }
        return true;
    }
#endif
    
private:
    
    template
    <
        class LuaCClosureType
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
    

    explicit LuaCFunctionArguments(const LuaAccess& luaAccess, int offset = 0)
        : luaAccess(luaAccess),
          numberArgs(lua_gettop(luaAccess.L)),
          offset(offset)
    {
    #ifdef DEBUG
        if (numberArgs > 1) {
            int s1          = luaAccess.getLuaStackChecker()->registerAndGetGeneration(1);
            stackGeneration = luaAccess.getLuaStackChecker()->registerAndGetGeneration(numberArgs);
            ASSERT(s1 == stackGeneration);
        } else if (numberArgs == 1) {
            stackGeneration = luaAccess.getLuaStackChecker()->registerAndGetGeneration(1);
        }
    #endif
        ASSERT(isCorrect());
    }
    
    ~LuaCFunctionArguments()
    {
        ASSERT(isCorrect());
    #ifdef DEBUG
        if (numberArgs > 0) {
            luaAccess.getLuaStackChecker()->truncateGenerationAtStackIndex(stackGeneration, 1);
        }
    #endif
    }
    
    LuaAccess luaAccess;
    int       numberArgs;
    const int offset;
    
#ifdef DEBUG
    int stackGeneration;
#endif
};

} // namespace LucED


#endif // LUA_C_FUNCTION_ARGUMENTS_HPP
