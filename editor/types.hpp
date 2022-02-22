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

#ifndef TYPES_H
#define TYPES_H


namespace LucED {

typedef unsigned char byte;

namespace ScrollStep
{
    enum Type {LINE_UP, LINE_DOWN, PAGE_UP, PAGE_DOWN};
}

namespace Direction
{
    enum Type {UP, RIGHT, DOWN, LEFT};
}

namespace Orientation
{
    enum Type {HORIZONTAL, VERTICAL};
}

namespace VerticalAdjustment
{
    enum Type {TOP, CENTER, BOTTOM};
}

} // namespace LucED


#endif // TYPES_H
