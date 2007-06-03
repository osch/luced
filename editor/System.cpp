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

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include "System.hpp"
#include "BaseException.hpp"
#include "SystemException.hpp"

using namespace LucED;

SingletonInstance<System> System::instance;

System::System()
{
    const char* ptr = getenv("HOME");
    
    if (ptr == NULL)
    {
       struct passwd* passwdEntry = getpwuid(getuid());
       if (passwdEntry != NULL && passwdEntry->pw_dir[0] != '\0') {
          ptr = passwdEntry->pw_dir;
       } else {
          throw SystemException("system call getpwuid() failed");
       }
    }
    homeDirectory = ptr;
}

