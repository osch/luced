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

#ifndef HEAPOBJECT_H
#define HEAPOBJECT_H

#include <stdio.h>
#include <stddef.h>

#include "NonCopyable.h"

namespace LucED {

class HeapObject : NonCopyable
{
public:
    
    HeapObject() {
        refCounter = 0;
    }
    
    /**
     * Virtual Destructor.
     */
    virtual ~HeapObject() {}
    
private:

    friend class HeapObjectRefManipulator;

    mutable int refCounter;
    
};

class HeapObjectRefManipulator
{
protected:
    static void incRefCounter(const HeapObject *obj) {
        if (obj != NULL)
            obj->refCounter += 1;
    }
    static void decRefCounter(const HeapObject *obj) {
        if (obj != NULL) {
            obj->refCounter -= 1;
            if (obj->refCounter == 0) {
                delete obj;
            }
        }
    }
};

template<class T> class HeapObjectPtr : private HeapObjectRefManipulator
{
public:
    
    HeapObjectPtr(T *ptr = NULL) {
        this->ptr = ptr;
        incRefCounter(ptr);
    }
    
    ~HeapObjectPtr() {
        decRefCounter(ptr);
    }
    
    HeapObjectPtr(const HeapObjectPtr& src) {
        ptr = src.ptr;
        incRefCounter(ptr);
    }
    
    template<class S> HeapObjectPtr(const HeapObjectPtr<S>& src) {
        ptr = src.getRawPtr();
        incRefCounter(ptr);
    }
    
    HeapObjectPtr& operator=(const HeapObjectPtr& src) {
        if (this != &src) {
            decRefCounter(ptr);
            ptr = src.ptr;
            incRefCounter(ptr);
        }
        return *this;
    }
    
    template<class S> HeapObjectPtr& operator=(const HeapObjectPtr<S>& src) {
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
    
    bool operator==(const HeapObjectPtr& rhs) const {
        return ptr == rhs.ptr;
    }
    
    template<class S> bool operator==(const HeapObjectPtr<S>& rhs) const {
        return ptr == rhs.ptr;
    }
    
private:
    
    T *ptr;
    
};

} // namespace LucED

#endif // HEAPOBJECT_H
