/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2012 Oliver Schmidt, oliver at luced dot de
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

#include "LuaCClosure.hpp"
#include "BaseException.hpp"
#include "LuaException.hpp"

using namespace LucED;

void LuaCClosureImplBase::handleCatchedException(LuaAccess luaAccess)
{
    try
    {
        throw;
    }
    catch (LuaException& ex)
    {
        luaAccess.push(ex.getExceptionLuaInterface());
    }
    catch (BaseException& ex)
    {
        luaAccess.push(ExceptionLuaInterface::create(luaAccess, ex.getMessage()));
    }
    catch (std::exception& ex)
    {
        luaAccess.push(ExceptionLuaInterface::create(luaAccess, ex.what()));
    }
    catch (...)
    {
        luaAccess.push(ExceptionLuaInterface::create(luaAccess, "unknown error"));
    }
}

