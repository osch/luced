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

#include "LuaVar.hpp"

#ifndef LUA_EXCEPTION_HPP
#define LUA_EXCEPTION_HPP

#include "BaseException.hpp"
#include "ExceptionLuaInterface.hpp"

namespace LucED
{


class LuaExceptionBase
{
public:
    ExceptionLuaInterface::Ptr getExceptionLuaInterface() const {
        return luaInterface;
    }
    ~LuaExceptionBase() throw()
    {}
protected:
    LuaExceptionBase(ExceptionLuaInterface::Ptr luaInterface);

    ExceptionLuaInterface::Ptr luaInterface;
};



class LuaException : public LuaExceptionBase,
                     public BaseException
{
public:
    LuaException(const LuaAccess& luaAccess,
                 const String& message)
        : LuaExceptionBase(ExceptionLuaInterface::create(luaAccess,
                                                         message)),
          BaseException(luaInterface->getMessage())
    {}
    
    LuaException(ExceptionLuaInterface::Ptr luaInterface)
        : LuaExceptionBase(luaInterface),
          BaseException(luaInterface->getMessage())
    {}
    

    LuaException(const LuaVar& errorObject)
        : LuaExceptionBase(ExceptionLuaInterface::create(errorObject)),
          BaseException(luaInterface->getMessage())
    {}
    
    virtual ~LuaException() throw()
    {}

    virtual const char* what() const throw();
    
    bool hasFileSource() const {
        return luaInterface->hasFileSource();
    }
    bool isBuiltinFile() const {
        return luaInterface->isBuiltinFile();
    }
    String getFileName() const {
        return luaInterface->getFileName();
    }
    int getFileLineNumber() const {
        return luaInterface->getFileLineNumber();
    }

    bool hasScriptBytes() const {
        return luaInterface->hasScriptBytes();
    }
    RawPtr<ByteBuffer> getPtrToScriptBytes() {
        return luaInterface->getPtrToScriptBytes();
    }
    int getScriptLineNumber() const {
        return luaInterface->getScriptLineNumber();
    }
    
    virtual String toString() const;
};

} // namespace LucED

#endif // LUA_EXCEPTION_HPP
