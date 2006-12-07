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

#ifndef CALLBACKCONTAINER_H
#define CALLBACKCONTAINER_H

#include "ObjectArray.h"
#include "Slot.h"

namespace LucED {

class Callback0Container
{
public:
    void registerCallback(const Callback0& callback) {
        listeners.append(callback);
    }
    
    void deregisterAllCallbacksFor(HeapObject* callbackObject) {
        for (int i = 0; i < listeners.getLength();) {
            if (listeners[i].getObjectPtr() == callbackObject) {
                listeners.remove(i);
            } else {
                ++i;
            }
        }
    }

    void invokeAllCallbacks() {
        for (long i = 0; i < listeners.getLength();) {
            if (!listeners[i].isValid()) {
                listeners.remove(i);
            } else {
                listeners[i].call();
                ++i;
            }
        }
    }

private:
    ObjectArray< Callback0 > listeners;
};

template<class T> class Callback1Container
{
public:
    void registerCallback(const Callback1<T>& callback) {
        listeners.append(callback);
    }

    void invokeAllCallbacks(T argument) {
        for (long i = 0; i < listeners.getLength();) {
            if (!listeners[i].isValid()) {
                listeners.remove(i);
            } else {
                listeners[i].call(argument);
                ++i;
            }
        }
    }

private:
    ObjectArray< Callback1<T> > listeners;
};

template<class S, class T> class Callback2Container
{
public:
    void registerCallback(const Callback2<S,T>& callback) {
        listeners.append(callback);
    }

    void invokeAllCallbacks(S argument1, T argument2) {
        for (long i = 0; i < listeners.getLength();) {
            if (!listeners[i].isValid()) {
                listeners.remove(i);
            } else {
                listeners[i].call(argument1, argument2);
                ++i;
            }
        }
    }

private:
    ObjectArray< Callback2<S,T> > listeners;
};

} // namespace LucED

#endif // CALLBACKCONTAINER_H

