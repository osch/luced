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

#ifndef UTIL_INCLUDED
#define UTIL_INCLUDED

#include <stdio.h>

#define ROUNDED_DIV(a,b) (((a) + ((b)/2) )/(b))

#define ROUNDED_UP_DIV(a,b) (((a) + (b) - 1)/(b))

#define MINIMIZE(a,b) \
{ \
    if (*(a) > (b)) \
        *(a) = (b); \
}

#define MAXIMIZE(a,b) \
{ \
    if (*(a) < (b)) \
        *(a) = (b); \
}

namespace LucED {
namespace util {

template<class T> static inline void maximize(T* a, T b)
{
    if (*a < b)
        *a = b;
}

template<class T> static inline void minimize(T* a, T b)
{
    if (*a > b)
        *a = b;
}

template<class T> static inline T minimum(T a, T b)
{
    if (a < b)
        return a;
    else
        return b;
}

template<class T> static inline T maximum(T a, T b)
{
    if (a > b)
        return a;
    else
        return b;
}

template<class T, class S> static inline T roundedUpDiv(T a, S b)
{
    return (((a) + (b) - 1)/(b));
}

} // namespace util
} // namespace LucED

#endif // UTIL_INCLUDED
