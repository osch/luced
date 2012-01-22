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

#ifndef MUTEX_HPP
#define MUTEX_HPP

#include "config.h"

#if LUCED_USE_PTHREAD
#  include <pthread.h>
#endif

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "SystemException.hpp"
#include "String.hpp"

namespace LucED
{

class Mutex : public HeapObject
{
public:
    typedef OwningPtr<Mutex> Ptr;
    
    static Ptr create() {
        return Ptr(new Mutex());
    }
    
    ~Mutex();

    class Lock
    {
    public:
        explicit Lock(Mutex::Ptr mutex)
        #if LUCED_USE_MULTI_THREAD
            : mutex(mutex)
        #endif
        {
        #if LUCED_USE_MULTI_THREAD
            mutex->lock();
        #endif
        }
        ~Lock() throw()
        {
        #if LUCED_USE_MULTI_THREAD
            mutex->unlock();
        #endif
        }
        void waitForNotify() {
        #if LUCED_USE_MULTI_THREAD
            mutex->wait();
        #endif
        }
        void notify() {
        #if LUCED_USE_MULTI_THREAD
            mutex->notify();
        #endif
        }
        void notifyAll() {
        #if LUCED_USE_MULTI_THREAD
            mutex->notifyAll();
        #endif
        }
    private:
        Lock(const Lock& rhs);
        Lock& operator=(const Lock& rhs);

    #if LUCED_USE_MULTI_THREAD
        Mutex::Ptr mutex;
    #endif
    };
    

private:
    class Impl;

#if LUCED_USE_MULTI_THREAD
    void lock();
    void unlock();
    void wait();
    void notify();
    void notifyAll();
#endif

private:
    Mutex();

#if LUCED_USE_PTHREAD
    pthread_mutexattr_t attr;
    pthread_mutex_t     mutex;
    pthread_cond_t      condition;
#endif
};

} // namespace LucED

#endif // MUTEX_HPP
