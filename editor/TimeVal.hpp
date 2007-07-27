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

#ifndef TIMEVAL_HPP
#define TIMEVAL_HPP

#include <sys/time.h>
#include <sys/select.h>

#include "debug.hpp"
#include "Seconds.hpp"
#include "MicroSeconds.hpp"

namespace LucED {

class TimeVal
{
public:

    friend int select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, TimeVal* timeVal);
    
    /**
     * Time t2 should be later than t1.
     */
    static MicroSeconds diffMicroSecs(const TimeVal& t1, const TimeVal& t2)
    {
        ASSERT(0 <= t1.timeval.tv_usec && t1.timeval.tv_usec < 1000000);
        ASSERT(0 <= t2.timeval.tv_usec && t2.timeval.tv_usec < 1000000);
        ASSERT(t2.isLaterOrEqualThan(t1));
        
        if (t1.timeval.tv_sec == t2.timeval.tv_sec) {
            return MicroSeconds(t2.timeval.tv_usec - t1.timeval.tv_usec);
        } else {
            return MicroSeconds((t2.timeval.tv_sec - 1 - t1.timeval.tv_sec) * 1000000
                              + (t2.timeval.tv_usec + (1000000 - t1.timeval.tv_usec)));
        }
    }
    
    TimeVal() {
        timeval.tv_sec = 0;
        timeval.tv_usec = 0;
    }
    
    TimeVal(Seconds seconds, MicroSeconds microSeconds) {
        timeval.tv_sec = seconds;
        timeval.tv_usec = microSeconds;
    }
    
    Seconds getSeconds() const {
        return Seconds(timeval.tv_sec);
    }
    MicroSeconds getMicroSeconds() const {
        return MicroSeconds(timeval.tv_usec);
    }
    
    MicroSeconds getMicroSecsBefore(const TimeVal &t) const
    {
        return diffMicroSecs(*this, t);
    }
    MicroSeconds getMicroSecsAfter(const TimeVal &t) const
    {
        return diffMicroSecs(t, *this);
    }
    
    bool isLaterThan(const TimeVal& t) const
    {
        if (timeval.tv_sec > t.timeval.tv_sec) {
            return true;
        }
        if (timeval.tv_sec == t.timeval.tv_sec) {
            return timeval.tv_usec > t.timeval.tv_usec;
        }
        return false;
    }
    
    bool isLaterOrEqualThan(const TimeVal& t) const
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
    
    TimeVal& add(MicroSeconds usec)
    {
        long diff;
        long sec;

        ASSERT(0 <= timeval.tv_usec && timeval.tv_usec < 1000000);

        if (usec >= 1000000) {
            sec  = usec / 1000000;
            usec = MicroSeconds(usec - sec * 1000000);
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

    TimeVal& add(Seconds sec, MicroSeconds usec = MicroSeconds(0))
    {
        add(usec);
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

        if (current.isLaterOrEqualThan(t)) {
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


inline int select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, TimeVal* timeVal)
{
    return select(n, readfds, writefds, exceptfds, &timeVal->timeval);
}


} // namespace LucED

#endif // TIMEVAL_HPP

