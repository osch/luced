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

#include "LuaInterpreter.hpp"
#include "ByteBuffer.hpp"
#include "LuaException.hpp"
#include "LuaVar.hpp"
#include "LuaStackChecker.hpp"
#include "NonCopyable.hpp"
#include "LuaStateAccess.hpp"
#include "LuaCMethod.hpp"
#include "LuaFunctionArguments.hpp"

using namespace LucED;


LuaCFunctionResult LuaInterpreter::printFunction(const LuaCFunctionArguments& args)
{
    LuaAccess              luaAccess      = args.getLuaAccess();
    RawPtr<LuaInterpreter> luaInterpreter = luaAccess.getLuaInterpreter();
    
    int       numberOfArguments = args.getLength();
    LuaVar toStringFunction  = luaAccess.retrieve(luaInterpreter->storedObjects->originalToStringFunction);
    
    ASSERT(toStringFunction.isFunction());

    for (int i = 0; i < numberOfArguments; ++i)
    {
        if (i >= 1) {
            luaInterpreter->printBuffer.append("\t");
        }
        LuaVar s = toStringFunction.call(args[i]);
        luaInterpreter->printBuffer.append(s.toString());
    }
    luaInterpreter->printBuffer.append("\n");

    return LuaCFunctionResult(luaAccess);
}

LuaCFunctionResult LuaInterpreter::stdoutWriteFunction(const LuaCFunctionArguments& args)
{
    LuaAccess              luaAccess      = args.getLuaAccess();
    RawPtr<LuaInterpreter> luaInterpreter = luaAccess.getLuaInterpreter();

    int numberOfArguments = args.getLength();

    for (int i = 1; i < numberOfArguments; ++i) // args[0] is "this"-Table
    {
        luaInterpreter->printBuffer.append(args[i].toString());
    }
    return LuaCFunctionResult(luaAccess);
}

LuaCFunctionResult LuaInterpreter::ioWriteFunction(const LuaCFunctionArguments& args)
{
    LuaAccess              luaAccess      = args.getLuaAccess();
    RawPtr<LuaInterpreter> luaInterpreter = luaAccess.getLuaInterpreter();

    int numberOfArguments = args.getLength();

    if (luaInterpreter->isLucedStdoutActive) {
        for (int i = 0; i < numberOfArguments; ++i)
        {
            luaInterpreter->printBuffer.append(args[i].toString());
        }
    } else {
        LuaVar originalIoWriteFunction = luaAccess.retrieve(luaInterpreter->storedObjects->originalIoWriteFunction);
        for (int i = 0; i < numberOfArguments; ++i)
        {
            originalIoWriteFunction.call(args[i]);
        }
    }
    return LuaCFunctionResult(luaAccess);
}

LuaCFunctionResult LuaInterpreter::ioOutputFunction(const LuaCFunctionArguments& args)
{
    LuaAccess              luaAccess      = args.getLuaAccess();
    RawPtr<LuaInterpreter> luaInterpreter = luaAccess.getLuaInterpreter();

    int numberOfArguments = args.getLength();

    LuaVar lucedStdout = luaAccess.retrieve(luaInterpreter->storedObjects->lucedStdout);
    
    if (numberOfArguments >= 1) {
        if (args[0] == lucedStdout) {
            luaInterpreter->isLucedStdoutActive = true;
            return LuaCFunctionResult(luaAccess) << lucedStdout;
        } else {
            luaInterpreter->isLucedStdoutActive = false;
            LuaVar originalIoOutput = luaAccess.retrieve(luaInterpreter->storedObjects->originalIoOutputFunction);
            return LuaCFunctionResult(luaAccess) << originalIoOutput.call(args[0]);
        }
    } else {
        if (luaInterpreter->isLucedStdoutActive) {
            return LuaCFunctionResult(luaAccess) << lucedStdout;
        } else {
            LuaVar originalIoOutput = luaAccess.retrieve(luaInterpreter->storedObjects->originalIoOutputFunction);
            return LuaCFunctionResult(luaAccess) << originalIoOutput.call();
        }
    }
}

LuaCFunctionResult LuaInterpreter::testFunction(const LuaCFunctionArguments& args)
{
    LuaAccess              luaAccess      = args.getLuaAccess();

    int numberOfArguments = args.getLength();

    LuaCFunctionResult rslt(luaAccess);
    for (int i = 0; i < numberOfArguments; ++i)
    {
        String value = args[i].toString();
        rslt << args[i];
    }
    return rslt;
}

LuaCFunctionResult LuaInterpreter::doNothingFunction(const LuaCFunctionArguments& args)
{
    LuaAccess  luaAccess = args.getLuaAccess();
    
    return LuaCFunctionResult(luaAccess);
}


extern int luaopen_posix (lua_State *L);


inline lua_State* LuaInterpreter::initState(LuaInterpreter* luaInterpreter, lua_State* L)
{
    luaL_openlibs(L);
    luaopen_posix(L);
    LuaStateAccess::setLuaInterpreter(L, luaInterpreter);
    return L;
}

LuaInterpreter::LuaInterpreter()
    : rootAccess(initState(this, luaL_newstate())),
      currentAccess(rootAccess.L)
{
    storedObjects = StoredObjects::create(currentAccess);
    
    storedObjects->originalToStringFunction      = currentAccess.getGlobal("tostring")    .store();
    storedObjects->originalIoOutputFunction      = currentAccess.getGlobal("io")["output"].store();
    storedObjects->originalIoWriteFunction       = currentAccess.getGlobal("io")["write"] .store();
    storedObjects->requireFunctionStoreReference = currentAccess.getGlobal("require")     .store();
                   
    currentAccess.setGlobal("print", LuaCClosure::create<printFunction>());
    currentAccess.setGlobal("trt",   LuaCClosure::create<testFunction>());

    LuaVar lucedStdout = currentAccess.newTable();
    
    lucedStdout["write"] = LuaCClosure::create<stdoutWriteFunction>();
    lucedStdout["close"] = LuaCClosure::create<doNothingFunction>();
    lucedStdout["flush"] = LuaCClosure::create<doNothingFunction>();
    
    LuaVar io = currentAccess.getGlobal("io");
    ASSERT(io.isTable());

    io["stdout"] = lucedStdout;
    io["output"] = LuaCClosure::create<ioOutputFunction>();
    io["write"]  = LuaCClosure::create<ioWriteFunction>();
    
    storedObjects->lucedStdout = currentAccess.store(lucedStdout);
    isLucedStdoutActive = true;

    LuaVar ptrMapMetaTable = currentAccess.newTable();
           ptrMapMetaTable["__mode"] = "v";
           
    LuaVar owningPtrMap    = currentAccess.newTable();
    LuaVar   weakPtrMap    = currentAccess.newTable();
           owningPtrMap.setMetaTable(ptrMapMetaTable);
             weakPtrMap.setMetaTable(ptrMapMetaTable);
    
    storedObjects->owningPtrMapStoreReference = owningPtrMap.store();
    storedObjects->weakPtrMapStoreReference   =   weakPtrMap.store();
}


LuaInterpreter::~LuaInterpreter()
{
    storedObjects.invalidate();
    
    lua_close(rootAccess.L);
}



LuaVar LuaInterpreter::require(const String& packageName) const
{
    LuaVar requireFunction = currentAccess.retrieve(storedObjects->requireFunctionStoreReference);
    LuaVar packageLuaName  = currentAccess.toLua(packageName);
    return requireFunction.call(packageLuaName);
}

