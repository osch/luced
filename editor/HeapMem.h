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

#ifndef HEAPMEM_H
#define HEAPMEM_H

#include <stdlib.h>

#include "debug.h"
#include "types.h"
#include "NonCopyable.h"
#include "String.h"

namespace LucED {

class HeapMem
{
public:
    
    HeapMem() {
        capacity = 0;
        buffer = NULL;
    }
    
    HeapMem(const HeapMem& src) {
        capacity = 0;
        buffer = NULL;
        increaseTo(src.capacity);
        memcpy(buffer, src.buffer, src.capacity);
    }

    ~HeapMem() {
        if (buffer != NULL) {
            free(buffer);
        }
    }
    
    HeapMem& operator=(const HeapMem& src) {
        increaseTo(src.capacity);
        memcpy(buffer, src.buffer, src.capacity);
        return *this;
    }
    
    long getCapacity() const {
        return capacity;
    }
    
    void increase(long plusAmount, long blockSize = 1);
    void increaseTo(long newCapacity, long blockSize = 1) {
        if (newCapacity > capacity) {
            increase(newCapacity - capacity, blockSize);
        }
    }
    
    void replace(long dstPos, const byte* src, long srcLen) {
        ASSERT(0 <= srcLen);
        ASSERT(0 <= dstPos && dstPos + srcLen <= capacity);

        memcpy(buffer + dstPos, (char*) src, srcLen);
    }
    
    void replace(long dstPos, const byte* srcBegin, const byte* srcEnd) {
        replace(dstPos, srcBegin, srcEnd - srcBegin);
    }
    
    
    byte* getPtr(long pos) {
        ASSERT(0 <= pos && pos <= capacity);
        ASSERT(buffer != NULL || pos == 0);

        return buffer + pos;
    }
    
    const byte* getPtr(long pos) const {
        ASSERT(0 <= pos && pos <= capacity);
        ASSERT(buffer != NULL || pos == 0);

        return buffer + pos;
    }
    
private:
    long capacity;
    byte *buffer;

};

} // namespace LucED

#endif // HEAPMEM_H
