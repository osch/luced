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

#ifndef LUACFUNCTION_H
#define LUACFUNCTION_H

#include <exception>

#include "LuaCFunctionArguments.hpp"
#include "LuaCFunctionResult.hpp"
#include "BaseException.hpp"

namespace LucED
{

template<class ImplFunction> class LuaCFunction
{
private:
    friend class LuaInterpreter;
    friend class LuaObject;

    static int invokeFunction(lua_State* L)
    {
        int numberOfResults = 0;
        bool wasError = false;
        {
            LuaCFunctionArguments args(L);

            try
            {
                numberOfResults = ImplFunction()(args).numberOfResults;
            }
            catch (BaseException& ex)
            {
                lua_pushstring(L, ex.getMessage().toCString());
                wasError = true;
            }
            catch (std::exception& ex)
            {
                lua_pushstring(L, ex.what());
                wasError = true;
            }
            catch (...)
            {
                lua_pushstring(L, "unknown error");
                wasError = true;
            }
        }
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

#endif // LUACFUNCTION_H
