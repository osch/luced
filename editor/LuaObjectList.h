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

#include "LuaObject.h"
#include "HeapObjectArray.h"

namespace LucED {

class LuaObjectList
{
public:
    LuaObjectList() : objects(HeapObjectArray<LuaObject>::create()) {}
    void append(LuaObject object) {
        objects->append(object);
    }
    const LuaObject& operator[](int index) const {
        return objects->at(index);
    }
    int getLength() const {
        return objects->getLength();
    }
private:
    HeapObjectArray<LuaObject>::Ptr objects;
};

LuaObjectList operator&(const LuaObject& object1, const LuaObject& object2)
{
    LuaObjectList list;
    list.append(object1);
    list.append(object2);
    return list;
}

LuaObjectList operator&(LuaObjectList list, const LuaObject& object)
{
    list.append(object);
    return list;
}

} // namespace LucED

#endif // LUAOBJECTLIST_H
