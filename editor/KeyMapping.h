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
    typedef void (*FunctionPtr)(T*);
    
    FunctionPtr find(int keyState, KeySym keySym)        { return find(Id(keyState, keySym)); }

    void set(int keyState, KeySym keySym, FunctionPtr m) { set(Id(keyState, keySym), m); }
    
private:

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

    class HashFunction
    {
    public:
        size_t operator()(const Id& id) const {
            size_t rslt = id.keyState;
            rslt = (rslt << 16) ^ id.keyState ^ (id.keySym << 16) ^ (id.keySym);
            return rslt;
        }
    };
    typedef HashMap<Id, FunctionPtr, HashFunction>  MyMap;


    void set(Id id, FunctionPtr m)
    {
        map.set(id, m);
    }
    

    FunctionPtr find(Id id)
    {
        FunctionPtr rslt = NULL;
        
        typename MyMap::Value foundMethod = map.get(id);
        if (foundMethod.isValid()) {
            rslt = foundMethod.get();
        }
        return rslt;
    }

    MyMap map;
};

} // namespace LucED

#endif // KEYMAPPING_H
