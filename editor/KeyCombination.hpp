/////////////////////////////////////////////////////////////////////////////////////
//
//   LucED - The Lucid Editor
//
//   Copyright (C) 2005-2008 Oliver Schmidt, oliver at luced dot de
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

#ifndef KEY_COMBINATION_HPP
#define KEY_COMBINATION_HPP

#include "String.hpp"
#include "KeyModifier.hpp"
#include "ConfigException.hpp"

namespace LucED
{

class KeyCombination
{
public:
    KeyCombination(const String& keyModifier, const String& keyIds)
        : keyModifier(keyModifier),
          keyIds(keyIds)
    {}
    KeyModifier getKeyModifier() const {
        return keyModifier;
    }
    bool hasKeyIds() const {
        return keyIds.getLength() > 0;
    }
    KeyId getFirstKeyId() const {
        int p = keyIds.findFirstOf(',');
        if (p <= 0) {
            return KeyId(keyIds);
        } else {
            return KeyId(keyIds.getSubstringBetween(0, p));
        }
    }
    void removeFirstKeyId() {
        int p = keyIds.findFirstOf(',', 1);
        if (p <= 0) {
            keyIds = "";
        } else {
            keyIds = keyIds.getTail(p + 1);
        }
    }
    
private:
    KeyModifier keyModifier;
    String keyIds;
};

} // namespace LucED

#endif // KEY_COMBINATION_HPP
