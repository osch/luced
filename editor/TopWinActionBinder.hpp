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

#ifndef TOP_WIN_ACTION_BINDER_HPP
#define TOP_WIN_ACTION_BINDER_HPP

#include "TopWinActions.hpp"
#include "TopWinActionsRegistry.hpp"

namespace LucED
{

class TopWinActionBinder : public HeapObject
{
private:
    typedef TopWinActions::Binding                    Binding;
    typedef HeapHashMap< String, Binding::Ptr >       BindingMap;
    typedef HeapHashMap< String, TopWinActions::Ptr > ActionsMap;

public:
    typedef OwningPtr<TopWinActionBinder> Ptr;
    
    typedef TopWinActions::Parameter         Parameter;
    
    static Ptr create(const Parameter& parameter)
    {
        return Ptr(new TopWinActionBinder(parameter, 
                                          TopWinActionsRegistry::BinderAccess::getActionsMap()));
    }


    bool execute(const String& className, const String& methodName)
    {
        Binding::Ptr binding = getBinding(className);
        if (binding.isValid()) {
            return binding->execute(methodName);
        } else {
            return false;
        }
    }

    Callback<>::Ptr getCallback(const String& className, const String& methodName)
    {
        Callback<>::Ptr rslt;
        
        Binding::Ptr binding = getBinding(className);
        if (binding.isValid()) {
            rslt = binding->getCallback(methodName);
        }
        return rslt;
    }

private:
    TopWinActionBinder(const TopWinActions::Parameter& parameter,
                       ActionsMap::Ptr                 actionsMap)
        : parameter(parameter),
          actionsMap(actionsMap),
          bindingMap(BindingMap::create())
    {}
    
    Binding::Ptr getBinding(const String& className)
    {
        Binding::Ptr rslt;
        
        BindingMap::Value foundBinding = bindingMap->get(className);
        if (foundBinding.isValid())
        {
            rslt = foundBinding.get();
        }
        else {
            ActionsMap::Value foundActions = actionsMap->get(className);
            if (foundActions.isValid())
            {
                TopWinActions::Binding::Ptr newBinding = foundActions.get()->createNewBinding(parameter);
                bindingMap->set(className, newBinding);
                rslt = newBinding;
            }
        }
        return rslt;
    }
    
    TopWinActions::Parameter parameter;
    ActionsMap::Ptr          actionsMap;
    BindingMap::Ptr          bindingMap;
};

} // namespace LucED

#endif // TOP_WIN_ACTION_BINDER_HPP
