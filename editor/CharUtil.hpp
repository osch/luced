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

#ifndef CHAR_UTIL_HPP
#define CHAR_UTIL_HPP

extern "C"
{
    #include "../pcre/config.h"
    #include "../pcre/pcre_internal.h"
}

#include "types.hpp"

namespace LucED
{

class CharUtil
{
public:
    static bool isAsciiChar(byte b)  {
        return (b & 0x80) == 0x00;             // 0x80 = 1000 0000
    }                                          // 0x00 = 0000 0000
    static bool isUft8FollowerChar(byte b) { 
        return (b & 0xC0) == 0x80;             // 0xC0 = 1100 0000
    }                                          // 0x80 = 1000 0000
    static bool isNumber(int unicodeChar)
    {
        unsigned char chartype = GET_UCD(unicodeChar)->chartype;
        return    chartype == ucp_Nd
               || chartype == ucp_Nl
               || chartype == ucp_No;
    }
    static bool isLetter(int unicodeChar)
    {
        unsigned char chartype = GET_UCD(unicodeChar)->chartype;
        return    chartype == ucp_Ll
               || chartype == ucp_Lm
               || chartype == ucp_Lo
               || chartype == ucp_Lt
               || chartype == ucp_Lu;
    }
};

} // namespace LucED

#endif // CHAR_UTIL_HPP
