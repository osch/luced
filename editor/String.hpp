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

#ifndef STRING_H
#define STRING_H

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <sstream>

#include "debug.hpp"
#include "types.hpp"

namespace LucED
{

class String
{
public:
    String()
    {}

    String(const char* rhs)
        : s(rhs)
    {}

    String(const char* rhs, int length)
        : s(rhs, length)
    {}

    String(const std::string& rhs)
        : s(rhs)
    {}

    explicit String(int i) 
        : s(stringify(i))
    {}

    int getLength() const {
        return s.length();
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
    char& operator[](int i) {
        ASSERT(0 <= i && i < getLength());
        return s[i];
    }
    const char& operator[](int i) const {
        ASSERT(0 <= i && i < getLength());
        return s[i];
    }
    String getSubstring(int pos, int length) const {
        ASSERT(0 <= length);
        ASSERT(0 <= pos && pos <= getLength());
        if (pos + length > getLength()) {
            length = getLength() - pos;
        }
        return s.substr(pos, length);
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
        s.append(rhs);
        return *this;
    }
    String& append(const char* rhs, int length) {
        s.append(rhs, length);
        return *this;
    }
    String& append(const byte* rhs, int length) {
        s.append((const char*)rhs, length);
        return *this;
    }
    String& append(char c) {
        s.push_back(c);
        return *this;
    }
    String& appendLowerChar(char c) {
        s.push_back((char)tolower(c));
        return *this;
    }
    String& append(int rhs) {
        return append(String(rhs));
    }

    bool consistsOfDigits() const {
        for (int i = 0, n = s.length(); i < n; ++i) {
            if (!isdigit(s[i])) {
                return false;
            }
        }
        return true;
    }
    
    int toInt() const {
        return atoi(s.c_str());
    }
    
    bool isInt() const {
        int i = 0;
        int n = getLength();
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
        for (int i = 0, n = getLength(); i < n; ++i) {
            if (!isxdigit(s[i])) {
                return false;
            }
        }
        return true;
    }
    
    bool contains(char c) const {
        for (int i = 0, n = s.length(); i < n; ++i) {
            if (s[i] == c) {
                return true;
            }
        }
        return false;
    }
    
    bool endsWith(const char* str) const {
        long len = strlen(str);
        if (getLength() < len) {
            return false;
        } else {
            return memcmp(toCString() + getLength() - len, str, len) == 0;
        }
    }
    
    template<class T> String& operator<<(const T& rhs) {
        return append(rhs);
    }
    
    bool operator==(const String& rhs) const {
        return s == rhs.s;
    }
    bool operator!=(const String& rhs) const {
        return s != rhs.s;
    }
    void removeAmount(int pos, int amount) {
        ASSERT(0 <= amount);
        ASSERT(0 <= pos && pos + amount < getLength());
        s.erase(pos, amount);
    }
    void removeBetween(int pos1, int pos2) {
        removeAmount(pos1, pos2 - pos1);
    }
    void removeTail(int pos) {
        removeAmount(pos, getLength() - pos);
    }
    String getTail(int pos) const {
        ASSERT(0 <= pos && pos <= getLength());
        return s.substr(pos);
    }
    int findFirstOf(char c, int startPos = 0) const {
        ASSERT(0 <= startPos && startPos <= getLength());
        int rslt = s.find_first_of(c, startPos);
        if (rslt == std::string::npos) {
            return -1;
        } else {
            return rslt;
        }
    }
    
    void clear() {
        s.clear();
    }

private:
    static std::string stringify(int i) {
        std::ostringstream o;
        o << i;
        return o.str();
    }
    
    std::string s;
};

} // namespace LucED

#endif // STRING_H
