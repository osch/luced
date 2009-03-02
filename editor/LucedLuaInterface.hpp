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

#ifndef LUCED_LUA_INTERFACE_HPP
#define LUCED_LUA_INTERFACE_HPP

#include "HeapObject.hpp"
#include "SingletonInstance.hpp"
#include "LuaCFunctionResult.hpp"
#include "LuaCFunctionArguments.hpp"
#include "ViewLuaInterface.hpp"
#include "LuaArgException.hpp"

namespace LucED
{

class LucedLuaInterface : public HeapObject
{
public:
    static LucedLuaInterface* getInstance();

    LuaCFunctionResult getModulePrefix(const LuaCFunctionArguments& args);

    LuaCFunctionResult getCurrentView(const LuaCFunctionArguments& args);

    LuaCFunctionResult resetModules(const LuaCFunctionArguments& args);

    LuaCFunctionResult getLoadedPackageModules(const LuaCFunctionArguments& args);
    
    void setCurrentView(RawPtr<ViewLuaInterface> view) {
        currentView = view;
    }

private:
    friend class SingletonInstance<LucedLuaInterface>;
    static       SingletonInstance<LucedLuaInterface> instance;
    
    WeakPtr<ViewLuaInterface> currentView;
};

} // namespace LucED

#endif // LUCED_LUA_INTERFACE_HPP
