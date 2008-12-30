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

#include "headers.hpp"
#include "KeyModifier.hpp"
#include "GuiRoot.hpp"
#include "SingletonInstance.hpp"
#include "HashMap.hpp"

using namespace LucED;

const KeyModifier KeyModifier::NONE;


class KeyModifier::SingletonData : public HeapObject
{
public:
    static SingletonData* getInstance() {
        return instance.getPtr();
    }

    KeyModifier createFromX11KeyState(int x11KeyState)
    {
        Value value;
    
        if ((x11KeyState & shiftKeyStates) != 0) {
            value.set(SHIFT);
        }
        if ((x11KeyState & controlKeyStates) != 0) {
            value.set(CONTROL);
        }
        if ((x11KeyState & altKeyStates) != 0) {
            value.set(ALT);
        }
        if ((x11KeyState & altgrKeyStates) != 0) {
            value.set(ALT_GR);
        }
        return KeyModifier(value);
    }
    
private:
    friend class SingletonInstance<SingletonData>;

    SingletonData()
        : shiftKeyStates(0),
          controlKeyStates(0),
          altKeyStates(0),
          altgrKeyStates(0)
    {
        Display* display = GuiRoot::getInstance()->getDisplay();
        
        XModifierKeymap* map = XGetModifierMapping(display);
    
        const int max_keypermod = map->max_keypermod;
        
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < max_keypermod; ++j)
            {
                const KeyCode keyCode = map->modifiermap[i * max_keypermod + j];
                const KeySym  keySym  = XKeycodeToKeysym(display, keyCode, 0);
                
                switch (keySym)
                {
                    case XK_Shift_L:
                    case XK_Shift_R:
                    {
                        shiftKeyStates |= (1 << i);
                        break;
                    }
                    case XK_Control_L:
                    case XK_Control_R:
                    {
                        controlKeyStates |= (1 << i);
                        break;
                    }
                    case XK_Alt_L:  case XK_Meta_L:
                    case XK_Alt_R:  case XK_Meta_R:
                    {
                        altKeyStates |= (1 << i);
                        break;
                    }
                    case XK_Mode_switch:
                    case XK_ISO_Level3_Shift:
                    {
                        altgrKeyStates |= (1 << i);
                        break;
                    }
                }
            }
        }
        
        XFreeModifiermap(map);
    }
    
    int shiftKeyStates;
    int controlKeyStates;
    int altKeyStates;
    int altgrKeyStates;
    
    static SingletonInstance<SingletonData> instance;
};
SingletonInstance<KeyModifier::SingletonData> KeyModifier::SingletonData::instance;


KeyModifier KeyModifier::createFromX11KeyState(int x11KeyState)
{
    return SingletonData::getInstance()->createFromX11KeyState(x11KeyState);
}

KeyModifier::KeyModifier(const String& asString)
{
    int i = 0;

    while (i < asString.getLength())
    {    
        while (i < asString.getLength() && !isalnum(asString[i])) ++i;
        int j = i;
        while (j < asString.getLength() &&  isalnum(asString[j])) ++j;
        
        String w = asString.getSubstring(i, j - i);
        
        if (w.equalsIgnoreCase("Shift")) {
            value.set(SHIFT);
        } else if (w.equalsIgnoreCase("Ctrl") || w.equalsIgnoreCase("Control")) {
            value.set(CONTROL);
        } else if (w.equalsIgnoreCase("Alt")) {
            value.set(ALT);
        } else if (w.equalsIgnoreCase("AltGr")) {
            value.set(ALT_GR);
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

    if (value.isSet(SHIFT)) {
        rslt.add("Shift");
    }
    if (value.isSet(CONTROL)) {
        rslt.add("Control");
    }
    if (value.isSet(ALT)) {
        rslt.add("Alt");
    }
    if (value.isSet(ALT_GR)) {
        rslt.add("AltGr");
    }
    return rslt.toString();
}


