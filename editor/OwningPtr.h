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

#ifndef OWNINGPTR_H
#define OWNINGPTR_H

#include "HeapObject.h"

namespace LucED {

template<class T> class OwningPtr : private HeapObjectRefManipulator
{
public:
    
    OwningPtr(T *ptr = NULL) {
        this->ptr = ptr;
        incRefCounter(ptr);
    }
    
    ~OwningPtr() {
        decRefCounter(ptr);
    }
    
    OwningPtr(const OwningPtr& src) {
        ptr = src.ptr;
        incRefCounter(ptr);
    }
    
    template<class S> OwningPtr(const OwningPtr<S>& src) {
        ptr = src.getRawPtr();
        incRefCounter(ptr);
    }
    
    OwningPtr& operator=(const OwningPtr& src) {
        if (this != &src) {
            decRefCounter(ptr);
            ptr = src.ptr;
            incRefCounter(ptr);
        }
        return *this;
    }
    
    template<class S> OwningPtr& operator=(const OwningPtr<S>& src) {
        if (this != &src) {
            decRefCounter(ptr);
            ptr = src.ptr;
            incRefCounter(ptr);
        }
    }
    
    void invalidate() {
        decRefCounter(ptr);
        ptr = NULL;
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
    
    bool operator==(const OwningPtr& rhs) const {
        return ptr == rhs.ptr;
    }
    
    template<class S> bool operator==(const OwningPtr<S>& rhs) const {
        return ptr == rhs.ptr;
    }
    
private:
    
    T *ptr;
    
};


} // namespace LucED

#endif // OWNINGPTR_H
