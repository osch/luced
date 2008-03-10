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

#ifndef OPTION_BITS_HPP
#define OPTION_BITS_HPP

#include "debug.hpp"

namespace LucED
{

/**
 * Use this class only to wrap low level C flags. 
 * Prefer using Flags.hpp!
 */
template<class B, class T=int> class OptionBits
{
public:
    OptionBits() : bits(0)
    {
        ASSERT(sizeof(B) <= sizeof(T));
    }

    OptionBits(B bit) : bits(bit)
    {
        ASSERT(sizeof(B) <= sizeof(T));
    }
    
    OptionBits operator|(B bit) {
        OptionBits rslt = *this;
        rslt.bits |= bit;
        return rslt;
    }
    OptionBits& operator|=(B bit) {
        bits |= bit;
        return *this;
    }
    T getOptions() {
        return bits;
    }
    bool isSet(B bit) const {
        return bits & bit;
    }
    void clear() {
        bits = 0;
    }
    void set(B bit) {
        bits |= bit;
    }
    void clear(B bit) {
        bits &= ~bit;
    }
private:
    T bits;
};

} // namespace LucED

#endif // OPTION_BITS_HPP
