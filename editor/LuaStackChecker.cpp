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

#include <stdio.h>

#include "LuaStackChecker.hpp"

using namespace LucED;

#ifdef DEBUG


int LuaStackChecker::registerAndGetGeneration(int stackIndex)
{
    ASSERT(highestStackIndexForGeneration(newestGeneration) < stackIndex);

    // truncate all older generations
    // highestStackIndexForGeneration(i) might be incorrect because of 
    // temporarily pushed stack indices that do not belong to any LuaVar
    // or LuaVarRef object
    for (int i = generationOffset; i <= newestGeneration; ++i) {
        if (highestStackIndexForGeneration(i) >= stackIndex) {
            truncateGenerationsAtStackIndex(stackIndex);
            break;
        }
    }

    highestStackIndexForGeneration(newestGeneration) = stackIndex;
    
    if (lowestStackIndexForGeneration(newestGeneration) == 0) {
        lowestStackIndexForGeneration(newestGeneration) = stackIndex;
    }

    return newestGeneration;
}



void LuaStackChecker::truncateGenerationsAtStackIndex(int stackIndex)
{
    for (int i = generationOffset; i <= newestGeneration; ++i)
    {
        if (highestStackIndexForGeneration(i) > stackIndex - 1) {
            highestStackIndexForGeneration(i) = stackIndex - 1;
        }
    }

    generations.appendAmount(1);
    newestGeneration += 1;

    while (generations.getLength() > 1 && (generations[0].highest < generations[0].lowest || generations[0].lowest == 0))
    {
        generationOffset += 1;
        generations.removeAmount(0, 1);
    }
}

void LuaStackChecker::truncateGenerationAtStackIndex(int generation, int stackIndex)
{
    ASSERT(generation <= newestGeneration);
    
    truncateGenerationsAtStackIndex(stackIndex);
}

#endif // DEBUG
