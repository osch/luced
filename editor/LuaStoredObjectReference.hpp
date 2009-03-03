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

#ifndef LUA_STORED_OBJECT_REFERENCE_HPP
#define LUA_STORED_OBJECT_REFERENCE_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "RawPtr.hpp"

struct lua_State;

namespace LucED
{

class  LuaAccess;
class  LuaInterpreter;

class LuaStoredObjectReference
{
public:
    LuaStoredObjectReference();
    
    bool hasLuaInterpreter(RawPtr<LuaInterpreter> luaInterpreter) const {
        return ptr->getLuaInterpreter() == luaInterpreter;
    }

    bool isValid() const {
        return ptr.isValid();
    }

    int getRegistryReference() const {
        ASSERT(ptr->getRegistryReference() == registryReference);
        return registryReference;
    }
    
private:
    friend class LuaAccess;
    
    LuaStoredObjectReference(int                    registryReference,
                             RawPtr<LuaInterpreter> luaInterpreter);

    class Data : public HeapObject
    {
    public:
        typedef OwningPtr<Data> Ptr;
        
        static Ptr create(int registryReference, RawPtr<LuaInterpreter> luaInterpreter);
        
        RawPtr<LuaInterpreter> getLuaInterpreter() const;
    
        int getRegistryReference() const {
            return registryReference;
        }
        
    private:
        Data(int registryReference, RawPtr<LuaInterpreter> luaInterpreter);
        ~Data();

        int registryReference;
        RawPtr<LuaInterpreter> luaInterpreter;
    };
    
    static lua_State* getL(const LuaAccess& luaAccess);

    OwningPtr<Data> ptr;
    int             registryReference;
};

} // namespace LucED

#include "LuaClassRegistry.hpp"
#include "LuaInterpreter.hpp"

namespace LucED
{

inline LuaStoredObjectReference::Data::Ptr LuaStoredObjectReference::Data::create(int registryReference, RawPtr<LuaInterpreter> luaInterpreter)
{
    return Ptr(new Data(registryReference, luaInterpreter));
}

inline RawPtr<LuaInterpreter> LuaStoredObjectReference::Data::getLuaInterpreter() const
{
    return luaInterpreter;
}

inline LuaStoredObjectReference::Data::Data(int registryReference, RawPtr<LuaInterpreter> luaInterpreter)
    : registryReference(registryReference),
      luaInterpreter(luaInterpreter)
{}

inline LuaStoredObjectReference::LuaStoredObjectReference()
    : registryReference(0)
{}

inline LuaStoredObjectReference::LuaStoredObjectReference(int                    registryReference,
                                                          RawPtr<LuaInterpreter> luaInterpreter)
    : ptr(Data::create(registryReference, 
                       luaInterpreter)),
      registryReference(registryReference)
{}

inline lua_State* LuaStoredObjectReference::getL(const LuaAccess& luaAccess)
{
    return luaAccess.L;
}

} // namespace LucED

#endif // LUA_STORED_OBJECT_REFERENCE_HPP
