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

#ifndef KEYPRESSREPEATER_H
#define KEYPRESSREPEATER_H

#include "HeapObject.hpp"
#include "Callback.hpp"
#include "EventDispatcher.hpp"
#include "TimeVal.hpp"
#include "SingletonInstance.hpp"

namespace LucED {

class KeyPressRepeater : public HeapObject
{
public:
    
    static KeyPressRepeater* getInstance();
    static bool isInstanceValid();
    
    void repeatEvent(const XEvent *event);
    void reset();
    
    bool isRepeating() const;
    bool isRepeatingEvent(const XEvent *event) const;
    bool addKeyModifier(const XEvent *event);
    bool removeKeyModifier(const XEvent *event);
    
private:
    friend class SingletonInstance<KeyPressRepeater>;
    static SingletonInstance<KeyPressRepeater> instance;
    
    KeyPressRepeater();
    void processRepeatingEvent();

    Callback0 eventRepeatingCallback;
    XEvent event;
    int repeatCount;
    bool isRepeatingFlag;
    
    TimeVal when;
};

} // namespace LucED

#endif // KEYPRESSREPEATER_H
