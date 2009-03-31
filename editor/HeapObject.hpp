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

#include "RawPtrGuardHolder.hpp"

#ifndef HEAP_OBJECT_HPP
#define HEAP_OBJECT_HPP

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <new>
#include <typeinfo>

#include "debug.hpp"
#include "NonCopyable.hpp"
#include "String.hpp"
#include "RawPtrGuarded.hpp"

#undef HEAP_OBJECT_USES_DYNAMIC_CAST
//#define PRINT_MALLOCS

namespace LucED
{

class HeapObject;
class RawPtrGuard;
      
#ifdef DEBUG
class HeapObjectChecker : private NonCopyable
{
public:
    static void assertAllCleared();
private:
    friend class HeapObjectBase;
    friend class HeapObjectRefManipulator;
    HeapObjectChecker();
    
    static int initCounter;
    static int allocCounter;
    static int destructCounter;
    // no freeCounter, because some WeakPtrs are remaining until program termination
};
#endif // DEBUG

class HeapObjectCounters : private NonCopyable
{
private:
    friend class HeapObjectBase;
    friend class HeapObjectRefManipulator;
    HeapObjectCounters() : wasNeverOwned(true),
                           weakCounter(0), 
                           strongCounter(1)
#ifdef DEBUG
        , magic(MAGIC)
#endif
    {}
    void clear() {
#ifdef DEBUG
        weakCounter = 0;
        strongCounter = 0;
        magic = -1;
#endif
        wasNeverOwned = false;
    }
    bool wasNeverOwned;
    int weakCounter;
    int strongCounter;
#ifdef DEBUG
    enum {MAGIC = 12345678};
    int magic;
#endif
public:
    bool hasWeakReferences() const {
        return weakCounter >= 1;
    }
    bool hasOwningReferences() const {
        return strongCounter >= 1;
    }
    int getWeakCounter() const {
        return weakCounter;
    }
    int getOwningCounter() const {
        return strongCounter;
    }
};


class HeapObjectBase : private NonCopyable
{
private:
    friend class HeapObject;
#ifdef DEBUG
    friend class RawPtrGuard;
#endif
    HeapObjectBase()
    {}
    
public:

    /**
     * Virtual Destructor.
     */
    virtual ~HeapObjectBase() {}

protected:
    
    void* operator new(size_t size) {
        HeapObjectCounters* allocated = static_cast<HeapObjectCounters*>(
                malloc(sizeof(HeapObjectCounters) + size));
        new(allocated) HeapObjectCounters();
    #ifdef PRINT_MALLOCS
        printf("----> HeapObjectBase %p : allocating %8.d bytes \n", allocated + 1, size);
        printf("***** HeapObjects allocated: %d \n", HeapObjectChecker::allocCounter - HeapObjectChecker::destructCounter);
    #endif
    #ifdef DEBUG
        HeapObjectChecker::allocCounter += 1;
    #endif
        return allocated + 1;
    }

    void operator delete(void* ptr, size_t size)
    {
        HeapObjectCounters* counters = const_cast<HeapObjectCounters*>(
            static_cast<const HeapObjectCounters*>(ptr) - 1
        );
        if (counters->wasNeverOwned && counters->weakCounter > 0)
        {
            // Object is destructed before adopted by OwningPtr,
            // but there are WeakPtrs already referencing ->
            // so pretend that object is desctruced, but leave
            // storage for WeakPtrs.

            ASSERT(counters->strongCounter == 1);
        #ifdef DEBUG
            memset(ptr, 'X', size);
        #endif
            counters->strongCounter = 0;
        }
        else
        {
        #ifdef DEBUG
            memset(counters, 'X', size + sizeof(HeapObjectCounters));
        #endif
            free(counters);
        }
    #ifdef DEBUG
        HeapObjectChecker::allocCounter -= 1;
    #endif
    }

private:
    friend class HeapObjectRefManipulator;

};

class HeapObjectRefManipulator
{
protected:
    template<class T
            >
    friend class OwningPtr;
    template<class T
            >
    friend class WeakPtr;
    friend class RawPtrGuardHolder;

