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

#ifndef LUAOBJECTLIST_H
#define LUAOBJECTLIST_H

#include "LuaObject.hpp"
#include "HeapObjectArray.hpp"

namespace LucED {

class LuaObjectList
{
public:
    LuaObjectList() : objects(HeapObjectArray<LuaObject>::create()) {}

    void append(const LuaObject& object) {
        objects->append(object);
    }
    const LuaObject& operator[](int index) const {
        return objects->at(index);
    }
    int getLength() const {
        return objects->getLength();
    }
private:
    friend class LuaInterpreter;
    
    void appendObjectWithStackIndex(int stackIndex) {
        objects->appendNew(stackIndex);
    }
    
    HeapObjectArray<LuaObject>::Ptr objects;
};

} // namespace LucED

#endif // LUAOBJECTLIST_H
