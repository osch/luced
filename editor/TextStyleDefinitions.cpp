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

#include "TextStyleDefinitions.hpp"
#include "ConfigException.hpp"

using namespace LucED;

TextStyleDefinitions::TextStyleDefinitions(TextStyleDefinitions::FontList::Ptr      fonts, 
                                           TextStyleDefinitions::TestStyleList::Ptr textStyles)
{
    if (textStyles.isValid())
    {
        bool isFirst = true;
        
        for (int i = 0; i < textStyles->getLength(); ++i)
        {
            ConfigDataTextStyle::Ptr s = textStyles->get(i);
            
            String styleName = s->getName();
            String fontName  = s->getFont();
            String x11FontId = fontName;
            
            if (isFirst) {
                if (styleName != "default") {
                    throw ConfigException("first textstyle must be named 'default'");
                }
                isFirst = false;
            }
            
            if (fonts.isValid()) {
                for (int j = 0; j < fonts->getLength(); ++j) {
                    if (fonts->get(j)->getName() == fontName) {
                        x11FontId = fonts->get(j)->getX11FontId();
                        break;
                    }
                }
            }
            this->append(TextStyleDefinition(styleName, 
                                             x11FontId, 
                                             s->getColor()));
        }
    }
}
