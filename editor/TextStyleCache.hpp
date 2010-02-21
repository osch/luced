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

#ifndef TEXT_STYLE_CACHE_HPP
#define TEXT_STYLE_CACHE_HPP

#include "SingletonInstance.hpp"
#include "RawPtr.hpp"
#include "String.hpp"
#include "TextStyle.hpp"
#include "ObjectArray.hpp"
#include "WeakPtr.hpp"
#include "TextStyleDefinition.hpp"

namespace LucED
{

class TextStyleCache : public HeapObject
{
public:
    static RawPtr<TextStyleCache> getInstance() {
        return instance.getPtr();
    }
    
    TextStyle::Ptr getTextStyle(const String& fontname, const String& colorName);

    TextStyle::Ptr getTextStyle(const TextStyleDefinition& definition) {
        return getTextStyle(definition.getFontName(),
                            definition.getColorName());
    }

    FontInfo::Ptr getFontInfo(const String& fontname);

private:
    friend class SingletonInstance<TextStyleCache>;
    
    static SingletonInstance<TextStyleCache> instance;
    
    TextStyleCache()
    {}
    
    ObjectArray<TextStyle::Ptr> list;
    ObjectArray<FontInfo::Ptr> fontInfos;
};

} // namespace LucED

#endif // TEXT_STYLE_CACHE_HPP
