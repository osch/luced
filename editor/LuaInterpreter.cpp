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
#include "File.hpp"
#include "ByteBuffer.hpp"
#include "LuaException.hpp"
#include "LuaObject.hpp"
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
    LuaObject toStringFunction  = luaAccess.retrieve(luaInterpreter->storedObjects->originalToStringFunction);
    
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
        LuaObject originalIoWriteFunction = luaAccess.retrieve(luaInterpreter->storedObjects->originalIoWriteFunction);
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

    LuaObject lucedStdout = luaAccess.retrieve(luaInterpreter->storedObjects->lucedStdout);
    
    if (numberOfArguments >= 1) {
        if (args[0] == lucedStdout) {
            luaInterpreter->isLucedStdoutActive = true;
            return LuaCFunctionResult(luaAccess) << lucedStdout;
        } else {
            luaInterpreter->isLucedStdoutActive = false;
            LuaObject originalIoOutput = luaAccess.retrieve(luaInterpreter->storedObjects->originalIoOutputFunction);
            return LuaCFunctionResult(luaAccess) << originalIoOutput.call(args[0]);
        }
    } else {
        if (luaInterpreter->isLucedStdoutActive) {
            return LuaCFunctionResult(luaAccess) << lucedStdout;
        } else {
            LuaObject originalIoOutput = luaAccess.retrieve(luaInterpreter->storedObjects->originalIoOutputFunction);
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


namespace LucED
{
class LuaInterpreter::TestClass : public HeapObject
{
public:
    static OwningPtr<TestClass> create(String name) {
        return OwningPtr<TestClass>(new TestClass(name));
    }
    LuaCFunctionResult testMethod(const LuaCFunctionArguments& args)
    {
        LuaAccess luaAccess = args.getLuaAccess();
        
        int numberOfArguments = args.getLength();
    
        LuaCFunctionResult rslt(luaAccess);
        
        rslt << luaAccess.newObject(String() << "name=" << name);
        for (int i = 0; i < numberOfArguments; ++i)
        {
            String value = args[i].toString();
            rslt << args[i];
        }
        return rslt;
    }
private:
    TestClass(String name)
        : name(name)
    {}
    String name;
};
class LuaInterpreter::TestClass2 : public HeapObject
{
public:
    static OwningPtr<TestClass2> create(String name) {
        return OwningPtr<TestClass2>(new TestClass2(name));
    }
    LuaCFunctionResult testMethod(const LuaCFunctionArguments& args)
    {
        LuaAccess luaAccess = args.getLuaAccess();

        int numberOfArguments = args.getLength();
    
        LuaCFunctionResult rslt(luaAccess);
        
        rslt << luaAccess.newObject(String() << "name=" << name);
        for (int i = 0; i < numberOfArguments; ++i)
        {
            String value = args[i].toString();
            rslt << args[i];
        }
        return rslt;
    }
private:
    TestClass2(String name)
        : name(name)
    {}
    String name;
};
}

LuaInterpreter::LuaInterpreter()
    : rootAccess(luaL_newstate()),
      currentAccess(rootAccess.L)
{
    luaL_openlibs(currentAccess.L);

    LuaStateAccess::setLuaInterpreter(currentAccess.L, this);

    storedObjects = StoredObjects::create();
    storedObjects->originalToStringFunction = currentAccess.getGlobal("tostring")    .store();
    storedObjects->originalIoOutputFunction = currentAccess.getGlobal("io")["output"].store();
    storedObjects->originalIoWriteFunction  = currentAccess.getGlobal("io")["write"] .store();

    currentAccess.setGlobal("print", LuaCFunction<printFunction>::createWrapper());
    currentAccess.setGlobal("trt",   LuaCFunction<testFunction>::createWrapper());
    
    testClassPtr   = TestClass::create("testClassObject1");
    testClassPtr2  = TestClass::create("testClassObject2");
    testClass2Ptr = TestClass2::create("testClass2Object1");
    
    currentAccess.setGlobal("ttt",   currentAccess.newObject(testClassPtr));
    currentAccess.setGlobal("t22",   currentAccess.newObject(testClassPtr2));

    currentAccess.setGlobal("tmt",    LuaCMethod<TestClass, &TestClass::testMethod>::createWrapper());
    currentAccess.setGlobal("tmt2",   LuaCMethod<TestClass2, &TestClass2::testMethod>::createWrapper());

        
    LuaObject metaTable   = currentAccess.newTable();
    LuaObject methodTable = currentAccess.newTable();
    
    methodTable["m1"] = currentAccess.getGlobal("tmt");
    methodTable["m2"] = currentAccess.getGlobal("tmt2");

    metaTable["__index"] = methodTable;
    
    currentAccess.getGlobal("ttt").setMetaTable(metaTable);
    currentAccess.getGlobal("t22").setMetaTable(metaTable);
    
    LuaObject lucedStdout = currentAccess.newTable();
    
    lucedStdout["write"] = LuaCFunction<stdoutWriteFunction>::createWrapper();
    lucedStdout["close"] = LuaCFunction<doNothingFunction>::createWrapper();
    lucedStdout["flush"] = LuaCFunction<doNothingFunction>::createWrapper();
    
    LuaObject io = currentAccess.getGlobal("io");
    ASSERT(io.isTable());

    io["stdout"] = lucedStdout;
    io["output"] = LuaCFunction<ioOutputFunction>::createWrapper();
    io["write"]  = LuaCFunction<ioWriteFunction>::createWrapper();
    
    storedObjects->lucedStdout = currentAccess.store(lucedStdout);
    isLucedStdoutActive = true;
}


LuaInterpreter::~LuaInterpreter()
{
    storedObjects.invalidate();
    
    lua_close(rootAccess.L);
}



