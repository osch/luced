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

#ifndef CONFIG_PACKAGE_LOADER_HPP
#define CONFIG_PACKAGE_LOADER_HPP

#include "LuaAccess.hpp"
#include "LuaVar.hpp"
#include "Nullable.hpp"

namespace LucED
{

class ConfigPackageLoader
{
public:
    ConfigPackageLoader();
    
    enum Mode { MODE_NORMAL,
                MODE_FALLBACK
              };
    
    void setMode(Mode mode) {
        this->mode = mode;
    }
    
    void setConfigDir(const String& configDir);
    
    LuaVar loadPackageModule(LuaAccess     luaAccess,
                             const String& moduleName) const;
    
    LuaVar loadGeneralConfigModule(LuaAccess  luaAccess,
                                   String     moduleName) const;

private:
    Mode mode;
    Nullable<String> configDir;
};

} // namespace LucED

#endif // CONFIG_PACKAGE_LOADER_HPP
