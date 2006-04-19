/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch@luced.de
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

#ifndef SLOT_H
#define SLOT_H

#include "Callback.h"
#include "debug.h"

namespace LucED {

class Slot0 : public Callback0, private NonCopyable
{
public:

    template<class T> Slot0(T* objectPtr, void (T::*methodPtr)()) 
        : Callback0(objectPtr, methodPtr) {}

    ~Slot0() {
        this->disable();
    }
    
};

template<class A1> class Slot1 : public Callback1<A1>, private NonCopyable
{
public:

    template<class T> Slot1(T* objectPtr, void (T::*methodPtr)(A1)) 
        : Callback1<A1>(objectPtr, methodPtr) {}

    ~Slot1() {
        this->disable();
    }

};

template<class A1> class Slot1Holder : private NonCopyable
{
public:
    template<class T> Callback1<A1> addSlot(T* objectPtr, void (T::*methodPtr)(A1)) {
        slots.appendNew(objectPtr, methodPtr);
        return slots[slots.getLength() - 1];
    }
private:
    ObjectArray< Slot1<A1> > slots;
};

template<class A1, class A2> class Slot2 : public Callback2<A1,A2>, private NonCopyable
{
public:

    template<class T> Slot2(T* objectPtr, void (T::*methodPtr)(A1,A2)) 
        : Callback2<A1,A2>(objectPtr, methodPtr) {}

    ~Slot2() {
        this->disable();
    }

};

template<class A1, class A2, class A3> class Slot3: public Callback3<A1,A2,A2>, private NonCopyable
{
public:

    template<class T> Slot3(T* objectPtr, void (T::*methodPtr)(A1,A2,A3)) 
        : Callback3<A1,A2,A3>(objectPtr, methodPtr) {}

    ~Slot3() {
        this->disable();
    }

};

} // namespace LucED

#endif // SLOT_H
