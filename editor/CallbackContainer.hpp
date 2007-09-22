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

#ifndef CALLBACKCONTAINER_H
#define CALLBACKCONTAINER_H

#include "ObjectArray.hpp"
#include "Callback.hpp"

namespace LucED {



template
<
    class A1 = EmptyClass,
    class A2 = EmptyClass,
    class A3 = EmptyClass
>
class CallbackContainer;


/////////////////////////////////////////////////////////////////////////////////////////

template
<
>
class CallbackContainer<EmptyClass,EmptyClass,EmptyClass>
{
public:
    void registerCallback(Callback<>::Ptr callback) {
        listeners.append(callback);
    }

    void deregisterAllCallbacksFor(HeapObject* callbackObject) {
        for (int i = 0; i < listeners.getLength();) {
            if (listeners[i]->getObjectPtr() == callbackObject) {
                listeners.remove(i);
            } else {
                ++i;
            }
        }
    }

    void invokeAllCallbacks() {
        for (long i = 0; i < listeners.getLength();) {
            if (!listeners[i]->isEnabled()) {
                listeners.remove(i);
            } else {
                listeners[i]->call();
                ++i;
            }
        }
    }


private:
    ObjectArray< Callback<>::Ptr > listeners;
};

/////////////////////////////////////////////////////////////////////////////////////////


template
<
    class A1
>
class CallbackContainer<A1,EmptyClass,EmptyClass>
{
public:
    void registerCallback(typename Callback<A1>::Ptr callback) {
        listeners.append(callback);
    }

    void deregisterAllCallbacksFor(HeapObject* callbackObject) {
        for (int i = 0; i < listeners.getLength();) {
            if (listeners[i]->getObjectPtr() == callbackObject) {
                listeners.remove(i);
            } else {
                ++i;
            }
        }
    }

    void invokeAllCallbacks(A1 argument1) {
        for (long i = 0; i < listeners.getLength();) {
            if (!listeners[i]->isEnabled()) {
                listeners.remove(i);
            } else {
                listeners[i]->call(argument1);
                ++i;
            }
        }
    }

private:
    ObjectArray< typename Callback<A1>::Ptr > listeners;
};

/////////////////////////////////////////////////////////////////////////////////////////


template
<
    class A1,
    class A2
>
class CallbackContainer<A1,A2,EmptyClass>
{
public:
    void registerCallback(typename Callback<A1,A2>::Ptr callback) {
        listeners.append(callback);
    }

    void deregisterAllCallbacksFor(HeapObject* callbackObject) {
        for (int i = 0; i < listeners.getLength();) {
            if (listeners[i]->getObjectPtr() == callbackObject) {
                listeners.remove(i);
            } else {
                ++i;
            }
        }
    }

    void invokeAllCallbacks(A1 a1, A2 a2) {
        for (long i = 0; i < listeners.getLength();) {
            if (!listeners[i]->isEnabled()) {
                listeners.remove(i);
            } else {
                listeners[i]->call(a1, a2);
                ++i;
            }
        }
    }

private:
    ObjectArray< typename Callback<A1,A2>::Ptr > listeners;
};

/////////////////////////////////////////////////////////////////////////////////////////


template
<
    class A1,
    class A2,
    class A3
>
class CallbackContainer
{
public:
    void registerCallback(typename Callback<A1,A2,A3>::Ptr callback) {
        listeners.append(callback);
    }

    void deregisterAllCallbacksFor(HeapObject* callbackObject) {
        for (int i = 0; i < listeners.getLength();) {
            if (listeners[i]->getObjectPtr() == callbackObject) {
                listeners.remove(i);
            } else {
                ++i;
            }
        }
    }

    void invokeAllCallbacks(A1 a1, A2 a2, A3 a3) {
        for (long i = 0; i < listeners.getLength();) {
            if (!listeners[i]->isEnabled()) {
                listeners.remove(i);
            } else {
                listeners[i]->call(a1, a2, a3);
                ++i;
            }
        }
    }

private:
    ObjectArray< typename Callback<A1,A2,A3>::Ptr > listeners;
};

/////////////////////////////////////////////////////////////////////////////////////////


} // namespace LucED

#endif // CALLBACKCONTAINER_H

