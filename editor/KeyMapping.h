/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch@luced.de
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

namespace LucED {

template<class T> class KeyMapping : private NonCopyable
{
public:
    typedef void (T::*MethodPtr)();
    
    KeyMapping(T* objectPtr) : objectPtr(objectPtr) {}
    
    struct Id
    {
        Id(int keyState, KeySym keySym) :
            keyState(keyState), keySym(keySym)
        {}
        
        bool operator<(const Id& rhs) const {
            return keyState < rhs.keyState || keySym < rhs.keySym;
        }
        bool operator==(const Id& rhs) const {
            return keyState == rhs.keyState && keySym == rhs.keySym;
        }
        int keyState;
        KeySym keySym;
    };
    
private:
    class HashFunction
    {
    public:
        size_t operator()(const Id& id) const {
            size_t rslt = id.keyState;
            rslt = (rslt << 16) ^ id.keyState ^ (id.keySym << 16) ^ (id.keySym);
            return rslt;
        }
    };
    typedef HashMap<Id, MethodPtr, HashFunction>  MyMap;

public:

    MethodPtr find(Id id) {
        MethodPtr rslt = NULL;
        
        typename MyMap::Value foundMethod = map.get(id);
        if (foundMethod.isValid()) {
            rslt = foundMethod.get();
        }
        return rslt;
    }

    MethodPtr find(int keyState, KeySym keySym) {
        return find(Id(keyState, keySym));
    }
    
    void add(Id id, MethodPtr m) {
        map.set(id, m);
    }
    
    void add(int keyState, KeySym keySym, MethodPtr m) {
        add(Id(keyState, keySym), m);
    }
    
private:
    T* objectPtr;
    MyMap map;
};

} // namespace LucED

#endif // KEYMAPPING_H
