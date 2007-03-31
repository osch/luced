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

#include "LuaStackChecker.h"

using namespace LucED;

SingletonInstance<LuaStackChecker> LuaStackChecker::instance;



int LuaStackChecker::getHighestStackIndexForGeneration(int generation) const
{
    ASSERT(generation <= newestGeneration);
    return highestStackIndices[generation - generationOffset];
}



int LuaStackChecker::registerAndGetGeneration(int stackIndex)
{
    ASSERT(highestStackIndices[newestGeneration - generationOffset] < stackIndex);
    
    highestStackIndices[newestGeneration - generationOffset] = stackIndex;

    return newestGeneration;
}



void LuaStackChecker::truncateGenerationAtStackIndex(int generation, int stackIndex)
{
    ASSERT(generation >= generationOffset);

    newestGeneration += 1;


    highestStackIndices.append(0);
    ASSERT(highestStackIndices.getLength() - 1 == (newestGeneration - generationOffset));

    if (stackIndex - 1 < highestStackIndices[generation - generationOffset]) {
        highestStackIndices[generation - generationOffset] = stackIndex - 1;
    }

    while (highestStackIndices.getLength() > 1 && highestStackIndices[0] == 0)
    {
        generationOffset += 1;
        highestStackIndices.removeAmount(0, 1);
    }
}
