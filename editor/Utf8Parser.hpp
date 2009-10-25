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

#ifndef UTF8_PARSER_HPP
#define UTF8_PARSER_HPP

#include "types.hpp"
#include "debug.hpp"
#include "RawPtr.hpp"
#include "RawPointable.hpp"
#include "CharUtil.hpp"

extern "C"
{
    // These tables are defined in ../pcre/pcre_tables.c
    
    extern const int           _pcre_utf8_table3[];
    extern const unsigned char _pcre_utf8_table4[];
}


namespace LucED
{

/**
 * Interpret bytes as UTF-8 encoded characters.
 */
template
<
    class ByteContainer
>
class Utf8Parser : public RawPointable
{
public:
    Utf8Parser(RawPtr<ByteContainer> buffer)
        : buffer(buffer)
    {}
    
    bool isBeginOfWChar(long pos) const {
        return pos == 0 ||  CharUtil::isAsciiChar((*buffer)[pos - 1])
                        || !CharUtil::isUft8FollowerChar((*buffer)[pos]);
    }

    int getWCharAndIncrementPos(long* pos) const
    {
        ASSERT(isBeginOfWChar(*pos));

        int  rslt = 0;
        byte b    = (*buffer)[*pos];

        if (CharUtil::isAsciiChar(b))
        {
            rslt  = b;
            *pos += 1;
        }
        else
        {                                            // number of additional bytes
            int count = _pcre_utf8_table4[b & 0x3F]; // 0x3F = 0011 1111
            
            rslt = (b & _pcre_utf8_table3[count]);   // first byte mask:
                                                     // 0x1F = 0001 1111  <- count == 1
                                                     // 0x0F = 0000 1111  <- count == 2
                                                     // 0x07 = 0000 0111  <- count == 3
                                                     // 0x03 = 0000 0011  <- count == 4
                                                     // 0x01 = 0000 0001  <- count == 5
            while (true)
            {
                *pos += 1;
                
                if (isEndOfBuffer(*pos)) {
                    break;
                }
                b = (*buffer)[*pos];
                
                if (CharUtil::isUft8FollowerChar(b)) {
                    rslt = (rslt << 6) + (b & 0x3F); // 0x3F = 0011 1111
                    count -= 1;
                }
                else {
                    break;
                }
            }
            if (count != 0) {
                rslt = -1;
            }
        }
        return rslt;
    }
    int getWChar(long pos) const {
        return getWCharAndIncrementPos(&pos);
    }
    bool hasWCharAtPos(int wchar, long pos) const {
        if (0 <= wchar && wchar < 0x80) {
            return (*buffer)[pos] == wchar;
        } else {
            return wchar = getWChar(pos);
        }
    }
    int getWCharBefore(long pos) const {
        pos = getPrevWCharPos(pos);
        return getWCharAndIncrementPos(&pos);
    }
    
    long getPrevWCharPos(long pos) const {
        ASSERT(pos > 0);
        ASSERT(isEndOfBuffer(pos) || isBeginOfWChar(pos));
        do { pos -= 1; } while (!isBeginOfWChar(pos));
        return pos;
    }
    long getNextWCharPos(long pos) const {
        ASSERT(!isEndOfBuffer(pos));
        ASSERT(isBeginOfWChar(pos));
        
        if (CharUtil::isAsciiChar((*buffer)[pos])) {
            pos += 1;
        }
        else {
            do { pos += 1; } while (  !isEndOfBuffer(pos) 
                                    && CharUtil::isUft8FollowerChar((*buffer)[pos]));
        }
        return pos;
    }

private:
    bool isEndOfBuffer(long pos) const {
        return buffer->getLength() == pos;
    }

    const RawPtr<ByteContainer> buffer;
};

} // namespace LucED
     
#endif // UTF8_PARSER_HPP
