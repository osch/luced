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

#ifndef GLOBAL_LUA_INTERPRETER_HPP
#define GLOBAL_LUA_INTERPRETER_HPP

#include "SingletonInstance.hpp"
#include "LuaInterpreter.hpp"
#include "RawPtr.hpp"
#include "ConfigPackageLoader.hpp"

namespace LucED
{

class GlobalLuaInterpreter : public LuaInterpreter
{
public:
    static RawPtr<GlobalLuaInterpreter> getInstance() {
        return instance.getPtr();
    }

    void resetModules();
    
    void setConfigDir(const String& configDir);

    void setMode(ConfigPackageLoader::Mode mode);
    
    LuaVar requireConfigPackage(const String& packageName);
    
    LuaVar getGeneralConfigModule(const String& moduleName);
    
    LuaVar getLoadedPackageModules() {
        return LuaInterpreter::getCurrentLuaAccess().retrieve(loadedPackagesStoreReference);
    }
private:
    friend class SingletonInstance<GlobalLuaInterpreter>;
    
    static SingletonInstance<GlobalLuaInterpreter> instance;

    GlobalLuaInterpreter();
    
    String modulesDir;
    
    static LuaCFunctionResult packageLocalRequireFunction(const LuaCFunctionArguments& args, 
                                                          LuaVarRef luaInterpreterVar,
                                                          LuaVarRef currentPackageNameVar,
                                                          LuaVarRef environmentVar);
    
    LuaStoredObjectReference lucedLuaInterfaceStoreReference;
    LuaStoredObjectReference loadedModulesStoreReference;
    LuaStoredObjectReference initialModulesStoreReference;
    LuaStoredObjectReference packageModuleStoreReference;

    LuaStoredObjectReference loadedPackagesStoreReference;
    
    ConfigPackageLoader configPackageLoader;
};

} // namespace LucED

#endif // GLOBAL_LUA_INTERPRETER_HPP
