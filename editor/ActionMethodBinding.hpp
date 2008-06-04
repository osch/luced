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

#ifndef ACTION_METHOD_BINDING_HPP
#define ACTION_METHOD_BINDING_HPP

#include "ActionMethods.hpp"
#include "RawPtr.hpp"

namespace LucED
{

template < class T
         >
class ActionMethodBinding : public ActionMethods
{
public:
    typedef void (T::*MethodPtr)();

    virtual bool hasActionMethod(ActionId actionId)
    {
        return (getActionMethod(actionId) != NULL);
    }

    virtual bool invokeActionMethod(ActionId actionId)
    {
        MethodPtr methodPtr = getActionMethod(actionId);
    
        if (methodPtr != NULL) {
            (impl->*methodPtr)(); // invoke Method
            return true;
        } else {
            return false;
        }
    }
    
protected:
    ActionMethodBinding(T* impl)
        : impl(impl)
    {}       
        
private:
    static MethodPtr getActionMethod(ActionId actionId);

    RawPtr<T> impl;
};

} // namespace LucED

#endif // ACTION_METHOD_BINDING_HPP
