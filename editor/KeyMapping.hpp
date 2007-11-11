/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2007 Oliver Schmidt, oliver at luced dot de
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

#ifndef KEYMAPPING_HPP
#define KEYMAPPING_HPP

#include "debug.hpp"
#include "HeapObject.hpp"
#include "GuiRoot.hpp"
#include "HashMap.hpp"
#include "Callback.hpp"
#include "KeyId.hpp"
#include "KeyModifier.hpp"

namespace LucED
{

class KeyMapping : private NonCopyable
{
public:
    
    class Id
    {
    public:
        Id(KeyModifier keyState, KeyId keyId)
            : keyState(keyState), keyId(keyId)
        {}

        Id(String keyState, String keyName)
            : keyState(keyState), keyId(keyName)
        {}
        
        bool operator<(const Id& rhs) const {
            return keyState < rhs.keyState || keyId < rhs.keyId;
        }
        bool operator==(const Id& rhs) const {
            return keyState == rhs.keyState && keyId == rhs.keyId;
        }
        KeyModifier getKeyState() const { return keyState; }
        KeyId       getKeyId() const { return keyId; }

        class HashFunction
        {
        public:
            size_t operator()(const KeyMapping::Id& id) const {
                size_t rslt = id.getKeyState().toHashValue();
                rslt = (rslt << 16) ^ id.getKeyState().toHashValue() ^ (id.getKeyId() << 16) ^ (id.getKeyId());
                return rslt;
            }
        };

    private:
        KeyModifier keyState;
        KeyId keyId;
    };

    Callback<>::Ptr find(KeyModifier keyState, KeyId keyId)        { return find(Id(keyState, keyId)); }

    Callback<>::Ptr find(Id id) {
        Callback<>::Ptr rslt;
        
        MyMap::Value foundCallback = map.get(id);
        if (foundCallback.isValid()) {
            rslt = foundCallback.get();
        }
        return rslt;
    }

    void set(KeyModifier keyState, KeyId keyId, Callback<>::Ptr cb) { set(Id(keyState, keyId), cb); }
    

    void set(Id id, Callback<>::Ptr cb)
    {
        map.set(id, cb);
    }
    

private:

    typedef HashMap<Id, Callback<>::Ptr, Id::HashFunction>  MyMap;


    MyMap map;
};

template<> class HashFunction<KeyMapping::Id>
{
public:
    size_t operator()(const KeyMapping::Id& id) const {
        return KeyMapping::Id::HashFunction()(id);
    }
};

} // namespace LucED

#endif // KEYMAPPING_HPP
