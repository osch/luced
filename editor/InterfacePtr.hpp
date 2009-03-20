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
 * This class could also have been called "MemberPtr",
 * because this is technically a pointer to an object's
 * data member that becomes invalid if the owning
 * object is destructed.
 *
 * However it is only used as pointer to an interface 
 * implementation, the owning object isn't visible 
 * from outside. Therefore the name "InterfacePtr".
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
