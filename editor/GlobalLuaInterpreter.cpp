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

    String packagesDir = File(GlobalConfig::getInstance()->getConfigDirectory(),
                              "packages").getAbsoluteName();
    
    package["path"] = String() << packagesDir << "/?.lua;"
                               << packagesDir << "/?/init.lua";


    LuaVar initialModules = luaAccess.newTable();
    
    for (LuaIterator i(luaAccess); i.in(loaded);)
    {
        initialModules[i.key()] = i.value();
    }

    lucedLuaInterfaceStoreReference = luced.store();
    packageStoreReference           = package.store();
    packageLoadedStoreReference     = loaded.store();
    initialModulesStoreReference    = initialModules.store();
}


void GlobalLuaInterpreter::resetModules()
{
    LuaAccess luaAccess = LuaInterpreter::getCurrentLuaAccess();

    LuaVar loadedModules  = luaAccess.retrieve(packageLoadedStoreReference);
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
    GlobalConfig::getInstance()->readConfig();
}


void GlobalLuaInterpreter::resetModule(const String& moduleName)
{
    LuaAccess luaAccess = LuaInterpreter::getCurrentLuaAccess();

    LuaVar loadedModules  = luaAccess.retrieve(packageLoadedStoreReference);
    LuaVar initialModules = luaAccess.retrieve(initialModulesStoreReference);

    if (initialModules[moduleName].isNil())
    {
        loadedModules[moduleName].setNil();
    }
    if (GlobalConfig::getInstance()->dependsOnPackage(moduleName))
    {
        GlobalConfig::getInstance()->readConfig();
    }
}

