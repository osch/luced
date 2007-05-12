/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#include <X11/keysym.h>

#include "KeyPressRepeater.h"
#include "GlobalConfig.h"

using namespace LucED;

SingletonInstance<KeyPressRepeater> KeyPressRepeater::instance;


KeyPressRepeater* KeyPressRepeater::getInstance()
{
    return instance.getPtr();
}

bool KeyPressRepeater::isInstanceValid()
{
    return instance.isValid();
}

KeyPressRepeater::KeyPressRepeater()
    : eventRepeatingCallback(this, &KeyPressRepeater::processRepeatingEvent)
{
    isRepeatingFlag = false;
}

void KeyPressRepeater::processRepeatingEvent()
{
    TimeVal now;
    now.setToCurrentTime();
    
    if (isRepeatingFlag && now.isLaterThan(when))
    {
        ++repeatCount;
        EventDispatcher::getInstance()->processEvent(&event);
        
//        TimeVal when;
//        when.setToCurrentTime().addMicroSecs();
//        EventDispatcher::getInstance()->registerTimerCallback();
    }
}

void KeyPressRepeater::repeatEvent(const XEvent *event)
{
    when.setToCurrentTime();
    if (repeatCount == 0 || !isRepeatingEvent(event)) {
        when.addMicroSecs(GlobalConfig::getInstance()->getKeyPressRepeatFirstMicroSecs());
    } else {
        when.addMicroSecs(GlobalConfig::getInstance()->getKeyPressRepeatNextMicroSecs());
    }
    this->event = *event;
    isRepeatingFlag = true;
    EventDispatcher::getInstance()->registerTimerCallback(when, eventRepeatingCallback);
}


void KeyPressRepeater::reset()
{
    isRepeatingFlag = false;
    repeatCount = 0;
}


bool KeyPressRepeater::isRepeating() const {
    return isRepeatingFlag;
}

bool KeyPressRepeater::isRepeatingEvent(const XEvent *event) const {
    return isRepeatingFlag && this->event.xkey.keycode == event->xkey.keycode;
}

bool KeyPressRepeater::addKeyModifier(const XEvent *event)
{
    if (IsModifierKey(XLookupKeysym((XKeyEvent*)&event->xkey, 0)))
    {
        XModifierKeymap *map = XGetModifierMapping(GuiRoot::getInstance()->getDisplay());
        
        bool found = false;
        unsigned int keyCode = event->xkey.keycode;
        const int max_keypermod = map->max_keypermod;
        
        for (int i = 0; !found && i < 8; ++i) {
            for (int j = 0; !found && j < max_keypermod; ++j) {
                if (map->modifiermap[i * max_keypermod + j] == keyCode) {
                    this->event.xkey.state |= ShiftMask << i;
                    found = true;
                }
            }
        }
        
        XFreeModifiermap(map);
        return true;
    }
    else {
        return false;
    }
}

bool KeyPressRepeater::removeKeyModifier(const XEvent *event)
{
    if (IsModifierKey(XLookupKeysym((XKeyEvent*)&event->xkey, 0)))
    {
        XModifierKeymap *map = XGetModifierMapping(GuiRoot::getInstance()->getDisplay());
    
        bool found = false;
        unsigned int keyCode = event->xkey.keycode;
        const int max_keypermod = map->max_keypermod;
        
        for (int i = 0; !found && i < 8; ++i) {
            for (int j = 0; !found && j < max_keypermod; ++j) {
                if (map->modifiermap[i * max_keypermod + j] == keyCode) {
                    this->event.xkey.state &= ~(ShiftMask << i);
                    found = true;
                }
            }
        }
        
        XFreeModifiermap(map);
        return true;
    }
    else {
        return false;
    }
}

