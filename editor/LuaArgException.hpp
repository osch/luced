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

#ifndef LUA_ARG_EXCEPTION_HPP
#define LUA_ARG_EXCEPTION_HPP

#include "BaseException.hpp"
#include "LuaException.hpp"

namespace LucED
{

/**
 * Wrong arguments in call from Lua to C++ functions/methods.
 */
class LuaArgException : public LuaException
{
public:
    LuaArgException(const LuaAccess& luaAccess,
                    const String&    message = "")
        : LuaException(luaAccess, message)
    {}
    virtual const char* what() const throw();
};

} // namespace LucED

#endif // LUA_ARG_EXCEPTION_HPP
