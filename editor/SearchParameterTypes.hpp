/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
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

#ifndef SEARCH_PARAMETER_TYPES_HPP
#define SEARCH_PARAMETER_TYPES_HPP

#include "Flags.hpp"

namespace LucED
{

class SearchParameterTypes
{
public:
    enum Option
    {
        REGEX,
        WHOLE_WORD,
        IGNORE_CASE,
        BACKWARD,
        NOT_AT_START
    };
    
    typedef Flags<Option> Options;
};

} // namespace LucED

#endif // SEARCH_PARAMETER_TYPES_HPP
