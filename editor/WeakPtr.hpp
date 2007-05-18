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

#ifndef WEAKPTR_H
#define WEAKPTR_H

#include "HeapObject.hpp"
#include "OwningPtr.hpp"

namespace LucED {


template<class T> class WeakPtr : private HeapObjectRefManipulator
{
public:
    
    WeakPtr() : ptr(NULL), heapObjectCounters(NULL) {
    }
    
    WeakPtr(T* ptr) : ptr(ptr), heapObjectCounters(getHeapObjectCounters(ptr)) {
        incWeakCounter(heapObjectCounters);
    }
    
    WeakPtr(const WeakPtr& src) : ptr(src.getRawPtr()), heapObjectCounters(getHeapObjectCounters(ptr)) {
        incWeakCounter(heapObjectCounters);
    }
    
    template<class S> WeakPtr(const WeakPtr<S>& src) {
        if (src.isValid()) {
            ptr = src.getRawPtr();
            heapObjectCounters = getHeapObjectCounters(ptr);
            incWeakCounter(heapObjectCounters);
        } else {
            ptr = NULL;
            heapObjectCounters = NULL;
        }
    }
    
    WeakPtr(const OwningPtr<T>& src) {
        if (src.isValid()) {
            ptr = src.getRawPtr();
            heapObjectCounters = getHeapObjectCounters(ptr);
            incWeakCounter(heapObjectCounters);
        } else {
            ptr = NULL;
            heapObjectCounters = NULL;
        }
    }

    template<class S> WeakPtr(const OwningPtr<S>& src) {
        if (src.isValid()) {
            ptr = src.getRawPtr();
            heapObjectCounters = getHeapObjectCounters(ptr);
            incWeakCounter(heapObjectCounters);
        } else {
            ptr = NULL;
            heapObjectCounters = NULL;
        }
    }
    
    ~WeakPtr() {
        decWeakCounter(heapObjectCounters);
    }
    
    WeakPtr& operator=(const WeakPtr& src) {
        checkOwningReferences();
        if (src.isValid()) {
            HeapObjectCounters* heapObjectCounters1 = heapObjectCounters;
            ptr = src.getRawPtr();
            heapObjectCounters = getHeapObjectCounters(ptr);
            incWeakCounter(heapObjectCounters);
            decWeakCounter(heapObjectCounters1);
        } else {
            invalidate();
        }
        return *this;
    }
    
    template<class S> WeakPtr& operator=(const WeakPtr<S>& src) {
        checkOwningReferences();
        if (src.isValid()) {
            HeapObject* heapObjectCounters1 = heapObjectCounters;
            ptr = src.getRawPtr();
            heapObjectCounters = getHeapObjectCounters(ptr);
            incWeakCounter(heapObjectCounters);
            decWeakCounter(heapObjectCounters1);
        } else {
            invalidate();
        }
        return *this;
    }
    
    template<class S> WeakPtr& operator=(const OwningPtr<S>& src) {
        checkOwningReferences();
        if (src.isValid()) {
            HeapObjectCounters* heapObjectCounters1 = heapObjectCounters;
            ptr = src.getRawPtr();
            heapObjectCounters = getHeapObjectCounters(ptr);
            incWeakCounter(heapObjectCounters);
            decWeakCounter(heapObjectCounters1);
        } else {
            invalidate();
        }
        return *this;
    }
    
    void invalidate() {
        decWeakCounter(heapObjectCounters);
        ptr = NULL;
        heapObjectCounters = NULL;
    }
    
    bool isValid() const {
        checkOwningReferences();
        return ptr != NULL;
    }

    bool isInvalid() const {
        checkOwningReferences();
        return ptr == NULL;
    }
    
    operator T*() const {
        checkOwningReferences();
        return getRawPtr();
    }
    
    T* operator->() const {
        checkOwningReferences();
        return ptr;
    }
    
    T* getRawPtr() const {
        checkOwningReferences();
        return ptr;
    }

private:

    void checkOwningReferences() const {
        if (ptr != NULL && !heapObjectCounters->hasOwningReferences()) {
            decWeakCounter(heapObjectCounters);
            ptr = NULL;
            heapObjectCounters = NULL;
        }
    }
    
    mutable T *ptr;
    
    // seperate Copy of HeapObjectCounters* is needed for the case, that HeapObject
    // is a virtual base class of T. In this case the upcast from T* to
    // HeapObject* is dynamic and doesn't work, after *ptr was desctructed,
    // i.e. after the destructor ~T() was invoked.
    mutable HeapObjectCounters *heapObjectCounters;
    
};


} // namespace LucED

#endif // WEAKPTR_H
