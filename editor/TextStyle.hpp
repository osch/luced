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

#ifndef TEXT_STYLE_HPP
#define TEXT_STYLE_HPP

#include "String.hpp"

#include "HeapObject.hpp"
#include "OwningPtr.hpp"
#include "FontInfo.hpp"
#include "RawPtr.hpp"

namespace LucED
{

class TextStyle : public HeapObject
{
public:
    typedef OwningPtr<TextStyle> Ptr;
    
    class CacheAccess
    {
        friend class TextStyleCache;
        
        static TextStyle::Ptr create(FontInfo::Ptr fontInfo, const String& colorName) {
            return Ptr(new TextStyle(fontInfo, colorName));
        }
    };
    RawPtr<FontInfo> getFontInfo() const {
        return fontInfo;
    }
    GuiColor getColor() const {
        return color;
    }
    String getColorName() const {
        return colorName;
    }
    short getCharWidth(Char2b c) const {
        return fontInfo->getCharWidth(c);
    }
    short getCharLBearing(Char2b c) const {
        return fontInfo->getCharLBearing(c);
    }
    short getCharRBearing(Char2b c) const {
        return fontInfo->getCharRBearing(c);
    }
    short getSpaceWidth() const {
        return fontInfo->getSpaceWidth();
    }
    int getLineHeight() const {
        return fontInfo->getLineHeight();
    }
    int getLineAscent() const {
        return fontInfo->getLineAscent();
    }
    int getLineDescent() const {
        return fontInfo->getLineDescent();
    }
    FontHandle getFontHandle() const {
        return fontInfo->getFontHandle();
    }
    int getTextWidth(const char* str, int length) const {
        return fontInfo->getTextWidth(str, length);
    }
    int getTextWidth(const String& str) const {
        return fontInfo->getTextWidth(str);
    }
    int getTextWidth(const char* str) const {
        return fontInfo->getTextWidth(str);
    }
    String getFontName() const {
        return fontInfo->getFontName();
    }
    Char2b getDefaultChar() const {
        return fontInfo->getDefaultChar();
    }

private:
    TextStyle(FontInfo::Ptr fontInfo, const String& colorName);

    const FontInfo::Ptr fontInfo;
    const String        colorName;
    const GuiColor      color;
};

} // namespace LucED

#endif // TEXTSTYLE_HPP
