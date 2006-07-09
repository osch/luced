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

#ifndef HEAPOBJECT_H
#define HEAPOBJECT_H

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <new>
#include <typeinfo>

#include "debug.h"
#include "NonCopyable.h"

//#define PRINT_MALLOCS

namespace LucED {


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
        return allocated + 1;
    }

    // should be private, but gcc doesn't allow this    
    void operator delete(void* ptr, size_t size) {
        ASSERT(true == false);
    }

private:
    friend class HeapObjectRefManipulator;

};

class HeapObjectRefManipulator
{
protected:
    static bool hasOwningReferences(const HeapObject *obj) {
        return (getCounters(obj)->strongCounter >= 1);
    }
    static int obtainInitialOwnership(const HeapObject *obj) {
#ifdef DEBUG
        if (obj != NULL) {
            #ifdef PRINT_MALLOCS
            printf("----> HeapObject %p : initial Ownership\n", obj);
            #endif
            ASSERT(getCounters(obj)->wasNeverOwned);
            ASSERT(getCounters(obj)->strongCounter == 1);
            getCounters(obj)->wasNeverOwned = false;
        }
#endif
    }
    static void incRefCounter(const HeapObject *obj) {
        if (obj != NULL) {
            getCounters(obj)->strongCounter += 1;
        }
    }
    static void decRefCounter(const HeapObject *obj) {
        if (obj != NULL) {
            HeapObjectCounters* counters = getCounters(obj);
            ASSERT(counters->strongCounter >= 1);
            if (counters->strongCounter == 1) {
                // keep strongCounter == 1 while destructing
                obj->~HeapObject();
                ASSERT(counters->weakCounter >= 0);
                if (counters->weakCounter == 0) {
                    counters->clear();
                    #ifdef PRINT_MALLOCS
                    printf("----> HeapObject %p : deleting\n", obj);
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
    
    static bool hasWeakReferences(const HeapObject *obj) {
        return (getCounters(obj)->weakCounter >= 1);
    }
    static void incWeakCounter(const HeapObject *obj) {
        if (obj != NULL) {
            getCounters(obj)->weakCounter += 1;
        }
    }
    static void decWeakCounter(const HeapObject *obj) {
        if (obj != NULL) {
            ASSERT(hasWeakReferences(obj));
            getCounters(obj)->weakCounter -= 1;
            if (!hasWeakReferences(obj) && !hasOwningReferences(obj)) {
                #ifdef PRINT_MALLOCS
                printf("----> HeapObject %p : deleting\n", obj);
                #endif
                HeapObjectCounters* counters = getCounters(obj);
                counters->clear();
                free(counters);
            }
        }
    }

private:
    static HeapObjectCounters* getCounters(const void* heapObject) {
        HeapObjectCounters* rslt = const_cast<HeapObjectCounters*>(
            static_cast<const HeapObjectCounters*>(heapObject) - 1
        );
#ifdef DEBUG
        ASSERT(rslt->magic == HeapObjectCounters::MAGIC);
#endif        
        return rslt;
    }
};


} // namespace LucED

#endif // HEAPOBJECT_H
