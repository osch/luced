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

#include "LuaObject.hpp"
#include "NonCopyable.hpp"

#ifndef LUA_FUNCTION_ARGUMENTS_HPP
#define LUA_FUNCTION_ARGUMENTS_HPP

namespace LucED
{


class LuaFunctionArguments : private LuaAccess,
                             private NonCopyable
{
public:

    explicit LuaFunctionArguments(const LuaAccess& luaAccess)
        : LuaAccess(luaAccess),
          numberArguments(0),
          isOnStack(true)
    {
        lua_checkstack(L, 20);

        lua_pushnil(L); // placeholder for function
    #ifdef DEBUG
        startStackIndex  = lua_gettop(L);
    #endif
    }

    LuaFunctionArguments(const LuaObject& arg)
        : LuaAccess(arg.getLuaAccess()),
          numberArguments(1),
          isOnStack(true)
    {
        lua_checkstack(L, 20);

        lua_pushnil(L); // placeholder for function
    #ifdef DEBUG
        startStackIndex  = lua_gettop(L);
    #endif
        lua_pushvalue(L, arg.stackIndex);
    }

    ~LuaFunctionArguments() {
        if (isOnStack)
        {
        #ifdef DEBUG
            luaStackChecker->truncateGenerationsAtStackIndex(startStackIndex);
        #endif
            lua_pop(L, numberArguments + 1);
        }
    }

    template
    <
        class T
    >
    LuaFunctionArguments& operator<<(const T& arg)
    {
        ASSERT(isCorrect());

        push(arg);
        ++numberArguments;
        if (numberArguments % 10 == 0) {
            lua_checkstack(L, 20);
        }
        return *this;
    }

    int getLength() const {
        return numberArguments;
    }
    
    class LuaObjectAccess
    {
    private:
        friend class LuaObject;
        
        static void clearAfterCall(LuaFunctionArguments& args) {
            args.isOnStack = false;
            args.numberArguments = 0;
        }
    };

#ifdef DEBUG
    bool isCorrect() const {
        return    (LuaAccess::isCorrect())
               && (startStackIndex + numberArguments == lua_gettop(L))
               && (luaStackChecker->getHighestStackIndexForNewestGeneration() < startStackIndex)
               && (luaStackChecker->getHighestStackIndex() <= startStackIndex + numberArguments);
    }
#endif

private:
#ifdef DEBUG
    int  startStackIndex;
#endif
    int numberArguments;
    bool isOnStack;
};

} // namespace LucED

#endif // LUA_FUNCTION_ARGUMENTS_HPP
