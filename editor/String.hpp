/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2009 Oliver Schmidt, oliver at luced dot de
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

#ifndef STRING_HPP
#define STRING_HPP

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <sstream>

#include "debug.hpp"
#include "types.hpp"

#include "Pos.hpp"
#include "Len.hpp"

namespace LucED
{

class String
{
public:
    String()
    {}

    String(const char* rhs)
#ifdef DEBUG
    {
        ASSERT(rhs != NULL);
        s = std::string(rhs);
    }
#else
        : s(rhs)
    {}
#endif
    

    String(const char* rhs, long length)
#ifdef DEBUG
    {
        ASSERT(length == 0 || rhs != NULL);
        s = std::string(rhs, length);
    }
#else
        : s(rhs, length)
    {}
#endif

    String(const std::string& rhs)
        : s(rhs)
    {}

    explicit String(long i) 
        : s(stringify(i))
    {}

    Len getLength() const {
        return Len(s.length());
    }
    const char* toCString() const {
        return s.c_str();
    }
    const std::string& toStdString() const {
        return s;
    }
    std::string toStdString() {
        return s;
    }
    char& operator[](long i) {
        ASSERT(0 <= i && i < getLength());
        return s[i];
    }
    char operator[](long i) const {
        ASSERT(0 <= i && i < getLength());
        return s[i];
    }
    String getSubstring(Pos pos, Len length) const {
        if (length == 0) {
            return String();
        }
        ASSERT(0 <= length);
        ASSERT(0 <= pos && pos <= getLength());
        if (pos + length > getLength()) {
            length = Pos(0) + getLength() - pos;
        }
        return s.substr(pos, length);
    }
    String getSubstring(Pos pos1, Pos pos2) const {
        return getSubstring(pos1, pos2 - pos1);
    }
    String& append(const String& rhs) {
        s.append(rhs.s);
        return *this;
    }
    String& append(const std::string& rhs) {
        s.append(rhs);
        return *this;
    }
    String& append(const char* rhs) {
        ASSERT(rhs != NULL);
        s.append(rhs);
        return *this;
    }
    String& append(const char* rhs, long length) {
        ASSERT(rhs != NULL);
        s.append(rhs, length);
        return *this;
    }
    String& appendSubstring(const String& rhs, Pos pos, Len length) {
        ASSERT(pos + length <= rhs.getLength());
        s.append(rhs.toCString() + pos, length);
        return *this;
    }
    String& append(const byte* rhs, long length) {
        ASSERT(rhs != NULL);
        s.append((const char*)rhs, length);
        return *this;
    }
    String& append(char c) {
        s.push_back(c);
        return *this;
    }
    String& append(byte c) {
        s.push_back(c);
        return *this;
    }
    String& appendLowerChar(char c) {
        s.push_back((char)::tolower(c));
        return *this;
    }
    String& appendUpperChar(char c) {
        s.push_back((char)::toupper(c));
        return *this;
    }
    String& append(int rhs) {
        return append(String(rhs));
    }
    String& append(long rhs) {
        return append(String(rhs));
    }
    String& append(double d) {
        char buffer[20];
        sprintf(buffer, "%.0f", d);
        return append(buffer);
    }

    bool consistsOfDigits() const {
        for (long i = 0, n = s.length(); i < n; ++i) {
            if (!isdigit(s[i])) {
                return false;
            }
        }
        return true;
    }
    
    int toInt() const {
        return atoi(s.c_str());
    }
    long toLong() const {
        return atol(s.c_str());
    }
    
    String toUpper() const {
        String rslt;
        for (long i = 0, n = getLength(); i < n; ++i) {
            rslt << (char) ::toupper((*this)[i]);
        }
        return rslt;
    }
    
    String toLower() const {
        String rslt;
        for (long i = 0, n = getLength(); i < n; ++i) {
            rslt << (char) ::tolower((*this)[i]);
        }
        return rslt;
    }
    
    bool equals(const char* rhs, long len) const
    {
        ASSERT(len >= 0);
             
        if (len != getLength()) {
            return false;
        } else {
            return memcmp(toCString(), rhs, len) == 0;
        }
    }
    
    bool equalsSubstringAt(Pos pos, const char* rhs) const
    {
        long len = strlen(rhs);
        
        ASSERT(pos + len >= 0);
             
        if (pos + len > getLength()) {
            return false;
        } else {
            return memcmp(toCString() + pos, rhs, len) == 0;
        }
    }
    
    bool equalsIgnoreCase(const String& rhs) const {
        long j = getLength(), k = rhs.getLength();
        if (j != k) {
            return false;
        }
        for (long i = 0; i < j; ++i) {
            if (::tolower(s[i]) != ::tolower(rhs[i])) {
                return false;
            }
        }
        return true;
    }
    
