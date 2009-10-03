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

#ifndef TEXTSTYLE_HPP
#define TEXTSTYLE_HPP

#include "String.hpp"

#include "NonCopyable.hpp"
#include "GuiRoot.hpp"
#include "HeapObject.hpp"
#include "ObjectArray.hpp"
#include "RawPtr.hpp"
#include "OwningPtr.hpp"
#include "FontHandle.hpp"
#include "Char2b.hpp"

namespace LucED
{

class TextStyle : public HeapObject
{
public:
    typedef OwningPtr<TextStyle> Ptr;
    
    class CacheAccess
    {
        friend class TextStyleCache;
        
        static Ptr create(const String& fontname, const String& colorName) {
            return Ptr(new TextStyle(fontname, colorName));
        }
    };
    
    ~TextStyle();

    GuiColor getColor() const {
        return color;
    }    
    short getCharWidth(Char2b c) const {
        if (hasCharInfoFor(c)) {
            return charWidths[getCharInfoIndex(c)];
        } else {
            return unknownWidth;
        }
    }
    short getCharLBearing(Char2b c) const {
        if (hasCharInfoFor(c)) {
            return charLBearings[getCharInfoIndex(c)];
        } else {
            return unknownLBearing;
        }
    }
    short getCharRBearing(Char2b c) const {
        if (hasCharInfoFor(c)) {
            return charRBearings[getCharInfoIndex(c)];
        } else {
            return unknownRBearing;
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
    String getFontName() const {
        return fontName;
    }
    String getColorName() const {
        return colorName;
    }
    Char2b getDefaultChar() const {
        return defaultChar;
    }

private:
    TextStyle(const String& fontname, const String& colorName);

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

    String fontName;
    String colorName;
#ifdef X11_GUI
    XFontStruct* font;
#endif
    FontHandle fontHandle;
    GuiColor color;

    MemArray<short> charWidths;
    MemArray<short> charLBearings;
    MemArray<short> charRBearings;
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
    short unknownLBearing;
    short unknownRBearing;
    
    Char2b defaultChar;
};

} // namespace LucED

#endif // TEXTSTYLE_HPP
