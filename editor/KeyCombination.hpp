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

template<class T> class HashFunction;

class KeyCombination
{
public:
    KeyCombination(const String& keyModifier, const String& keyIds)
        : keyModifier(keyModifier),
          keyIds(keyIds)
    {}

    explicit KeyCombination(const String& combination)
    {
        int p = -1;
        for (int i = combination.getLength() - 1; i > 0; --i) {
            if (combination[i] == '+') {
                p = i;
                break;
            }
        }
        if (p >= 0) {
            keyModifier = KeyModifier(combination.getHead(p));
            keyIds      = combination.getTail(p + 1);
        } else {
            keyIds = combination;
        }
    }

    String toString() const {
        String km = keyModifier.toString();
        return String() << km << (km.getLength() > 0 ? "+" : "") << keyIds;
    }

    KeyModifier getKeyModifier() const {
        return keyModifier;
    }
    bool hasKeyIds() const {
        return keyIds.getLength() > 0;
    }
    KeyId getFirstKeyId() const {
        int p = keyIds.findFirstOf(',');
        String rslt;
        if (p <= 0) {
            rslt = keyIds;
        } else {
            rslt = keyIds.getHead(p);
        }
        if (rslt.getLength() == 1) {
            rslt = rslt.toUpper();
        }
        return KeyId(rslt);
    }
    void removeFirstKeyId() {
        int p = keyIds.findFirstOf(',', Pos(1));
        if (p <= 0) {
            keyIds = "";
        } else {
            keyIds = keyIds.getTail(p + 1);
        }
    }
    
    bool operator==(const KeyCombination& rhs) const {
        return    this->keyModifier == rhs.keyModifier
               && this->keyIds      == rhs.keyIds;
    }

    bool operator!=(const KeyCombination& rhs) const {
        return !(*this == rhs);
    }
    
    bool operator<(const KeyCombination& rhs) const {
        if (this->keyModifier == rhs.keyModifier) {
            return this->keyIds < rhs.keyIds;
        } else {
            return this->keyModifier < rhs.keyModifier;
        }
    }

private:
    friend class HashFunction<KeyCombination>;

    KeyModifier keyModifier;
    String keyIds;
};

template<> class HashFunction<KeyCombination>
{
public:
    size_t operator()(const KeyCombination& key) const {
        size_t rslt = key.keyModifier.toHashValue();
        for (int i = 0, n = key.keyIds.getLength(); i < n; ++i) {
            rslt = 5 * rslt + key.keyIds[i];
        }
        return rslt;
    }
};


} // namespace LucED

#endif // KEY_COMBINATION_HPP
