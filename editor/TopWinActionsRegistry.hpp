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

class TopWinActionsRegistry : public HeapObject
{
public:
    static RawPtr<TopWinActionsRegistry> getInstance();

    typedef HashMapValue<TopWinActions::Ptr> FoundActions;
    typedef TopWinActions::HandlerMap HandlerMap;
    typedef TopWinActions::ActionsMap ActionsMap;

    void registerActions(const String& id, TopWinActions::Ptr actions) {
        actionsMap->set(id, actions);
    }
    
    class Handler : public HeapObject
    {
    public:
        typedef OwningPtr<Handler> Ptr;
        
        bool execute(const String& className, const String& methodName)
        {
            HandlerMap::Value foundHandler = handlerMap->get(className);
            if (foundHandler.isValid())
            {
                return foundHandler.get()->execute(methodName);
            }
            else {
                ActionsMap::Value foundActions = actionsMap->get(className);
                if (foundActions.isValid())
                {
                    TopWinActions::Handler::Ptr newHandler = foundActions.get()->createNewHandler(parameter);
                    handlerMap->set(className, newHandler);
                    return newHandler->execute(methodName);
                }
                else {
                    return false;
                }
            }
        }
        
    private:
        friend class TopWinActionsRegistry;
        
        Handler(const TopWinActions::Parameter& parameter,
                ActionsMap::Ptr                 actionsMap)
            : parameter(parameter),
              actionsMap(actionsMap),
              handlerMap(HandlerMap::create())
        {}
        TopWinActions::Parameter parameter;
        ActionsMap::Ptr          actionsMap;
        HandlerMap::Ptr          handlerMap;
    };
    
    Handler::Ptr createNewHandler(const TopWinActions::Parameter& parameter)
    {
        return Handler::Ptr(new Handler(parameter, actionsMap));
    }
    
private:
    friend class SingletonInstance<TopWinActionsRegistry>;
    static SingletonInstance<TopWinActionsRegistry> instance;

    TopWinActionsRegistry()
        : actionsMap(ActionsMap::create())
    {}
    ActionsMap::Ptr actionsMap;
};

} // namespace LucED

#endif // TOP_WIN_ACTIONS_REGISTRY_HPP

