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
#include "LuaObject.hpp"

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


class LuaCFunctionArguments : public NonCopyable
{
public:

    const LuaObject& operator[](int i) const {
        return argArray[i];
    }
    
    int getLength() const {
        return argArray.getLength();
    }
    
    LuaAccess getLuaAccess() const {
        return luaAccess;
    }
    
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
    

    LuaCFunctionArguments(const LuaAccess& luaAccess)
        : luaAccess(luaAccess)
    {
        int numberArgs = lua_gettop(luaAccess.L);
            
        argArray.appendAmount(numberArgs);
        
        ASSERT(argArray.getLength() == numberArgs);
        
        for (int i = 0; i < numberArgs; ++i) {
            new (&argArray[i]) LuaObject(luaAccess, i + 1);
        }
    }
    
    ~LuaCFunctionArguments()
    {
    #ifdef DEBUG
        for (int i = 0; i < argArray.getLength(); ++i) {
            luaAccess.luaStackChecker->truncateGenerationAtStackIndex(argArray[i].stackGeneration, 
                                                                      argArray[i].stackIndex);
        }
    #endif
    }
    MemArray<LuaObject> argArray;

    LuaAccess luaAccess;
};

} // namespace LucED


#endif // LUA_C_FUNCTION_ARGUMENTS_HPP
