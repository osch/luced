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

#ifndef BASIC_REGEX_HPP
#define BASIC_REGEX_HPP

#include "String.hpp"
#include <pcre.h>

#include "MemArray.hpp"
#include "ByteArray.hpp"
#include "OptionBits.hpp"

namespace LucED {

class BasicRegexEnums
{
public:
    enum CreateOption {
        MULTILINE = PCRE_MULTILINE,
        EXTENDED = PCRE_EXTENDED,
        IGNORE_CASE = PCRE_CASELESS
    };
    typedef OptionBits<CreateOption> CreateOptions;
    
    enum MatchOption {
        NOTBOL = PCRE_NOTBOL,
        NOTEOL = PCRE_NOTEOL,
        ANCHORED = PCRE_ANCHORED,
        NOTEMPTY = PCRE_NOTEMPTY
    };
    typedef OptionBits<MatchOption> MatchOptions;
};


class BasicRegex : public BasicRegexEnums
{
public:

    BasicRegex() : re(NULL) {
        pcre_callout = pcreCalloutCallback;
    }
    BasicRegex(const String&    expr, CreateOptions createOptions = CreateOptions());
    BasicRegex(const ByteArray& expr, CreateOptions createOptions = CreateOptions());
    BasicRegex(const BasicRegex& src);
    BasicRegex& operator=(const BasicRegex& src);
    
    ~BasicRegex();
    
    bool isValid() const {
        return re != NULL;
    }

    int getStringNumber(const String& substringName) const;
    int getStringNumber(const ByteArray& substringName) const;

    int getOvecSize() const {
        return 3 * (pcre_info(re, NULL, NULL) + 1);
    }
    int getNumberOfCapturingSubpatterns() const {
        return pcre_info(re, NULL, NULL);
    }
    
    bool findMatch( const char* subject, int length, int startoffset,
            MatchOptions matchOptions, MemArray<int>& ovector )
    {
        ASSERT(pcre_callout == pcreCalloutCallback);

        return pcre_exec(re, NULL, subject, length, startoffset, matchOptions.getOptions(), 
                ovector.getPtr(0), ovector.getLength()) > 0;
    }
    
    typedef int CalloutFunction(void*, pcre_callout_block*);
    
    bool findMatch(void* object, CalloutFunction* calloutFunctionX,
                   const char* subject, int length, int startoffset,
                   MatchOptions matchOptions, MemArray<int>& ovector)
    {
        ASSERT(pcre_callout == pcreCalloutCallback);
        
        CalloutData calloutData;
                    calloutData.object = object;
                    calloutData.calloutFunction = calloutFunctionX;
                    
        pcre_extra extra;
                   extra.flags        = PCRE_EXTRA_CALLOUT_DATA;
                   extra.callout_data = &calloutData;
        
        bool rslt = pcre_exec(re, &extra, subject, length, startoffset, matchOptions.getOptions(), 
                    ovector.getPtr(0), ovector.getLength()) > 0;

        return rslt;
    }
    
    
private:

    struct CalloutData
    {
        void* object;
        CalloutFunction* calloutFunction;
    };
    static int pcreCalloutCallback(pcre_callout_block*);
    static const unsigned char* pcreCharTable;
    pcre* re;
};

} // namespace LucED

#endif // BASIC_REGEX_HPP
