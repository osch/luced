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

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "LuaInterpreter.hpp"
#include "File.hpp"
#include "ByteBuffer.hpp"
#include "LuaException.hpp"
#include "LuaObject.hpp"
#include "LuaStoredObject.hpp"
#include "LuaStackChecker.hpp"
#include "NonCopyable.hpp"

using namespace LucED;

namespace LucED
{


class LuaPrintFunction
{
public:

    LuaCFunctionResult operator()(const LuaCFunctionArguments& args)
    {
        int numberOfArguments = args.getLength();
        LuaInterpreter* luaInterpreter = LuaInterpreter::getInstance();
        LuaObject toStringFunction = luaInterpreter->storedObjects->originalToStringFunction->retrieve();
        
        ASSERT(toStringFunction.isFunction());

        for (int i = 0; i < numberOfArguments; ++i)
        {
            if (i >= 1) {
                luaInterpreter->printBuffer.append("\t");
            }
            LuaObject s = toStringFunction.call(args[i]);
            luaInterpreter->printBuffer.append(s.toString());
        }
        luaInterpreter->printBuffer.append("\n");

        return LuaCFunctionResult();
    }
};

class LuaStdoutWriteFunction
{
public:

    LuaCFunctionResult operator()(const LuaCFunctionArguments& args)
    {
        int numberOfArguments = args.getLength();

        LuaInterpreter* luaInterpreter = LuaInterpreter::getInstance();

        for (int i = 1; i < numberOfArguments; ++i) // args[0] is "this"-Table
        {
            luaInterpreter->printBuffer.append(args[i].toString());
        }
        return LuaCFunctionResult();
    }
};

class LuaIoWriteFunction
{
public:

    LuaCFunctionResult operator()(const LuaCFunctionArguments& args)
    {
        int numberOfArguments = args.getLength();

        LuaInterpreter* luaInterpreter = LuaInterpreter::getInstance();

        if (luaInterpreter->isLucedStdoutActive) {
            for (int i = 0; i < numberOfArguments; ++i)
            {
                luaInterpreter->printBuffer.append(args[i].toString());
            }
        } else {
            LuaObject originalIoWriteFunction = luaInterpreter->storedObjects->originalIoWriteFunction->retrieve();
            for (int i = 0; i < numberOfArguments; ++i)
            {
                originalIoWriteFunction.call(args[i]);
            }
        }
        return LuaCFunctionResult();
    }
};

class LuaIoOutputFunction
{
public:

    LuaCFunctionResult operator()(const LuaCFunctionArguments& args)
    {
        int numberOfArguments = args.getLength();

        LuaInterpreter* luaInterpreter = LuaInterpreter::getInstance();
        LuaObject lucedStdout = luaInterpreter->storedObjects->lucedStdout->retrieve();
        if (numberOfArguments >= 1) {
            if (args[0] == lucedStdout) {
                luaInterpreter->isLucedStdoutActive = true;
                return LuaCFunctionResult() << lucedStdout;
            } else {
                luaInterpreter->isLucedStdoutActive = false;
                LuaObject originalIoOutput = luaInterpreter->storedObjects->originalIoOutputFunction->retrieve();
                return LuaCFunctionResult() << originalIoOutput.call(args[0]);
            }
        } else {
            if (luaInterpreter->isLucedStdoutActive) {
                return LuaCFunctionResult() << lucedStdout;
            } else {
                LuaObject originalIoOutput = luaInterpreter->storedObjects->originalIoOutputFunction->retrieve();
                return LuaCFunctionResult() << originalIoOutput.call();
            }
        }
    }
};

class LuaTestFunction
{
public:

    LuaCFunctionResult operator()(const LuaCFunctionArguments& args)
    {
        int numberOfArguments = args.getLength();

        LuaCFunctionResult rslt;
        for (int i = 0; i < numberOfArguments; ++i)
        {
            String value = args[i].toString();
            rslt << args[i];
        }
        return rslt;
    }
};

class LuaDoNothingFunction
{
public:

    LuaCFunctionResult operator()(const LuaCFunctionArguments& args)
    {
        return LuaCFunctionResult();
    }
};

} // namespace LucED

