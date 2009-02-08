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

#ifndef SYNTAX_PATTERS_CONFIG_HPP
#define SYNTAX_PATTERS_CONFIG_HPP

#include "HeapObject.hpp"
#include "SyntaxPatterns.hpp"
#include "ObjectArray.hpp"
#include "HashMap.hpp"
#include "String.hpp"
#include "Callback.hpp"
#include "CallbackContainer.hpp"

namespace LucED
{

class SyntaxPatternsConfig : public HeapObject
{
public:
    typedef OwningPtr<SyntaxPatternsConfig> Ptr;

    static SyntaxPatternsConfig::Ptr create() {
        return Ptr(new SyntaxPatternsConfig());
    }

    typedef HeapHashMap<String,int> NameToIndexMap;

    SyntaxPatterns::Ptr getSyntaxPatterns(const String& syntaxNameString, 
                                          NameToIndexMap::Ptr textStyleToIndexMap,
                                          Callback<SyntaxPatterns::Ptr>::Ptr changedCallback);

    void refresh(NameToIndexMap::Ptr textStyleToIndexMap);
    
private:
    SyntaxPatternsConfig()
    {}

    SyntaxPatterns::Ptr loadSyntaxPatterns(const String&       syntaxName,
                                           NameToIndexMap::Ptr textStyleToIndexMap);

    class Entry : public HeapObject
    {
    public:
        typedef OwningPtr<Entry> Ptr;
        
        static Ptr create(SyntaxPatterns::Ptr syntaxPatterns) {
            return Ptr(new Entry(syntaxPatterns));
        }

        void addChangedCallback(Callback<SyntaxPatterns::Ptr>::Ptr callback) {
            changedCallbacks.registerCallback(callback);
        }
        SyntaxPatterns::Ptr getSyntaxPatterns() const {
            return syntaxPatterns;
        }
        void refreshWithNewSyntaxPatterns(SyntaxPatterns::Ptr syntaxPatterns) {
            this->syntaxPatterns = syntaxPatterns;
            changedCallbacks.invokeAllCallbacks(syntaxPatterns);
        }
    private:
        Entry(SyntaxPatterns::Ptr syntaxPatterns)
            : syntaxPatterns(syntaxPatterns)
        {}
        SyntaxPatterns::Ptr                    syntaxPatterns;
        CallbackContainer<SyntaxPatterns::Ptr> changedCallbacks;
    };

    HashMap<String,Entry::Ptr> patterns;
};

} // namespace LucED

#endif // SYNTAX_PATTERS_CONFIG_HPP
