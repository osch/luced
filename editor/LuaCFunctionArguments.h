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

#ifndef LUACFUNCTIONARGUMENTS_H
#define LUACFUNCTIONARGUMENTS_H

#include "NonCopyable.h"
#include "LuaObject.h"

namespace LucED
{

class LuaCFunctionArguments : public NonCopyable
{
public:

    const LuaObject& operator[](int i) const {
        return argArray[i];
    }
    
    int getLength() const {
        return argArray.getLength();
    }
    
private:
    
    template<class ImplFunction>
    friend class LuaCFunction;
    
    LuaCFunctionArguments(lua_State* L)
    {
        int numberArgs = lua_gettop(L);
            
        argArray.appendAmount(numberArgs);
        
        ASSERT(argArray.getLength() == numberArgs);
        
        for (int i = 0; i < numberArgs; ++i) {
            argArray[i].stackIndex = i + 1;
        #ifdef DEBUG
            argArray[i].stackGeneration = LuaStackChecker::getInstance()->registerAndGetGeneration(i + 1);
        #endif
        }
    }
    
    ~LuaCFunctionArguments() {
    #ifdef DEBUG
        for (int i = 0; i < argArray.getLength(); ++i) {
            LuaStackChecker::getInstance()->truncateGenerationAtStackIndex(argArray[i].stackGeneration, 
                                                                           argArray[i].stackIndex);
        }
    #endif
    }
    MemArray<LuaObject> argArray;
};

} // namespace LucED

#endif // LUACFUNCTIONARGUMENTS_H
