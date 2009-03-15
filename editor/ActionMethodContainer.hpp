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

#ifndef ACTION_METHOD_CONTAINER_HPP
#define ACTION_METHOD_CONTAINER_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "RawPtr.hpp"
#include "ObjectArray.hpp"
#include "ActionMethods.hpp"

namespace LucED
{

class ActionMethodContainer : public HeapObject
{
public:
    typedef OwningPtr<ActionMethodContainer> Ptr;
    
    static Ptr create() {
        return Ptr(new ActionMethodContainer());
    }
    
    void addActionMethods(ActionMethods::Ptr methods) {
        actionMethods.append(methods);
    }
    void addActionMethodContainer(RawPtr<ActionMethodContainer> rhs) {
        for (int i = 0, n = rhs->actionMethods.getLength(); i < n; ++i) {
            actionMethods.append(rhs->actionMethods[i]);
        }
    }
    
    virtual bool invokeActionMethod(ActionId actionId);
    virtual bool hasActionMethod(ActionId actionId);

protected:
    ActionMethodContainer()
    {}

private:
    ObjectArray<ActionMethods::Ptr> actionMethods;
};

} // namespace LucED

#endif // ACTION_METHOD_CONTAINER_HPP