SingletonInstance<LuaInterpreter> LuaInterpreter::instance;;


LuaInterpreter::LuaInterpreter()
{
#ifdef DEBUG
    LuaStackChecker::getInstance(); // assure that StackChecker exists for all LuaObjects
#endif
    
    L = luaL_newstate();

    LuaObject::L       = L;
    LuaStoredObject::L = L;
    
    luaL_openlibs(L);

    storedObjects = StoredObjects::create();
    storedObjects->originalToStringFunction = LuaStoredObject::store(getGlobal("tostring"));
    storedObjects->originalIoOutputFunction = LuaStoredObject::store(getGlobal("io")["output"]);
    storedObjects->originalIoWriteFunction  = LuaStoredObject::store(getGlobal("io")["write"]);

    setGlobal("print", LuaCFunction<LuaPrintFunction>());
    setGlobal("trt",   LuaCFunction<LuaTestFunction>());
    
    LuaObject lucedStdout = LuaObject::Table();
    
    lucedStdout["write"] = LuaCFunction<LuaStdoutWriteFunction>();
    lucedStdout["close"] = LuaCFunction<LuaDoNothingFunction>();
    lucedStdout["flush"] = LuaCFunction<LuaDoNothingFunction>();
    
    LuaObject io = getGlobal("io");
    ASSERT(io.isTable());

    io["stdout"] = lucedStdout;
    io["output"] = LuaCFunction<LuaIoOutputFunction>();
    io["write"]  = LuaCFunction<LuaIoWriteFunction>();
    
    storedObjects->lucedStdout = LuaStoredObject::store(lucedStdout);
    isLucedStdoutActive = true;
}

LuaInterpreter::~LuaInterpreter()
{
    storedObjects.invalidate();
    
    lua_close(L);
    
    LuaObject::L       = NULL;
    LuaStoredObject::L = NULL;
}



LuaInterpreter::Result LuaInterpreter::executeScript(const char* scriptBegin, long scriptLength, String name)
{
    printBuffer = "";
    int oldTop = lua_gettop(L);
    int error = luaL_loadbuffer(L, scriptBegin, scriptLength, name.toCString())
            || lua_pcall(L, 0, LUA_MULTRET, 0);

    if (error) {
        LuaException ex(lua_tostring(L, -1));
        lua_pop(L, 1);
        throw ex;
    }
    Result rslt;
           rslt.output = printBuffer;
    for (int i = oldTop + 1, n = lua_gettop(L); i <= n; ++i) {
        rslt.objects.appendObjectWithStackIndex(i);
    }
    printBuffer = "";

    return rslt;
}

LuaInterpreter::Result LuaInterpreter::executeExpression(const char* scriptBegin, long scriptLength, String name)
{
    printBuffer = "";
    
    String script = "return ";
    script.append(scriptBegin, scriptLength);
    
    int oldTop = lua_gettop(L);
    int error = luaL_loadbuffer(L, script.toCString(), script.getLength(), name.toCString())
            || lua_pcall(L, 0, LUA_MULTRET, 0);

    if (error) {
        LuaException ex(lua_tostring(L, -1));
        lua_pop(L, 1);
        throw ex;
    }

    Result rslt;
           rslt.output = printBuffer;
    for (int i = oldTop + 1, n = lua_gettop(L); i <= n; ++i) {
        rslt.objects.appendObjectWithStackIndex(i);
    }
    printBuffer = "";

    return rslt;
}

LuaInterpreter::Result LuaInterpreter::executeFile(String name)
{
    ByteBuffer buffer;
    File(name).loadInto(buffer);
    return executeScript((const char*) buffer.getTotalAmount(), buffer.getLength(), name);
}


LuaObject LuaInterpreter::getGlobal(const char* name)
{
    lua_getglobal(L, name);
    return LuaObject(lua_gettop(L));
}


void LuaInterpreter::setGlobal(const char* name, LuaObject value)
{
    lua_pushvalue(L, value.stackIndex);
    lua_setglobal(L, name);
}


void LuaInterpreter::clearGlobal(const char* name)
{
    lua_pushnil(L);
    lua_setglobal(L, name);
}

