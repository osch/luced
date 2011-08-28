/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2011 Oliver Schmidt, oliver at luced dot de
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

#ifndef ARRAY_MAP_HPP
#define ARRAY_MAP_HPP

#include "Nullable.hpp"
#include "ObjectArray.hpp"

namespace LucED
{

template 
<
    class K,
    class V
>
class ArrayMap
{
public:
    typedef Nullable<V> Value;

    ArrayMap()
    {}
    
    int getLength() const {
        return keys.getLength();
    }
    
    Key getKeyByIndex(int i) const {
        return keys[i];
    }
    V getValueByIndex(int i) const {
        return values[i];
    }
    
    int getIndex(const K& key) const {
        for (int i = 0, j = keys.getLength(); i < j; ++i) {
            if (keys[i] == key) {
                return values[i];
            }
        }
        return -1;
    }
    
    Value get(const K& key) const {
        int i = getIndex(key);
        if (i >= 0) {
            return values[i];
        } else {
            return Null;
        }
    }
    
    bool remove(const K& key) const {
        int i = getIndex(key);
        if (i >= 0) {
            keys.remove(i);
            values.remove(i);
            return true;
        } else {
            return false;
        }
    }
    
    void set(const K& key, const V& value) {
        int i = getIndex(key);
        if (i >= 0) {
            values[i] = value;
        } else {
            keys.append(key);
            values.append(value);
        }
    }
    
private:
    ObjectArray<K> keys;
    ObjectArray<V> values;
};

} // namespace LucED

#endif // ARRAY_MAP_HPP
