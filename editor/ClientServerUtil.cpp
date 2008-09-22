/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2007 Oliver Schmidt, oliver at luced dot de
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

#include "ClientServerUtil.hpp"
#include "System.hpp"
#include "RawPtr.hpp"

using namespace LucED;


GuiRootProperty ClientServerUtil::getServerRunningProperty(const String& instanceName)
{
    RawPtr<System> sys = System::getInstance();
    
    String name = String() << "LUCED_SERVER_HOST_" << sys->getHostName() << "_USER_" << sys->getUserName();

    if (instanceName.getLength() > 0) {
        name << "_INSTANCE_" << instanceName;
    }
    
    return GuiRootProperty(name);
}


GuiRootProperty ClientServerUtil::getServerCommandProperty(const String& instanceName)
{
    RawPtr<System> sys = System::getInstance();

    String name = String() << "LUCED_COMMAND_HOST_" << sys->getHostName() << "_USER_" << sys->getUserName();

    if (instanceName.getLength() > 0) {
        name << "_INSTANCE_" << instanceName;
    }

    return GuiRootProperty(name);
}




