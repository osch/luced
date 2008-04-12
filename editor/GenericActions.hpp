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

#ifndef GENERIC_ACTIONS_HPP
#define GENERIC_ACTIONS_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "String.hpp"
#include "HashMap.hpp"

namespace LucED
{

template < class P // handler parameter
         >
class GenericActions : public HeapObject
{
public:
    typedef OwningPtr<GenericActions> Ptr;
    typedef P                         Parameter;

    class Handler : public HeapObject
    {
    public:
        typedef OwningPtr<Handler> Ptr;

        virtual bool execute(const String& methodName) = 0;

    protected:
        Handler()
        {}
    };
    
    
    virtual OwningPtr<Handler> createNewHandler(const Parameter& parameter) = 0;

protected:
    GenericActions()
    {}
};

} // namespace LucED

#endif // GENERIC_ACTIONS_HPP
