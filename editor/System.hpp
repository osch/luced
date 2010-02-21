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

#ifndef SYSTEM_HPP
#define SYSTEM_HPP

#include "HeapObject.hpp"
#include "SingletonInstance.hpp"
#include "String.hpp"
#include "TimeVal.hpp"
#include "RawPtr.hpp"

namespace LucED
{

class System : public HeapObject
{
public:
    static RawPtr<System> getInstance() {
        return instance.getPtr();
    }
    
    String getHomeDirectory() const {
        return homeDirectory;
    }
    
    String getUserName() const {
        return userName;
    }
    
    String getHostName() const {
        return hostName;
    }
    
    String getCurrentDirectory() const;
    
    void setCurrentDirectory(const String& dir);
    
    static int select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, TimeVal* timeVal)
    {
        return ::select(n, readfds, writefds, exceptfds, &timeVal->timeval);
    }
    
    static void setCloseOnExecFlag(int fileDescriptor);
    
    String getDefaultEncoding() const {
        return defaultEncoding;
    }
    String getCEncoding() const {
        return cEncoding;
    }
    String getDefaultLocale() const {
        return defaultLocale;
    }

private:
    friend class SingletonInstance<System>;
    static SingletonInstance<System> instance;
    
    System();

    String homeDirectory;
    String userName;
    String hostName;
    String defaultLocale;
    String defaultEncoding;
    String cEncoding;
};


} // namespace LucED

#endif // SYSTEM_HPP
