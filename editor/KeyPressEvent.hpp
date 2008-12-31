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

#ifndef KEY_PRESS_EVENT_HPP
#define KEY_PRESS_EVENT_HPP

#include "headers.hpp"
#include "KeyId.hpp"
#include "KeyModifier.hpp"
#include "String.hpp"

namespace LucED
{

class KeyPressEvent
{
public:
    KeyPressEvent(KeyModifier keyModifier, KeyId keyId)
        : keyModifier(keyModifier),
          keyId(keyId)
    {}

    KeyPressEvent(const XEvent* event) 
    {
        char buffer[1000];
        KeySym keySym;
        
        int len = XLookupString(&((XEvent*)event)->xkey, buffer, sizeof(buffer), &keySym, NULL);

    #ifdef XK_ISO_Left_Tab
        // Shift + Tab becomes XK_ISO_Left_Tab through XLookupString,
        // however we need XLookupString for otherwise correct
        // interpretation of num_lock
        if (keySym == XK_ISO_Left_Tab) {
            keySym = XK_Tab;
        }
    #endif
    #ifdef XK_KP_Tab
        if (keySym == XK_KP_Tab) {
            keySym = XK_Tab;
        }
    #endif
        keyId       = KeyId(keySym);
        keyModifier = KeyModifier::createFromX11KeyState(event->xkey.state);
        
        if (len > 0) {
            input = String(buffer, len);
        }
    }
    
    KeyId getKeyId() const {
        return keyId;
    }
    
    KeyModifier getKeyModifier() const {
        return keyModifier;
    }
    
    String getInputString() const {
        return input;
    }
    
    bool hasInputString() const {
        return input.getLength() > 0;
    }
    
private:
    KeyId       keyId;
    KeyModifier keyModifier;
    String      input;
};

} // namespace LucED

#endif // KEY_PRESS_EVENT_HPP
