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

#ifndef FLAGS_HPP
#define FLAGS_HPP

#include "debug.hpp"

namespace LucED
{

template
<
    class EnumType,
    class ValueType = unsigned long
>
class Flags
{
public:
    typedef ValueType ImplType;
    
    Flags()
        : value()
    {}
    
    Flags(EnumType initialFlagNumber)
        : value(1 << initialFlagNumber)
    {
        ASSERT(initialFlagNumber < sizeof(ImplType) * 8);
    }
    
    void clear() {
        value = ImplType();
    }
    
    void set(EnumType flagNumber)
    {
        ASSERT(flagNumber < sizeof(ImplType) * 8);
        value |= (1 << flagNumber);
    }
    
    void clear(EnumType flagNumber)
    {
        ASSERT(flagNumber < sizeof(ImplType) * 8);
        value &= ~(1 << flagNumber);
    }
    
    bool isSet(EnumType flagNumber) const
    {
        ASSERT(flagNumber < sizeof(ImplType) * 8);
        return value & (1 << flagNumber);
    }
    
    bool contains(EnumType flagNumber) const {
        return isSet(flagNumber);
    }
    
    Flags operator|(EnumType flagNumber) const
    {
        Flags rslt = *this;
        rslt.set(flagNumber);
        return rslt;
    }
    
    Flags& operator|=(EnumType flagNumber)
    {
        this->set(flagNumber);
        return *this;
    }
    
    ImplType toImplType() const {
        return value;
    }
    
    bool operator==(const Flags& rhs) const {
        return value == rhs.value;
    }
    bool operator!=(const Flags& rhs) const {
        return value != rhs.value;
    }
    bool operator<(const Flags& rhs) const {
        return value < rhs.value;
    }

private:
    ImplType value;
};

} // namespace LucED

#endif // FLAGS_HPP
