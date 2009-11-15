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

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <locale.h>
#include <langinfo.h>

#include "System.hpp"
#include "SystemException.hpp"
#include "FileException.hpp"

using namespace LucED;

SingletonInstance<System> System::instance;

System::System()
{
    struct passwd* passwdEntry = getpwuid(getuid());

    const char* homeVarPtr = getenv("HOME");
    
    if (homeVarPtr == NULL)
    {
        if (passwdEntry == NULL || passwdEntry->pw_dir[0] == '\0') {
            throw SystemException("environment var HOME not set and system call getpwuid() failed");
        }
        homeDirectory = passwdEntry->pw_dir;
    } else {
        homeDirectory = homeVarPtr;
    }
    
    
    if (passwdEntry == NULL || passwdEntry->pw_name[0] == '\0')
    {
        const char* userVarPtr = getenv("USER");

        if (userVarPtr == NULL) {
            userVarPtr = getenv("LOGNAME");
        }
        if (userVarPtr == NULL) {
            throw SystemException("environment vars USER and LOGNAME not set and system call getpwuid() failed");
        }
        userName = userVarPtr;
    } else {
        userName = passwdEntry->pw_name;
    }
    
    struct utsname utsNameData;
    if (uname(&utsNameData) < 0)
    {
        const char* hostNameVarPtr = getenv("HOSTNAME");
        if (hostNameVarPtr == NULL) {
            throw SystemException("environment var HOSTNAME not set and system call uname() failed");
        }
        hostName = hostNameVarPtr;
    } else {
        hostName = utsNameData.nodename;
    }
    
    defaultEncoding = nl_langinfo(CODESET);
    localeName  = setlocale(LC_ALL, NULL); // only query, because second param is NULL
}


String System::getCurrentDirectory() const
{
    MemArray<char> cwd(2000);
    do {
        if (getcwd(cwd.getPtr(), cwd.getLength()) == NULL && errno == ERANGE) {
            cwd.increaseTo(1000 + cwd.getLength());
            continue;
        }
    } while (false);
    return String() << cwd.getPtr();
}


void System::setCurrentDirectory(const String& dir)
{
    if (chdir(dir.toCString()) != 0) {
        throw FileException(errno,
                            String() << "Failed setting current directory to "
                                     << dir << ": " << strerror(errno));
    }
}

void System::setCloseOnExecFlag(int fileDescriptor)
{
    int flags = ::fcntl(fileDescriptor, F_GETFD);
    if (flags == -1) {
        throw SystemException(String() << "Error while setting close on exec flag: " << strerror(errno));
    }
    flags |= FD_CLOEXEC;
    if (::fcntl(fileDescriptor, F_SETFD, flags) == 1) {
        throw SystemException(String() << "Error while setting close on exec flag: " << strerror(errno));
    }
}
