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

#ifndef TOP_WIN_ACTIONS_REGISTRY_HPP
#define TOP_WIN_ACTIONS_REGISTRY_HPP

#include "TopWinActions.hpp"
#include "SingletonInstance.hpp"
#include "HashMap.hpp"
#include "HeapHashMap.hpp"
#include "String.hpp"

namespace LucED
{

class TopWinActionBinder;

class TopWinActionsRegistry : public HeapObject
{
public:
    static RawPtr<TopWinActionsRegistry> getInstance();

    typedef HashMapValue<TopWinActions::Ptr> FoundActions;
    typedef TopWinActions::BindingMap        BindingMap;
    typedef TopWinActions::ActionsMap        ActionsMap;

    void registerActions(const String& id, TopWinActions::Ptr actions) {
        actionsMap->set(id, actions);
    }

    class BinderAccess
    {
        friend class TopWinActionBinder;

        static ActionsMap::Ptr getActionsMap() {
            return TopWinActionsRegistry::getInstance()->actionsMap;
        }    
    };    
    
private:
    static       SingletonInstance<TopWinActionsRegistry> instance;
    friend class SingletonInstance<TopWinActionsRegistry>;

    TopWinActionsRegistry()
        : actionsMap(ActionsMap::create())
    {}
    
    ActionsMap::Ptr actionsMap;
};

} // namespace LucED

#endif // TOP_WIN_ACTIONS_REGISTRY_HPP

