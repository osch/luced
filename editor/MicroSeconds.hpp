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

#ifndef MICRO_SECONDS_HPP
#define MICRO_SECONDS_HPP

namespace LucED
{

class MicroSeconds
{
public:

    MicroSeconds()
        : microSeconds(0)
    {}
    
    explicit MicroSeconds(long microSeconds)
        : microSeconds(microSeconds)
    {}

    operator long() const {
        return microSeconds;
    }
    MicroSeconds operator-() const {
        return MicroSeconds(-microSeconds);
    }
private:
    long microSeconds;
};

} // namespace LucED

#endif // MICRO_SECONDS_HPP
