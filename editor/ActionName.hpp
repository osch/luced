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

#ifndef ACTION_NAMEP_HPP
#define ACTION_NAMEP_HPP

#include "String.hpp"
#include "ConfigException.hpp"

namespace LucED
{

class ActionName
{
public:
    ActionName()
    {}

    ActionName(const String& className,
               const String& methodName)
        : className(className),
          methodName(methodName)
    {}
    
    explicit ActionName(const String& name) {
        int p = name.findFirstOf('.');
        if (p < 0) {
            throw ConfigException(String() << "Invalid action name: '" << name << "'");
        }
        className  = name.getSubstringBetween(0, p);
        methodName = name.getTail(p + 1);
    }
    
    String getClassName() const {
        return className;
    }
    
    String getMethodName() const {
        return methodName;
    }

private:
    String className;
    String methodName;
};

} // namespace LucED

#endif // ACTION_NAMEP_HPP
