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

using namespace LucED;

HeapObject* LuaCMethodBase::getCheckedObjectPtr(LuaObject methodLuaObject, const std::type_info& typeInfo)
{
    if (!methodLuaObject.isUserData()) {
        throw LuaException(String() << "method needs object of type " << typeInfo.name()
                                                                      << " as first argument");
    }
    
    WeakPtr<HeapObject> methodHeapObject = methodLuaObject.toUserDataPtr();

    if (!methodHeapObject.isValid()) {
        throw LuaException(String() << "method needs valdid object of type " << typeInfo.name()
                                                                             << " as first argument");
    }

    HeapObject*         rawObjectPtr     = methodHeapObject.getRawPtr();
    
    if (typeid(*rawObjectPtr) != typeInfo) {
        throw LuaException(String() << "method needs object of type " << typeInfo.name()
                                                                      << " as first argument, "
                                       " but has argument of type " << typeid(*rawObjectPtr).name());
    }
    
    return rawObjectPtr;
}

void LuaCMethodBase::handleException(lua_State* L)
{
    try
    {
        throw;
    }
    catch (BaseException& ex)
    {
        lua_pushstring(L, ex.getMessage().toCString());
    }
    catch (std::exception& ex)
    {
        lua_pushstring(L, ex.what());
    }
    catch (...)
    {
        lua_pushstring(L, "unknown error");
    }
}

