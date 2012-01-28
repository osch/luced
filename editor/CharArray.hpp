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

#ifndef CHAR_ARRAY_H
#define CHAR_ARRAY_H

#include "String.hpp"
#include "String.hpp"

#include "MemArray.hpp"

namespace LucED
{

class CharArray : public MemArray<char>
{
public:
    CharArray& appendCStr(const char* cstring) {
        append((const char*) cstring, strlen(cstring));
        return *this;
    }
    CharArray& appendString(const String& s) {
        append((const char*) s.toCString(), s.getLength());
        return *this;
    }
    const char* toCString() const {
        if (getLength() <= 0 || (*this)[getLength() - 1] != 0) {
            mem.increaseTo(getLength() + 1);
            *mem.getPtr(getLength()) = 0;
        }
        return (const char*) getPtr();
    }
    String toString() const {
        return String( (const char*) getPtr(0), getLength() );
    }
    void fillAmountWith(long startPos, long amount, char fillChar) {
        ASSERT(0 <= startPos && startPos + amount <= getLength());
        memset(getPtr(startPos), fillChar, amount);
    }
    char* appendAndFillAmountWith(long amount, char fillChar) {
        long pos = getLength();
        char* rslt = appendAmount(amount);
        fillAmountWith(pos, amount, fillChar);
        return rslt;
    }
};

} // namespace LucED

#endif // CHAR_ARRAY_H
