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

#include <ctype.h>

#include "KeyModifier.hpp"

using namespace LucED;

KeyModifier::KeyModifier(const String& asString)
    : keyState(0)
{
    int i = 0;

    while (i < asString.getLength())
    {    
        while (i < asString.getLength() && !isalnum(asString[i])) ++i;
        int j = i;
        while (j < asString.getLength() &&  isalnum(asString[j])) ++j;
        
        String w = asString.getSubstring(i, j - i);
        
        if (w.equalsIgnoreCase("Shift")) {
            keyState |= ShiftMask;
        } else if (w.equalsIgnoreCase("Ctrl") || w.equalsIgnoreCase("Control")) {
            keyState |= ControlMask;
        } else if (w.equalsIgnoreCase("Mod1") || w.equalsIgnoreCase("Modifier1") || w.equalsIgnoreCase("Alt")) {
            keyState |= Mod1Mask;
        } else if (w.equalsIgnoreCase("Mod2") || w.equalsIgnoreCase("Modifier2")) {
            keyState |= Mod2Mask;
        } else if (w.equalsIgnoreCase("Mod3") || w.equalsIgnoreCase("Modifier3")) {
            keyState |= Mod3Mask;
        } else if (w.equalsIgnoreCase("Mod4") || w.equalsIgnoreCase("Modifier4")) {
            keyState |= Mod4Mask;
        } else if (w.equalsIgnoreCase("Mod5") || w.equalsIgnoreCase("Modifier5")) {
            keyState |= Mod5Mask;
        }
        i = j;
    }    
}

namespace // anonymos namespace
{
class StringConcatenator
{
public:
    StringConcatenator(const String& separator)
        : separator(separator)
    {}
    void add(const String& nextValue) {
        if (rslt.getLength() > 0) {
            rslt << separator;
        }
        rslt << nextValue;
    }
    String toString() const {
        return rslt;
    }
private:
    String rslt;
    String separator;
};
} // anonymos namespace

String KeyModifier::toString() const
{
    StringConcatenator rslt("+");

    if (keyState & ShiftMask) {
        rslt.add("Shift");
    }
    if (keyState & ControlMask) {
        rslt.add("Control");
    }
    if (keyState & Mod1Mask) {
        //rslt.add("Modifier1");
        rslt.add("Alt");
    }
    if (keyState & Mod2Mask) {
        rslt.add("Modifier2");
    }
    if (keyState & Mod3Mask) {
        rslt.add("Modifier3");
    }
    if (keyState & Mod4Mask) {
        rslt.add("Modifier4");
    }
    if (keyState & Mod5Mask) {
        rslt.add("Modifier5");
    }
    return rslt.toString();
}


