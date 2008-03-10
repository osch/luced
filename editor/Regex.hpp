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

#ifndef REGEX_HPP
#define REGEX_HPP

#include "String.hpp"
#include "MemArray.hpp"
#include "ByteArray.hpp"
#include "BasicRegex.hpp"


namespace LucED
{

class Regex : public BasicRegexEnums
{
public:

    Regex()
        : matchFlag(false)
    {}

    Regex(const String& expr, CreateOptions createOptions = CreateOptions())
        : regex(expr, createOptions),
          matchFlag(false)
    {}
    
    bool isValid() const {
        return regex.isValid();
    }

    int getCaptureNumberByName(const String& substringName) const {
        return regex.getStringNumber(substringName);
    }

    int getNumberOfCaptures() const {
        return regex.getNumberOfCapturingSubpatterns();
    }
    
    int getCaptureBegin(int i) const
    {
        ASSERT(matchFlag);
        ASSERT(i <= getNumberOfCaptures());
        return ovec[i * 2];
    }

    int getCaptureEnd(int i) const
    {
        ASSERT(matchFlag);
        ASSERT(i <= getNumberOfCaptures());
        return ovec[i * 2 + 1];
    }
    
    int getCaptureLength(int i) const {
        return getCaptureEnd(i) - getCaptureBegin(i);
    }
    
    bool findMatch(const char* subject, int length, int startoffset, MatchOptions matchOptions)
    {
        if (ovec.getLength() < regex.getOvecSize()) {
            ovec.increaseTo(regex.getOvecSize());
        }
        
        matchFlag = regex.findMatch(subject, length, startoffset, matchOptions, ovec);
        
        return matchFlag;
    }
    
    bool findMatch(const String& subject, MatchOptions matchOptions) {
        return findMatch(subject.toCString(), subject.getLength(), 0, matchOptions);
    }
    
    bool matches(const String& subject) {
        bool rslt = findMatch(subject, MatchOptions() | ANCHORED);
        return rslt && getCaptureEnd(0) == subject.getLength();
    }
    
private:
    MemArray<int> ovec;
    BasicRegex regex;
    bool matchFlag;
};

} // namespace LucED

#endif // REGEX_HPP
