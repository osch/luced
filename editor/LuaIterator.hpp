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

#ifndef LUA_ITERATOR_HPP
#define LUA_ITERATOR_HPP

#include "LuaObject.hpp"
#include "LuaStackChecker.hpp"
#include "LuaStateAccess.hpp"

namespace LucED
{

class LuaIterator : private LuaAccess
{
public:
    LuaIterator(const LuaAccess& luaAccess, int startCounter = 0)
        : LuaAccess(luaAccess),
          keyObject(luaAccess),
          valueObject(luaAccess),
          counter(startCounter)
    {
    }
    
    LuaIterator& operator=(const int& newCounterValue) {
        counter = newCounterValue;
        return *this;
    }
    
    bool in(const LuaObject& table)
    {
        isSameLuaAccess(table);

        ASSERT(  keyObject.stackIndex <= luaStackChecker->getHighestStackIndexForGeneration(  keyObject.stackGeneration));
        ASSERT(valueObject.stackIndex <= luaStackChecker->getHighestStackIndexForGeneration(valueObject.stackGeneration));
        
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
    
    const LuaObject& key() const
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

#endif // LUA_ITERATOR_HPP
