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

#ifndef BYTEARRAY_H
#define BYTEARRAY_H

#include <string.h>
#include <string>

#include "MemArray.h"

namespace LucED {

using std::string;

class ByteArray : public MemArray<byte>
{
public:
    ByteArray& appendCStr(const char* cstring) {
        append((const byte*) cstring, strlen(cstring));
        return *this;
    }
    ByteArray& appendString(const string& string) {
        append((const byte*) string.c_str(), string.length());
        return *this;
    }
    const char* toCStr() const {
        if (getLength() <= 0 || (*this)[getLength() - 1] != 0) {
            mem.increaseTo(getLength() + 1);
            *mem.getPtr(getLength()) = 0;
        }
        return (const char*) getPtr();
    }
    string toString() const {
        return string( (const char*) getPtr(0), getLength() );
    }
    void fillAmountWith(long startPos, long amount, byte fillByte) {
        ASSERT(0 <= startPos && startPos + amount <= getLength());
        memset(getPtr(startPos), fillByte, amount);
    }
    void appendAndFillAmountWith(long amount, byte fillByte) {
        long pos = getLength();
        appendAmount(amount);
        fillAmountWith(pos, amount, fillByte);
    }
};

static inline bool operator==(const ByteArray& lhs, const ByteArray& rhs)
{
    return lhs.getLength() == rhs.getLength() 
            && memcmp(lhs.getPtr(0), rhs.getPtr(0), lhs.getLength()) == 0;
}

} // namespace LucED

#endif // BYTEARRAY_H
