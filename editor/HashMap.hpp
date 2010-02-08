/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#ifndef HASH_MAP_HPP
#define HASH_MAP_HPP

#include <sys/types.h>

#include "config.h"

#if    HAVE_UNORDERED_MAP
#  include <unordered_map>
#elif HAVE_TR1_UNORDERED_MAP && !UNORDERED_MAP_UNDER_STD_TR1_IS_BROKEN
#  include <tr1/unordered_map>
#elif HAVE_EXT_HASH_MAP
#  include <ext/hash_map>
#else
#  error No STL container hash_map or unordered_map found
#endif


#include "String.hpp"
#include "Nullable.hpp"

namespace LucED
{

#if HAVE_UNORDERED_MAP || (HAVE_TR1_UNORDERED_MAP && !UNORDERED_MAP_UNDER_STD_TR1_IS_BROKEN)
  #if UNORDERED_MAP_UNDER_STD
    #define STD_HASH_MAP_TYPE std::unordered_map
  #elif UNORDERED_MAP_UNDER_STD_TR1 && !UNORDERED_MAP_UNDER_STD_TR1_IS_BROKEN
    #define STD_HASH_MAP_TYPE std::tr1::unordered_map
  #else
    #error Needs unordered_map implementation namespace
  #endif
#else
  #if HASH_MAP_UNDER_STD
    #define STD_HASH_MAP_TYPE std::hash_map
  #elif HASH_MAP_UNDER_GNU_CXX
    #define STD_HASH_MAP_TYPE __gnu_cxx::hash_map
  #else
    #error Needs hash_map implementation namespace
  #endif
#endif


template<class T> class HashFunction;

template<> class HashFunction<String>
{
public:
    size_t operator()(const String& key) const {
        size_t rslt = 0;
        for (int i = 0; i < key.getLength(); ++i) {
            rslt = 5 * rslt + key[i];
        }
        return rslt;
    }
};

template<> class HashFunction<long>
{
public:
    size_t operator()(long key) const {
        return (size_t) key;
    }
};

template<> class HashFunction<unsigned long>
{
public:
    size_t operator()(unsigned long key) const {
        return (size_t) key;
    }
};

template<> class HashFunction<pid_t>
{
public:
    size_t operator()(pid_t key) const {
        return (size_t) key;
    }
};

template<> class HashFunction<void*>
{
public:
    size_t operator()(void* key) const {
        return (size_t) key;
    }
};

template<> class HashFunction<const void*>
{
public:
    size_t operator()(const void* key) const {
        return (size_t) key;
    }
};


template
<
    class K, 
    class V, 
    class H = HashFunction<K> 
>
class HashMap
{
public:
    typedef Nullable<V> Value;
 
    class Iterator
    {
    public:
        bool isAtEnd() const {
            return iter == end;
        }
        void gotoNext() {
            if (iter != end) {
                ++iter;
            }
        }
        K getKey() const {
            return iter->first;
        }
        V getValue() const {
            return iter->second;
        }
    private:
        friend class HashMap;
        typedef typename STD_HASH_MAP_TYPE<K,V,H>::const_iterator Iter;
        Iterator(Iter begin, Iter end) : iter(begin), end(end)
        {}
        Iter iter;
        Iter end;
    };
    
    HashMap()
    {}
    
    void set(const K& key, const V& value) {
        map[key] = value;
    }
    Value get(const K& key) const {
        typename STD_HASH_MAP_TYPE<K,V,H>::const_iterator foundPair = map.find(key);
        if (foundPair == map.end()) {
            return Value();
        } else {
            return Value(foundPair->second);
        }
    }
    
    Iterator getIterator() const {
        return Iterator(map.begin(), map.end());
    }
    bool hasKey(const K& key) const {
        typename STD_HASH_MAP_TYPE<K,V,H>::const_iterator foundPair = map.find(key);
        return foundPair != map.end();
    }
    void remove(const K& key) {
        map.erase(key);
    }
    void clear() {
        map.clear();
    }
    bool isEmpty() const {
        return getIterator().isAtEnd();
    }
    bool operator==(const HashMap& rhs) const {
        return map == rhs.map;
    }

protected:
    HashMap(const HashMap& rhs)
        : map(rhs.map)
    {}
    
private:
    STD_HASH_MAP_TYPE<K,V,H> map;
};

#undef STD_HASH_MAP_TYPE

} // namespace LucED

#endif // HASH_MAP_HPP

