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
#include "FileOpener.hpp"

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
        throw LuaArgException(luaAccess);
    }

    String thisModuleName = args[0].toString();
    
    Regex  r("^(.*\\.)[^.]*?$");
    
    String rslt;
    
    if (r.matches(thisModuleName))
    {
        rslt = thisModuleName.getSubstring(Pos(r.getCaptureBegin(1)),
                                           Len(r.getCaptureLength(1)));
    }
    
    return LuaCFunctionResult(luaAccess) << rslt;
}

LuaCFunctionResult LucedLuaInterface::resetModules(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();

    if (args.getLength() > 0) {
        throw LuaArgException(luaAccess);
    }

    GlobalLuaInterpreter::getInstance()->resetModules();
    
    GlobalConfig::getInstance()->readConfig();

    return LuaCFunctionResult(luaAccess);
}


LuaCFunctionResult LucedLuaInterface::getCurrentView(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    if (args.getLength() > 0) {
        throw LuaArgException(luaAccess);
    }
    
    return LuaCFunctionResult(luaAccess) << currentView;
}


LuaCFunctionResult LucedLuaInterface::getLoadedPackageModules(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    if (args.getLength() > 0) {
        throw LuaArgException(luaAccess);
    }
    return GlobalLuaInterpreter::getInstance()->getLoadedPackageModules();
}


LuaCFunctionResult LucedLuaInterface::toUpper(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    if (args.getLength() != 1 || !args[0].isString()) {
        throw LuaArgException(luaAccess);
    }

    return LuaCFunctionResult(luaAccess) << args[0].toString().toUpperUtf8();
}


LuaCFunctionResult LucedLuaInterface::toLower(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    if (args.getLength() != 1 || !args[0].isString()) {
        throw LuaArgException(luaAccess);
    }

    return LuaCFunctionResult(luaAccess) << args[0].toString().toLowerUtf8();
}

LuaCFunctionResult LucedLuaInterface::openFile(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    if (args.getLength() != 1 || !args[0].isString()) {
        throw LuaArgException(luaAccess);
    }
    
    FileOpener::start(args[0].toString());

    return LuaCFunctionResult(luaAccess);
}

LuaCFunctionResult LucedLuaInterface::existsFile(const LuaCFunctionArguments& args)
{
    LuaAccess luaAccess = args.getLuaAccess();
    
    if (args.getLength() != 1 || !args[0].isString()) {
        throw LuaArgException(luaAccess);
    }
    
    return LuaCFunctionResult(luaAccess) << File(args[0].toString()).exists();
}

