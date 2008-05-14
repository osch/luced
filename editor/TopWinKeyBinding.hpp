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

#ifndef TOP_WIN_KEY_BINDING_HPP
#define TOP_WIN_KEY_BINDING_HPP

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "KeyMapping.hpp"
#include "HashMap.hpp"
#include "ActionName.hpp"
#include "TopWinKeyBindingConfig.hpp"
#include "UnknownActionNameException.hpp"

namespace LucED
{

class TopWinKeyBinding : public HeapObject
{
public:
    typedef OwningPtr<TopWinKeyBinding> Ptr;
    
    static Ptr create(TopWinKeyBindingConfig::Ptr config,
                      TopWinActionBinder::Ptr     actionBinder)
    {
        return Ptr(new TopWinKeyBinding(config, actionBinder));
    }
    
    class Node
    {
    public:
        Node()
        {}
        bool isCallable() const {
            return next.isInvalid();
        }
        bool hasNext() const {
            return next.isValid();
        }
        TopWinKeyBinding::Ptr getNext() const {
            ASSERT(hasNext());
            return next;
        }
        void call() const {
            ASSERT(isCallable());
            callback->call();
        }
    private:
        friend class TopWinKeyBinding;
        
        Node(Callback<>::Ptr callback)
            : callback(callback)
        {}
        Node(TopWinKeyBinding::Ptr next)
            : next(next)
        {}
        Callback<>::Ptr callback;
        TopWinKeyBinding::Ptr next;
    };
    
    typedef HashMap<KeyMapping::Id, Node> NodeMap;
    typedef NodeMap::Value                FoundValue;
    
    FoundValue find(KeyModifier keyState, KeyId keyId)
    {
        KeyMapping::Id id(keyState, keyId);
    
        FoundValue rslt = nodeMap.get(id);
    
        if (!rslt.isValid())
        {
            Config::FoundValue foundConfig = config->find(id);
            if (foundConfig.isValid())
            {
                if (foundConfig.get().hasNext())
                {
                    Config::Ptr nextConfig = foundConfig.get().getNext();
                    rslt = Node(TopWinKeyBinding::create(nextConfig,
                                                         actionBinder));
                    nodeMap.set(id, rslt);
                }
                else
                {
                    String className  = foundConfig.get().getActionName().getClassName();
                    String methodName = foundConfig.get().getActionName().getMethodName();
                    
                    Callback<>::Ptr callback = actionBinder->getCallback(className, methodName);
                    if (callback.isValid()) {
                        rslt = Node(callback);
                        nodeMap.set(id, rslt);
                    }
                    else {
                        throw UnknownActionNameException(String() << "Unknown action name: '"
                                                                  << className << "." << methodName
                                                                  << "'");
                    }
                }
            }
        }
        return rslt;
    }

private:
    TopWinKeyBinding(TopWinKeyBindingConfig::Ptr config,
                     TopWinActionBinder::Ptr     actionBinder)
        : config(config),
          actionBinder(actionBinder)
    {}
    
    typedef TopWinKeyBindingConfig Config;

    Config::Ptr                 config;
    TopWinActionBinder::Ptr     actionBinder;

    NodeMap                     nodeMap;
};

} // LucED

#endif // TOP_WIN_KEY_BINDING_HPP
