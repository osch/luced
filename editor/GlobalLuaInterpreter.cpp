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

#include "GlobalLuaInterpreter.hpp"
#include "LucedLuaInterface.hpp"
#include "GlobalConfig.hpp"
#include "File.hpp"
#include "LuaIterator.hpp"
#include "LuaCClosure.hpp"

using namespace LucED;

SingletonInstance<GlobalLuaInterpreter> GlobalLuaInterpreter::instance;;

GlobalLuaInterpreter::GlobalLuaInterpreter()
{
    LuaAccess luaAccess = LuaInterpreter::getCurrentLuaAccess();
    
    LuaVar luced = luaAccess.toLua(LucedLuaInterface::getInstance());

    luaAccess.setGlobal("luced", luced);
    
    LuaVar package = luaAccess.getGlobal("package");
    LuaVar loaded = package["loaded"];
    loaded["luced"] = luced;

    packagesDir = File(GlobalConfig::getInstance()->getConfigDirectory(),
                       "packages").getAbsoluteName();
    
    modulesDir  = File(GlobalConfig::getInstance()->getConfigDirectory(),
                       "modules").getAbsoluteName();

    package["path"] = String() << modulesDir << "/?.lua;"
                               << modulesDir << "/?/init.lua";


    LuaVar initialModules = luaAccess.newTable();
    
    for (LuaIterator i(luaAccess); i.in(loaded);)
    {
        initialModules[i.key()] = i.value();
    }

    lucedLuaInterfaceStoreReference = luced.store();
    loadedModulesStoreReference     = loaded.store();
    initialModulesStoreReference    = initialModules.store();
    loadedPackagesStoreReference    = luaAccess.newTable().store();
}


void GlobalLuaInterpreter::resetModules()
{
    LuaAccess luaAccess = LuaInterpreter::getCurrentLuaAccess();

    LuaVar loadedModules  = luaAccess.retrieve(loadedModulesStoreReference);
    LuaVar initialModules = luaAccess.retrieve(initialModulesStoreReference);

    for (LuaIterator i(luaAccess); i.in(loadedModules);)
    {
        if (initialModules[i.key()].isNil())
        {
            loadedModules[i.key()].setNil();
        }
    }
    for (LuaIterator i(luaAccess); i.in(initialModules);)
    {
        loadedModules[i.key()] = i.value();
    }

    loadedPackagesStoreReference    = luaAccess.newTable().store();
}


static const char* THIS_PREFIX = "this.";

LuaCFunctionResult GlobalLuaInterpreter::packageLocalRequireFunction(const LuaCFunctionArguments& args, 
                                                                     LuaVarRef luaInterpreterVar,
                                                                     LuaVarRef currentPackageNameVar,
                                                                     LuaVarRef environmentVar)
{
    LuaAccess luaAccess = args.getLuaAccess();

    WeakPtr<GlobalLuaInterpreter> luaInterpreter = luaInterpreterVar.toWeakPtr<GlobalLuaInterpreter>();
    
    if (!luaInterpreter.isValid() || !currentPackageNameVar.isString()) {
        throw LuaException("internal error");
    }
    String currentPackageName = currentPackageNameVar.toString();

    if (args.getLength() == 0 || !args[0].isString())
    {
        throw LuaException(String() << "function 'require' needs string argument but got " << args[0].getTypeName());
    }
    String requiredModuleName = args[0].toString();
    
    if (requiredModuleName == "this") {
        throw LuaException(String() << "Module name '" << requiredModuleName << "' not allowed");
    }
    if (!requiredModuleName.startsWith(THIS_PREFIX))
    {
        return luaInterpreter->require(requiredModuleName);
    }
    
    String absoluteRequiredModuleName = String() << currentPackageName << "." 
                                                 << requiredModuleName.getTail(strlen(THIS_PREFIX));

    LuaVar loadedPackages = luaAccess.retrieve(luaInterpreter->loadedPackagesStoreReference);
    LuaVar loadedPackage  = loadedPackages[absoluteRequiredModuleName];
    if (!loadedPackage.isNil()) {
        return loadedPackage;
    }

    String absoluteRequiredModuleFileNamePart  = absoluteRequiredModuleName.toSubstitutedString('.', '/');

    String fileName = String() << luaInterpreter->packagesDir << "/" << absoluteRequiredModuleFileNamePart 
                                                              << ".lua";
    LuaVar startModule = luaAccess.loadFile(fileName);
    
    if (startModule.isNil())
    {
        fileName = String() << luaInterpreter->packagesDir << "/" << absoluteRequiredModuleFileNamePart 
                                                           << "/init.lua";
        startModule = luaAccess.loadFile(fileName);
    }
    
    if (startModule.isNil()) {
        throw LuaException(String() << "cannot find module '" << absoluteRequiredModuleName << "'");
    }

    startModule.setFunctionEnvironment(environmentVar);
    
    LuaVar rslt = startModule.call(absoluteRequiredModuleName);

    if (rslt.isNil()) {
        rslt = true;
    }
    loadedPackages[absoluteRequiredModuleName] = rslt;
    return rslt;
}


LuaVar GlobalLuaInterpreter::requirePackage(const String& packageName)
{
    LuaAccess luaAccess = LuaInterpreter::getCurrentLuaAccess();

    if (packageName.startsWith(THIS_PREFIX) || packageName == "this") {
        throw LuaException(String() << "Package name '" << packageName << "' not allowed");
    }
    
    LuaVar loadedPackages = luaAccess.retrieve(loadedPackagesStoreReference);
    
    LuaVar loadedPackage = loadedPackages[packageName];
    
    if (!loadedPackage.isNil()) {
        return loadedPackage;
    }
    
    String packageFileNamePart = packageName.toSubstitutedString('.', '/');
    
    LuaVar startModule = luaAccess.loadFile(String() << packagesDir << "/" << packageFileNamePart << ".lua");
    
    if (startModule.isNil()) {
        startModule    = luaAccess.loadFile(String() << packagesDir << "/" << packageFileNamePart << "/init.lua");
    }
    

    if (startModule.isNil()) {
        throw LuaException(String() << "cannot find package '" << packageName << "'");
    }
    
    LuaVar env = luaAccess.newTable();

    env["require"] = LuaCClosure::create<GlobalLuaInterpreter::packageLocalRequireFunction>(luaAccess.toLua(this), 
                                                                                            luaAccess.toLua(packageName), 
                                                                                            env);
    LuaVar metaEnv = luaAccess.newTable();
    
    metaEnv["__index"] = luaAccess.getGlobalVariables();
    
    env.setMetaTable(metaEnv);
    
    startModule.setFunctionEnvironment(env);
    
    LuaVar rslt = startModule.call(packageName);

    if (rslt.isNil()) {
        rslt = true;
    }
    loadedPackages[packageName] = rslt;
    return rslt;
}

