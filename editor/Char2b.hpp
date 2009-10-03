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

#ifndef CHAR2B_HPP
#define CHAR2B_HPP

#include "headers.hpp"
#include "types.hpp"

namespace LucED
{

/**
 * Wrapper for XLib XChar2b.
 *
 * Since we assume, that all used xfonts are iso01646-1 unicode fonts,
 * the value of Char2b is unicode value.
 */
class Char2b : public XChar2b
{
public:
    Char2b() {
        XChar2b::byte1 = 0xff; // invalid char
        XChar2b::byte2 = 0xff;
    }
    
    Char2b(byte byte1, byte byte2) {
        XChar2b::byte1 = byte1;
        XChar2b::byte2 = byte2;
    }
    Char2b(byte byte2) {
        XChar2b::byte1 = 0;
        XChar2b::byte2 = byte2;
    }
    Char2b(signed char byte2) {
        XChar2b::byte1 = 0;
        XChar2b::byte2 = (byte)byte2;
    }
    
    Char2b(int wchar)
    {
        if (0 <= wchar && wchar <= 0xffff)
        {
            *this =  Char2b( (wchar & 0xff00) >> 8,
                              wchar & 0x00ff );
        } 
        else if (wchar == -1) {
            *this = Char2b( 0xff, 0xff );  // invalid char
        } else {
            *this = Char2b( 0xff, 0xfd );  // replacement char
        }
    }
    
    Char2b(XChar2b xchar)
    {
        *this = xchar;
    }
    
    bool operator==(const Char2b& rhs) const {
        return    byte2 == rhs.byte2
               && byte1 == rhs.byte1;
    }
    bool operator!=(const Char2b& rhs) const {
        return !(*this == rhs);
    }

    bool isValid() const {
        return *this != Char2b(0xff, 0xff);
    }
    
};

} // namespace LucED

#endif // FONT_HANDLE_HPP
