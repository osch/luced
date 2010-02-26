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

#ifndef LUA_C_METHOD_ARG_CHECKER_HPP
#define LUA_C_METHOD_ARG_CHECKER_HPP

#include "LuaCFunctionArguments.hpp"
#include "LuaException.hpp"
#include "LuaVar.hpp"
#include "MoreArgs.hpp"
#include "OptionalArgs.hpp"
#include "EmptyClass.hpp"
#include "LuaArgException.hpp"

namespace LucED
{



template
<
    class Type1 = EmptyClass,
    class Type2 = EmptyClass,
    class Type3 = EmptyClass
>
class LuaCMethodArgChecker;


template
<
    class Type1,
    class Type2
>
class LuaCMethodArgChecker<Type1,Type2,MoreArgs>
{
public:

    static bool isValid(const LuaCFunctionArguments& args)
    {
        return    args.getLength() >= 2
               && args[0].is<Type1>()
               && args[1].is<Type2>();

    }

    static void check(const LuaCFunctionArguments& args)
    {
        if (!isValid(args))
        {
            throw LuaArgException();
        }
    }
    
private:
};
template
<
    class Type1,
    class Type2,
    class Optional1,
    class Optional2,
    class Optional3
>
class LuaCMethodArgChecker< Type1,Type2,OptionalArgs<Optional1,Optional2,Optional3> >
{
public:

    static bool isValid(const LuaCFunctionArguments& args, int index = 0)
    {
        return    args.getLength() > index + 1
               && args[index + 0].is<Type1>()
               && args[index + 1].is<Type2>();

    }

    static void check(const LuaCFunctionArguments& args)
    {
        if (!isValid(args) || !OptionalArgs<Optional1,Optional2,Optional3>::isValid(args, 2))
        {
            throw LuaArgException();
        }
    }
    
private:
};

template
<
    class Type1,
    class Type2
>
class LuaCMethodArgChecker<Type1,Type2,EmptyClass>
{
public:

    static bool isValid(const LuaCFunctionArguments& args)
    {
        return    args.getLength() == 2
               && args[0].is<Type1>()
               && args[1].is<Type2>();

    }

    static void check(const LuaCFunctionArguments& args)
    {
        if (!isValid(args))
        {
            throw LuaArgException();
        }
    }
    
private:
};

template
<
    class Type1
>
class LuaCMethodArgChecker<Type1,EmptyClass,EmptyClass>
{
public:

    static bool isValid(const LuaCFunctionArguments& args)
    {
        return    args.getLength() == 1
               && args[0].is<Type1>();

    }

    static void check(const LuaCFunctionArguments& args)
    {
        if (!isValid(args))
        {
            throw LuaArgException();
        }
    }
    
private:
};
} // namespace LucED

#endif // LUA_C_METHOD_ARG_CHECKER_HPP
