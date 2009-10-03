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

#include "TextStyle.hpp"
#include "GuiRoot.hpp"
#include "ConfigException.hpp"


using namespace LucED;

TextStyle::TextStyle(const String& fontName, const String& colorName)
    : fontName(fontName),
      colorName(colorName)
{
    XCharStruct* min_bounds;
    XCharStruct* max_bounds;
    XColor xcolor1_st, xcolor2_st;
    
    this->font = XLoadQueryFont(GuiRoot::getInstance()->getDisplay(), fontName.toCString());
    if (font == NULL) {
        throw ConfigException(String() << "invalid font name: " << fontName);
    }
    this->fontHandle = FontHandle(font->fid);

    this->color = GuiRoot::getInstance()->getGuiColor(colorName);

    min_bounds = &(this->font->min_bounds);
    max_bounds = &(this->font->max_bounds);
    
    minByte1     = font->min_byte1;
    minByte2     = font->min_char_or_byte2;
    numberBytes2 = font->max_char_or_byte2 - font->min_char_or_byte2 + 1;

/*
    printf("min_byte1        : %d, max_byte1        : %d,\n"
           "min_char_or_byte2: %d, max_char_or_byte2: %d    unknown: %d\n", 
           this->font->min_byte1,
           this->font->max_byte1,
           this->font->min_char_or_byte2,
           this->font->max_char_or_byte2,
           this->font->default_char);
*/
       
    int myMaxAscent = 0;        
    int myMaxDescent = 0;
    
    defaultChar = Char2b( font->default_char & 0xff00 >> 8,
                          font->default_char & 0x00ff       );
                    
    if (this->font->per_char == NULL) {
        int i;
        short width    = this->font->max_bounds.width;
        short lbearing = this->font->max_bounds.lbearing;
        short rbearing = this->font->max_bounds.rbearing;
//        printf("no per char\n");
        if (lbearing > 0)
            lbearing = 0;
        if (rbearing < width) 
            rbearing = width;
        for (i=0; i<0x80; ++i) {
            this->charWidths[i]    = width;
            this->charLBearings[i] = lbearing;
            this->charRBearings[i] = rbearing;
        }
    } else {
        XFontStruct* font = this->font;
        int i;
        short unknown_width;
        short unknown_lbearing;
        short unknown_rbearing;

        if (font->default_char < font->min_char_or_byte2
                || font->default_char > font->max_char_or_byte2) {
//            printf("unknown not printed\n");
            unknown_width    = 0;
            unknown_lbearing = 0;
            unknown_rbearing = 0;
        } else {
            const XCharStruct* def = getXCharStructFor(defaultChar);
            unknown_width    = def->width;
            unknown_lbearing = def->lbearing;
            unknown_rbearing = def->rbearing;
            if (unknown_lbearing > 0)
                unknown_lbearing = 0;
            if (unknown_rbearing < unknown_width)
                unknown_rbearing = unknown_width;
        }
        for (i=0; i<0x80; ++i) {
            if (i < font->min_char_or_byte2 || i > font->max_char_or_byte2) {
                this->charWidths[i]    = unknown_width;
                this->charLBearings[i] = unknown_lbearing;
                this->charRBearings[i] = unknown_rbearing;
            } else {
                XCharStruct xcharstr;
                char c = (char)i;
                int direction;
                int ascent, descent;
                short width;
                short lbearing;
                short rbearing;

                XTextExtents(this->font, &c, 1, &direction, &ascent, &descent, &xcharstr);
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

TextStyle::~TextStyle()
{
#ifdef WIND32_GUI
    DeleteObject(fontHandle);
#else
    XFreeFontInfo(NULL, font, 0);
#endif
}

inline const XCharStruct* TextStyle::getXCharStructFor(Char2b c) const
{   
    return &font->per_char[(c.byte1 - minByte1) * numberBytes2 + (c.byte2 - minByte2)];
}


short TextStyle::internalGetCharWidth(Char2b c) const
{
    if (font->per_char == NULL) {
        return charWidths[0];
    } else {
        return getXCharStructFor(c)->width;
    }
}
short TextStyle::internalGetCharLBearing(Char2b c) const
{
    if (font->per_char == NULL) {
        return charLBearings[0];
    } else {
        return getXCharStructFor(c)->lbearing;
    }
}
short TextStyle::internalGetCharRBearing(Char2b c) const
{
    if (font->per_char == NULL) {
        return charRBearings[0];
    } else {
        return getXCharStructFor(c)->rbearing;
    }
}


