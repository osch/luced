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

#ifndef LUASTOREDOBJECT_H
#define LUASTOREDOBJECT_H

#include "HeapObject.hpp"
#include "LuaObject.hpp"
#include "OwningPtr.hpp"

namespace LucED
{

class LuaStoredObject : public HeapObject
{
public:
    typedef OwningPtr<LuaStoredObject> Ptr;
    
    static Ptr store(const LuaObject& object) {
        return Ptr(new LuaStoredObject(object));
    }
    
    LuaObject retrieve() const {
        lua_rawgeti(L, LUA_REGISTRYINDEX, registryReference);
        return LuaObject(lua_gettop(L));
    }
    
private:
    friend class LuaInterpreter;
    
    static lua_State* L;    
    
    LuaStoredObject(const LuaObject& object) {
        lua_pushvalue(L, object.stackIndex);
        registryReference = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    
    ~LuaStoredObject() {
        luaL_unref(L, LUA_REGISTRYINDEX, registryReference);
    }
    
    int registryReference;
};

} // namespace LucED

#endif // LUASTOREDOBJECT_H
