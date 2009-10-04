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

#include "FontInfo.hpp"
#include "GuiRoot.hpp"
#include "ConfigException.hpp"


using namespace LucED;

FontInfo::FontInfo(const String& fontName)
    : fontName(fontName)
{
    XCharStruct* min_bounds;
    XCharStruct* max_bounds;
    
    this->font = XLoadQueryFont(GuiRoot::getInstance()->getDisplay(), fontName.toCString());
    if (font == NULL) {
        throw ConfigException(String() << "invalid font name: " << fontName);
    }
    this->fontHandle = FontHandle(font->fid);

    min_bounds = &(this->font->min_bounds);
    max_bounds = &(this->font->max_bounds);
    
    minByte1     = font->min_byte1;
    maxByte1     = font->max_byte1;
    minByte2     = font->min_char_or_byte2;
    maxByte2     = font->max_char_or_byte2;
    numberBytes1 = font->max_byte1         - font->min_byte1 + 1;
    numberBytes2 = font->max_char_or_byte2 - font->min_char_or_byte2 + 1;

    const int numberOfCharInfos = getNumberOfCharInfos();
    
    charWidths   .appendAmount(numberOfCharInfos);
    charLBearings.appendAmount(numberOfCharInfos);
    charRBearings.appendAmount(numberOfCharInfos);

//    printf("min_byte1        : %p, max_byte1        : %p,\n"
//           "min_char_or_byte2: %p, max_char_or_byte2: %p    unknown: %p\n", 
//           this->font->min_byte1,
//           this->font->max_byte1,
//           this->font->min_char_or_byte2,
//           this->font->max_char_or_byte2,
//           this->font->default_char);
       
    int myMaxAscent = 0;        
    int myMaxDescent = 0;
    
    defaultChar = Char2b( font->default_char & 0xff00 >> 8,
                          font->default_char & 0x00ff       );
                    
    if (this->font->per_char == NULL) {
        int i;
        short width    = this->font->max_bounds.width;
        short lbearing = this->font->max_bounds.lbearing;
        short rbearing = this->font->max_bounds.rbearing;

        if (lbearing > 0)
            lbearing = 0;
        if (rbearing < width) 
            rbearing = width;
        
        for (int b1 = minByte1; b1 <= maxByte1; ++b1)
        {
            for (int b2 = minByte2; b2 <= maxByte2; ++b2)
            {
                Char2b c(b1, b2);
                long i = getCharInfoIndex(c);
             
                this->charWidths[i]    = width;
                this->charLBearings[i] = lbearing;
                this->charRBearings[i] = rbearing;
            }
        }
    } else {
        XFontStruct* font = this->font;
        int i;

        if (hasCharInfoFor(defaultChar))
        {
            const XCharStruct* def = &font->per_char[getCharInfoIndex(defaultChar)];
            unknownWidth    = def->width;
            unknownLBearing = def->lbearing;
            unknownRBearing = def->rbearing;
            if (unknownLBearing > 0)
                unknownLBearing = 0;
            if (unknownRBearing < unknownWidth)
                unknownRBearing = unknownWidth;
        } else {
            unknownWidth    = 0;
            unknownLBearing = 0;
            unknownRBearing = 0;
        }
        for (int b1 = minByte1; b1 <= maxByte1; ++b1)
        {
            for (int b2 = minByte2; b2 <= maxByte2; ++b2)
            {
                Char2b c(b1, b2);
                if (hasCharInfoFor(c))
                {
                    long i = getCharInfoIndex(c);
                    XCharStruct xcharstr;
                    int direction;
                    int ascent, descent;
                    short width;
                    short lbearing;
                    short rbearing;
    
                    XTextExtents16(this->font, &c, 1, &direction, &ascent, &descent, &xcharstr);
                    if (ascent > myMaxAscent) {
                        myMaxAscent = ascent;
                    }
                    if (descent > myMaxDescent) {
                        myMaxDescent = descent;
                    }
                    width    = xcharstr.width;
                    lbearing = xcharstr.lbearing;
                    rbearing = xcharstr.rbearing;
                    if (lbearing > 0)
                        lbearing = 0;
                    if (rbearing < width) 
                        rbearing = width;
                    this->charWidths[i]    = width; 
                    this->charLBearings[i] = lbearing;
                    this->charRBearings[i] = rbearing;
                }
            }
        }
    }
    {
        charWidths8   .appendAmount(0x100);
        charLBearings8.appendAmount(0x100);
        charRBearings8.appendAmount(0x100);
        
        for (int i = 0; i < 0x100; ++i)
        {
            charWidths8[i]    = internalGetCharWidth(i); 
            charLBearings8[i] = internalGetCharLBearing(i);
            charRBearings8[i] = internalGetCharRBearing(i);
        }
    }
    this->lineAscent  = max_bounds->ascent; // -1 ?!?!?
    this->lineDescent = max_bounds->descent;
    if (myMaxDescent > 0 && myMaxDescent < this->lineDescent) {
        this->lineDescent = myMaxDescent;
    }
    if (myMaxAscent > 0 && myMaxAscent < this->lineAscent) {
        this->lineAscent = myMaxAscent;
    }
    this->lineHeight  = this->lineAscent + this->lineDescent;

//printf("######## ascent  %d %d %d    (%s)\n", font->ascent,  max_bounds->ascent, myMaxAscent, fontName.toCString());
//printf("######## descent %d %d\n", font->descent, max_bounds->descent);
    this->spaceWidth = this->charWidths[(unsigned char)' '];
}

FontInfo::~FontInfo()
{
#ifdef WIND32_GUI
    DeleteObject(fontHandle);
#else
    XFreeFontInfo(NULL, font, 0);
#endif
}

short FontInfo::internalGetCharWidth(Char2b c) const {
    if (hasCharInfoFor(c)) {
        return charWidths[getCharInfoIndex(c)];
    } else {
        return unknownWidth;
    }
}
short FontInfo::internalGetCharLBearing(Char2b c) const {
    if (hasCharInfoFor(c)) {
        return charLBearings[getCharInfoIndex(c)];
    } else {
        return unknownLBearing;
    }
}
short FontInfo::internalGetCharRBearing(Char2b c) const {
    if (hasCharInfoFor(c)) {
        return charRBearings[getCharInfoIndex(c)];
    } else {
        return unknownRBearing;
    }
}
