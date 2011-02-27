/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2011 Oliver Schmidt, oliver at luced dot de
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

#ifndef LANGUAGE_MODE_SELECTORS_HPP
#define LANGUAGE_MODE_SELECTORS_HPP

#include "String.hpp"
#include "HeapObject.hpp"
#include "ObjectArray.hpp"
#include "BasicRegex.hpp"
#include "MemArray.hpp"
#include "HashMap.hpp"
#include "SyntaxPatterns.hpp"
#include "OwningPtr.hpp"
#include "ConfigData.hpp"
#include "RawPtr.hpp"
#include "ByteBuffer.hpp"


namespace LucED
{

typedef ConfigData::LanguageModeSelectors::Element::LanguageModeSelector LanguageModeSelector;

class LanguageModeSelectors : public HeapObject
{
public:
    typedef OwningPtr<LanguageModeSelectors> Ptr;
    
    static Ptr create() {
        return Ptr(new LanguageModeSelectors());
    }
    
    void append(LanguageModeSelector::Ptr languageModeSelector);

    int getLength() const {
        return selectors.getLength();
    }
    
    LanguageModeSelector::Ptr get(int i) const {
        return selectors[i];
    }
    
    String getLanguageModeNameForFileName(const String& fileName);
    String getLanguageModeNameForFileNameAndContent(const String& fileName, RawPtr<const ByteBuffer> fileContent);

private:
    LanguageModeSelectors()
    {}
    
    ObjectArray<LanguageModeSelector::Ptr> selectors;
    MemArray<int> ovector;
};

} // namespace LucED

#endif // LANGUAGE_MODE_SELECTORS_HPP
