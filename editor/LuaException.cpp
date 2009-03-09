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

using namespace LucED;

static inline String getErrorMessageFromErrorObject(const LuaVar& errorObject)
{
    if (errorObject.isString()) {
        return errorObject.toString();
    }
    else if (errorObject.isTable() && errorObject["errorMessage"].isString()) {
        return errorObject["errorMessage"].toString();
    }
    else {
        ASSERT(false);
        return "unknown error";
    }
}



LuaException::LuaException(const String& message,
                           int           lineNumber,
                           const String& fileName)
    : BaseException(message),
      lineNumber(lineNumber),
      fileName(fileName)
{
    //StackTrace::print();
}
LuaException::LuaException(const LuaVar& errorObject)
    : BaseException(getErrorMessageFromErrorObject(errorObject)),
      lineNumber(-1)
{
    if (errorObject.isTable()) {
        if (errorObject["lineNumber"].isNumber()) {
            lineNumber = errorObject["lineNumber"].toInt() - 1;
        }
        if (errorObject["fileName"].isString()) {
            fileName = errorObject["fileName"].toString();
        }
    }
}


const char* LuaException::what() const throw()
{
    static const char* whatString = "LuaException";
    return whatString;
}

String LuaException::toString() const
{
    String rslt;
    if (fileName.getLength() > 0) {
        rslt << fileName << ": ";
    }
    if (lineNumber >= 0) {
        rslt << "line " << lineNumber << ": ";
    }
    rslt << BaseException::toString();
    return rslt;
}

