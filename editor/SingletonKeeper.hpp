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

#ifndef SINGLETONKEEPER_H
#define SINGLETONKEEPER_H

#include "debug.hpp"
#include "HeapObject.hpp"
#include "ObjectArray.hpp"
#include "OwningPtr.hpp"

namespace LucED {

class SingletonKeeper : public HeapObject
{
public:
    typedef OwningPtr<SingletonKeeper>     Ptr;
    
    static Ptr              create();
    static SingletonKeeper* getInstance();
    
    template<class T> WeakPtr<T> add(OwningPtr<T> singletonPtr) {
        singletons.append(singletonPtr);
        return singletonPtr;
    }
    
    long getGenerationCounter() const {
        return generationCounter;
    }
    
private:
    SingletonKeeper() {}

    ObjectArray< OwningPtr<HeapObject> > singletons;
    
    static WeakPtr<SingletonKeeper> instance;
    static long generationCounter;
};


} // namespace LucED

#endif // SINGLETONKEEPER_H
