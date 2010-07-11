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

#ifndef FONT_INFO_HPP
#define FONT_INFO_HPP

#include "String.hpp"

#include "NonCopyable.hpp"
#include "GuiRoot.hpp"
#include "HeapObject.hpp"
#include "ObjectArray.hpp"
#include "RawPtr.hpp"
#include "OwningPtr.hpp"
#include "FontHandle.hpp"
#include "Char2b.hpp"
#include "Char2bArray.hpp"

namespace LucED
{

class FontInfo : public HeapObject
{
public:
    typedef OwningPtr<FontInfo> Ptr;
    
    class CacheAccess
    {
        friend class TextStyleCache;
        
        static Ptr create(const String& fontname) {
            return Ptr(new FontInfo(fontname));
        }
    };
    
    ~FontInfo();

    short getCharWidth(Char2b c) const {
        if (c.byte1 == 0) {
            return charWidths8[c.byte2];
        } else {
            return internalGetCharWidth(c);
        }
    }
    short getCharAscent(Char2b c) const {
        if (c.byte1 == 0) {
            return charAscents8[c.byte2];
        } else {
            return internalGetCharAscent(c);
        }
    }
    short getCharDescent(Char2b c) const {
        if (c.byte1 == 0) {
            return charDescents8[c.byte2];
        } else {
            return internalGetCharDescent(c);
        }
    }
    short getCharLBearing(Char2b c) const {
        if (c.byte1 == 0) {
            return charLBearings8[c.byte2];
        } else {
            return internalGetCharLBearing(c);
        }
    }
    short getCharRBearing(Char2b c) const {
        if (c.byte1 == 0) {
            return charRBearings8[c.byte2];
        } else {
            return internalGetCharRBearing(c);
        }
    }
    short getSpaceWidth() const {
        return spaceWidth;
    }
    int getLineHeight() const {
        return lineHeight;
    }
    int getLineAscent() const {
        return lineAscent;
    }
    int getLineDescent() const {
        return lineDescent;
    }
    FontHandle getFontHandle() const {
        return fontHandle;
    }
    int getTextWidth(const char* str, int length) const {
        int rslt = 0;
        for (int i = 0; i < length; ++i) {
            rslt += getCharWidth(str[i]);
        }
        return rslt;
    }
    int getTextWidth(const String& str) const {
        return getTextWidth(str.toCString(), str.getLength());
    }
    int getTextWidth(const char* str) const {
        return getTextWidth(str, strlen(str));
    }
    int getTextWidth(const Char2b* p, long len) const {
        int rslt = 0;
        for (int i = 0; i < len; ++i) {
            rslt += getCharWidth(p[i]);
        }
        return rslt;
    }
    int getTextWidth(const Char2bArray& wcharArray) const {
        return getTextWidth(wcharArray.getPtr(0), wcharArray.getLength());
    }
    String getFontName() const {
        return fontName;
    }
    Char2b getDefaultChar() const {
        return defaultChar;
    }

private:
    explicit FontInfo(const String& fontname);

    long getCharInfoIndex(Char2b c) const {
        return (c.byte1 - minByte1) * numberBytes2 + (c.byte2 - minByte2);
    }
    bool hasCharInfoFor(Char2b c) const {
        return minByte1 <= c.byte1 && c.byte1 <= maxByte1
            && minByte2 <= c.byte2 && c.byte2 <= maxByte2;
    }
    long getNumberOfCharInfos() const {
        return numberBytes2 * numberBytes1;
    }

    short internalGetCharWidth(Char2b c) const;
    short internalGetCharAscent(Char2b c) const;
    short internalGetCharDescent(Char2b c) const;
    short internalGetCharLBearing(Char2b c) const;
    short internalGetCharRBearing(Char2b c) const;
    
    String fontName;

    XFontStruct* font;

    FontHandle fontHandle;

    MemArray<short> charWidths;
    MemArray<short> charAscents;
    MemArray<short> charDescents;
    MemArray<short> charLBearings;
    MemArray<short> charRBearings;

    MemArray<short> charWidths8;
    MemArray<short> charAscents8;
    MemArray<short> charDescents8;
    MemArray<short> charLBearings8;
    MemArray<short> charRBearings8;

    short spaceWidth;

    int lineHeight;
    int lineAscent;
    int lineDescent;
    
    int minByte1;
    int maxByte1;
    int minByte2;
    int maxByte2;
    int numberBytes2;
    int numberBytes1;
    
    short unknownWidth;
    short unknownAscent;
    short unknownDescent;
    short unknownLBearing;
    short unknownRBearing;
    
    Char2b defaultChar;
};

} // namespace LucED

#endif // FONT_INFO_HPP
