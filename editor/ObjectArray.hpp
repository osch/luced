/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#ifndef OBJECT_ARRAY_HPP
#define OBJECT_ARRAY_HPP

#include <new>
#include <algorithm>

#include "debug.hpp"
#include "HeapMem.hpp"
#include "MemArray.hpp"
#include "RawPtrGuardHolder.hpp"
#include "RawPointable.hpp"

namespace LucED
{

/**
 * Array for objects with default-constructors, copy-constructors and 
 * destructors that can be moved by memmove
 */
template
<
    class T
>
class ObjectArray : public  RawPointable
{
public:
    ObjectArray() : memArray(0) {}
    
    ObjectArray(long size) : memArray(size) {
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
    ObjectArray(const ObjectArray<T>& src) : memArray(0) {
        for (long i = 0; i < src.getLength(); ++i) {
            append(src[i]);
        }
    }
    template< class T2
            >
    explicit ObjectArray(const ObjectArray<T2>& src) : memArray(0) {
        for (long i = 0; i < src.getLength(); ++i) {
            append(src[i]);
        }
    }
    ObjectArray<T>& operator=(const ObjectArray<T>& src) {
        clear();
        for (long i = 0; i < src.getLength(); ++i) {
            append(src[i]);
        }
        return *this;
    }
    template< class T2
            >
    ObjectArray<T>& operator=(const ObjectArray<T2>& src) {
        clear();
        for (long i = 0; i < src.getLength(); ++i) {
            append(src[i]);
        }
        return *this;
    }
    T& operator[](long i) {
        ASSERT(0 <= i && i < getLength());
        return *this->getPtr(i);
    }
    T* getPtr(long i) {
        return memArray.getPtr(i);
    }
    T get(long i) const {
        ASSERT(0 <= i && i < getLength());
        return *this->getPtr(i);
    }
    T& get(long i) {
        ASSERT(0 <= i && i < getLength());
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
        ASSERT(0 <= i && i < getLength());
        return *this->getPtr(i);
    }
    const T* getPtr(long i) const {
        return memArray.getPtr(i);
    }
    long getLength() const {
        return memArray.getLength();
    }
    ObjectArray<T>& clear()
    {
        invalidateAllPtrGuards();
        for (long i = 0; i < getLength(); ++i) {
            this->getPtr(i)->~T();
        }
        memArray.clear();
        return *this;
    }
    ObjectArray<T>& removeAmount(long pos, long amount)
    {
        invalidateAllPtrGuards();
        ASSERT(0 <= pos && pos + amount <= getLength());
        for (long p = pos; p < pos + amount; ++p) {
            this->getPtr(p)->~T();
        }
        memArray.removeAmount(pos, amount);
        return *this;
    }
    ObjectArray<T>& remove(long pos) {
        removeAmount(pos, 1);
        return *this;
    }
    ObjectArray<T>& removeLast() {
        ASSERT(getLength() > 0);
        removeAmount(getLength() - 1, 1);
        return *this;
    }
    ObjectArray<T>& removeBetween(long pos1, long pos2) {
        removeAmount(pos1, pos2 - pos1);
        return *this;
    }
    ObjectArray<T>& insert(long pos, const T* source, long sourceLength)
    {
        invalidateAllPtrGuards();
        memArray.insertAmount(pos, sourceLength);
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
    ObjectArray<T>& appendAmount(long amount)
    {
        invalidateAllPtrGuards();
        long oldSize = getLength();
        memArray.appendAmount(amount);
        for (long i = oldSize; i < getLength(); ++i) {
            new(this->getPtr(i)) T();
        }
        return *this;
    }
    
    ObjectArray<T>& appendNew() {
        return appendAmount(1);
    }
    template<class T1> 
    ObjectArray<T>& appendNew(const T1& t1)
    {
        invalidateAllPtrGuards();
        long pos = getLength();
        memArray.insertAmount(pos, 1);
        new(this->getPtr(pos)) T(t1);
        return *this;
    }
    template<class T1, class T2> 
    ObjectArray<T>& appendNew(const T1& t1, const T2& t2)
    {
        invalidateAllPtrGuards();
        long pos = getLength();
        memArray.insertAmount(pos, 1);
        new(this->getPtr(pos)) T(t1, t2);
        return *this;
    }
    template<class T1, class T2, class T3> 
    ObjectArray<T>& appendNew(const T1& t1, const T2& t2, const T3& t3)
    {
        invalidateAllPtrGuards();
        long pos = getLength();
        memArray.insertAmount(pos, 1);
        new(this->getPtr(pos)) T(t1, t2, t3);
        return *this;
    }
private:
    void invalidateAllPtrGuards()
    {
    #ifdef DEBUG
        for (int i = 0, n = getLength(); i < n; ++i) {
            RawPtrGuardHolder::RawPtrGuardAccess::invalidateRawPtrGuard(getPtr(i));
        }
    #endif
    }

    MemArray<T> memArray;
};

} // namespace LucED

#endif // OBJECT_ARRAY_HPP
