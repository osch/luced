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

#ifndef WEAKPTRQUEUE_H
#define WEAKPTRQUEUE_H

#include "ObjectArray.h"
#include "OwningPtr.h"
#include "WeakPtr.h"

namespace LucED {


template<class T> class WeakPtrQueue : public HeapObject
{
public:
    typedef OwningPtr<WeakPtrQueue> Ptr;

    static Ptr create() {
        return Ptr(new WeakPtrQueue());
    }
    
    void append(T* x) {
        ptrArray.append(x);
    }
    
    WeakPtr<T> getLast() const {
        WeakPtr<T> rslt;
        while (ptrArray.getLength() > 0 && rslt.isInvalid()) {
            rslt = ptrArray.getLast();
            if (rslt.isInvalid()) {
                ptrArray.removeLast();
            }
        }
        return rslt;
    }
    
    void removeLast() {
        T* rslt = NULL;
        while (ptrArray.getLength() > 0 && rslt == NULL) {
            rslt = ptrArray.getLast();
            if (rslt == NULL) {
                ptrArray.removeLast();
            }
        }
        if (rslt != NULL) {
            ptrArray.removeLast();
        }
    }
    int getLength() { return ptrArray.getLength(); }
    
private:
    WeakPtrQueue() {}
    
    mutable ObjectArray< WeakPtr<T> > ptrArray;
};

} // namespace LucED

#endif // WEAKPTRQUEUE_H
