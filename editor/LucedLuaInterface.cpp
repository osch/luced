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

#include "LucedLuaInterface.hpp"
#include "LuaCMethodArgChecker.hpp"
#include "LuaArgException.hpp"
#include "Regex.hpp"
#include "GlobalLuaInterpreter.hpp"
#include "GlobalConfig.hpp"
#include "LuaIterator.hpp"

using namespace LucED;

SingletonInstance<LucedLuaInterface> LucedLuaInterface::instance;;


LucedLuaInterface* LucedLuaInterface::getInstance()
{
    return instance.getPtr();
}


LuaCFunctionResult LucedLuaInterface::getModulePrefix(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();

    if (args.getLength() <= 0 || !args[0].isString()) {
        throw LuaArgException();
    }

    String thisModuleName = args[0].toString();
    
    Regex  r("^(.*\\.)[^.]*?$");
    
    String rslt;
    
    if (r.matches(thisModuleName))
    {
        rslt = thisModuleName.getSubstring(r.getCaptureBegin(1),
                                           r.getCaptureLength(1));
    }
    
    return LuaCFunctionResult(luaAccess) << rslt;
}

LuaCFunctionResult LucedLuaInterface::resetModules(const LuaCFunctionArguments& args)
{
    if (args.getLength() > 0) {
        throw LuaArgException();
    }

    GlobalLuaInterpreter::getInstance()->resetModules();
    
    GlobalConfig::getInstance()->readConfig();

    return LuaCFunctionResult(args.getLuaAccess());
}


LuaCFunctionResult LucedLuaInterface::getCurrentView(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    if (args.getLength() > 0) {
        throw LuaArgException();
    }
    
    return LuaCFunctionResult(luaAccess) << currentView;
}

LuaCFunctionResult LucedLuaInterface::getLoadedPackageModules(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    if (args.getLength() > 0) {
        throw LuaArgException();
    }
    return GlobalLuaInterpreter::getInstance()->getLoadedPackageModules();
}

