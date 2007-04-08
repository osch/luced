/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#include "LuaObject.h"

#ifndef LUA_FUNCTION_ARGUMENTS_H
#define LUA_FUNCTION_ARGUMENTS_H

namespace LucED
{

using std::string;

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
        }
    #ifdef DEBUG
        if (refCounter == 1) {
            newestStackGeneration = LuaStackChecker::getInstance()->getNewestGeneration();
            highestStackIndex     = LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(newestStackGeneration);
            startStackIndex       = lua_gettop(LuaObject::L) - 1;
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
        if (refCounter == 0 && numberArguments > 0) {
            #ifdef DEBUG
                ASSERT(newestStackGeneration == LuaStackChecker::getInstance()->getNewestGeneration());
                ASSERT(highestStackIndex     == LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(newestStackGeneration));
            #endif
            lua_pop(LuaObject::L, numberArguments + 1);
            numberArguments = 0;
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

    LuaFunctionArguments& operator<<(const string& arg)
    {
    #ifdef DEBUG
        checkStack();
    #endif
        lua_pushlstring(LuaObject::L, arg.c_str(), arg.length());
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
        ASSERT(startStackIndex + 1 + numberArguments == lua_gettop(LuaObject::L));
        ASSERT(newestStackGeneration == LuaStackChecker::getInstance()->getNewestGeneration());
        ASSERT(highestStackIndex     == LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(newestStackGeneration));
    }
    static int  newestStackGeneration;
    static int  highestStackIndex;
    static int  startStackIndex;
#endif
    static int numberArguments;
    static int refCounter;
};

} // namespace LucED

#endif // LUA_FUNCTION_ARGUMENTS_H
