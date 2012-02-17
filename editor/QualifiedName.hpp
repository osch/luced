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

#ifndef QUALIFIED_NAME_HPP
#define QUALIFIED_NAME_HPP

#include "String.hpp"

namespace LucED
{

class QualifiedName
{
public:
    explicit QualifiedName(const String& str);
    
    explicit QualifiedName(const String& qualifier, const String& name)
        : qualifier(qualifier),
          name(name)
    {}
    
    String getQualifier() const {
        return qualifier;
    }

    String getName() const {
        return name;
    }

    String toString() const {
        if (qualifier.getLength() > 0) {
            return String() << qualifier << "." << name;
        } else {
            return String() << qualifier << "." << name;
        }
    }
private:
    String qualifier;
    String name;
};

} // namespace LucED

#endif // QUALIFIED_NAME_HPP
