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

#ifndef OWNINGPTR_H
#define OWNINGPTR_H

#include "HeapObject.h"

namespace LucED {

template<class T> class OwningPtr : private HeapObjectRefManipulator
{
public:
    
    OwningPtr(T *ptr = NULL) : ptr(ptr) {
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
        return ptr;
    }
    
    T* getRawPtr() const {
        return ptr;
    }
    
    operator T*() {
        return ptr;
    }
    
    bool operator==(const OwningPtr& rhs) const {
        return ptr == rhs.ptr;
    }
    
    bool operator==(const T* ptr) const {
        return this->ptr == ptr;
    }
    
    template<class S> bool operator==(const OwningPtr<S>& rhs) const {
        return ptr == rhs.getRawPtr();
    }
    
    template<class S> bool operator==(const S* ptr) const {
        return this->ptr == ptr;
    }
    
private:
    
    T *ptr;
    
};


} // namespace LucED

#endif // OWNINGPTR_H
