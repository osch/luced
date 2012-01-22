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

#ifndef THREAD_HPP
#define THREAD_HPP

#include "config.h"

#if LUCED_USE_PTHREAD
#  include <pthread.h>
#endif

#include "RunningComponent.hpp"
#include "Nullable.hpp"
#include "String.hpp"


namespace LucED
{

#if LUCED_USE_MULTI_THREAD
class Thread : public RunningComponent
{
public:
    typedef LucED::OwningPtr<Thread> Ptr;
    typedef LucED::WeakPtr  <Thread> WeakPtr;
    
    ~Thread();

    static void start(Thread::Ptr thread);
    
    bool hasError() const {
        return errorMessage.isValid();
    }
    String getError() const {
        return errorMessage.get();
    }
    bool isRunning() const {
        return runningFlag;
    }
    
    void waitForFinished();
    
    
protected:
    Thread()
        : runningFlag(false),
          finishedFlag(true)
    {}
    
    void handleThreadTermination();
    
    class Impl;

    virtual void main() = 0;
    
    Nullable<String> errorMessage;

#ifdef LUCED_USE_PTHREAD
    bool             runningFlag;
    bool             finishedFlag;
    pthread_t        threadHandle;
#endif
};
#endif // LUCED_USE_MULTI_THREAD

} // namespace LucED

#endif // THREAD_HPP
