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

#ifndef ACTION_KEY_CONFIG_HPP
#define ACTION_KEY_CONFIG_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "KeyMapping.hpp"
#include "String.hpp"
#include "ActionId.hpp"
#include "HashMap.hpp"
#include "ConfigException.hpp"
#include "KeyCombination.hpp"

namespace LucED
{

class ActionKeyConfig : public HeapObject
{
public:
    typedef OwningPtr<ActionKeyConfig> Ptr;
    
    typedef HeapObjectArray<KeyCombination> KeyCombinations;
    typedef HeapObjectArray<ActionId>       ActionIds;
    
    static Ptr create() {
        return Ptr(new ActionKeyConfig());
    }
    
    class Node : public HeapObject
    {
    public:
        typedef OwningPtr<Node> Ptr;
        
        bool hasNext() const {
            return next.isValid();
        }
        bool hasActionIds() const {
            return (actionIds.isValid() && actionIds->getLength() > 0);
        }
        ActionIds::Ptr getActionIds() const {
            return actionIds;
        }
        ActionKeyConfig::Ptr getNext() const {
            return next;
        }
    private:
        friend class ActionKeyConfig;
        
        static Ptr create(ActionIds::Ptr actionIds) {
            return Ptr(new Node(actionIds));
        }
        static Ptr create(ActionKeyConfig::Ptr next) {
            return Ptr(new Node(next));
        }

        explicit Node(ActionIds::Ptr actionIds)
            : actionIds(actionIds)
        {}
        explicit Node(ActionKeyConfig::Ptr next)
            : next(next)
        {}
        void appendActionId(ActionId actionId) {
            if (!actionIds.isValid()) {
                actionIds = ActionIds::create();
            }
            actionIds->append(actionId);
        }
        void setNext(ActionKeyConfig::Ptr next) {
            this->next = next;
        }

        ActionIds::Ptr       actionIds;
        ActionKeyConfig::Ptr next;
    };
    
    typedef HashMap<KeyMapping::Id, Node::Ptr> NodeMap;
    
    Node::Ptr find(KeyMapping::Id id)
    {
        Node::Ptr      rslt;
        NodeMap::Value foundNode = nodeMap.get(id);
        if (foundNode.isValid()) {
            rslt = foundNode.get();
        }
        return rslt;    
    }
    
    Node::Ptr find(KeyModifier keyState, KeyId keyId)
    {
        return find(KeyMapping::Id(keyState, keyId));
    }
    
    void append(KeyCombination    keyCombination,
                ActionId          actionId);

  
    KeyCombinations::Ptr getKeyCombinationsForAction(ActionId actionId)
    {
        KeyCombinations::Ptr rslt;
        ActionsToKeysMap::Value foundValue = actionsToKeysMap.get(actionId);
        if (foundValue.isValid()) {
            rslt = foundValue.get();
        }
        return rslt;
    }
    
    ActionIds::Ptr getActionIdsForKeyCombination(KeyCombination keyCombination)
    {
        ActionIds::Ptr rslt;
        KeysToActionsMap::Value foundValue = keysToActionsMap.get(keyCombination);
        if (foundValue.isValid()) {
            rslt = foundValue.get();
        }
        return rslt;
    }
    
private:
    ActionKeyConfig()
    {}
    
    NodeMap nodeMap;
    
    typedef HashMap<ActionId, KeyCombinations::Ptr> ActionsToKeysMap;
    typedef HashMap<KeyCombination, ActionIds::Ptr> KeysToActionsMap;

    ActionsToKeysMap actionsToKeysMap;
    KeysToActionsMap keysToActionsMap;
};

} // namespace LucED

#endif // ACTION_KEY_CONFIG_HPP
