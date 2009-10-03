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
        if (c.byte1 == 0) {
            return charWidths[c.byte2];
        } else {
            return internalGetCharWidth(c);
        }
    }
    short getCharLBearing(Char2b c) const {
        if (c.byte1 == 0) {
            return charLBearings[c.byte1];
        } else {
            return internalGetCharLBearing(c);
        }
    }
    short getCharRBearing(Char2b c) const {
        if (c.byte1 == 0) {
            return charRBearings[c.byte1];
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

    const XCharStruct* getXCharStructFor(Char2b c) const;

    short internalGetCharWidth(Char2b c) const;
    short internalGetCharLBearing(Char2b c) const;
    short internalGetCharRBearing(Char2b c) const;
    
    String fontName;
    String colorName;
#ifdef X11_GUI
    XFontStruct* font;
#endif
    FontHandle fontHandle;
    GuiColor color;

    short charWidths[256];
    short charLBearings[256];
    short charRBearings[256];
    short spaceWidth;

    int lineHeight;
    int lineAscent;
    int lineDescent;
    
    byte minByte1;
    byte minByte2;
    byte numberBytes2;
    
    Char2b defaultChar;
};

} // namespace LucED

#endif // TEXTSTYLE_HPP
