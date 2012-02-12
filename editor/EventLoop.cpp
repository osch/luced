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

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "EventLoop.hpp"
#include "util.hpp"
#include "Nullable.hpp"
#include "TimeStamp.hpp"
#include "TimePeriod.hpp"
#include "SystemException.hpp"
#include "String.hpp"

using namespace LucED;

void EventLoop::processStep()
{
    fd_set readfds;   FD_ZERO(&readfds);
    fd_set writefds;  FD_ZERO(&writefds);
    fd_set exceptfds; FD_ZERO(&exceptfds);
    
    int    maxFileDescriptor = -1;
    
    for (int i = 0; i < fileDescriptorListeners.getLength();)
    {
        FileDescriptorListener::Ptr listener = fileDescriptorListeners[i]; 
        if (listener->isWaitingForRead())
        {
            util::maximize(&maxFileDescriptor, listener->getFileDescriptor());
            FD_SET(listener->getFileDescriptor(), &readfds);
            if (listener->isWaitingForWrite()) {
                FD_SET(listener->getFileDescriptor(), &writefds);
            }
            ++i;
        }
        else if (listener->isWaitingForWrite())
        {
            util::maximize(&maxFileDescriptor, listener->getFileDescriptor());
            FD_SET(listener->getFileDescriptor(), &writefds);
            ++i;
        }
    }
    Nullable<TimeStamp> nextEventTime;
    
    
    if (maxFileDescriptor >= 0 || nextEventTime.isValid())
    {
        struct timeval  maxWaitTime;
        struct timeval* maxWaitTimePtr = NULL;
        
        if (nextEventTime.isValid()) {
            maxWaitTime    = (nextEventTime.get() - TimeStamp::now()).toTimeval();
            maxWaitTimePtr = &maxWaitTime;
        }
        
        int selectResult = select(maxFileDescriptor + 1, &readfds, &writefds, &exceptfds, maxWaitTimePtr);
        
        if (selectResult <0) {
            int e = errno;
            throw SystemException(String() << "error (" << e << ") while invoking 'select': " << strerror(e));
        }
        else
        {
            if (selectResult > 0)
            {
                for (int i = 0; i < fileDescriptorListeners.getLength(); ++i)
                {
                    FileDescriptorListener::Ptr listener = fileDescriptorListeners[i];
                    int                         fd       = listener->getFileDescriptor();
                
                    if (FD_ISSET(fd, &readfds)) {
                        listener->handleReading();
                    }
                    if (FD_ISSET(fd, &writefds)) {
                        listener->handleWriting();
                    }
                }
            }
            if (nextEventTime.isValid() && nextEventTime.get() >= TimeStamp::now())
            {
            
            }
        }
    }
}
