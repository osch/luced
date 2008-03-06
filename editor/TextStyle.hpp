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
#include "OwningPtr.hpp"
#include "FontHandle.hpp"

namespace LucED {



class TextStyle : NonCopyable
{
public:
    TextStyle(const String& fontname, const String& colorName);
    ~TextStyle();

    GuiColor getColor() const {
        return color;
    }    
    short getCharWidth(byte c) const {
        return charWidths[c];
    }
    short getCharLBearing(byte c) const {
        return charLBearings[c];
    }
    short getCharRBearing(byte c) const {
        return charRBearings[c];
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

private:
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
};

class TextStyles : public HeapObject
{
public:
    typedef OwningPtr<TextStyles> Ptr;
    
    static TextStyles::Ptr create() {
        return TextStyles::Ptr(new TextStyles());
    }
    
    const TextStyle* get(int index) const {
        return &textStyles[index];
    }
    
    void appendNewStyle(const String& fontname, const String& color) {
        textStyles.appendNew(fontname, color);
    }
    
    long getLength() const {
        return textStyles.getLength();
    }
    
private:
    TextStyles() {}

    ObjectArray<TextStyle> textStyles;
};

}

#endif // TEXTSTYLE_HPP
