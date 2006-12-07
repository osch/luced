/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2006 Oliver Schmidt, osch at luced dot de
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

#ifndef REGEX_H
#define REGEX_H

#include <string>
#include <pcre.h>

#include "MemArray.h"
#include "ByteArray.h"
#include "OptionBits.h"

namespace LucED {

using std::string;

class Regex
{
public:
    enum CreateOption {
        MULTILINE = PCRE_MULTILINE,
        EXTENDED = PCRE_EXTENDED,
        ANCHORED = PCRE_ANCHORED
    };
    typedef OptionBits<CreateOption> CreateOptions;
    
    enum MatchOption {
        NOTBOL = PCRE_NOTBOL,
        NOTEOL = PCRE_NOTEOL,
    };
    typedef OptionBits<MatchOption> MatchOptions;

    Regex() : re(NULL) {}
    Regex(const string&    expr, CreateOptions createOptions = CreateOptions());
    Regex(const ByteArray& expr, CreateOptions createOptions = CreateOptions());
    Regex(const Regex& src);
    Regex& operator=(const Regex& src);
    
    ~Regex();
    
    bool isValid() const {
        return re != NULL;
    }

    int getStringNumber(const string& substringName) const;
    int getStringNumber(const ByteArray& substringName) const;

    int getOvecSize() const {
        return 3 * (pcre_info(re, NULL, NULL) + 1);
    }
    
    bool findMatch( const char *subject, int length, int startoffset,
            MatchOptions matchOptions, MemArray<int>& ovector ) {
        return pcre_exec(re, NULL, subject, length, startoffset, matchOptions.getOptions(), 
                ovector.getPtr(0), ovector.getLength()) > 0;
    }
    
    
private:
    static const unsigned char* pcreCharTable;
    pcre *re;
};

} // namespace LucED

#endif // REGEX_H
