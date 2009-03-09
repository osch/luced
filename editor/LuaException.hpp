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

#ifndef LUA_EXCEPTION_HPP
#define LUA_EXCEPTION_HPP

#include "BaseException.hpp"

namespace LucED
{

class LuaException;
class LuaVar;

class LuaException : public BaseException
{
public:
    LuaException(const String& message, int lineNumber = -1, const String& fileName = "");
    
    LuaException(const LuaVar& errorObject);
    
    virtual ~LuaException() throw()
    {}

    virtual const char* what() const throw();
    
    int getLineNumber() const {
        return lineNumber;
    }
    
    String getFileName() const {
        return fileName;
    }
    
    virtual String toString() const;

private:
    int lineNumber;
    String fileName;
};

} // namespace LucED

#endif // LUA_EXCEPTION_HPP
