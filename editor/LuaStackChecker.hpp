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

#ifndef LUA_STACK_CHECKER_HPP
#define LUA_STACK_CHECKER_HPP

#include <limits.h>

#include "SingletonInstance.hpp"
#include "MemArray.hpp"

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
    
    int getHighestStackIndexForGeneration(int generation)
    {
        if (generation < generationOffset) {
            return 0;
        } else {
            return highestStackIndexForGeneration(generation);
        }
    }
    
    int getNewestGeneration() {
        return newestGeneration;
    }

    int getHighestStackIndexForNewestGeneration() {
        return getHighestStackIndexForGeneration(getNewestGeneration());
    }

    int getHighestStackIndex()
    {
        int rslt = 0;
        for (int i = newestGeneration; i >= generationOffset; --i) {
            if (rslt < highestStackIndexForGeneration(i)) {
                rslt = highestStackIndexForGeneration(i);
            }
        }
        return rslt;
    }
    
    int registerAndGetGeneration(int stackIndex);
    
    void truncateGenerationAtStackIndex(int generation, int stackIndex);

    void truncateGenerationsAtStackIndex(int stackIndex);

private:
    friend class SingletonInstance<LuaStackChecker>;

    static SingletonInstance<LuaStackChecker> instance;
    
    LuaStackChecker()
        : newestGeneration(0),
          generationOffset(0)
    {
        generations.appendAmount(1);
    }
    
    int& highestStackIndexForGeneration(int generation)
    {
        ASSERT(generationOffset <= generation && generation <= newestGeneration);

        ASSERT(generations.getLength() - 1 == (newestGeneration - generationOffset));
        
        return generations[generation - generationOffset].highest;
    }

    int& lowestStackIndexForGeneration(int generation)
    {
        ASSERT(generationOffset <= generation && generation <= newestGeneration);

        ASSERT(generations.getLength() - 1 == (newestGeneration - generationOffset));
        
        return generations[generation - generationOffset].lowest;
    }
            
    int newestGeneration;
    int generationOffset;
    
    struct Generation
    {
        Generation()
            : lowest(0), highest(0)
        {}
        int lowest;
        int highest;
    };
    ObjectArray<Generation> generations;
};
#endif // DEBUG

} // namespace LucED

#endif // LUA_STACK_CHECKER_HPP
