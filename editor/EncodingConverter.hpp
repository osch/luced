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

#ifndef ENCODING_CONVERTER_HPP
#define ENCODING_CONVERTER_HPP


#include "types.hpp"
#include "String.hpp"
#include "Utf8Parser.hpp"
#include "RawPointable.hpp"
#include "RawPtr.hpp"
#include "CharUtil.hpp"
#include "ByteBuffer.hpp"
#include "RawPtr.hpp"
#include "File.hpp"

namespace LucED
{

class EncodingConverter
{
public:

    //
    // Static Methods
    // 

    static String convertUtf8ToLatin1String(const byte* bytes, long length)
    {
        String rslt;
        
        Adapter adapter(bytes, length);
        
        Utf8Parser<Adapter> utf8Parser(&adapter);
        
        long pos = 0;
        
        while (pos < length)
        {
            long appendedPos = pos;

            while (pos < length && CharUtil::isAsciiChar(adapter[pos])) { ++pos; }

            rslt.append(bytes + appendedPos, pos - appendedPos);
            
            while (pos < length && !CharUtil::isAsciiChar(adapter[pos]))
            {
                int wchar = utf8Parser.getWCharAndIncrementPos(&pos);
                if (0 <= wchar && wchar <= 0xff) {
                    rslt.append((byte)(wchar));
                } else {
                    rslt.append('?');
                }
            }
        }
        return rslt;
    }
    static String convertUtf8ToLatin1String(const String& utf8String)
    {
        return convertUtf8ToLatin1String((const byte*)utf8String.toCString(), 
                                                      utf8String.getLength());
    }

    static String convertLocaleToUtf8StringIgnoreErrors(const String& fromString);

    static String convertUtf8ToLocaleStringIgnoreErrors(const String& fromString);

    static String convertLatin1ToUtf8String(const byte* bytes, long length)
    {
        String rslt;
        
        Adapter adapter(bytes, length);
        
        long pos = 0;
        
        while (pos < length)
        {
            long appendedPos = pos;

            while (pos < length && CharUtil::isAsciiChar(adapter[pos])) { ++pos; }

            rslt.append(bytes + appendedPos, pos - appendedPos);
            
            while (pos < length && !CharUtil::isAsciiChar(adapter[pos]))
            {
                int c = adapter[pos++];                        // 0xC0 = 1100 0000
                rslt.append((byte)(0xC0 | ((c >> 6) & 0x1F))); // 0x1F = 0001 1111
                rslt.append((byte)(0x80 | (c & 0x3F)));        // 0x80 = 1000 0000
                                                               // 0x3F = 0011 1111
            }
        }
        return rslt;
    }

    //
    // Object Methods
    // 

    EncodingConverter(const String& fromCodeset, const String& toCodeset);

    bool isConvertingBetweenDifferentCodesets();
    
    void convertInPlace(RawPtr<ByteBuffer> buffer);
    void convertToFile (const ByteBuffer&  buffer, const File& file);

    String convertStringToString(const String& fromString);
    
    
private:
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
    
    String fromCodeset;
    String toCodeset;
};

} // namespace LucED

#endif // ENCODING_CONVERTER_HPP
