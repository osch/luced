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

#ifndef LUA_STACK_CHECKER_HPP
#define LUA_STACK_CHECKER_HPP

#include "SingletonInstance.h"
#include "MemArray.h"

namespace LucED
{

#ifdef DEBUG
class LuaStackChecker : public HeapObject
{
public:

    static LuaStackChecker* getInstance()
    {
        return instance.getPtr();
    }
    
    int getHighestStackIndexForGeneration(int generation) const;
    
    int registerAndGetGeneration(int stackIndex);
    
    void truncateGenerationAtStackIndex(int generation, int stackIndex);

    int getNewestGeneration() {
        return newestGeneration;
    }

private:
    friend class SingletonInstance<LuaStackChecker>;

    static SingletonInstance<LuaStackChecker> instance;
    
    LuaStackChecker()
        : newestGeneration(0),
          generationOffset(0)
    {
        highestStackIndices.append(0);
    }
        
    int newestGeneration;
    int generationOffset;
    MemArray<int> highestStackIndices;
};
#endif // DEBUG

} // namespace LucED

#endif // LUA_STACK_CHECKER_HPP
