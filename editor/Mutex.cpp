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

#include "Mutex.hpp"
#include "SystemException.hpp"

using namespace LucED;


Mutex::Mutex()
{
#if LUCED_USE_MULTI_THREAD
    {
        int rc = pthread_mutexattr_init(&attr);

        if (rc != 0) {
            throw SystemException(String() << "Error in pthread_mutexattr_init (rc=" << rc << ")");
        }
    }
    {
        int rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

        if (rc != 0) {
            throw SystemException(String() << "Error in pthread_mutexattr_settype (rc=" << rc << ")");
        }
    }
    {
        int rc = pthread_mutex_init(&mutex, &attr);
        if (rc != 0) {
            throw SystemException(String() << "Error in pthread_mutex_init (rc=" << rc << ")");
        }
    }
    {
        int rc = pthread_cond_init(&condition, NULL);
        if (rc != 0) {
            throw SystemException(String() << "Error in pthread_cond_init (rc=" << rc << ")");
        }
    }
#endif
}



Mutex::~Mutex()
{
#if LUCED_USE_MULTI_THREAD
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&attr);
#endif
}



#if LUCED_USE_MULTI_THREAD

void Mutex::lock()
{
    int rc = pthread_mutex_lock(&mutex);

    if (rc != 0) {
        throw SystemException(String() << "Error in pthread_mutex_lock (rc=" << rc << ")");
    }
}

void Mutex::unlock()
{
    int rc = pthread_mutex_unlock(&mutex);

    if (rc != 0) {
        throw SystemException(String() << "Error in pthread_mutex_unlock (rc=" << rc << ")");
    }
}

void Mutex::wait()
{
    int rc = pthread_cond_wait(&condition, &mutex);
    
    if (rc != 0) {
        throw SystemException(String() << "Error in pthread_cond_wait (rc=" << rc << ")");
    }
}

void Mutex::notify()
{
    int rc = pthread_cond_signal(&condition);
    
    if (rc != 0) {
        throw SystemException(String() << "Error in pthread_cond_signal (rc=" << rc << ")");
    }
}

void Mutex::notifyAll()
{
    int rc = pthread_cond_broadcast(&condition);
    
    if (rc != 0) {
        throw SystemException(String() << "Error in pthread_cond_broadcast (rc=" << rc << ")");
    }
}

#endif // LUCED_USE_MULTI_THREAD
