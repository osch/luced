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

#ifndef KEY_ID_HPP
#define KEY_ID_HPP

#include "headers.hpp"
#include "String.hpp"

namespace LucED
{

class KeyId
{
public:
    
#ifdef X11_GUI
    KeyId()
        : keySym(NoSymbol)
    {}
    explicit KeyId(const char* name)
        : keySym(XStringToKeysym(name))
    {}

    explicit KeyId(const String& name)
        : keySym(XStringToKeysym(name.toCString()))
    {}

    explicit KeyId(KeySym keySym)
        : keySym(keySym)
    {}
    
    operator KeySym() const {
        return keySym;
    }
    
    bool isValid() const {
        return keySym != NoSymbol;
    }
    String toString() const {
        const char* name = XKeysymToString(keySym);
        if (name != NULL) {
            return String(name);
        } else {
            return String();
        }
    }
    bool operator==(const KeyId& rhs) const {
        return keySym == rhs.keySym;
    }
    bool operator<(const KeyId& rhs) const {
        return keySym < rhs.keySym;
    }
    bool isModifierKey() const {
        if (isValid()) {
            return IsModifierKey(keySym);
        } else {
            return false;
        }
    }
#endif

private:

#ifdef X11_GUI
    KeySym keySym;
#endif
};

} // namespace LucED

#endif // KEY_ID_HPP
