/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2012 Oliver Schmidt, oliver at luced dot de
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

#include <stdio.h>

#include "debug.hpp"
#include "Thread.hpp"
#include "EventDispatcher.hpp"
#include "BaseException.hpp"
#include "SystemException.hpp"

using namespace LucED;

#if LUCED_USE_MULTI_THREAD

class Thread::Impl
{
public:
    static void* startedThreadMainFunction(void* userData)
    {
        Thread* thread = (Thread*) userData;

        try {
            thread->main();
        }
        catch (BaseException& ex) {
            thread->errorMessage = ex.toString();
            fprintf(stderr, "Error in thread: %s\n", ex.toString().toCString());
        }
        thread->runningFlag = false;
        EventDispatcher::getInstance()->executeTaskOnMainThread(newCallback(thread, &Thread::handleThreadTermination));
        return NULL;
    }
};

void Thread::handleThreadTermination()
{
    EventDispatcher::getInstance()->deregisterRunningComponent(this);
}


void Thread::start(Thread::Ptr thread)
{
    ASSERT(!thread->runningFlag);
    
    thread->runningFlag  = true;
    thread->finishedFlag = false;

    EventDispatcher::getInstance()->registerRunningComponent(thread);

    int rc = pthread_create(&thread->threadHandle, NULL, Impl::startedThreadMainFunction, thread);

    if (rc != 0) {
        thread->runningFlag  = false;
        thread->finishedFlag = true;
        throw SystemException(String() << "Error in pthread_create (rc=" << rc << ")");
    }
}

void Thread::waitForFinished()
{
    if (!finishedFlag)
    {
        void* rsltPtr;
        int rc = pthread_join(threadHandle, &rsltPtr);
    
        if (rc != 0) {
            throw SystemException(String() << "Error in pthread_join (rc=" << rc << ")");
        }
        finishedFlag = true;
    }

}

Thread::~Thread()
{
    waitForFinished();
}

#endif // LUCED_USE_MULTI_THREAD
