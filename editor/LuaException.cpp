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

#include "LuaException.hpp"
#include "StackTrace.hpp"
#include "LuaVar.hpp"
#include "ExceptionLuaInterface.hpp"

using namespace LucED;


LuaExceptionBase::LuaExceptionBase(ExceptionLuaInterface::Ptr luaInterface)
    : luaInterface(luaInterface)
{
    //StackTrace::print();
}


const char* LuaException::what() const throw()
{
    static const char* whatString = "LuaException";
    return whatString;
}

String LuaException::toString() const
{
    LuaStackTrace::Ptr        stackTrace = getLuaStackTrace();
    LuaStackTrace::Entry::Ptr entry      = stackTrace->findFirstFileEntry();

    String rslt;
    if (entry.isValid())
    {
        if (entry->isBuiltinFile()) {
            rslt << "Builtin config file '";
        } else {
            rslt << "File '";
        }
        rslt << entry->getFileName() << "'";
        if (entry->getLineNumber() >= 0) {
            rslt << ", line " << entry->getLineNumber();
        }
        rslt << ": ";
    }
    rslt << BaseException::toString();
    return rslt;
}

