/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2012 Oliver Schmidt, oliver at luced dot de
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

#ifndef CHAR_BUFFER_H
#define CHAR_BUFFER_H

#include "MemBuffer.hpp"

namespace LucED
{

class CharBuffer : public MemBuffer<char> 
{
public:
    CharBuffer(long size = 0)
        : MemBuffer<char>(size)
    {}
    CharBuffer& appendCStr(const char* cstring) {
        append(cstring, strlen(cstring));
        return *this;
    }
    CharBuffer& appendString(const String& String) {
        append(String.toCString(), String.getLength());
        return *this;
    }
    String toString() const {
        long len = getLength();
        return String( (const char*) getAmount(0, len), len );
    }
    void fillAmountWith(long startPos, long amount, char fillChar) {
        ASSERT(0 <= startPos && startPos + amount <= getLength());
        memset(getAmount(startPos, amount), fillChar, amount);
    }
    char* appendAndFillAmountWith(long amount, char fillChar) {
        long pos = getLength();
        char* rslt = appendAmount(amount);
        fillAmountWith(pos, amount, fillChar);
        return rslt;
    }
};

} // namespace LucED

#endif // CHAR_BUFFER_H
