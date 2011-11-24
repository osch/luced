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
#include "Nullable.hpp"

namespace LucED {

class ConfigException : public BaseException
{
public:
    class Error
    {
    public:
        Error(const String& configFileName, int lineNumber, const String& message)
            : configFileName(configFileName),
              lineNumber(lineNumber),
              message(message)
        {}
        Error(const String& message)
            : configFileName(Null),
              lineNumber(-1),
              message(message)
        {}
        bool hasConfigFileName() const {
            return configFileName.isValid();
        }
        String getConfigFileName() const {
            return configFileName.get();
        }
        int getLineNumber() const {
            return lineNumber;
        }
        String getMessage() const {
            return message;
        }
    private:
        Nullable<String> configFileName;
        int              lineNumber;
        String           message;
    };
    
    class ErrorList : public HeapObjectArray<Error>
    {
    public:
        typedef OwningPtr<ErrorList> Ptr;
        
        static Ptr create() {
            return Ptr(new ErrorList());
        }
        
        void appendCatchedException();
        void appendErrorMessage(const String& message) {
            this->append(Error(message));
        }

    private:
        ErrorList()
        {}
    };
    
    ConfigException(const String& message);

    ConfigException(ErrorList::Ptr errorList);

    virtual ~ConfigException() throw()
    {}
    
    virtual const char* what() const throw();

    ErrorList::Ptr getErrorList() {
        return errorList;
    }
    
private:
    ErrorList::Ptr errorList;
};

} // namespace LucED

#endif // CONFIG_EXCEPTION_HPP
