/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
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

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "Null.hpp"

#ifndef WEAK_PTR_HPP
#define WEAK_PTR_HPP

#undef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS

namespace LucED
{

template<class T> class RawPtr;

template
<
    class T
>
class WeakPtr
{
public:
    
    WeakPtr(const NullType& nullValue = Null) 
        : ptr(NULL)
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS    
        , heapObjectCounters(NULL)
#endif
    {}
    
    WeakPtr(T* ptr)
        : ptr(ptr)
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS    
        , heapObjectCounters(HeapObjectRefManipulator::getHeapObjectCounters(ptr))
#endif
    {
        HeapObjectRefManipulator::incWeakCounter(getHeapObjectCounters());
    }
    
    WeakPtr(RawPtr<T> ptr);

    WeakPtr(const WeakPtr& src)
        : ptr(src.getRawPtr())
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS    
        , heapObjectCounters(HeapObjectRefManipulator::getHeapObjectCounters(ptr))
#endif
    {
        HeapObjectRefManipulator::incWeakCounter(getHeapObjectCounters());
    }
    
    template<class S> WeakPtr(const WeakPtr<S>& src) {
        if (src.isValid()) {
            ptr = src.getRawPtr();
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS    
            heapObjectCounters = HeapObjectRefManipulator::getHeapObjectCounters(ptr);
#endif
            HeapObjectRefManipulator::incWeakCounter(getHeapObjectCounters());
        } else {
            ptr = NULL;
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS    
            heapObjectCounters = NULL;
#endif
        }
    }
    
    WeakPtr(const OwningPtr<T>& src) {
        if (src.isValid()) {
            ptr = src.getRawPtr();
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS    
            heapObjectCounters = HeapObjectRefManipulator::getHeapObjectCounters(ptr);
#endif
            HeapObjectRefManipulator::incWeakCounter(getHeapObjectCounters());
        } else {
            ptr = NULL;
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS    
            heapObjectCounters = NULL;
#endif
        }
    }

    template<class S> WeakPtr(const OwningPtr<S>& src) {
        if (src.isValid()) {
            ptr = src.getRawPtr();
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS
            heapObjectCounters = HeapObjectRefManipulator::getHeapObjectCounters(ptr);
#endif
            HeapObjectRefManipulator::incWeakCounter(getHeapObjectCounters());
        } else {
            ptr = NULL;
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS    
            heapObjectCounters = NULL;
#endif
        }
    }
    
    ~WeakPtr() {
        HeapObjectRefManipulator::decWeakCounter(getHeapObjectCounters());
    }
    
    WeakPtr& operator=(const WeakPtr& src) {
        checkOwningReferences();
        if (src.isValid()) {
            HeapObjectCounters* heapObjectCounters1 = getHeapObjectCounters();
            ptr = src.getRawPtr();
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS
            heapObjectCounters = HeapObjectRefManipulator::getHeapObjectCounters(ptr);
#endif
            HeapObjectRefManipulator::incWeakCounter(getHeapObjectCounters());
            HeapObjectRefManipulator::decWeakCounter(heapObjectCounters1);
        } else {
            invalidate();
        }
        return *this;
    }
    
    template<class S> WeakPtr& operator=(const WeakPtr<S>& src) {
        checkOwningReferences();
        if (src.isValid()) {
            HeapObjectCounters* heapObjectCounters1 = getHeapObjectCounters();
            ptr = src.getRawPtr();
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS
            heapObjectCounters = HeapObjectRefManipulator::getHeapObjectCounters(ptr);
#endif
            HeapObjectRefManipulator::incWeakCounter(getHeapObjectCounters());
            HeapObjectRefManipulator::decWeakCounter(heapObjectCounters1);
        } else {
            invalidate();
        }
        return *this;
    }
    
    template<class S> WeakPtr& operator=(const OwningPtr<S>& src) {
        checkOwningReferences();
        if (src.isValid()) {
            HeapObjectCounters* heapObjectCounters1 = getHeapObjectCounters();
            ptr = src.getRawPtr();
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS
            heapObjectCounters = getHeapObjectCounters(ptr);
#endif
            HeapObjectRefManipulator::incWeakCounter(getHeapObjectCounters());
            HeapObjectRefManipulator::decWeakCounter(heapObjectCounters1);
        } else {
            invalidate();
        }
        return *this;
    }
    
    void invalidate() {
        HeapObjectRefManipulator::decWeakCounter(getHeapObjectCounters());
        ptr = NULL;
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS    
        heapObjectCounters = NULL;
#endif
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
        ASSERT(ptr != NULL);
        checkOwningReferences();
        ASSERT(ptr != NULL);
        return ptr;
    }
    
    T* getRawPtr() const {
        checkOwningReferences();
        return ptr;
    }

private:

    void checkOwningReferences() const {
        HeapObjectCounters* counters = getHeapObjectCounters();
        if (ptr != NULL && !counters->hasOwningReferences()) {
            HeapObjectRefManipulator::decWeakCounter(counters);
            ptr = NULL;
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS
            heapObjectCounters = NULL;
#endif
        }
    }
    
    HeapObjectCounters* getHeapObjectCounters() const
    {
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS
        return heapObjectCounters;
#else
        return HeapObjectRefManipulator::getHeapObjectCounters(ptr);
#endif
    }
    
    mutable T *ptr;
    
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS    
    // seperate Copy of HeapObjectCounters* is needed for the case, that HeapObject
    // is a virtual base class of T. In this case the upcast from T* to
    // HeapObject* is dynamic and doesn't work, after *ptr was desctructed,
    // i.e. after the destructor ~T() was invoked.
    mutable HeapObjectCounters* heapObjectCounters;
#endif
    
};


} // namespace LucED

#include "RawPtr.hpp"

namespace LucED
{

template
<
    class T
>
inline WeakPtr<T>::WeakPtr(RawPtr<T> ptr)
    : ptr(ptr)
#ifdef WEAK_PTR_HAS_EXTRA_PTR_TO_COUNTERS    
    , heapObjectCounters(HeapObjectRefManipulator::getHeapObjectCounters(ptr))
#endif
{
    HeapObjectRefManipulator::incWeakCounter(getHeapObjectCounters());
}


} // namespace LucED

#endif // WEAK_PTR_HPP
