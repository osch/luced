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

#include "LuaAccess.hpp"
#include "LuaStoredObjectReference.hpp"
#include "LuaObject.hpp"

#ifndef LUA_INTERPRETER_HPP
#define LUA_INTERPRETER_HPP

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "HeapObject.hpp"
#include "ObjectArray.hpp"
#include "OwningPtr.hpp"
#include "LuaObjectList.hpp"
#include "String.hpp"
#include "LuaAccess.hpp"

namespace LucED
{

class LuaCFunctionArguments;
class LuaCFunctionResult;
template
<
    LuaCFunctionResult F(const LuaCFunctionArguments& args)
>
class LuaCFunction;
template
<
    class C,
    LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
>
class LuaCMethod;


class LuaInterpreter : public HeapObject
{
public:
    typedef OwningPtr<LuaInterpreter> Ptr;
    
    static Ptr create() {
        return Ptr(new LuaInterpreter());
    }

    LuaAccess getCurrentLuaAccess() const {
        return currentAccess;
    }
    
    class LuaCFunctionAccess
    {
    private:
        template
        <
            LuaCFunctionResult F(const LuaCFunctionArguments& args)
        >
        friend class LuaCFunction;

        template
        <
            class C,
            LuaCFunctionResult (C::*M)(const LuaCFunctionArguments& args)
        >
        friend class LuaCMethod;
        
        static void setCurrentLuaAccess(LuaInterpreter* luaInterpreter, const LuaAccess& luaAccess) {
            luaInterpreter->currentAccess = luaAccess;
        }
    };
    
    class PrintBufferAccess
    {
    private:
        friend class LuaAccess;
        
        static String getPrintBuffer(LuaInterpreter* luaInterpreter) {
            return luaInterpreter->printBuffer;
        }
        static void setPrintBuffer(LuaInterpreter* luaInterpreter, const String& printBuffer) {
            luaInterpreter->printBuffer = printBuffer;
        }
    };

#ifdef DEBUG
    bool isCorrect() const {
        return    (rootAccess.isCorrect())
               && (rootAccess.getLuaInterpreter() == this)
               && (currentAccess.isCorrect())
               && (currentAccess.getLuaInterpreter() == this);
    }
#endif
    
protected:
    LuaInterpreter();
    ~LuaInterpreter();

private:
    static LuaCFunctionResult printFunction      (const LuaCFunctionArguments& args);
    static LuaCFunctionResult stdoutWriteFunction(const LuaCFunctionArguments& args);
    static LuaCFunctionResult ioWriteFunction    (const LuaCFunctionArguments& args);
    static LuaCFunctionResult ioOutputFunction   (const LuaCFunctionArguments& args);
    static LuaCFunctionResult testFunction       (const LuaCFunctionArguments& args);
    static LuaCFunctionResult doNothingFunction  (const LuaCFunctionArguments& args);

    class StoredObjects : public HeapObject
    {
    public:
        typedef OwningPtr<StoredObjects> Ptr;
        
        static Ptr create() {
            return Ptr(new StoredObjects());
        }
      
        LuaStoredObjectReference originalToStringFunction;
        LuaStoredObjectReference originalIoOutputFunction;
        LuaStoredObjectReference originalIoWriteFunction;
        LuaStoredObjectReference lucedStdout;

    private:
        StoredObjects()
        {}
    };
    
    StoredObjects::Ptr storedObjects;
    bool isLucedStdoutActive;
    
    String printBuffer;

    LuaAccess rootAccess;
    LuaAccess currentAccess;

////////////////// TEST:
    class TestClass;
    class TestClass2;

    OwningPtr<TestClass> testClassPtr;
    OwningPtr<TestClass> testClassPtr2;
    OwningPtr<TestClass2> testClass2Ptr;
};



} // namespace LucED



#endif // LUA_INTERPRETER_HPP
