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

#ifndef TOP_WIN_KEY_BINDING_CONFIG_HPP
#define TOP_WIN_KEY_BINDING_CONFIG_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "KeyMapping.hpp"
#include "String.hpp"
#include "ActionName.hpp"
#include "HashMap.hpp"
#include "ConfigException.hpp"
#include "KeyCombination.hpp"

namespace LucED
{

class TopWinKeyBindingConfig : public HeapObject
{
public:
    typedef OwningPtr<TopWinKeyBindingConfig> Ptr;
    
    static Ptr create() {
        return Ptr(new TopWinKeyBindingConfig());
    }
    
    class Node
    {
    public:
        Node()
        {}
        bool hasNext() const {
            return next.isValid();
        }
        const ActionName& getActionName() const {
            ASSERT(!hasNext());
            return actionName;
        }
        TopWinKeyBindingConfig::Ptr getNext() const {
            ASSERT(hasNext());
            return next;
        }
    private:
        friend class TopWinKeyBindingConfig;
        
        Node(const ActionName& actionName)
            : actionName(actionName)
        {}
        Node(TopWinKeyBindingConfig::Ptr next)
            : next(next)
        {}
        ActionName actionName;
        TopWinKeyBindingConfig::Ptr next;
    };
    
    typedef HashMap<KeyMapping::Id, Node> NodeMap;
    typedef NodeMap::Value                FoundValue;
    
    FoundValue find(KeyMapping::Id id)
    {
        FoundValue rslt = nodeMap.get(id);
    
        return rslt;    
    }
    FoundValue find(KeyModifier keyState, KeyId keyId)
    {
        return find (KeyMapping::Id(keyState, keyId));
    }
    
    void set(KeyCombination    keyCombination,
             const ActionName& actionName)
    {
        if (!keyCombination.hasKeyIds()) {
            throw ConfigException(String() << "Invalid empty key combination");
        }
        
        KeyId keyId = keyCombination.getFirstKeyId();
                      keyCombination.removeFirstKeyId();
                
        if (keyCombination.hasKeyIds())
        {
            KeyMapping::Id id(keyCombination.getKeyModifier(), 
                              keyId);
            TopWinKeyBindingConfig::Ptr next = TopWinKeyBindingConfig::create();
            nodeMap.set(id, next);
            next->set(keyCombination, actionName);
        } else {
            KeyMapping::Id id(keyCombination.getKeyModifier(), 
                              keyId);
            nodeMap.set(id, actionName);
        }
    }
        
private:
    TopWinKeyBindingConfig()
    {}
    
    NodeMap nodeMap;
};

} // namespace LucED

#endif // TOP_WIN_KEY_BINDING_CONFIG_HPP
