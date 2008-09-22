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

#include "ActionKeyConfig.hpp"

using namespace LucED;


void ActionKeyConfig::append(KeyCombination    keyCombination,
                             ActionId          actionId)
{
    if (!keyCombination.hasKeyIds()) {
        throw ConfigException(String() << "Invalid empty key combination");
    }
    
    {
        KeyCombinations::Ptr keys;
        
        ActionsToKeysMap::Value foundValue = actionsToKeysMap.get(actionId);
        if (foundValue.isValid()) {
            keys = foundValue.get();
        } else {
            keys = KeyCombinations::create();
            actionsToKeysMap.set(actionId, keys);
        }
        keys->append(keyCombination);
    }
    {
        ActionIds::Ptr actions;
        
        KeysToActionsMap::Value foundValue = keysToActionsMap.get(keyCombination);
        if (foundValue.isValid()) {
            actions = foundValue.get();
        } else {
            actions = ActionIds::create();
            keysToActionsMap.set(keyCombination, actions);
        }
        actions->append(actionId);
    }
    
    KeyId keyId = keyCombination.getFirstKeyId();
                  keyCombination.removeFirstKeyId();

    KeyMapping::Id id(keyCombination.getKeyModifier(), 
                      keyId);

    NodeMap::Value foundNode = nodeMap.get(id);
    
    if (keyCombination.hasKeyIds())
    {
        ActionKeyConfig::Ptr next;

        if (foundNode.isValid()) {
            if (foundNode.get()->hasNext()) {
                next = foundNode.get()->getNext();
            } else {
                next = ActionKeyConfig::create();
                foundNode.get()->setNext(next);
            }
        } else {
            next = ActionKeyConfig::create();
            nodeMap.set(id, Node::create(next));
        }
        next->append(keyCombination, actionId);
    }
    else
    {
        if (foundNode.isValid()) {
            foundNode.get()->appendActionId(actionId);
        } else {
            ActionIds::Ptr actionIds = ActionIds::create();
                           actionIds->append(actionId);
            nodeMap.set(id, Node::create(actionIds));
        }
    }
}
