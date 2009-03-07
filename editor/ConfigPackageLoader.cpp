/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#include "ConfigPackageLoader.hpp"
#include "DefaultConfig.hpp"
#include "GlobalConfig.hpp"
#include "File.hpp"

using namespace LucED;

ConfigPackageLoader::ConfigPackageLoader()
    : mode(MODE_NORMAL)
{}


void ConfigPackageLoader::setConfigDir(const String& configDir)
{
    this->configDir = File(configDir).getAbsoluteName();
}


LuaVar ConfigPackageLoader::loadPackageModule(LuaAccess     luaAccess,
                                              const String& moduleName) const
{
    String moduleFileNamePart = moduleName.toSubstitutedString('.', '/');
    
    LuaVar rslt(luaAccess);
    
    if (mode == MODE_NORMAL)
    {
        File packagesDir(configDir, "packages");
        
        rslt = luaAccess.loadFile(String() << packagesDir << "/" << moduleFileNamePart << ".lua");
        
        if (rslt.isNil()) {
            rslt = luaAccess.loadFile(String() << packagesDir << "/" << moduleFileNamePart << "/init.lua");
        }
    } else {
        String pseudoFileName = String() << "packages/" << moduleFileNamePart << ".lua";

        const char* script = DefaultConfig::getDefaultModule(pseudoFileName);
        
        if (script == NULL) {
            pseudoFileName = String() << "packages/" << moduleFileNamePart << "/init.lua";
            script = DefaultConfig::getDefaultModule(pseudoFileName);
        }
        if (script != NULL) {
            rslt = luaAccess.loadString(script);
        }
    }
    return rslt;
}


LuaVar ConfigPackageLoader::loadGeneralConfigModule(LuaAccess  luaAccess,
                                                    String     moduleName) const
{
    LuaVar rslt(luaAccess);

    moduleName << ".lua";

    if (mode == MODE_NORMAL)
    {
        File fileName(configDir, moduleName);
        
        rslt = luaAccess.loadFile(fileName);
    }
    else {
        const char* script = DefaultConfig::getDefaultModule(moduleName);

        if (script != NULL) {
            rslt = luaAccess.loadString(script);
        }
    }
    return rslt;    
}

