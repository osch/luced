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

#ifndef PATTERN_STACK_HPP
#define PATTERN_STACK_HPP

#include "ByteArray.hpp"
#include "String.hpp"

namespace LucED
{

class PatternStack
{
public:

    class Lengths
    {
    public:
        Lengths(long dataLength, long lengthLength)
            : dataLength(dataLength),
              lengthLength(lengthLength)
        {}
        
        long getTotal() const {
            return dataLength + lengthLength;
        }
        
        long dataLength;
        long lengthLength;
    };

    template
    <
        class ByteContainer
    >
    static Lengths getAdditionalDataLength(const ByteContainer& stack, long endPos) 
    {
        if (stack[endPos - 1] != 0xFF) {
            return Lengths(0, 0);
        } else {
            long p = endPos - 3;
            byte b = stack[p];
            long l = 0;
            
            if (!(b & 0x80))
            {
                l = b;
            }
            else
            {
                do {
                    l = (l << 7) + (b & 0x7F);
                    --p;
                    b = stack[p];
                }
                while (b & 0x80);
                
                l = (l << 7) + b;
            }

            ASSERT(l > 0);
            return Lengths(l, endPos - 2 - p);
        }
    }

    void clear() {
        stack.clear();
    }
    void appendAmount(long amount) {
        stack.appendAmount(amount);
    }
    byte* getPtr(long index) {
        return stack.getPtr(index);
    }
    const byte* getPtr(long index) const {
        return stack.getPtr(index);
    }
    long getLength() const {
        return stack.getLength();
    }
    byte getLast() const {
        long len = stack.getLength(); 
        byte last = stack[len - 1];
        if (last != 0xFF) {
            return last;
        } else {
            return stack[len - 2];
        }
    }
    byte getFirst() const {
        return stack[0];
    }

    bool hasLastAdditionalData() const
    {
        return stack.getLast() == 0xFF;
    }
    
    Lengths getAdditionalDataLength() const
    {
        return getAdditionalDataLength(stack, stack.getLength());
    }
    
    String getAdditionalDataAsString() const
    {
        Lengths lens = getAdditionalDataLength();
        long len = lens.getTotal();
        if (len > 0) {
            return String((const char*)stack.getPtr(stack.getLength() - 2 - len),
                          lens.dataLength);
        } else {
            return String();
        }
    }
    
    void removeLast()
    {
        long len = getAdditionalDataLength().getTotal();

        if (len == 0) {
            stack.removeLast();
        } else {
            stack.removeTail(stack.getLength() - 2 - len);
        }
    }
    
    void append(byte b) {
        ASSERT(b != 0xFF);
        stack.append(b);
    }

    void append(byte b, String additionalData)
    {
        ASSERT(b != 0xFF);

        unsigned long len = additionalData.getLength();

        if (len > 0)
        {
            stack.append((const byte*)additionalData.toCString(), len);
    
            stack.append((len & 0x7F));
            len = (len >> 7);

            while (len > 0) {
                stack.append((len & 0x7F) | 0x80);
                len = (len >> 7);
            }
    
            stack.append(b);
            stack.append(0xFF);
        }
        else {
            append(b);
        }
    }

private:
    ByteArray stack;    
};

} // namespace LucED

#endif // PATTERN_STACK_HPP
