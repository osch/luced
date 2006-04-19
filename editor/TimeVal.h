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

#ifndef TIMEVAL_H
#define TIMEVAL_H

#include <sys/time.h>
#include <sys/select.h>

#include "debug.h"

namespace LucED {

class TimeVal
{
public:

    friend int select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, TimeVal* timeVal)
    {
        return select(n, readfds, writefds, exceptfds, &timeVal->timeval);
    }
    
    /**
     * Time t2 should be later than t1.
     */
    static long diffMicroSecs(const TimeVal& t1, const TimeVal& t2)
    {
        ASSERT(0 <= t1.timeval.tv_usec && t1.timeval.tv_usec < 1000000);
        ASSERT(0 <= t2.timeval.tv_usec && t2.timeval.tv_usec < 1000000);
        ASSERT(t2.isLaterThan(t1));
        
        if (t1.timeval.tv_sec == t2.timeval.tv_sec) {
            return t2.timeval.tv_usec - t1.timeval.tv_usec;
        } else {
            return (t2.timeval.tv_sec - 1 - t1.timeval.tv_sec) * 1000000
                    + (t2.timeval.tv_usec + (1000000 - t1.timeval.tv_usec));
        }
    }
    
    TimeVal() {
        timeval.tv_sec = 0;
        timeval.tv_usec = 0;
    }
    
    long getMicroSecsBefore(const TimeVal &t) const
    {
        return diffMicroSecs(*this, t);
    }
    long getMicroSecsAfter(const TimeVal &t) const
    {
        return diffMicroSecs(t, *this);
    }
    
    bool isLaterThan(const TimeVal &t) const
    {
        if (timeval.tv_sec > t.timeval.tv_sec) {
            return true;
        }
        if (timeval.tv_sec == t.timeval.tv_sec) {
            return timeval.tv_usec >= t.timeval.tv_usec;
        }
        return false;
    }
    
    bool isZero() const
    {
        return timeval.tv_sec == 0 && timeval.tv_usec == 0;
    }
    
    TimeVal& addMicroSecs(long usec)
    {
        long diff;
        long sec;

        ASSERT(0 <= timeval.tv_usec && timeval.tv_usec < 1000000);

        if (usec >= 1000000) {
            sec  = usec / 1000000;
            usec = usec - sec * 1000000;
        } else {
            sec = 0;
        }
        diff = 1000000 - timeval.tv_usec;
        if (diff <= usec) {
            timeval.tv_sec  += 1 + sec;
            timeval.tv_usec  = usec - diff;
        } else {
            timeval.tv_sec  += sec;
            timeval.tv_usec += usec;
        }
        return *this;
    }

    TimeVal& addSecs(long sec, long usec = 0)
    {
        addMicroSecs(usec);
        timeval.tv_sec += sec;
        return *this;
    }
    
    TimeVal& setToCurrentTime()
    {
        gettimeofday(&timeval, NULL);
        return *this;
    }
    
    TimeVal& setToRemainingTimeUntil(const TimeVal &t)
    {
        ASSERT(0 <= t.timeval.tv_usec && t.timeval.tv_usec < 1000000);

        TimeVal current;
        current.setToCurrentTime();

        ASSERT(0 <= current.timeval.tv_usec && current.timeval.tv_usec < 1000000);

        if (current.isLaterThan(t)) {
            timeval.tv_sec = 0;
            timeval.tv_usec = 0;
        } else {
            if (current.timeval.tv_sec == t.timeval.tv_sec) {
                timeval.tv_sec = 0;
                timeval.tv_usec = t.timeval.tv_usec - current.timeval.tv_usec;
            } else {
                timeval.tv_usec = 1000000 - current.timeval.tv_usec + t.timeval.tv_usec;
                if (timeval.tv_usec >= 1000000) {
                    timeval.tv_usec -= 1000000;
                    timeval.tv_sec   = 1;
                } else {
                    timeval.tv_sec   = 0;
                }
                timeval.tv_sec += t.timeval.tv_sec - current.timeval.tv_sec - 1;
            }
        }
        return *this;
    }

public:
    struct timeval timeval;
};


} // namespace LucED

#endif // TIMEVAL_H

