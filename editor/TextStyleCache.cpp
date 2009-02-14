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

#include "TextStyleCache.hpp"

using namespace LucED;

SingletonInstance<TextStyleCache> TextStyleCache::instance;


TextStyle::Ptr TextStyleCache::getTextStyle(const String& fontname, const String& colorName)
{
    TextStyle::Ptr rslt;
    
    for (int i = 0; i < list.getLength();)
    {
        if (   list[i]->getFontName() == fontname
            && list[i]->getColorName() == colorName)
        {
            rslt = list[i];
            ++i;
        }
        else
        { 
            if (list[i].getRefCounter() == 1) {
                list.remove(i);
            }
            else {
                ++i;
            }
        }
    }
    if (!rslt.isValid())
    {
        rslt = TextStyle::CacheAccess::create(fontname, colorName);
        list.append(rslt);
    }
    ASSERT(rslt.isValid());
    return rslt;
}
