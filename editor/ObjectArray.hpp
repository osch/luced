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

#ifndef OBJECTARRAY_H
#define OBJECTARRAY_H

#include <new>
#include <algorithm>

#include "HeapMem.hpp"
#include "MemArray.hpp"

namespace LucED {

using std::min;
using std::max;

/**
 * Array for objects with default-constructors, copy-constructors and 
 * destructors that can be moved by memmove
 */
template<class T> class ObjectArray : MemArray<T>
{
public:
    ObjectArray() : MemArray<T>(0) {}
    
    ObjectArray(long size) : MemArray<T>(size) {
        for (long i = 0; i < size; ++i) {
            new(this->getPtr(i)) T();
        }
    }
    /**
     * Desctruct elements in reverse order
     */
    ~ObjectArray() {
        for (long i = getLength() - 1; i >= 0; --i) {
            this->getPtr(i)->~T();
        }
    }
    ObjectArray(const ObjectArray<T>& src) : MemArray<T>(0) {
        for (long i = 0; i < src.getLength(); ++i) {
            append(src[i]);
        }
    }
    ObjectArray<T>& operator=(const ObjectArray<T>& src) {
        clear();
        for (long i = 0; i < src.getLength(); ++i) {
            append(src[i]);
        }
    }
    T& operator[](long i) {
        ASSERT(i < getLength());
        return *this->getPtr(i);
    }
    T* getPtr(long i) {
        return MemArray<T>::getPtr(i);
    }
    T get(long i) const {
        ASSERT(i < getLength());
        return *this->getPtr(i);
    }
    T& get(long i) {
        ASSERT(i < getLength());
        return *this->getPtr(i);
    }
    const T& getLast() const {
        ASSERT(getLength() > 0);
        return *this->getPtr(getLength() - 1);
    }
    T& getLast() {
        ASSERT(getLength() > 0);
        return *this->getPtr(getLength() - 1);
    }
    const T& operator[](long i) const {
        ASSERT(i < getLength());
        return *this->getPtr(i);
    }
    const T* getPtr(long i) const {
        return MemArray<T>::getPtr(i);
    }
    long getLength() const {
        return MemArray<T>::getLength();
    }
    ObjectArray<T>& clear() {
        for (long i = 0; i < getLength(); ++i) {
            this->getPtr(i)->~T();
        }
        MemArray<T>::clear();
        return *this;
    }
    ObjectArray<T>& removeAmount(long pos, long amount) {
        ASSERT(0 <= pos && pos + amount <= getLength());
        for (long p = pos; p < pos + amount; ++p) {
            this->getPtr(p)->~T();
        }
        MemArray<T>::removeAmount(pos, amount);
        return *this;
    }
    ObjectArray<T>& remove(long pos) {
        removeAmount(pos, 1);
    }
    ObjectArray<T>& removeLast() {
        ASSERT(getLength() > 0);
        removeAmount(getLength() - 1, 1);
    }
    ObjectArray<T>& removeBetween(long pos1, long pos2) {
        removeAmount(pos1, pos2 - pos1);
        return *this;
    }
    ObjectArray<T>& insert(long pos, const T* source, long sourceLength) {
        MemArray<T>::insertAmount(pos, sourceLength);
        for (long i = 0; i < sourceLength; ++i) {
            new (this->getPtr(pos + i)) T(source[i]);
        }
        return *this;
    }
    ObjectArray<T>& insert(long pos, const T& source) {
        insert(pos, &source, 1);
        return *this;
    }
    ObjectArray<T>& insert(long pos, const ObjectArray<T>& src, long srcPos, long srcLength) {
        ASSERT(0 <= srcLength && srcPos + srcLength <= src.getLength());
        insert(pos, src.getPtr(srcPos), srcLength);
        return *this;
    }
    ObjectArray<T>& insert(long pos, const ObjectArray<T>& src) {
        insert(pos, src, 0, src.getLength());
        return *this;
    }
    ObjectArray<T>& append(const T* source, long sourceLength) {
        insert(getLength(), source, sourceLength);
        return *this;
    }
    ObjectArray<T>& append(const ObjectArray<T> &src, long srcPos, long srcLength) {
        insert(getLength(), src, srcPos, srcLength);
        return *this;
    }
    ObjectArray<T>& append(const T& src) {
        append(&src, 1);
        return *this;
    }
    ObjectArray<T>& appendAmount(long amount) {
        long oldSize = getLength();
        MemArray<T>::appendAmount(amount);
        for (long i = oldSize; i < getLength(); ++i) {
            new(this->getPtr(i)) T();
        }
        return *this;
    }
    
    ObjectArray<T>& appendNew() {
        return appendAmount(1);
    }
    template<class T1> 
    ObjectArray<T>& appendNew(const T1& t1) {
        long pos = getLength();
        MemArray<T>::insertAmount(pos, 1);
        new(this->getPtr(pos)) T(t1);
        return *this;
    }
    template<class T1, class T2> 
    ObjectArray<T>& appendNew(const T1& t1, const T2& t2) {
        long pos = getLength();
        MemArray<T>::insertAmount(pos, 1);
        new(this->getPtr(pos)) T(t1, t2);
        return *this;
    }
    template<class T1, class T2, class T3> 
    ObjectArray<T>& appendNew(const T1& t1, const T2& t2, const T3& t3) {
        long pos = getLength();
        MemArray<T>::insertAmount(pos, 1);
        new(this->getPtr(pos)) T(t1, t2, t3);
        return *this;
    }
private:
};

} // namespace LucED

#endif // OBJECTARRAY_H
