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

#ifndef WEAKPTR_H
#define WEAKPTR_H

#include "HeapObject.h"
#include "OwningPtr.h"

namespace LucED {


template<class T> class WeakPtr : private HeapObjectRefManipulator
{
public:
    
    WeakPtr() : ptr(NULL) {
    }
    
    WeakPtr(T* ptr) : ptr(ptr) {
        incWeakCounter(ptr);
    }
    
    ~WeakPtr() {
        decWeakCounter(ptr);
    }
    
    WeakPtr(const WeakPtr& src) {
        ptr = src.ptr;
        incWeakCounter(ptr);
    }
    
    operator T*() {
        return getRawPtr();
    }
    
    template<class S> WeakPtr(const WeakPtr<S>& src) {
        ptr = src.getRawPtr();
        incWeakCounter(ptr);
    }
    
    template<class S> WeakPtr(const OwningPtr<S>& src) {
        ptr = src.getRawPtr();
        incWeakCounter(ptr);
    }
    
    WeakPtr& operator=(const WeakPtr& src) {
        if (this != &src) {
            decWeakCounter(ptr);
            ptr = src.ptr;
            incWeakCounter(ptr);
        }
        return *this;
    }
    
    template<class S> WeakPtr& operator=(const WeakPtr<S>& src) {
        if (this != &src) {
            decWeakCounter(ptr);
            ptr = src.ptr;
            incWeakCounter(ptr);
        }
        return *this;
    }
    
    template<class S> WeakPtr& operator=(const OwningPtr<S>& src) {
        decWeakCounter(ptr);
        ptr = src.getRawPtr();
        incWeakCounter(ptr);
        return *this;
    }
    
    void invalidate() {
        decWeakCounter(ptr);
        ptr = NULL;
    }
    
    bool isValid() {
        checkOwningReferences();
        return ptr != NULL;
    }
    
    bool isInvalid() {
        checkOwningReferences();
        return ptr == NULL;
    }
    
    T* operator->() {
        checkOwningReferences();
        return ptr;
    }
    
    T* getRawPtr() {
        checkOwningReferences();
        return ptr;
    }

    T* getRawPtr() const {
        return ptr;
    }
    
    bool operator==(const WeakPtr& rhs) const {
        return ptr == rhs.ptr;
    }
    
    template<class S> bool operator==(const WeakPtr<S>& rhs) const {
        return ptr == rhs.ptr;
    }
    
private:

    void checkOwningReferences() {
        if (ptr != NULL && !hasOwningReferences(ptr)) {
            invalidate();
        }
    }
    
    T *ptr;
    
};


} // namespace LucED

#endif // WEAKPTR_H
