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

#ifndef MEM_ARRAY_HPP
#define MEM_ARRAY_HPP

#include "HeapMem.hpp"
#include "RawPointable.hpp"
#include "RawPtr.hpp"

namespace LucED {

/**
 * Array for objects that do not need constructors 
 * and destructors and can be copied by memmove.
 */
template
<
    typename T
>
class MemArray : public RawPointable
{
public:
    MemArray(long size = 0) {
        this->size = size;
        mem.increaseTo(size * sizeof(T));
    }
    void takeOver(RawPtr< MemArray<T> > rhs)
    {
        this->size  = rhs->size;
        this->mem.takeOver(&rhs->mem);
        rhs->size   = 0;
    }
    T& operator[](long i) {
        ASSERT(0 <= i && i < size);
        return *getPtr(i);
    }
    const T& operator[](long i) const {
        ASSERT(0 <= i && i < size);
        return *getPtr(i);
    }
    T* getPtr(long pos = 0) {
        return (T*) mem.getPtr(pos * sizeof(T));
    }
    const T* getPtr(long pos = 0) const {
        return (const T*) mem.getPtr(pos * sizeof(T));
    }
    T* getAmount(long startPos, long amount) {
        long endPos = startPos + amount;
        ASSERT(0 <= startPos && startPos <= endPos
                && endPos <= getLength());
        return getPtr(startPos);
    }
    const T& getLast() const {
        return *getPtr(getLength() - 1);
    }
    T& getLast() {
        return *getPtr(getLength() - 1);
    }
    T getAndRemoveLast() {
        int length = getLength();
        T rslt = *getPtr(length - 1);
        removeAmount(length - 1, 1);
        return rslt;
    }
    void assureCapacityAtLeast(long capacity) const {
        if (mem.getCapacity() < capacity * sizeof(T)) {
            mem.increaseTo(capacity * sizeof(T));
        }
    }
    long getLength() const {
        return size;
    }
    MemArray& clear() {
        size = 0;
        return *this;
    }
    MemArray& removeAmount(long pos, long amount) {
        ASSERT(0 <= pos && pos + amount <= size);
        T* dst = getPtr(pos);
        const T* src = getPtr(pos + amount);
        memmove(dst, src, (size - (pos + amount)) * sizeof(T));
        size -= amount;
        return *this;
    }
    MemArray& remove(long pos) {
        return removeAmount(pos, 1);
    }
    MemArray& removeTail(long pos) {
        ASSERT(0 <= pos && pos <= size);
        removeAmount(pos, getLength() - pos);
        return *this;
    }
    MemArray& removeLast() {
        removeAmount(getLength() - 1, 1);
        return *this;
    }
    MemArray& removeBetween(long pos1, long pos2) {
        removeAmount(pos1, pos2 - pos1);
        return *this;
    }
    T* insertAmount(long pos, long amount) {
        mem.increaseTo((size + amount) * sizeof(T));
        T* dst = getPtr(pos + amount);
        T* src = getPtr(pos);
        memmove(dst, src, (size - pos) * sizeof(T));
        size += amount;
        return src;
    }
    MemArray& insert(long pos, const T* source, long sourceLength) {
        insertAmount(pos, sourceLength);
        T*       dst = getPtr(pos);
        const T* src = source;
        memcpy(dst, src, sourceLength * sizeof(T));
        return *this;
    }
    MemArray& insert(long pos, const MemArray& src) {
        insert(pos, src.getPtr(0), src.getLength());
        return *this;
    }
    MemArray& append(const T* source, long sourceLength) {
        insert(size, source, sourceLength);
        return *this;
    }
    MemArray& append(const MemArray& src) {
        append(src.getPtr(0), src.getLength());
        return *this;
    }
    MemArray& append(const T& source) {
        append(&source, 1);
        return *this;
    }
    T* appendAmount(long amount) {
        return insertAmount(size, amount);
    }
    MemArray& increaseTo(long newSize) {
        if (newSize > size) {
            appendAmount(newSize - size);
        }
        return *this;
    }
    
    int findFirstIndex(const T& rhs) const {
        for (int i =  0, n = getLength(); i < n; ++i) {
            if ((*this)[i] == rhs) {
                return i;
            }
        }
        return -1;
    }
    
    bool operator==(const MemArray<T>& rhs) const {
        return (getLength() == rhs.getLength()
             && memcmp(getPtr(0), rhs.getPtr(0), getLength() * sizeof(T)) == 0);
    }
    
    bool operator !=(const MemArray<T>& rhs) const {
        return !(*this == rhs);
    }
    
protected:
    long size;
    mutable HeapMem mem;
};

} // namespace LucED

#endif // MEM_ARRAY_HPP
