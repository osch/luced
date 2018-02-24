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

#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

#include "MemBuffer.hpp"
#include "String.hpp"

namespace LucED
{

class ByteBuffer : public MemBuffer<byte> 
{
public:
    ByteBuffer(long size = 0)
        : MemBuffer<byte>(size)
    {}
    ByteBuffer& appendCStr(const char* cstring) {
        append((const byte*) cstring, strlen(cstring));
        return *this;
    }
    ByteBuffer& appendString(const String& String) {
        append((const byte*) String.toCString(), String.getLength());
        return *this;
    }
    String toString() const {
        long len = getLength();
        return String( (const char*) getAmount(0, len), len );
    }
    void fillAmountWith(long startPos, long amount, byte fillByte) {
        ASSERT(0 <= startPos && startPos + amount <= getLength());
        memset(getAmount(startPos, amount), fillByte, amount);
    }
    byte* appendAndFillAmountWith(long amount, byte fillByte) {
        long pos = getLength();
        byte* rslt = appendAmount(amount);
        fillAmountWith(pos, amount, fillByte);
        return rslt;
    }
};

} // namespace LucED

#endif // BYTEBUFFER_H
