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

#include <X11/keysym.h>

#include "KeyPressRepeater.hpp"
#include "GlobalConfig.hpp"

using namespace LucED;

SingletonInstance<KeyPressRepeater> KeyPressRepeater::instance;



KeyPressRepeater::KeyPressRepeater()
    : eventRepeatingCallback(newCallback(this, &KeyPressRepeater::processRepeatingEvent))
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
        
        TopWin::AccessForKeyPressRepeater::repeat(repeatingTopWin, triggeredKeyCode, &event);
        //EventDispatcher::getInstance()->processEvent(&event);
        
//        TimeVal when;
//        when.setToCurrentTime().addMicroSecs();
//        EventDispatcher::getInstance()->registerTimerCallback();
    }
}

void KeyPressRepeater::triggerNextRepeatEventFor(unsigned int triggeredKeyCode, const XEvent* event, TopWin* topWin)
{
    when.setToCurrentTime();
    if (repeatCount == 0 || !isRepeatingEventForKeyCode(triggeredKeyCode)) {
        when.add(MilliSeconds(GlobalConfig::getConfigData()->getGeneralConfig()->getKeyPressRepeatFirstMilliSecs()));
    } else {
        when.add(MilliSeconds(GlobalConfig::getConfigData()->getGeneralConfig()->getKeyPressRepeatNextMilliSecs()));
    }
    this->event = *event;
    isRepeatingFlag = true;
    EventDispatcher::getInstance()->registerTimerCallback(when, eventRepeatingCallback);
    repeatingTopWin = topWin;
    this->triggeredKeyCode = triggeredKeyCode;
}


void KeyPressRepeater::reset()
{
    isRepeatingFlag = false;
    repeatCount = 0;
    repeatingTopWin.invalidate();
}



bool KeyPressRepeater::isRepeatingEventForKeyCode(unsigned int keycode) const {
    return isRepeatingFlag && this->triggeredKeyCode == keycode;
}

bool KeyPressRepeater::addKeyModifier(const XEvent* event)
{
    if (IsModifierKey(XLookupKeysym((XKeyEvent*)&event->xkey, 0)))
    {
        XModifierKeymap* map = XGetModifierMapping(GuiRoot::getInstance()->getDisplay());
        
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

bool KeyPressRepeater::removeKeyModifier(const XEvent* event)
{
    if (IsModifierKey(XLookupKeysym((XKeyEvent*)&event->xkey, 0)))
    {
        XModifierKeymap* map = XGetModifierMapping(GuiRoot::getInstance()->getDisplay());
    
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

