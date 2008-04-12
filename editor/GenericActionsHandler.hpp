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

#ifndef GENERIC_ACTIONS_HANDLER_HPP
#define GENERIC_ACTIONS_HANDLER_HPP

#include "GenericActions.hpp"
#include "HeapHashMap.hpp"

namespace LucED
{

template < class P, // handler parameter
           class I  // handler implementation
         >
class GenericActionsHandler : public GenericActions<P>::Handler
{
public:
    typedef OwningPtr<GenericActionsHandler>    Ptr;
    typedef I                                   Impl;
    typedef P                                   Parameter;
    typedef void (Impl::*MethodPtr)();
    typedef HeapHashMap< String, MethodPtr > MethodMap;
    typedef typename MethodMap::Value        MethodMapValue;

    virtual bool execute(const String& methodName)
    {
        MethodMapValue foundMethod = methodMap->get(methodName);
        if (foundMethod.isValid())
        {
            MethodPtr methodPtr = foundMethod.get();
            
            (this->*methodPtr)(); // invoke Method
            return true;
        }
        else {
            return false;
        }
    }

protected:
    GenericActionsHandler(const Parameter&     p, 
                          OwningPtr<MethodMap> methodMap)
        : p(p),
          methodMap(methodMap)
    {}
    
    Parameter            p;
    OwningPtr<MethodMap> methodMap;
};

} // namespace LucED

#endif // GENERIC_ACTIONS_HANDLER_HPP
