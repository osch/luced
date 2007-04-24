/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, oliver at luced dot de
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

#ifndef KEYMAPPING_H
#define KEYMAPPING_H

#include "debug.h"
#include "HeapObject.h"
#include "GuiRoot.h"
#include "HashMap.h"
#include "Callback.h"

namespace LucED {


class KeyMapping : private NonCopyable
{
public:
    
    class Id
    {
    public:
        Id(int keyState, KeySym keySym) :
            keyState(keyState), keySym(keySym)
        {}

        Id(int keyState, String keySym) :
            keyState(keyState), keySym(XStringToKeysym(keySym.toCString()))
        {}
        
        bool operator<(const Id& rhs) const {
            return keyState < rhs.keyState || keySym < rhs.keySym;
        }
        bool operator==(const Id& rhs) const {
            return keyState == rhs.keyState && keySym == rhs.keySym;
        }
        int getKeyState() const { return keyState; }
        KeySym getKeySym() const { return keySym; }

        class HashFunction
        {
        public:
            size_t operator()(const KeyMapping::Id& id) const {
                size_t rslt = id.getKeyState();
                rslt = (rslt << 16) ^ id.getKeyState() ^ (id.getKeySym() << 16) ^ (id.getKeySym());
                return rslt;
            }
        };

    private:
        int keyState;
        KeySym keySym;
    };

    Callback0 find(int keyState, KeySym keySym)        { return find(Id(keyState, keySym)); }

    Callback0 find(Id id) {
        Callback0 rslt;
        
        MyMap::Value foundCallback = map.get(id);
        if (foundCallback.isValid()) {
            rslt = foundCallback.get();
        }
        return rslt;
    }

    void set(int keyState, KeySym keySym, const Callback0& cb) { set(Id(keyState, keySym), cb); }
    

    void set(Id id, const Callback0& cb)
    {
        map.set(id, cb);
    }
    

private:

    typedef HashMap<Id, Callback0, Id::HashFunction>  MyMap;


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

#endif // KEYMAPPING_H
