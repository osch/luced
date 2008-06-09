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

#ifndef OWNING_PTR_HPP
#define OWNING_PTR_HPP

#include "HeapObject.hpp"

namespace LucED
{

template<class T> class OwningPtr : private HeapObjectRefManipulator
{
public:
    explicit OwningPtr(T* ptr = NULL) : ptr(ptr) {
        if (ptr != NULL) {
            obtainInitialOwnership(ptr);
        }
    }
    
    ~OwningPtr() {
        decRefCounter(ptr);
    }
    
    OwningPtr(const OwningPtr& src) {
        ptr = src.ptr;
        incRefCounter(ptr);
    }
    
    template<class S> OwningPtr(const OwningPtr<S>& src) {
        if (src.isValid()) {
            ptr = src.getRawPtr();
            incRefCounter(ptr);
        } else {
            ptr = NULL;
        }
    }
    
    OwningPtr& operator=(const OwningPtr& src) {
        if (src.isValid()) {
            T *ptr1 = ptr;
            ptr = src.getRawPtr();
            incRefCounter(ptr);
            if (ptr1 != NULL) decRefCounter(ptr1);
        } else {
            invalidate();
        }
        return *this;
    }
    
    template<class S> OwningPtr& operator=(const OwningPtr<S>& src) {
        if (src.isValid()) {
            T *ptr1 = ptr;
            ptr = src.getRawPtr();
            incRefCounter(ptr);
            if (ptr1 != NULL) decRefCounter(ptr1);
        } else {
            invalidate();
        }
        return *this;
    }
    
    void invalidate() {
        if (ptr != NULL) {
            decRefCounter(ptr);
            ptr = NULL;
        }
    }
    
    bool isValid() const {
        return ptr != NULL;
    }
    
    bool isInvalid() const {
        return ptr == NULL;
    }
    
    T* operator->() const {
        ASSERT(ptr != NULL);
        return ptr;
    }
    
    T* getRawPtr() const {
        return ptr;
    }
    
    operator T*() const {
        return ptr;
    }
    
    bool operator==(const OwningPtr& rhs) const {
        return ptr == rhs.ptr;
    }
    
    template<class S> bool operator==(const OwningPtr<S>& rhs) const {
        return ptr == rhs.getRawPtr();
    }
    
    template<class S> bool operator==(const S* ptr) const {
        return this->ptr == ptr;
    }
    
    int getRefCounter() const {
        if (isInvalid()) {
            return 0;
        } else {
            return HeapObjectRefManipulator::getHeapObjectCounters(ptr)->getWeakCounter()
                 + HeapObjectRefManipulator::getHeapObjectCounters(ptr)->getOwningCounter();
        }
    }
    
private:
    T* ptr;
    
};

} // namespace LucED

#endif // OWNING_PTR_HPP
