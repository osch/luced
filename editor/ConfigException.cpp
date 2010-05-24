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

#include "ConfigException.hpp"
#include "LuaException.hpp"

using namespace LucED;

ConfigException::ConfigException(const String& message)
    : BaseException(message)
{}

ConfigException::ConfigException(ErrorList::Ptr errorList)
    : BaseException("Error within config files."),
      errorList(errorList)
{}


const char* ConfigException::what() const throw()
{
    static const char* whatString = "ConfigException";
    return whatString;
}

void ConfigException::ErrorList::appendCatchedException()
{
    try
    {
        throw;
    }
    catch (LuaException& ex)
    {
        this->append(Error(ex.getFileName(), ex.getLineNumber(), ex.getMessage()));    
    }
    catch (ConfigException& ex)
    {
        ErrorList::Ptr list = ex.getErrorList();
        if (list.isValid() && list->getLength() > 0) {
            for (int i = 0, n = list->getLength(); i < n; ++i) {
                this->append(list->get(i));
            }
        } else {
            this->append(Error(fallbackFileName, -1, ex.getMessage()));    
        }
    }
    catch (BaseException& ex)
    {
        this->append(Error(fallbackFileName, -1, ex.getMessage()));    
    }
}