    bool isInt() const {
        long i = 0;
        long n = getLength();
        if (n == 0) {
            return false;
        }
        if (s[i] == '-' && i + 1 < n) {
            ++i;
        }
        for (; i < n; ++i) {
            if (!isdigit(s[i])) {
                return false;
            }
        }
        return true;
    }
    
    bool isHex() const {
        for (long i = 0, n = getLength(); i < n; ++i) {
            if (!isxdigit(s[i])) {
                return false;
            }
        }
        return true;
    }
    
    bool contains(char c) const {
        for (long i = 0, n = s.length(); i < n; ++i) {
            if (s[i] == c) {
                return true;
            }
        }
        return false;
    }
    
    bool endsWith(const char* str, long length) const {
        if (getLength() < length) {
            return false;
        } else {
            return memcmp(toCString() + getLength() - length, str, length) == 0;
        }
    }
    bool endsWith(const char* str) const {
        return endsWith(str, strlen(str));
    }
    bool endsWith(const String& str) const {
        return endsWith(str.toCString(), str.getLength());
    }
    
    bool startsWith(const char* str, long length) const {
        if (getLength() < length) {
            return false;
        } else {
            return memcmp(toCString(), str, length) == 0;
        }
    }
    bool startsWith(const char* str) const {
        return startsWith(str, strlen(str));
    }
    bool startsWith(const String& str) const {
        return startsWith(str.toCString(), str.getLength());
    }
    
    template<class T> String& operator<<(const T& rhs) {
        return append(rhs);
    }
    
    bool operator==(const char* rhs) const {
        if (rhs == NULL) {
            return false;
        }
        long len = strlen(rhs);
        if (this->getLength() != len) {
            return false;
        }
        return (memcmp(this->toCString(), rhs, len) == 0);
    }
    bool operator!=(const char* rhs) const {
        return !(s == rhs);
    }
    bool operator==(const String& rhs) const {
        return s == rhs.s;
    }
    bool operator!=(const String& rhs) const {
        return s != rhs.s;
    }
    void removeAmount(Pos pos, Len amount) {
        ASSERT(0 <= amount);
        ASSERT(0 <= pos && pos + amount <= getLength());
        s.erase(pos, amount);
    }
    void removeBetween(Pos pos1, Pos pos2) {
        removeAmount(pos1, pos2 - pos1);
    }
    void removeTail(Pos pos) {
        removeAmount(pos, Pos(0) + getLength() - pos);
    }
    String getHead(long len) const {
        ASSERT(0 <= len && len <= getLength());
        return getSubstring(Pos(0), Len(len));
    }
    String getTail(long pos) const {
        ASSERT(0 <= pos && pos <= getLength());
        return s.substr(pos);
    }
    long findFirstOf(char c, Pos startPos = Pos(0)) const {
        ASSERT(0 <= startPos && startPos <= getLength());
        std::string::size_type rslt = s.find_first_of(c, startPos);
        if (rslt == std::string::npos) {
            return -1;
        } else {
            return rslt;
        }
    }
    
    String getTrimmedSubstring() const {
        long len = getLength();
        long i = 0, j = len;
        while (i < len && ((*this)[i] == ' ' || (*this)[i] == '\t')) {
            ++i;
        }
        while (j > i && ((*this)[j - 1] == ' ' || (*this)[j - 1] == '\t')) {
            --j;
        }
        return getSubstring(Pos(i), Pos(j));
    }
    
    void trim() {
        *this = getTrimmedSubstring();
    }
    
    void clear() {
        s.clear();
    }
    
    String toSubstitutedString(char oldChar, char newChar) const {
        String rslt = *this;
        for (long i = 0, n = getLength(); i < n; ++i) {
            if (rslt[i] == oldChar) {
                rslt[i] = newChar;
            }
        }
        return rslt;
    }
    String toSubstitutedString(const char* oldPart, const char* newPart) const {
        String rslt;
        long oldPartLength = strlen(oldPart);
        if (oldPartLength == 0) {
            return "";
        }
        long newPartLength = strlen(newPart);
        long thisLength = getLength();
        long i = 0;
        while (i < thisLength) 
        {
            std::string::size_type n = s.find(oldPart, i);
            if (n == std::string::npos) {
                break;
            }
            rslt.appendSubstring(*this, Pos(i), Len(n - i));
            rslt.append(newPart, newPartLength);
            i = n + oldPartLength;
        }
        rslt.appendSubstring(*this, Pos(i), Len(thisLength - i));
        return rslt;
    }

private:
    static std::string stringify(long i) {
        std::ostringstream o;
        o << i;
        return o.str();
    }
    
    std::string s;
};

inline bool operator==(const char* lhs, const String& rhs) {
    return rhs == lhs;
}

inline bool operator!=(const char* lhs, const String& rhs) {
    return rhs != lhs;
}

} // namespace LucED

#endif // STRING_HPP
