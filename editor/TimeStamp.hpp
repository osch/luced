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

#ifndef TIME_HPP
#define TIME_HPP

#include <time.h>
#include <sys/time.h>
#include <sys/select.h>

#include "debug.hpp"
#include "Seconds.hpp"
#include "MicroSeconds.hpp"
#include "String.hpp"
#include "TimePeriod.hpp"

namespace LucED
{

class TimeStamp
{
public:
    static TimeStamp now() {
        return TimeStamp(NOW);
    }
    static TimeStamp forTimePeriodSincePosixEpoch(const TimePeriod& timePeriodSinceEpoch) {
        return TimeStamp(timePeriodSinceEpoch);
    }
    String toString() const;

    Seconds getSeconds() const {
        return period.getSeconds();
    }
    MicroSeconds getMicroSeconds() const {
        return period.getMicroSeconds();
    }

    template<class T
            >    
    TimeStamp& operator+=(const T& rhs) {
        period += rhs;
        return *this;
    }
    template<class T
            >    
    TimeStamp& operator-=(const T& rhs) {
        period -= rhs;
        return *this;
    }
    template<class T
            >
    TimeStamp operator+(const T& rhs) const {
        return TimeStamp(period + rhs);
    }
    template<class T
            >
    TimeStamp operator-(const T& rhs) const {
        return TimeStamp(period - rhs);
    }

    TimePeriod operator-(const TimeStamp& rhs) const {
        return period - rhs.period;
    }
    bool operator<(const TimeStamp& rhs) const {
        return this->period < rhs.period;
    }
    bool operator==(const TimeStamp& rhs) const {
        return this->period == rhs.period;
    }
    bool operator<=(const TimeStamp& rhs) const {
        return this->period <= rhs.period;
    }
    bool operator>(const TimeStamp& rhs) const {
        return this->period > rhs.period;
    }
    bool operator>=(const TimeStamp& rhs) const {
        return this->period >= rhs.period;
    }
    bool operator!=(const TimeStamp& rhs) const {
        return this->period != rhs.period;
    }
private:
    enum CurrentTimeFlag {
        NOW
    };
    TimeStamp(CurrentTimeFlag c) {
        gettimeofday(&period.timeval, NULL);
    }
    TimeStamp(const TimePeriod& period)
        : period(period)
    {}

    TimePeriod period;
};

} // namesapce LucED

#endif // TIME_HPP
