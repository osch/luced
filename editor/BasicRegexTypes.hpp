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

#ifndef BASIC_REGEX_TYPES_HPP
#define BASIC_REGEX_TYPES_HPP

#include <pcre.h>

#include "OptionBits.hpp"

namespace LucED
{

class BasicRegexTypes
{
public:
    enum CreateOption
    {
        MULTILINE = PCRE_MULTILINE,
        EXTENDED = PCRE_EXTENDED,
        IGNORE_CASE = PCRE_CASELESS
    };
    typedef OptionBits<CreateOption> CreateOptions;
    
    enum MatchOption
    {
        NOTBOL = PCRE_NOTBOL,
        NOTEOL = PCRE_NOTEOL,
        ANCHORED = PCRE_ANCHORED,
        NOTEMPTY = PCRE_NOTEMPTY
    };
    typedef OptionBits<MatchOption> MatchOptions;
};

} // namespace LucED

#endif // BASIC_REGEX_TYPES_HPP
