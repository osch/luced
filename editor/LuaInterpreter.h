/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch at luced dot de
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

#ifndef LUAINTERPRETER_H
#define LUAINTERPRETER_H

#include <string>


#include "HeapObject.h"
#include "ObjectArray.h"
#include "LuaObject.h"
#include "OwningPtr.h"

namespace LucED {

using std::string;

class LuaInterpreter : public HeapObject, private LuaInterpreterAccessToLuaObject
{
public:
    typedef OwningPtr<LuaInterpreter> Ptr;
    
    static Ptr create() {
        return Ptr(new LuaInterpreter());
    }
    
    virtual ~LuaInterpreter();
    
    void executeFile(string name);
    LuaObject getGlobal(const char* name);
    void setGlobal(const char* name, LuaObject value);
    void clearGlobal(const char* name);
    
private:
    friend class LuaObjectAccessToLuaInterpreter;
    
    class TableInfo
    {
    public:
        TableInfo(int stackIndex) : usageCounter(0), stackIndex(stackIndex) {}
        int usageCounter;
        int stackIndex;
    };
    
    LuaInterpreter();
    void releaseTable(int index);
    int newLuaObject(int stackIndex);
    
    lua_State *L;
    ObjectArray<TableInfo> tables;
};


inline void LuaObjectAccessToLuaInterpreter::incTableRefCounter(LuaInterpreter *lua, int index) {
    if (lua != NULL) {
        lua->tables[index].usageCounter += 1;
        incRefCounter(lua);
    }
}

inline void LuaObjectAccessToLuaInterpreter::decTableRefCounter(LuaInterpreter *lua, int index) {
    if (lua != NULL) {
        lua->tables[index].usageCounter -= 1;
        if (lua->tables[index].usageCounter == 0) {
            lua->releaseTable(index);
        }
        decRefCounter(lua);
    }
}

inline lua_State*  LuaObjectAccessToLuaInterpreter::getL(LuaInterpreter *lua)
{
    return lua->L;
}

inline int LuaObjectAccessToLuaInterpreter::getStackIndex(LuaInterpreter *lua, int index)
{
    return lua->tables[index].stackIndex;
}

inline int LuaObjectAccessToLuaInterpreter::newLuaObject(LuaInterpreter *lua, int stackIndex)
{
    return lua->newLuaObject(stackIndex);
}


} // namespace LucED


#endif // LUAINTERPRETER_H
