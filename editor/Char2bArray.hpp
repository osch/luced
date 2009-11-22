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

#ifndef CHAR2B_ARRAY_H
#define CHAR2B_ARRAY_H

#include "String.hpp"

#include "MemArray.hpp"
#include "Char2b.hpp"
#include "Utf8Parser.hpp"

namespace LucED
{

class Char2bArray : public MemArray<Char2b>
{
public:
    Char2bArray& appendUtf8String(const String& s)
    {
        appendGenericUtf8Bytes(s);
        return *this;
    }
    Char2bArray& appendUtf8String(const char* s)
    {
        long len = strlen(s);
        Adapter a((const byte*)s, len);
        appendGenericUtf8Bytes(a);
        return *this;
    }
    Char2bArray& setToUtf8String(const String& s) {
        clear();
        appendUtf8String(s);
        return *this;
    }
    
    long findFirstOf(Char2b c, Pos startPos = Pos(0)) const {
        long len  = getLength();
        long p    = startPos;
        
        while (p < len) {
            if ((*this)[p] == c) {
                return p;
            } else {
                p += 1;
            }
        }
        return -1;
    }

private:
    template<class ByteContainer
            >
    Char2bArray& appendGenericUtf8Bytes(const ByteContainer& s)
    {
        Utf8Parser<ByteContainer> utf8Parser(&s);
        long fromPos = 0;
        long fromLen = s.getLength();
        long toPos   = this->getLength();
        long toLen   = utf8Parser.getNumberOfWChars();
        this->appendAmount(toLen);
        while (fromPos < fromLen) {
            (*this)[toPos++] = utf8Parser.getWCharAndIncrementPos(&fromPos);
        }
        ASSERT(toPos == getLength());
        return *this;
    }

    class Adapter : public RawPointable
    {
    public:
        Adapter(const byte* bytes, long length)
            : bytes(bytes),
              length(length)
        {}
        
        long getLength() const {
            return length;
        }
        byte operator[](long index) const {
            ASSERT(0 <= index && index < length);
            return bytes[index];
        }
        
    private:
        const byte* bytes;
        const long  length;
    };
};

} // namespace LucED

#endif // CHAR2B_ARRAY_H
