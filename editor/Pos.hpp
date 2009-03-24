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

#ifndef POS_HPP
#define POS_HPP

#include <limits.h>

#include "Len.hpp"

namespace LucED
{

class Pos
{
public:
    struct MaxType {};
    static const MaxType MAX;
    
    Pos()
        : value(0)
    {}
    
    Pos(MaxType dummy) 
        : value(INT_MAX)
    {}
    
    explicit Pos(int value)
        : value(value)
    {}
    
    operator int() const {
        return value;
    }
    
    Pos& operator+=(Len len) {
        value += len;
        return *this;
    }
    
    Pos operator+(Len len) const {
        return Pos(value + len);
    }

    Pos& operator-=(Len len) {
        value -= len;
        return *this;
    }
    
    Pos operator-(Len len) const {
        return Pos(value - len);
    }
    
    Len operator-(Pos len) const {
        return Len(value - len);
    }
    
    Pos& operator++() {
        ++value;
        return *this;
    }
    Pos& operator--() {
        --value;
        return *this;
    }
    
private:
    int value;
};

} // namespace LucED

#endif // POS_HPP
