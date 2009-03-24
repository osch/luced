/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#ifndef INTERFACE_PTR_HPP
#define INTERFACE_PTR_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "WeakPtr.hpp"
#include "RawPtr.hpp"

namespace LucED
{

/**
 * This pointer type can contain a pointer to an object's
 * data member or a pointer to an object's base object,
 * that becomes invalid if the owning object resp. the
 * derived object is destructed.
 *
 * This can be used for interface implementation by
 * derivation from the interface class or be containing 
 * a implementing data member object.
 */
template
<
    class I
>
class InterfacePtr
{
public:
    InterfacePtr()
        : holder(),
          interface(NULL)
    {}
    
    //
    // Constructors for data member objects
    //
    
    template<class T,
             class I2
            >
    InterfacePtr(OwningPtr<T> holder, I2 T::*interface)
        : holder(holder),
          interface(&(holder->*interface))
    {}

    template<class T,
             class I2
            >
    InterfacePtr(WeakPtr<T> holder, I2 T::*interface)
        : holder(holder),
          interface(&(holder->*interface))
    {}

    template<class T,
             class I2
            >
    InterfacePtr(RawPtr<T> holder, I2 T::*interface)
        : holder(holder),
          interface(&(holder->*interface))
    {}

    template<class T,
             class I2
            >
    InterfacePtr(T* holder, I2 T::*interface)
        : holder(holder),
          interface(&(holder->*interface))
    {}


    //
    // Constructors for base class objects
    //

    template<class T
            >
    InterfacePtr(OwningPtr<T> object)
        : holder(object),
          interface(object)
    {}
    
    template<class T
            >
    InterfacePtr(WeakPtr<T> object)
        : holder(object),
          interface(object)
    {}
    template<class T
            >
    InterfacePtr(RawPtr<T> object)
        : holder(object),
          interface(object)
    {}
    template<class T
            >
    InterfacePtr(T* object)
        : holder(object),
          interface(object)
    {}


    //
    // pointer methods
    //

    I* operator->() const {
        ASSERT(holder.isValid());
        return interface;
    }
    
    bool isValid() const {
        return holder.isValid();
    }

private:
    WeakPtr<HeapObject> holder;
    I*                  interface;
};

} // namespace LucED;

#endif // INTERFACE_PTR_HPP
