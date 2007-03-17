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

#ifndef MEMBUFFER_H
#define MEMBUFFER_H

#include "HeapMem.h"
#include "MemArray.h"

namespace LucED {

/**
 * Gap-buffer based array for objects that do not need constructors 
 * and destructors and can be copied by memmove.
 */
template<typename T> class MemBuffer : NonCopyable
{
public:

    MemBuffer(long size = 0) {
        this->gapPos = 0;
        this->gapSize = 0;
        insertAmount(0, size);
    }
    long getLength() const {
        return (mem.getCapacity() - gapSize) / sizeof(T);
    }
    T* getPtr(long pos) {
        if (pos < gapPos) {
            return (T*) mem.getPtr(pos * sizeof(T));
        } else {
            return (T*) mem.getPtr(pos * sizeof(T) + gapSize);
        }
    }
    const T* getPtr(long pos) const {
        if (pos < gapPos) {
            return (const T*) mem.getPtr(pos * sizeof(T));
        } else {
            return (const T*) mem.getPtr(pos * sizeof(T) + gapSize);
        }
    }
    T& operator[](long pos) {
        return *getPtr(pos);
    }
    const T& operator[](long pos) const {
        return *getPtr(pos);
    }
    T* getAmount(long startPos, long amount) {
        long endPos = startPos + amount;
        ASSERT(0 <= startPos && startPos <= endPos
                && endPos <= getLength());
        if (gapPos < startPos || endPos <= gapPos) {
            return getPtr(startPos);
        } else {
            long newGap;
            if (gapPos - startPos < endPos - gapPos) {
                newGap = startPos;
            } else {
                newGap = endPos;
            }
            moveGap(newGap);
            return getPtr(startPos);
        }
    }
    const T* getAmount(long startPos, long amount) const {
        long endPos = startPos + amount;
        ASSERT(0 <= startPos && startPos <= endPos
                && endPos <= getLength());
        if (gapPos < startPos || endPos <= gapPos) {
            return getPtr(startPos);
        } else {
            long newGap;
            if (gapPos - startPos < endPos - gapPos) {
                newGap = startPos;
            } else {
                newGap = endPos;
            }
            moveGap(newGap);
            return getPtr(startPos);
        }
    }
    T* getTotalAmount() {
        return getAmount(0, getLength());
    }
    T* insertAmount(long startPos, long amount) {
        T* rslt;
        moveGap(startPos);
        increaseGapTo(amount);
        rslt = (T*) mem.getPtr(startPos * sizeof(T));
        gapPos += amount;
        gapSize -= amount * sizeof(T);
        return rslt;
    }
    void insert(long startPos, const T* source, long amount) {
        T* rslt = insertAmount(startPos, amount);
        memcpy(rslt, source, amount * sizeof(T));
    }
    MemBuffer& insert(long pos, const T& src) {
        insert(pos, &src, 1);
        return *this;
    }
    MemBuffer& insert(long pos, const MemArray<T>& src) {
        insert(pos, src.getPtr(0), src.getLength());
        return *this;
    }
    T* appendAmount(long amount) {
        return insertAmount(getLength(), amount);
    }
    MemBuffer& append(const T& src) {
        *appendAmount(1) = src;
        return *this; 
    }
    MemBuffer& removeAmount(long pos, long amount) {
        ASSERT(0 <= pos && pos <= getLength());
        ASSERT(pos < getLength() || amount == 0);
        moveGap(pos);
        gapSize += amount * sizeof(T);
        return *this;
    }
    void clear() {
        gapPos = 0;
        gapSize = mem.getCapacity();
    }
    MemBuffer& removeTail(long pos) {
        return removeAmount(pos, getLength() - pos);
    }

private:
    mutable long gapPos;
    mutable long gapSize; // in Bytes 
    mutable HeapMem mem;

    void moveGap(long pos) const {
        if (gapPos != pos) {
            byte* buffer = mem.getPtr(0);
            long len;
            byte *src;
            byte *dst;
            if (pos < gapPos) {
                len = (gapPos - pos) * sizeof(T);
                src = buffer + pos * sizeof(T);
                dst = buffer + gapPos * sizeof(T) + gapSize - len;
            } else {
                len = (pos - gapPos) * sizeof(T);
                src = buffer + gapPos * sizeof(T) + gapSize;
                dst = buffer + gapPos * sizeof(T);
            }
            
            ASSERT(buffer <= dst && dst + len <= buffer + mem.getCapacity());
            ASSERT(buffer <= src && src + len <= buffer + mem.getCapacity());
            memmove(dst, src, len);
            gapPos = pos;
        }
    }
    void increaseGapTo(long newGapSize) {
        long len, srcPos;
        byte *dst, *src;
        newGapSize *= sizeof(T);
        if (newGapSize > gapSize) {
            srcPos = gapPos * sizeof(T) + gapSize;
            len = mem.getCapacity() - srcPos;
            mem.increaseTo(mem.getCapacity() - gapSize + newGapSize, sizeof(T));
            dst = mem.getPtr(0) + mem.getCapacity() - len;
            src = mem.getPtr(0) + srcPos;
            
            ASSERT(mem.getPtr(0) <= dst && dst + len <= mem.getPtr(0) + mem.getCapacity());
            ASSERT(mem.getPtr(0) <= src && src + len <= mem.getPtr(0) + mem.getCapacity());
            memmove(dst, src, len);
            gapSize += dst - src;
        }
    }
};


} // namespace LucED


#endif // MEMBUFFER_H
