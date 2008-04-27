/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
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

#ifndef SEARCH_PARAMTER_HPP
#define SEARCH_PARAMTER_HPP

#include "debug.hpp"
#include "String.hpp"
#include "SearchParameterTypes.hpp"

namespace LucED
{

class SearchParameter : public SearchParameterTypes
{
public:
    SearchParameter()
        : searchForwardFlag(true),
          ignoreCaseFlag(true),
          regexFlag(false),
          wholeWordFlag(false),
          allowMatchAtStartOfSearchFlag(true),
          replaceStringFlag(false)
    {}
    
    SearchParameter(Options options) {
        setOptions(options);
    }
    
    void setOptions(Options options) {
        setRegexFlag                     ( options.contains(REGEX));
        setWholeWordFlag                 ( options.contains(WHOLE_WORD));
        setIgnoreCaseFlag                ( options.contains(IGNORE_CASE));
        setSearchForwardFlag             (!options.contains(BACKWARD));
        setAllowMatchAtStartOfSearchFlag (!options.contains(NOT_AT_START));
    }

    SearchParameter& setSearchForwardFlag(bool flag) {
        this->searchForwardFlag = flag;
        return *this;
    }
    
    SearchParameter& setIgnoreCaseFlag(bool flag) {
        this->ignoreCaseFlag = flag;
        return *this;
    }
    
    SearchParameter& setRegexFlag(bool flag) {
        this->regexFlag = flag;
        return *this;
    }
    
    SearchParameter& setWholeWordFlag(bool flag) {
        this->wholeWordFlag = flag;
        return *this;
    }
    
    SearchParameter& setFindString(const String& findString) {
        this->findString = findString;
        return *this;
    }
    SearchParameter& setReplaceString(const String& replaceString) {
        this->replaceString = replaceString;
        this->replaceStringFlag = true;
        return *this;
    }

    SearchParameter& setAllowMatchAtStartOfSearchFlag(bool flag) {
        allowMatchAtStartOfSearchFlag = flag;
        return *this;
    }

    bool hasSearchForwardFlag() const {
        return searchForwardFlag;
    }
    bool hasIgnoreCaseFlag() const {
        return ignoreCaseFlag;
    }
    bool hasRegexFlag() const {
        return regexFlag;
    }
    bool hasWholeWordFlag() const {
        return wholeWordFlag;
    }
    bool hasAllowMatchAtStartOfSearchFlag() const {
        return allowMatchAtStartOfSearchFlag; 
    }
    String getFindString() const {
        return findString;
    }
    String getReplaceString() const {
        ASSERT(replaceStringFlag);
        return replaceString;
    }
    bool hasReplaceString() const {
        return replaceStringFlag;
    }

private:
    bool searchForwardFlag;
    bool ignoreCaseFlag;
    bool regexFlag;
    bool wholeWordFlag;
    bool allowMatchAtStartOfSearchFlag;
    String findString;
    String replaceString;
    bool   replaceStringFlag;
};

} // namespace LucED

#endif // SEARCH_PARAMTER_HPP
