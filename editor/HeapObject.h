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

#ifndef HEAPOBJECT_H
#define HEAPOBJECT_H

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include <typeinfo>

#include "debug.h"
#include "NonCopyable.h"

// #define PRINT_MALLOCS

namespace LucED {

#ifdef DEBUG
class HeapObjectChecker : NonCopyable
{
public:
    static void assertAllCleared();
private:
    friend class HeapObject;
    friend class HeapObjectRefManipulator;
    HeapObjectChecker();
    
    static int initCounter;
    static int allocCounter;
    static int destructCounter;
    // no freeCounter, because some WeakPtrs are remaining until program termination
};
#endif // DEBUG

class HeapObjectCounters : NonCopyable
{
private:
    friend class HeapObject;
    friend class HeapObjectRefManipulator;
    HeapObjectCounters() : weakCounter(0), strongCounter(1) 
#ifdef DEBUG
        , wasNeverOwned(true)
        , magic(MAGIC)
#endif
    {}
    void clear() {
        weakCounter = 0;
        strongCounter = 0;
#ifdef DEBUG
        magic = -1;
        wasNeverOwned = false;
#endif
    }
#ifdef DEBUG
    enum {MAGIC = 12345678};
    int magic;
    bool wasNeverOwned;
#endif
    int weakCounter;
    int strongCounter;
public:
    bool hasWeakReferences() const {
        return weakCounter >= 1;
    }
    bool hasOwningReferences() const {
        return strongCounter >= 1;
    }
};

class HeapObject : NonCopyable
{
protected:

    HeapObject() {
    }
    
public:

    /**
     * Virtual Destructor.
     */
    virtual ~HeapObject() {}

protected:
    
    void* operator new(size_t size) {
        HeapObjectCounters *allocated = static_cast<HeapObjectCounters *>(
                malloc(sizeof(HeapObjectCounters) + size));
        new(allocated) HeapObjectCounters();
    #ifdef PRINT_MALLOCS
        printf("----> HeapObject %p : allocating %8.d bytes \n", allocated + 1, size);
    #endif
    #ifdef DEBUG
        HeapObjectChecker::allocCounter += 1;
    #endif
        return allocated + 1;
    }

    void operator delete(void* ptr, size_t size) {
        HeapObjectCounters* beginPtr = const_cast<HeapObjectCounters*>(
            static_cast<const HeapObjectCounters*>(ptr) - 1
        );
    #ifdef DEBUG
        HeapObjectChecker::allocCounter -= 1;
        memset(beginPtr, 'X', size + sizeof(HeapObjectCounters));
    #endif
        free(beginPtr);
    }

private:
    friend class HeapObjectRefManipulator;

};

class HeapObjectRefManipulator
{
protected:
    static int obtainInitialOwnership(const HeapObject *obj) {
#ifdef DEBUG
        if (obj != NULL) {
            #ifdef PRINT_MALLOCS
            printf("----> HeapObject %p : initial Ownership\n", obj);
            printf("----- New %s at %p\n", typeid(*obj).name(), dynamic_cast<const void*>(obj));
            #endif
            #ifdef DEBUG
            HeapObjectChecker::initCounter += 1;
            #endif
            HeapObjectCounters* heapObjectCounters = getHeapObjectCounters(obj);
            ASSERT(heapObjectCounters->wasNeverOwned);
            ASSERT(heapObjectCounters->strongCounter == 1);
            heapObjectCounters->wasNeverOwned = false;
        }
#endif
    }

    static void incRefCounter(const HeapObject *obj) {
        if (obj != NULL) {
            incRefCounter(getHeapObjectCounters(obj));
        }
    }
    static void incRefCounter(HeapObjectCounters *counters) {
        if (counters != NULL) {
            counters->strongCounter += 1;
        }
    }

    static void decRefCounter(const HeapObject *obj) {
        if (obj != NULL) {
            decRefCounter(getHeapObjectCounters(obj));
        }
    }
    static void decRefCounter(HeapObjectCounters *counters) {
        if (counters != NULL) {
            ASSERT(counters->strongCounter >= 1);
            if (counters->strongCounter == 1) {
                // keep strongCounter == 1 while destructing
                void* rawPtr = counters + 1;
                static_cast<HeapObject*>(rawPtr)->~HeapObject();
                #ifdef DEBUG
                HeapObjectChecker::destructCounter += 1;
                #endif
                ASSERT(counters->weakCounter >= 0);
                if (counters->weakCounter == 0) {
                    counters->clear();
                    #ifdef PRINT_MALLOCS
                    printf("----> HeapObject %p : deleting\n", counters + 1);
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
    
    static void incWeakCounter(const HeapObject *obj) {
        if (obj != NULL) {
            incWeakCounter(getHeapObjectCounters(obj));
        }
    }
    static void incWeakCounter(HeapObjectCounters *counters) {
        if (counters != NULL) {
            counters->weakCounter += 1;
        }
    }

    static void decWeakCounter(const HeapObject *obj) {
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
                printf("----> HeapObject %p : deleting\n", counters + 1);
                #endif
                counters->clear();
                free(counters);
            }
        }
    }

    static HeapObjectCounters* getHeapObjectCounters(const HeapObject* heapObject)
    {
        if (heapObject != NULL)
        {
            const void* rawAddress = dynamic_cast<const void*>(heapObject);
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


} // namespace LucED

#endif // HEAPOBJECT_H
