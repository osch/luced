/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2012 Oliver Schmidt, oliver at luced dot de
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

#include "UserDefinedActionMethods.hpp"
#include "QualifiedName.hpp"
#include "GlobalLuaInterpreter.hpp"
#include "LuaException.hpp"
#include "ConfigException.hpp"
#include "GlobalConfig.hpp"
#include "ViewLuaInterface.hpp"

using namespace LucED;

LuaVar UserDefinedActionMethods::getLuaActionFunction(ActionId actionId)
{
    QualifiedName fullActionName(actionId.toString());
    String        moduleName = fullActionName.getQualifier();
    LuaAccess     luaAccess  = GlobalLuaInterpreter::getInstance()->getCurrentLuaAccess();
    LuaVar        rslt(luaAccess);
    
    if (moduleName.getLength() > 0 && moduleName != "builtin")
    {
        LuaVar module(luaAccess);
        try
        {
            module = GlobalLuaInterpreter::getInstance()->requireConfigPackage(moduleName);
        } 
        catch (LuaException& ex) {
        }
        if (!module.isNil())
        {
            LuaVar actionGetter = module["getAction"];
            if (!actionGetter.isNil()) {
                if (!actionGetter.isFunction()) {
                    throw ConfigException(String() << "'" << moduleName 
                                                   << ".getAction' must be function");
                }
                rslt = actionGetter.call(fullActionName.getName());
            }
        }
    }
    else if (moduleName.getLength() == 0)
    {
        rslt = GlobalConfig::getInstance()->getTemporaryUserDefinedAction(luaAccess, actionId);
    }
    return rslt;
}

bool UserDefinedActionMethods::hasActionMethod(ActionId actionId)
{
    if (actionId.isBuiltin()) {
        return false;
    }
    LuaVar a = getLuaActionFunction(actionId);
    
    if (shellInvocationHandler.isValid()) {
        return !a.isNil();
    }
    else {
        return a.isFunction();
    }
}


bool UserDefinedActionMethods::UserDefinedActionMethods::invokeActionMethod(ActionId actionId)
{
    if (actionId.isBuiltin() || !textEditor.isValid()) {
        return false;
    }

    bool processed = false;
    
    LuaVar luaActionFunction = getLuaActionFunction(actionId);
            
    if (luaActionFunction.isFunction()) 
    {
        TextData::HistorySection::Ptr historySectionHolder = textEditor->getTextData()->createHistorySection();
        
        luaActionFunction.call(textEditor->getViewLuaInterface());

        processed = true;
    }
    else if (luaActionFunction.isTable())
    {
        LuaVar shellScript = luaActionFunction["shellScript"];
        if (shellScript.isString())
        {
            if (shellInvocationHandler.isValid())
            {
                String script = shellScript.toString();
    
                if (script.getLength() > 0)
                {
                    shellInvocationHandler->beforeShellInvocation();

                    HeapHashMap<String,String>::Ptr env = HeapHashMap<String,String>::create();
                                                    env->set("FILE", textEditor->getTextData()->getFileName());
    
                    Commandline::Ptr commandline = Commandline::create();
                                     commandline->append("/bin/sh");
                                     
                    ProgramExecutor::start(commandline,
                                           script,
                                           env,
                                           newCallback(shellInvocationHandler, &ShellInvocationHandler::afterShellInvocation));
                    processed = true;
                }
            }
        }
        else {
            throw ConfigException(String() << "Action '" << actionId.toString() << "' is table but has no field 'shellScript'");
        }
    }
    else {
        throw ConfigException(String() << "Action '" << actionId.toString() << "' must be function or table");
    }
    
    if (processed)
    {
        textEditor->showCursor();
        textEditor->rememberCursorPixX();
    }
    return processed;
}
