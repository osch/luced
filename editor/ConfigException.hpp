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

#ifndef CONFIG_EXCEPTION_HPP
#define CONFIG_EXCEPTION_HPP

#include "BaseException.hpp"
#include "HeapObjectArray.hpp"

namespace LucED {

class ConfigException : public BaseException
{
public:
    class Error
    {
    public:
        Error(const String& configFileName, const String& message)
            : configFileName(configFileName),
              message(message)
        {}
        String getConfigFileName() const {
            return configFileName;
        }
        String getMessage() const {
            return message;
        }
    private:
        String configFileName;
        String message;
    };
    typedef HeapObjectArray<Error> ErrorList;
    
    ConfigException(const String& message)
        : BaseException(message)
    {}

    ConfigException(ErrorList::Ptr errorList)
        : BaseException("Error within config files."),
          errorList(errorList)
    {}

    virtual ~ConfigException() throw()
    {}
    
    virtual const char *what();

    ErrorList::Ptr getErrorList() {
        return errorList;
    }
    
private:
    ErrorList::Ptr errorList;
};

} // namespace LucED

#endif // CONFIG_EXCEPTION_HPP
