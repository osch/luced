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

namespace LucED
{

class KeyModifier
{
public:

#ifdef X11_GUI
    KeyModifier()
        : keyState(0)
    {}
    
    KeyModifier(const String& asString);
    
    KeyModifier(int keyState)
        : keyState(keyState)
    {}
    
    String toString() const;

    bool operator==(const KeyModifier& rhs) const {
        return keyState == rhs.keyState;
    }
    bool operator<(const KeyModifier& rhs) const {
        return keyState < rhs.keyState;
    }
    
    int toHashValue() const {
        return keyState;
    }
    
    bool containsShift() const {
        return keyState & ShiftMask;
    }
    
    bool containsModifier1() const {
        return keyState & Mod1Mask;
    }
    
#endif
    
private:

#ifdef X11_GUI
    int keyState;
#endif

};      

} // namespace LucED

#endif // KEY_MODIFIER_HPP
