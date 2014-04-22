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

#ifndef HEAP_OBJECT_ARRAY_HPP
#define HEAP_OBJECT_ARRAY_HPP

#include <new>
#include <algorithm>

#include "HeapMem.hpp"
#include "MemArray.hpp"
#include "HeapObject.hpp"
#include "ObjectArray.hpp"
#include "OwningPtr.hpp"

namespace LucED
{

/**
 * Array on the heap for objects with default-constructors, copy-constructors and 
 * destructors that can be moved by memmove
 */
template<class T> class HeapObjectArray : public HeapObject, public ObjectArray<T>
{
public:
    typedef OwningPtr< HeapObjectArray<T> > Ptr;
    
    static Ptr create() {
        return Ptr(new HeapObjectArray());
    }
    
    static Ptr create(long size) {
        return Ptr(new HeapObjectArray(size));
    }
    
    virtual ~HeapObjectArray() {
    }

    T& at(long i) {
        return *ObjectArray<T>::getPtr(i);
    }
    const T& at(long i) const {
        return *ObjectArray<T>::getPtr(i);
    }
    const T& get(long i) const {
        ASSERT(0 <= i && i < ObjectArray<T>::getLength());
        return *ObjectArray<T>::getPtr(i);
    }
    const T& getLast() const {
        ASSERT(ObjectArray<T>::getLength() > 0);
        return get(ObjectArray<T>::getLength() - 1);
    }
    
    void set(long i, const T& value) {
        at(i) = value;
    }

protected:

    HeapObjectArray() {}

    HeapObjectArray(long size) : ObjectArray<T>(size)
    {}
};

} // namespace LucED

#endif // HEAP_OBJECT_ARRAY_HPP
