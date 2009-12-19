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

#ifndef KEY_MODIFIER_HPP
#define KEY_MODIFIER_HPP

#include "headers.hpp"
#include "String.hpp"
#include "Flags.hpp"
#include "HashMap.hpp"

namespace LucED
{

class KeyModifier
{
public:
    enum Flag { SHIFT,
                CONTROL,
                ALT,
                ALT_GR };
                
    typedef Flags<Flag> Value;

    static const KeyModifier NONE;

    KeyModifier()
    {}
    
    KeyModifier(const String& asString);
    
    KeyModifier(Value value)
        : value(value)
    {}

    KeyModifier(Flag flag)
        : value(flag)
    {}
    
    static KeyModifier createFromX11KeyState(int x11KeyState);
    
    String toString() const;

    bool operator==(const KeyModifier& rhs) const {
        return value == rhs.value;
    }
    bool operator<(const KeyModifier& rhs) const {
        return value < rhs.value;
    }
    
    size_t toHashValue() const {
        return HashFunction<Value::ImplType>()(value.toImplType());
    }
    
    bool containsShiftKey() const {
        return value.isSet(SHIFT);
    }
    
    bool containsAltKey() const {
        return value.isSet(ALT);
    }
    
private:
    class SingletonData;

    Value value;
};      

} // namespace LucED

#endif // KEY_MODIFIER_HPP