    static void obtainInitialOwnership(const HeapObjectBase* obj) {
        if (obj != NULL) {
    #ifdef DEBUG
        #ifdef PRINT_MALLOCS
            printf("----> HeapObjectBase %p : initial Ownership\n", obj);
            printf("----- New %s at %p\n", typeid(*obj).name(), dynamic_cast<const void*>(obj));
        #endif
            HeapObjectChecker::initCounter += 1;
    #endif
            HeapObjectCounters* heapObjectCounters = getHeapObjectCounters(obj);
            ASSERT(heapObjectCounters->wasNeverOwned);
            ASSERT(heapObjectCounters->strongCounter >= 1); // normal == 1, >= 1 after resetInitialOwnership

            heapObjectCounters->wasNeverOwned = false;
        }
    }
    static void resetInitialOwnership(const HeapObjectBase* obj) {
        if (obj != NULL) {
            HeapObjectCounters* heapObjectCounters = getHeapObjectCounters(obj);
            heapObjectCounters->wasNeverOwned = true;
#ifdef DEBUG
            HeapObjectChecker::initCounter -= 1;
#endif
        }
    }

    static void incRefCounter(const HeapObjectBase* obj) {
        if (obj != NULL) {
            incRefCounter(getHeapObjectCounters(obj));
        }
    }
    static void incRefCounter(HeapObjectCounters* counters) {
        if (counters != NULL) {
            counters->strongCounter += 1;
        }
    }

    static void decRefCounter(const HeapObjectBase* obj) {
        if (obj != NULL) {
            decRefCounter(getHeapObjectCounters(obj));
        }
    }
    static void decRefCounter(HeapObjectCounters* counters) {
        if (counters != NULL) {
            ASSERT(counters->strongCounter >= 1);
            if (counters->strongCounter == 1) {
                // keep strongCounter == 1 while destructing
                void* rawPtr = counters + 1;
                static_cast<HeapObjectBase*>(rawPtr)->~HeapObjectBase();
                #ifdef DEBUG
                HeapObjectChecker::destructCounter += 1;
                #endif
                ASSERT(counters->weakCounter >= 0);
                if (counters->weakCounter == 0) {
                    counters->clear();
                    #ifdef PRINT_MALLOCS
                    printf("----> HeapObjectBase %p : deleting\n", counters + 1);
                    #endif
                    free(counters);
                } else {
                    counters->strongCounter -= 1;
                }
            } else {
                counters->strongCounter -= 1;
            }
        }
    }
    
    static void incWeakCounter(const HeapObjectBase* obj) {
        if (obj != NULL) {
            incWeakCounter(getHeapObjectCounters(obj));
        }
    }
    static void incWeakCounter(HeapObjectCounters* counters) {
        if (counters != NULL) {
            counters->weakCounter += 1;
        }
    }

    static void decWeakCounter(const HeapObjectBase* obj) {
        if (obj != NULL) {
            decWeakCounter(getHeapObjectCounters(obj));
        }
    }
    static void decWeakCounter(HeapObjectCounters* counters) {
        if (counters != NULL) {
            ASSERT(counters->hasWeakReferences());
            counters->weakCounter -= 1;
            if (!counters->hasWeakReferences() && !counters->hasOwningReferences()) {
                #ifdef PRINT_MALLOCS
                printf("----> HeapObjectBase %p : deleting\n", counters + 1);
                #endif
                counters->clear();
                free(counters);
            }
        }
    }

    static HeapObjectCounters* getHeapObjectCounters(const HeapObjectBase* heapObject)
    {
        if (heapObject != NULL)
        {
#ifdef HEAP_OBJECT_USES_DYNAMIC_CAST
            const void* rawAddress = dynamic_cast<const void*>(heapObject);
#else
            const void* rawAddress = static_cast<const void*>(heapObject);
#endif
            HeapObjectCounters* rslt = const_cast<HeapObjectCounters*>(
                static_cast<const HeapObjectCounters*>(rawAddress) - 1
            );
#ifdef DEBUG
            ASSERT(rslt->magic == HeapObjectCounters::MAGIC);
#endif        
            return rslt;
        } else {
            return NULL;
        }
    }
};


class HeapObject : public HeapObjectBase,
                   public RawPtrGuarded
{
protected:
    HeapObject()
    {}
};

} // namespace LucED

#endif // HEAP_OBJECT_HPP
