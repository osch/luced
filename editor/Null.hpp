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

#ifndef NULL_HPP
#define NULL_HPP

namespace LucED
{

class NullType
{};

const NullType Null = NullType();

inline bool operator==(const void* lhs, const NullType& rhs)
{
    return lhs == NULL;
}
inline bool operator==(const NullType& lhs, const void* rhs)
{
    return rhs == NULL;
}

inline bool operator!=(const void* lhs, const NullType& rhs)
{
    return lhs != NULL;
}

inline bool operator!=(const NullType& lhs, const void* rhs)
{
    return rhs != NULL;
}

} // namespace LucED

#endif // NULL_HPP
