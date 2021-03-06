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

#ifndef WEAKPTRQUEUE_H
#define WEAKPTRQUEUE_H

#include "ObjectArray.hpp"
#include "OwningPtr.hpp"
#include "WeakPtr.hpp"

namespace LucED
{


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

    void removeAll(T* x) {
        for (int i = 0; i < ptrArray.getLength();) {
            if (ptrArray[i] == x) {
                ptrArray.remove(i);
            } else {
                ++i;
            }
        }
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
    
protected:
    WeakPtrQueue() {}
    
private:
    mutable ObjectArray< WeakPtr<T> > ptrArray;
};

} // namespace LucED

#endif // WEAKPTRQUEUE_H
