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

#ifndef TEXT_STYLE_DEFINITIONS_HPP
#define TEXT_STYLE_DEFINITIONS_HPP

#include "ObjectArray.hpp"
#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "TextStyleDefinition.hpp"
#include "Nullable.hpp"
          
namespace LucED
{

class TextStyleDefinitions : public HeapObject
{
public:
    typedef OwningPtr<      TextStyleDefinitions> Ptr;
    typedef OwningPtr<const TextStyleDefinitions> ConstPtr;
    
    static Ptr create() {
        return Ptr(new TextStyleDefinitions());
    }
    
    void append(const TextStyleDefinition& newDefinition) {
        definitions.append(newDefinition);
    }
    
    bool isEmpty() const {
        return definitions.getLength() == 0;
    }
    int getLength() const {
        return definitions.getLength();
    }
    const TextStyleDefinition& get(int i) const {
        return definitions[i];
    }
    Nullable<TextStyleDefinition> getFirstWithName(const String& name) const {
        for (int i = 0, n = getLength(); i < n; ++i) {
            if (definitions[i].getName() == name) {
                return definitions[i];
            }
        }
        return Null;
    }
     
    
private:
    ObjectArray<TextStyleDefinition> definitions;
};

} // namespace LucED
                    
#endif // TEXT_STYLE_DEFINITIONS_HPP
