/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2007 Oliver Schmidt, oliver at luced dot de
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

#ifndef LUA_FUNCTION_ARGUMENTS_H
#define LUA_FUNCTION_ARGUMENTS_H

namespace LucED
{



class LuaFunctionArguments
{
public:

    LuaFunctionArguments()
    {
        ++refCounter;
        lua_checkstack(LuaObject::L, 20);
        if (refCounter == 1) {
            ASSERT(numberArguments == 0);
            lua_pushnil(LuaObject::L); // placeholder for function
            isOnStack = true;
        }
    #ifdef DEBUG
        if (refCounter == 1) {
            startStackIndex  = lua_gettop(LuaObject::L);
        } else {
            checkStack();
        }
    #endif
    }

    LuaFunctionArguments(const LuaFunctionArguments& rhs)
    {
    #ifdef DEBUG
        checkStack();
    #endif
        ++refCounter;
    }

    ~LuaFunctionArguments() {
        --refCounter;
        if (refCounter == 0 && isOnStack) {
            #ifdef DEBUG
                LuaStackChecker::getInstance()->truncateGenerationsAtStackIndex(startStackIndex);
            #endif
            lua_pop(LuaObject::L, numberArguments + 1);
            numberArguments = 0;
            isOnStack = false;
        }
    }
    LuaFunctionArguments& operator<<(const LuaObject& arg)
    {
    #ifdef DEBUG
        checkStack();
    #endif
        lua_pushvalue(LuaObject::L, arg.stackIndex);
        ++numberArguments;
        if (numberArguments % 10 == 0) {
            lua_checkstack(LuaObject::L, 20);
        }
        return *this;
    }

    LuaFunctionArguments& operator<<(const String& arg)
    {
    #ifdef DEBUG
        checkStack();
    #endif
        lua_pushlstring(LuaObject::L, arg.toCString(), arg.getLength());
        ++numberArguments;
        if (numberArguments % 10 == 0) {
            lua_checkstack(LuaObject::L, 20);
        }
        return *this;
    }

    int getLength() const {
        return numberArguments;
    }

private:
    friend class LuaObject;

#ifdef DEBUG
    void checkStack() const {
        ASSERT(startStackIndex + numberArguments == lua_gettop(LuaObject::L));
        ASSERT(LuaStackChecker::getInstance()->getHighestStackIndexForNewestGeneration() < startStackIndex);
        ASSERT(LuaStackChecker::getInstance()->getHighestStackIndex() <= startStackIndex + numberArguments);
    }
    static int  startStackIndex;
#endif
    static int numberArguments;
    static bool isOnStack;
    static int refCounter;
};

} // namespace LucED

#endif // LUA_FUNCTION_ARGUMENTS_H
