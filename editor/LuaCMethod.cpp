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

#include "LuaCMethod.hpp"
#include "LuaArgException.hpp"

using namespace LucED;


void LuaCMethodBase::throwInvalidNumberArgsError(const LuaAccess& luaAccess, 
                                                 const char* luaClassName)
{
    throw LuaException(luaAccess,
                       String() << "method needs object of type " << luaClassName
                                                                  << " as first argument");
}


void LuaCMethodBase::throwInvalidArgumentError(const LuaVarRef& luaObject, 
                                               const char*      luaClassName)
{
    if (!luaObject.isPtr()) {
        throw LuaException(luaObject.getLuaAccess(),
                           String() << "method needs object of type " << luaClassName
                                                                      << " as first argument");
    }
    
    throw LuaException(luaObject.getLuaAccess(),
                       String() << "method needs valid object of type " << luaClassName
                                                                        << " as first argument");
}


void LuaCMethodBase::throwDynamicCastError(const LuaVarRef& luaObject,
                                           const char*      luaClassName)
{
    if (luaObject.isPtr() && luaObject["type"].isString()) 
    {
        throw LuaException(luaObject.getLuaAccess(),
                           String() << "method needs object of type " << luaClassName
                                                                      << " as first argument, "
                                       " but has argument of type "   << luaObject["type"].toString());
    } else {
        throwInvalidArgumentError(luaObject, luaClassName);
    }
}


void LuaCMethodBase::handleException(const LuaAccess& luaAccess, 
                                     const char*      className,
                                     const char*      methodName)
{
    try
    {
        throw;
    }
    catch (LuaArgException& ex)
    {
        ExceptionLuaInterface::Ptr luaInterface = ex.getExceptionLuaInterface();
        luaInterface->prependMessage(String() << "Invalid invocation arguments for LucED builtin function '" 
                                              << className << "." << methodName << "'");
        luaAccess.push(luaInterface);
    }
    catch (LuaException& ex)
    {
        ExceptionLuaInterface::Ptr luaInterface = ex.getExceptionLuaInterface();
        luaInterface->prependMessage(String() << "Error in LucED builtin function '" 
                                              << className << "." << methodName << "'");
        luaAccess.push(luaInterface);
    }
    catch (BaseException& ex)
    {
        luaAccess.push(ExceptionLuaInterface::create(luaAccess,
                                                     String() << "Error in LucED builtin function '"
                                                              << className << "." << methodName << "': " 
                                                              << ex.getMessage()));
    }
    catch (std::exception& ex)
    {
        luaAccess.push(ExceptionLuaInterface::create(luaAccess,
                                                     String() << "Error in LucED builtin function '"
                                                              << className << "." << methodName << "': " 
                                                              << ex.what()));
    }
    catch (...)
    {
        luaAccess.push(ExceptionLuaInterface::create(luaAccess,
                                                     String() << "Unknown error in LucED builtin function '"
                                                              << className << "." << methodName << "'"));
    }
}

