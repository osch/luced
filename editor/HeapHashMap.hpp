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

#ifndef HEAP_HASH_MAP_HPP
#define HEAP_HASH_MAP_HPP

#include "HeapObject.hpp"
#include "HashMap.hpp"
#include "OwningPtr.hpp"

namespace LucED {

template<class K, class V, class H = HashFunction<K> > class HeapHashMap
        : public HeapObject, public HashMap<K,V,H>
{
public:
    typedef OwningPtr<HeapHashMap> Ptr;
    typedef OwningPtr<const HeapHashMap> ConstPtr;
    
    static Ptr create() {
        return Ptr(new HeapHashMap());
    }

    OwningPtr<HeapHashMap> clone() const {
        return Ptr(new HeapHashMap(*this));
    }
    
    bool equals(RawPtr<const HeapHashMap> rhs) const {
        return *this == *rhs;
    }
    
protected:
    HeapHashMap()
    {}
    
    HeapHashMap(const HeapHashMap& rhs)
        : HashMap<K,V,H>(rhs)
    {}
};

} // namespace LucED

#endif // HEAP_HASH_MAP_HPP
