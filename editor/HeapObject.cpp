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

#include "HeapObject.hpp"
#include "String.hpp"
#include "StackTrace.hpp"

using namespace LucED;

#undef TRACE_HEAP_OBJECT_STACK_TRACES
//#define TRACE_HEAP_OBJECT_STACK_TRACES 1

#ifdef DEBUG

int HeapObjectChecker::initCounter = 0;
int HeapObjectChecker::allocCounter = 0;
int HeapObjectChecker::destructCounter = 0;

void HeapObjectChecker::assertAllCleared()
{
#ifdef PRINT_MALLOCS
    printf("------------------------>  init: %d, alloc: %d, destruct: %d\n", initCounter, allocCounter, destructCounter);
#else
    if (   initCounter != allocCounter
        || initCounter != destructCounter)
    {
printf("initCounter=%d, allocCounter=%d, destructCounter=%d\n",
        initCounter, allocCounter, destructCounter);
        HeapObject::printAllStackTraces();
    }
    ASSERT(initCounter == allocCounter);
    ASSERT(initCounter == destructCounter);
#endif
}

HeapObjectBase* HeapObjectBase::first = NULL;

HeapObjectBase::HeapObjectBase()
{
#ifdef TRACE_HEAP_OBJECT_STACK_TRACES
    stackTrace = StackTrace::getCurrent();
    prev = NULL;
    next = first;
    first = this;
    if (next != NULL) {
        next->prev = this;
    }
#endif
}

HeapObjectBase::~HeapObjectBase()
{
#ifdef TRACE_HEAP_OBJECT_STACK_TRACES
    HeapObjectBase** pprevnext;
    
    if (prev == NULL) {
        pprevnext = &first;
    } else {
        pprevnext = &(prev->next);
    }
    ASSERT(*pprevnext == this);
    
    *pprevnext = next;
    
    if (next != NULL) {
        next->prev = prev;
    }
#endif
}


class XXX : private HeapObjectRefManipulator
{
public:
    static HeapObjectCounters* getHeapObjectCounters(const HeapObjectBase* heapObject)
    {
        return HeapObjectRefManipulator::getHeapObjectCounters(heapObject);
    }
};

void HeapObjectBase::printAllStackTraces()
{
#ifdef TRACE_HEAP_OBJECT_STACK_TRACES
    printf("printAllStackTraces START\n");

    HeapObjectBase* p = first;
    int counter = 0;

    if (p != NULL)
    {
        while (p->next != NULL)
        {
            p = p->next;
        }
        while (p != NULL)
        {
            if (XXX::getHeapObjectCounters(p)->getWasNeverOwnedFlag()) {
                counter += 1;
                printf("HeapObjectConstructor: %p\n%s\n", p, p->stackTrace.c_str());
            }
            p = p->prev;
        }
    }
    printf("counter=%d\n", counter);
    printf("printAllStackTraces END\n");
#endif
}



#endif // DEBUG

