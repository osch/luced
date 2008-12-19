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

#ifndef OPTIONAL_ARGS_HPP
#define OPTIONAL_ARGS_HPP

#include "EmptyClass.hpp"
#include "LuaCFunctionArguments.hpp"
#include "MoreArgs.hpp"

namespace LucED
{

template
<
    class Type1 = EmptyClass,
    class Type2 = EmptyClass,
    class Type3 = EmptyClass
>         
class OptionalArgs;

template
<
    class Type1,
    class Type2
>         
class OptionalArgs<Type1,Type2,EmptyClass>
{
public:
    static bool isValid(const LuaCFunctionArguments& args, int startIndex)
    {
        int i = startIndex;
        
        if (i < args.getLength()) {
            if (!args[i].is<Type1>()) {
                return false;
            }
            i += 1;
        }
        if (i < args.getLength()) {
            if (!args[i].is<Type2>()) {
                return false;
            }
            i += 1;
        }
        return i == args.getLength();
    }
};

template
<
    class Type1,
    class Type2
>         
class OptionalArgs<Type1,Type2,MoreArgs>
{
public:
    static bool isValid(const LuaCFunctionArguments& args, int startIndex)
    {
        int i = startIndex;
        
        if (i < args.getLength() && !args[i].is<Type1>()) {
            return false;
        }
        if (++i < args.getLength() && !args[i].is<Type2>()) {
            return false;
        }
        return true;
    }
};

} // namespace LucED

#endif // OPTIONAL_ARGS_HPP
