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

#ifndef ACTION_ID_REGISTRY_HPP
#define ACTION_ID_REGISTRY_HPP

#include "HeapObject.hpp"
#include "SingletonInstance.hpp"
#include "String.hpp"
#include "ActionId.hpp"
#include "HashMap.hpp"
#include "ConfigException.hpp"
#include "ObjectArray.hpp"

namespace LucED
{

class ActionIdRegistry : public HeapObject
{
public:
    typedef HashMap<String, ActionId> NameToIdMap;
    typedef HashMap<ActionId, String> IdToNameMap;
    
    static ActionIdRegistry* getInstance();

    ActionId getActionId(const String& name) 
    {
        ActionId rslt;

        NameToIdMap::Value foundId = nameToIdMap.get(name);
        if (foundId.isValid()) {
            rslt = foundId.get();
        }
        else {
            ActionId newId(newActionIdCounter++);
            idToNameMap.set(newId, name);
            nameToIdMap.set(name, newId);
            rslt = newId;
        }

        return rslt;
    }
    
private:
    friend class SingletonInstance<ActionIdRegistry>;
    static SingletonInstance<ActionIdRegistry> instance;
    
    friend class ActionId;
    
    ActionIdRegistry()
        : newActionIdCounter(ActionId::MAX_BUILTIN + 1)
    {
        ActionId::setBuiltinMappingInto(nameToIdMap, idToNameMap);
    }
    
    String toString(ActionId actionId) const {
        IdToNameMap::Value foundName = idToNameMap.get(actionId);
        if (foundName.isValid()) {
            return foundName.get();
        }
        else {
            ASSERT(false);
            return "";
        }
    }
    
    IdToNameMap idToNameMap;
    NameToIdMap nameToIdMap;
    int newActionIdCounter;
};

} // namespace LucED

#endif // ACTION_ID_REGISTRY_HPP
