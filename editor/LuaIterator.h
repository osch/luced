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

#ifndef LUAITERATOR_H
#define LUAITERATOR_H

#include "LuaObject.h"
#include "LuaStackChecker.h"

namespace LucED
{

class LuaIterator
{
public:
    LuaIterator(int startCounter = 0)
        : counter(startCounter)
    {}
    
    LuaIterator& operator=(const int& newCounterValue) {
        counter = newCounterValue;
        return *this;
    }
    
    bool in(const LuaObject& table)
    {
        ASSERT(  keyObject.stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(  keyObject.stackGeneration));
        ASSERT(valueObject.stackIndex <= LuaStackChecker::getInstance()->getHighestStackIndexForGeneration(valueObject.stackGeneration));

        lua_State* L = LuaObject::L;
        
        lua_pushvalue(L, keyObject.stackIndex);

        bool rslt = lua_next(L, table.stackIndex);

        if (rslt) {
            lua_replace(L, valueObject.stackIndex);
            lua_replace(L,   keyObject.stackIndex);
        } else {
              keyObject.setToNil();
            valueObject.setToNil();
        }
        return rslt;
    }
    
    LuaObject key() const
    {
        return keyObject;
    }
    
    const LuaObject& value() const
    {
        return valueObject;
    }
    
    void operator+=(int addCounter) {
        counter += addCounter;
    }
    
    void operator-=(int minusCounter) {
        counter -= minusCounter;
    }
    
    int operator++() {
        return ++counter;
    }
    
    int operator++(int) {
        return counter++;
    }
    
    int operator--() {
        return --counter;
    }
    
    int operator--(int) {
        return counter--;
    }
    
    operator int() const {
        return counter;
    }
    
private:
    int       counter;
    LuaObject keyObject;
    LuaObject valueObject;
};

} // namespace LucED

#endif // LUAITERATOR_H
