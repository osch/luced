/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2010 Oliver Schmidt, oliver at luced dot de
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

#include "RawPointable.hpp"
#include "CharArray.hpp"

namespace LucED
{

class String : public RawPointable
{
public:
    String()
    {}

    String(const String& rhs)
    {
        ASSERT(this != &rhs);
        chars.append(rhs.chars);
    }

    String(const char* rhs)
    {
        ASSERT(rhs != NULL);
        chars.append(rhs, strlen(rhs));
    }
    

    String(const char* rhs, long length)
    {
        ASSERT(length == 0 || rhs != NULL);
        chars.append(rhs, length);
    }

    String(const std::string& rhs)
    {
        chars.append(rhs.c_str(), rhs.length());
    }

    explicit String(long i) 
    {
        char buffer[100];
        sprintf(buffer, "%ld", i);
        chars.append(buffer, strlen(buffer));
    }

    Len getLength() const {
        return Len(chars.getLength());
    }
    const char* toCString() const {
        return chars.toCString();
    }
    std::string toStdString() const {
        return std::string(chars.getPtr(), chars.getLength());
    }
    char& operator[](long i) {
        ASSERT(0 <= i && i < getLength());
        return chars[i];
    }
    char operator[](long i) const {
        ASSERT(0 <= i && i < getLength());
        return chars[i];
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
        return String(chars.getPtr(pos), length);
    }
    String getSubstring(Pos pos1, Pos pos2) const {
        return getSubstring(pos1, pos2 - pos1);
    }
    String& append(const String& rhs) {
        chars.append(rhs.chars);
        return *this;
    }
    String& append(const std::string& rhs) {
        chars.append(rhs.c_str(), rhs.length());
        return *this;
    }
    String& append(const char* rhs) {
        ASSERT(rhs != NULL);
        chars.append(rhs, strlen(rhs));
        return *this;
    }
    String& appendWCharAsUtf8(int unicodeChar)
    {
        int c = unicodeChar;
        ASSERT(c >= 0);
        if (c < 0) {
        } else if (c <= 0x7F) {
            append((char)c);
        } else if (c <= 0x7FF) {
            append((char)(0xC0 | ((c >> 6) & 0x1F)));
            append((char)(0x80 | (c & 0x3F)));
        } else if (c <= 0xFFFF) {
            append((char)(0xE0 | ((c >> 12) & 0x0F)));
            append((char)(0x80 | ((c >> 6) & 0x3F)));
            append((char)(0x80 | (c & 0x3F)));
        } else {
            append((char)(0xF0 | ((c >> 18) & 0x07)));
            append((char)(0x80 | ((c >> 12) & 0x3f)));
            append((char)(0x80 | ((c >> 6) & 0x3f)));
            append((char)(0x80 | (c & 0x3f)));
        }
        return *this;
    }
    String& append(const char* rhs, long length) {
        ASSERT(rhs != NULL);
        chars.append(rhs, length);
        return *this;
    }
    String& appendSubstring(const String& rhs, Pos pos, Len length) {
        ASSERT(pos + length <= rhs.getLength());
        chars.append(rhs.toCString() + pos, length);
        return *this;
    }
    String& append(const byte* rhs, long length) {
        ASSERT(rhs != NULL);
        chars.append((const char*)rhs, length);
        return *this;
    }
    String& append(char c) {
        chars.append(c);
        return *this;
    }
    String& append(byte c) {
        chars.append((char)c);
        return *this;
    }
    String& appendLowerChar(char c) {
        chars.append((char)::tolower(c));
        return *this;
    }
    String& appendUpperChar(char c) {
        chars.append((char)::toupper(c));
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
        for (long i = 0, n = chars.getLength(); i < n; ++i) {
            if (!isdigit(chars[i])) {
                return false;
            }
        }
        return true;
    }
    
    int toInt() const {
        return atoi(toCString());
    }
    long toLong() const {
        return atol(toCString());
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

    /**
     * Converts utf8 content to upper case.
     */    
    String toUpperUtf8() const;

    /**
     * Converts utf8 content to lower case.
     */    
    String toLowerUtf8() const;
    
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
            if (::tolower(chars[i]) != ::tolower(rhs[i])) {
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
        if (chars[i] == '-' && i + 1 < n) {
            ++i;
        }
        for (; i < n; ++i) {
            if (!isdigit(chars[i])) {
                return false;
            }
        }
        return true;
    }
    
    bool isHex() const {
        for (long i = 0, n = getLength(); i < n; ++i) {
            if (!isxdigit(chars[i])) {
                return false;
            }
        }
        return true;
    }
    
    bool contains(char c) const {
        for (long i = 0, n = chars.getLength(); i < n; ++i) {
            if (chars[i] == c) {
                return true;
            }
        }
        return false;
    }
    bool containsAny(const char* searchChars) const {
        for (long i = 0, n = this->chars.getLength(); i < n; ++i) {
            const char thisChar = this->chars[i];
            for (const char* c = searchChars; *c != '\0'; ++c) {
                if (thisChar == *c) {
                    return true;
                }
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
        return (memcmp(chars.getPtr(), rhs, len) == 0);
    }
    bool operator!=(const char* rhs) const {
        return !(*this == rhs);
    }
    bool operator==(const String& rhs) const {
        return chars == rhs.chars;
    }
    bool operator!=(const String& rhs) const {
        return chars != rhs.chars;
    }
    bool operator<(const String& rhs) const {
        int len = chars.getLength();
        if (len > rhs.chars.getLength()) {
            len = rhs.chars.getLength();
        }
        for (int i = 0; i < len; ++i) {
            if (chars[i] < rhs.chars[i]) {
                return true;
            }
        }
        return chars.getLength() < rhs.chars.getLength();
    }
    void removeAmount(Pos pos, Len amount) {
        ASSERT(0 <= amount);
        ASSERT(0 <= pos && pos + amount <= getLength());
        chars.removeAmount(pos, amount);
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
        return getSubstring(Pos(pos), Pos(getLength()));
    }
    long findFirstOf(char c, Pos startPos = Pos(0)) const {
        ASSERT(0 <= startPos && startPos <= getLength());
        for (int i = startPos, n = getLength(); i < n; ++i) {
            if (chars[i] == c) {
                return i;
            }
        }
        return -1;
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
        chars.clear();
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
            const char* nPtr = strstr(chars.getPtr(i), oldPart);
            if (nPtr == NULL) {
                break;
            }
            int n = nPtr - chars.getPtr(0);
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
    
    CharArray chars;
};

inline bool operator==(const char* lhs, const String& rhs) {
    return rhs == lhs;
}

inline bool operator!=(const char* lhs, const String& rhs) {
    return rhs != lhs;
}

} // namespace LucED

#endif // STRING_HPP
