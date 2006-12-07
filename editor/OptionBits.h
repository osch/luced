/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#ifndef OPTIONBITS_H
#define OPTIONBITS_H

namespace LucED {

template<class B, class T=int> class OptionBits
{
public:
    OptionBits() : bits(0) {}
    OptionBits(B bit) : bits(bit) {}
    
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
private:
    T bits;
};

template<class B> OptionBits<B> operator|(B bit1, B bit2) {
    OptionBits<B> rslt;
    rslt |= bit1;
    rslt |= bit2;
    return rslt;
}
    
} // namespace LucED

#endif // OPTIONBITS_H
