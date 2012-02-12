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

#ifndef TIME_PERIOD_HPP
#define TIME_PERIOD_HPP

#include "Seconds.hpp"
#include "MicroSeconds.hpp"

namespace LucED
{

class TimePeriod
{
public:
    TimePeriod() {
        timeval.tv_sec  = 0;
        timeval.tv_usec = 0;
    }
    TimePeriod(Seconds secs) {
        timeval.tv_sec = secs;
        timeval.tv_usec = 0;
    }
    TimePeriod(MicroSeconds usecs) {
        if (usecs >= 0) {
            long secs = usecs / (1000*1000);
            timeval.tv_sec = secs;
            timeval.tv_usec = usecs - (secs*1000*1000);
        } else {
            usecs = -usecs;
            long secs = usecs / (1000*1000);
            timeval.tv_sec = -secs;
            timeval.tv_usec = -(usecs - (secs*1000*1000));
        }
    }
    Seconds getSeconds() const {
        return Seconds(timeval.tv_sec);
    }
    MicroSeconds getMicroSeconds() const {
        return MicroSeconds(timeval.tv_usec);
    }
    TimePeriod& operator+=(Seconds secs) {
        timeval.tv_sec += secs;
        return *this;
    }
    TimePeriod& operator+=(MicroSeconds usecs) {
        long oldUsecs = timeval.tv_usec;
        setMicroSeconds(usecs);
        setMicroSeconds(timeval.tv_usec + oldUsecs);
        return *this;
    }
    TimePeriod& operator+=(const TimePeriod& rhs) {
        *this += rhs.getSeconds();
        *this += rhs.getMicroSeconds();
        return *this;
    }
    TimePeriod& operator-=(Seconds secs) {
        return *this +=(-secs);
    }
    TimePeriod& operator-=(MicroSeconds usecs) {
        return *this +=(-usecs);
    }
    TimePeriod& operator-=(const TimePeriod& rhs) {
        *this -= rhs.getSeconds();
        *this -= rhs.getMicroSeconds();
        return *this;
    }
    TimePeriod operator+(Seconds secs) const {
        TimePeriod rslt = *this;
        rslt += secs;
        return rslt;
    }
    TimePeriod operator+(MicroSeconds usecs) const {
        TimePeriod rslt = *this;
        rslt += usecs;
        return rslt;
    }
    TimePeriod operator+(const TimePeriod& rhs) const {
        TimePeriod rslt = *this;
        rslt += rhs;
        return rslt;
    }
    TimePeriod operator-(Seconds secs) const {
        return *this + (-secs);
    }
    TimePeriod operator-(MicroSeconds usecs) const {
        return *this + (-usecs);
    }
    TimePeriod operator-(const TimePeriod& rhs) const {
        return *this + (-rhs);
    }
    TimePeriod operator-() const {
        TimePeriod rslt;
        rslt.timeval.tv_sec  = -timeval.tv_sec;
        rslt.timeval.tv_usec = -timeval.tv_usec;
        return rslt;
    }
    bool operator<(const TimePeriod& rhs) const {
        if (timeval.tv_sec != rhs.timeval.tv_sec) {
            return timeval.tv_sec < rhs.timeval.tv_sec;
        } else {
            return timeval.tv_usec < rhs.timeval.tv_usec;
        }
    }
    bool operator==(const TimePeriod& rhs) const {
        return    timeval.tv_sec  == rhs.timeval.tv_sec
               && timeval.tv_usec == rhs.timeval.tv_usec;
    }
    bool operator<=(const TimePeriod& rhs) const {
        return *this < rhs || *this == rhs;
    }
    bool operator>(const TimePeriod& rhs) const {
        return rhs < *this;
    }
    bool operator>=(const TimePeriod& rhs) const {
        return rhs < *this || rhs == *this;
    }
    bool operator!=(const TimePeriod& rhs) const {
        return !(*this == rhs);
    }

    struct timeval toTimeval() const {
        struct timeval rslt = timeval;
        if (rslt.tv_sec < 0) {
            rslt.tv_sec = 0;
        }
        if (rslt.tv_usec < 0) {
            rslt.tv_usec = 0;
        }
        return rslt;
    }

protected:
    friend class TimeStamp;
    
    void setMicroSeconds(long usecs) {
        if (usecs >= 0) {
            long secs = usecs / (1000*1000);
            long remUsecs = usecs - (secs*1000*1000);
            timeval.tv_sec += secs;
            timeval.tv_usec = remUsecs;
        }
        else {
            usecs = -usecs;
            long secs = usecs / (1000*1000);
            long remUsecs = usecs - (secs*1000*1000);
            timeval.tv_sec -= (secs + 1);
            timeval.tv_usec = (1000*1000) - remUsecs;
        }
        if (timeval.tv_sec < 0 && timeval.tv_usec > 0) {
//printf("old:%ld.%ld\n",    timeval.tv_sec,    timeval.tv_usec);     
            timeval.tv_sec += 1;
            timeval.tv_usec = -((1000*1000) - timeval.tv_usec);
//printf("new:%ld.%ld\n",    timeval.tv_sec,    timeval.tv_usec);     
        }
    }
    struct timeval timeval;
};

} // namesapce LucED

#endif // TIME_PERIOD_HPP
