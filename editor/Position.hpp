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

#ifndef POSITION_H
#define POSITION_H

namespace LucED {

class Position
{
public:
    Position()
        : x(0), y(0), w(0), h(0) {}

    Position(int x, int y, int width, int height)
        : x(x), y(y), w(width), h(height) {}

    bool operator==(const Position& p) const {
        return x == p.x && y ==p.y && w == p.w && h == p.h;
    }

    bool operator!=(const Position& p) const {
        return !operator==(p);
    }

    int x, y;
    int w, h;
};
    
} // namespace LucED

#endif // POSITION_H
