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

#include "LuaStateAccess.hpp"
#include "LuaInterpreter.hpp"
#include "HeapObject.hpp"

using namespace LucED;

lua_State* LuaStateAccess::currentState = NULL;


namespace // anonymous namespace
{
class LuaStateAccess_HeapObjectRefManipulator : private HeapObjectRefManipulator
{
public:
    static void incRefCounter(const HeapObject* obj) {
        HeapObjectRefManipulator::incRefCounter(obj);
    }
    static void decRefCounter(const HeapObject* obj) {
        HeapObjectRefManipulator::decRefCounter(obj);
    }
    static void incWeakCounter(const HeapObject* obj) {
        HeapObjectRefManipulator::incWeakCounter(obj);
    }
    static void decWeakCounter(const HeapObject* obj) {
        HeapObjectRefManipulator::decWeakCounter(obj);
    }
    static void resetInitialOwnership(const HeapObject* obj) {
        HeapObjectRefManipulator::resetInitialOwnership(obj);
    }
};
} // anonymous namespace


#ifdef DEBUG
LuaStackChecker* LuaStateAccess::getLuaStackChecker(lua_State* L)
{
    ExtraLuaStateData* extra = (ExtraLuaStateData*)((char*)(L) - sizeof(ExtraLuaStateData));
    
    if (extra->stackChecker == NULL)
    {
        OwningPtr<LuaStackChecker> stackChecker = LuaStackChecker::create();
        LuaStateAccess_HeapObjectRefManipulator::incRefCounter(stackChecker);
        extra->stackChecker = stackChecker.getRawPtr();
    }
    return (LuaStackChecker*) extra->stackChecker;
}
#endif

#ifdef DEBUG
OwningPtr<LuaStackChecker> LuaStateAccess::replaceLuaStackChecker(lua_State* L, OwningPtr<LuaStackChecker> newStackChecker)
{
    ExtraLuaStateData* extra = (ExtraLuaStateData*)((char*)(L) - sizeof(ExtraLuaStateData));
    
    OwningPtr<LuaStackChecker> oldStackChecker;
    
    if (extra->stackChecker != NULL)
    {
        {
            LuaStackChecker* p = (LuaStackChecker*) extra->stackChecker;
            LuaStateAccess_HeapObjectRefManipulator::resetInitialOwnership(p);
            oldStackChecker = OwningPtr<LuaStackChecker>(p);
            // no decRefCounter, because OwningPtr takes given Ownerchip
        }
        
        LuaStateAccess_HeapObjectRefManipulator::incRefCounter(newStackChecker);
        extra->stackChecker = newStackChecker.getRawPtr();
    }
    return oldStackChecker;
}
#endif

void LucED::LuaStateAccess_freeExtraLuaStateData(ExtraLuaStateData* extraLuaStateData)
{
#ifdef DEBUG
    if (extraLuaStateData->stackChecker != NULL) {
        LuaStateAccess_HeapObjectRefManipulator::decRefCounter((LuaStackChecker*)(extraLuaStateData->stackChecker));
    }
#endif
    if (extraLuaStateData->luaInterpreter != NULL) {
        LuaStateAccess_HeapObjectRefManipulator::decWeakCounter((LuaInterpreter*)(extraLuaStateData->luaInterpreter));
    }
}


void LucED::LuaStateAccess_incRefForLuaInterpreter(void* luaInterpreter)
{
    if (luaInterpreter != NULL) {
        LuaStateAccess_HeapObjectRefManipulator::incWeakCounter((LuaInterpreter*)luaInterpreter);
    }
}


void LuaStateAccess::setLuaInterpreter(lua_State* L, RawPtr<LuaInterpreter> luaInterpreter)
{
    ExtraLuaStateData* extra = (ExtraLuaStateData*)((char*)(L) - sizeof(ExtraLuaStateData));
    ASSERT(extra->luaInterpreter == NULL);
    extra->luaInterpreter = luaInterpreter.getRawPtr();
    LuaStateAccess_HeapObjectRefManipulator::incWeakCounter(luaInterpreter.getRawPtr());
}



